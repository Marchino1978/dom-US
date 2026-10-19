#include <WiFi.h>
#include <Preferences.h>
#include <time.h>
#include <stdlib.h>
#include <esp_system.h>

#include "config.h"
#include "lib/checks.h"
#include "lib/debug.h"
#include "lib/language.h"
#include "lib/display.h"
#include "lib/sensors.h"
#include "lib/notifications.h"
#include "lib/storage_cloud.h"
#include "lib/alarm.h"
#include "lib/led_status.h"

bool alarmEnabled = false;
bool alarmTriggered = false;
Preferences preferences;

enum WifiState {
  WIFI_IDLE,
  WIFI_CONNECTING_HOME,
  WIFI_CONNECTING_OFFICE,
  WIFI_CONNECTING_HOTSPOT,
  WIFI_CONNECTED,
  WIFI_FAIL
};

WifiState wifiState = WIFI_IDLE;
unsigned long wifiAttemptStart = 0;
unsigned long lastWifiRetry    = 0;
const unsigned long wifiTimeoutMs        = 15000;
const unsigned long wifiRetryDelayMs     = 30000;
const unsigned long ntpTimeoutMs         = 15000;
const unsigned long timeGraceMs          = 300000;
const unsigned long provisionalIntervalMs = 3600000UL;
const unsigned long slotRetryMs          = 10000;
const unsigned long bootRetryMs          = 30000;
const int           slotMaxAttempts      = 3;
const char* const   timezoneRome         = "CET-1CEST,M3.5.0,M10.5.0/3";

static bool bootWasBlackout  = false;

static bool ntpWaiting = false;
static bool ntpFailShown = false;
static unsigned long ntpStartMs = 0;

static bool hourlySlotReady = false;
static int lastExecutedKey = -1;
static bool climateRefreshRequested = false;

#if defined(DEBUG_SERIAL) && defined(DISPLAY_KIND_LCD)
  #define BUFFER_SCREEN_ENABLED
  const unsigned long bufferScreenIntervalMs = 3000;
  static bool bufferScreenShown = false;
#endif

void readTelemetry(float& temp, float& hum, float& press) {
  #ifdef MODULE_TELEMETRY_ACTIVE
    temp  = roundf(readTemperature() * 10.0f) / 10.0f;
    hum   = roundf(readHumidity());
    press = roundf(readPressure());
  #else
    temp  = NAN;
    hum   = NAN;
    press = NAN;
  #endif
}

void onTimeSynced() {
  climateRefreshRequested = true;
  checkClimateDisplayTask();
  if (bootCheckPassed) {
    flushRamBuffer();
    flushLogBuffer();
    sendHeartbeat();
    setLedState(LED_STATE_IDLE);
  }
}

// Blackout check runs once per boot and is retried until last_ping is read successfully
void bootSequenceTask() {
  if (bootCheckPassed) return;
  if (ntpWaiting) return;
  if (!timeIsValid() || WiFi.status() != WL_CONNECTED) return;

  static unsigned long lastAttemptMs = 0;
  if (lastAttemptMs != 0 && millis() - lastAttemptMs < bootRetryMs) return;
  lastAttemptMs = millis();

  handleBootSequence();
}

void startNtp() {
  setLedState(LED_STATE_NTP);
  displayBootRow(DISPLAY_BOOT_NTP_ROW, TXT_NTP_CONN);
  displayBootRow(DISPLAY_BOOT_NTP_ROW + 1, "");
  displayBootCommit();
  DEBUG_LOG("NTP SYNC START");

  configTzTime(timezoneRome, "pool.ntp.org", "time.nist.gov");

  ntpWaiting = true;
  ntpFailShown = false;
  ntpStartMs = millis();
}

void ntpUpdate() {
  if (!ntpWaiting) return;
  if (WiFi.status() != WL_CONNECTED) return;

  if (timeIsValid()) {
    ntpWaiting = false;
    DEBUG_LOG("NTP SYNC OK");

    displayBootRow(DISPLAY_BOOT_NTP_ROW + 1, TXT_NTP_OK);
    displayBootCommit();
    displayBootEnd();

    onTimeSynced();
    return;
  }

  if (!ntpFailShown && millis() - ntpStartMs > ntpTimeoutMs) {
    ntpFailShown = true;
    displayBootRow(DISPLAY_BOOT_NTP_ROW + 1, TXT_NTP_FAIL);
    displayBootCommit();
    DEBUG_LOG("NTP SYNC FAIL, still waiting");
  }
}

void onWifiConnected() {
  startNtp();
}

void wifiStart(const char* ssid, const char* pass, WifiState nextState, const char* msg) {
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, pass);

  wifiAttemptStart = millis();
  wifiState = nextState;
  setLedState(LED_STATE_WIFI);

  displayBootRow(0, TXT_WIFI_CONN);
  displayBootRow(1, msg);
  displayBootCommit();
  DEBUG_LOG("WIFI TRY: %s", ssid);
}

void wifiUpdateState() {
  wl_status_t st = WiFi.status();
  switch (wifiState) {

    case WIFI_IDLE:
      wifiStart(ssid_home, pass_home, WIFI_CONNECTING_HOME, TXT_TRY_HOME);
      break;

    case WIFI_CONNECTING_HOME:
      if (st == WL_CONNECTED) {
        wifiState = WIFI_CONNECTED;
        displayBootRow(1, TXT_WIFI_OK_HOME);
        displayBootCommit();
        DEBUG_LOG("WIFI CONNECTED: HOME");
        onWifiConnected();
      } else if (millis() - wifiAttemptStart > wifiTimeoutMs) {
        DEBUG_LOG("WIFI HOME TIMEOUT, trying OFFICE");
        wifiStart(ssid_office, pass_office, WIFI_CONNECTING_OFFICE, TXT_TRY_OFFICE);
      }
      break;

    case WIFI_CONNECTING_OFFICE:
      if (st == WL_CONNECTED) {
        wifiState = WIFI_CONNECTED;
        displayBootRow(1, TXT_WIFI_OK_OFFICE);
        displayBootCommit();
        DEBUG_LOG("WIFI CONNECTED: OFFICE");
        onWifiConnected();
      } else if (millis() - wifiAttemptStart > wifiTimeoutMs) {
        DEBUG_LOG("WIFI OFFICE TIMEOUT, trying HOTSPOT");
        wifiStart(ssid_hotspot, pass_hotspot, WIFI_CONNECTING_HOTSPOT, TXT_TRY_HOTSPOT);
      }
      break;

    case WIFI_CONNECTING_HOTSPOT:
      if (st == WL_CONNECTED) {
        wifiState = WIFI_CONNECTED;
        displayBootRow(1, TXT_WIFI_OK_HOTSPOT);
        displayBootCommit();
        DEBUG_LOG("WIFI CONNECTED: HOTSPOT");
        onWifiConnected();
      } else if (millis() - wifiAttemptStart > wifiTimeoutMs) {
        wifiState = WIFI_FAIL;
        lastWifiRetry = millis();
        displayBootRow(1, TXT_WIFI_FAIL);
        displayBootCommit();
        DEBUG_LOG("WIFI HOTSPOT TIMEOUT, all attempts FAILED");
      }
      break;

    case WIFI_CONNECTED:
      if (st != WL_CONNECTED) {
        wifiState = WIFI_FAIL;
        lastWifiRetry = millis();
        displayBootRow(1, TXT_WIFI_LOST);
        displayBootCommit();
        setLedState(LED_STATE_WIFI);
        DEBUG_LOG("WIFI CONNECTION LOST");
      }
      break;

    case WIFI_FAIL:
      if (st == WL_CONNECTED) {
        wifiState = WIFI_CONNECTED;
        DEBUG_LOG("WIFI RECONNECTED (auto)");
        onWifiConnected();
      } else if (millis() - lastWifiRetry > wifiRetryDelayMs) {
        DEBUG_LOG("WIFI RETRY: HOME");
        wifiStart(ssid_home, pass_home, WIFI_CONNECTING_HOME, TXT_TRY_HOME);
      }
      break;
  }
}

// After minute 05 the hour in progress is skipped; before it, the reading is taken at minute 05
void prepareHourlySlot() {
  if (hourlySlotReady) return;
  if (!timeIsValid()) return;
  if (hasProvisionalReadings()) return;

  time_t nowEpoch = time(nullptr);
  struct tm nowInfo;
  localtime_r(&nowEpoch, &nowInfo);

  if (nowInfo.tm_min >= 5) {
    lastExecutedKey = nowInfo.tm_yday * 24 + nowInfo.tm_hour;
  }
  hourlySlotReady = true;
}

void checkHourlyTask(struct tm* timeinfo) {
  if (!hourlySlotReady) return;

  int currentKey = timeinfo->tm_yday * 24 + timeinfo->tm_hour;

  if (timeinfo->tm_min >= 5 && currentKey != lastExecutedKey) {
    lastExecutedKey = currentKey;

    float temp, hum, press;
    readTelemetry(temp, hum, press);

    saveTelemetryData(timeinfo, temp, hum, press);
  }
}

// Used only while the clock is not valid; the first reading is taken after timeGraceMs from boot
void provisionalTelemetryTask() {
  if (millis() < timeGraceMs) return;

  static bool started = false;
  static unsigned long nextMs = 0;

  if (!started) {
    started = true;
    nextMs = millis();
  }
  if ((long)(millis() - nextMs) < 0) return;
  nextMs += provisionalIntervalMs;

  float temp, hum, press;
  readTelemetry(temp, hum, press);

  if (bufferProvisionalReading(millis() / 1000, temp, hum, press)) {
    DEBUG_LOG("PROVISIONAL READING BUFFERED");
  } else {
    DEBUG_LOG("PROVISIONAL READING BUFFER FULL");
  }
}

void checkClimateDisplayTask() {
  #if defined(MODULE_TELEMETRY_ACTIVE) && defined(MODULE_DISPLAY_ACTIVE)
    static int lastMinuteKey = -1;
    static bool wasActive = false;

    if (displayBootActive()) return;

    #ifdef BUFFER_SCREEN_ENABLED
      if (bufferScreenShown) return;
    #endif

    bool activeNow  = isDisplayActive();
    bool justWokeUp = activeNow && !wasActive;
    wasActive = activeNow;
    if (!activeNow) return;
    if (!timeIsValid() && millis() < timeGraceMs) return;

    int minuteKey;
    if (timeIsValid()) {
      time_t nowEpoch = time(nullptr);
      struct tm nowInfo;
      localtime_r(&nowEpoch, &nowInfo);
      minuteKey = nowInfo.tm_yday * 1440 + nowInfo.tm_hour * 60 + nowInfo.tm_min;
    } else {
      minuteKey = (int)(millis() / 60000UL);
    }

    if (!justWokeUp && !climateRefreshRequested && minuteKey == lastMinuteKey) return;
    climateRefreshRequested = false;
    lastMinuteKey = minuteKey;

    float temp  = readTemperature();
    float hum   = readHumidity();
    float press = readPressure();

    refreshClimateDisplay(temp, hum, press);
  #endif
}

// Debug only: alternates the climate screen and the buffer screen while offline or while buffers are not empty
void checkBufferDisplayTask() {
  #ifdef BUFFER_SCREEN_ENABLED
    static unsigned long lastToggleMs = 0;
    static bool wasNeeded = false;

    bool needed = !alarmEnabled &&
                  !displayBootActive() &&
                  isDisplayActive() &&
                  (WiFi.status() != WL_CONNECTED || bufferCount > 0 || logBufferCount > 0);

    if (!needed) {
      wasNeeded = false;
      if (bufferScreenShown) {
        bufferScreenShown = false;
        climateRefreshRequested = true;
      }
      return;
    }

    if (!wasNeeded) {
      wasNeeded = true;
      lastToggleMs = millis();
      return;
    }

    if (millis() - lastToggleMs < bufferScreenIntervalMs) return;
    lastToggleMs = millis();

    bufferScreenShown = !bufferScreenShown;
    if (bufferScreenShown) {
      refreshBufferDisplay(bufferCount, MAX_OFFLINE_READINGS, logBufferCount, MAX_OFFLINE_LOGS);
    } else {
      climateRefreshRequested = true;
    }
  #endif
}

void setup() {
  #ifdef DEBUG_SERIAL
    Serial.begin(115200);
    delay(1000);
  #endif
  DEBUG_LOG("BOOT START");
  DEBUG_LOG("BUILD: %s %s", __DATE__, __TIME__);

  esp_reset_reason_t resetReason = esp_reset_reason();
  bootWasBlackout = (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_BROWNOUT);
  DEBUG_LOG("RESET REASON: %d", (int)resetReason);

  setenv("TZ", timezoneRome, 1);
  tzset();

  preferences.begin("domus-alarm", false);
  alarmEnabled = preferences.getBool("alarm_state", false);

  #ifdef MODULE_DISPLAY_ACTIVE
    initDisplay();
  #endif

  #if defined(MODULE_DISPLAY_ACTIVE) || defined(HAS_WAKEUP_ADDON)
    initDisplayAddons();
  #endif

  initSensors();
  initLed();
  setLedStatusEnabled(!alarmEnabled);

  wifiState = WIFI_IDLE;
}

void loop() {
  wifiUpdateState();
  ntpUpdate();
  bootSequenceTask();
  updateLed();

  if (timeIsValid()) {
    struct tm timeinfo;
    time_t now = time(nullptr);
    localtime_r(&now, &timeinfo);

    prepareHourlySlot();
    checkHourlyTask(&timeinfo);
  } else {
    provisionalTelemetryTask();
  }

  retryPendingUploads();

  if (millis() >= timeGraceMs) displayBootEnd();
  if (displayBootUpdate()) climateRefreshRequested = true;

  checkClimateDisplayTask();
  checkBufferDisplayTask();

  #if defined(MODULE_DISPLAY_ACTIVE) || defined(HAS_WAKEUP_ADDON)
    if (!alarmEnabled) {
      handleDisplayAutoWake();
    } else if (isDisplayActive()) {
      setDisplayPower(false);
    }
  #endif

  checkTelegramUpdates();

  #ifdef MODULE_ALARM_ACTIVE
    checkAlarmSystem();
  #endif

  sendHeartbeat();
}