#include <WiFi.h>
#include <Preferences.h>
#include <time.h>
#include <stdlib.h>
#include <esp_system.h>

#include "config.h"
#include "lib/checks.h"
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

void readTelemetry(float& temp, float& hum, float& press) {
  #ifdef MODULE_TELEMETRY_ACTIVE
    temp  = readTemperature();
    hum   = readHumidity();
    press = readPressure();
  #else
    temp  = NAN;
    hum   = NAN;
    press = NAN;
  #endif
}

void onTimeSynced() {
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
  showMessage(TXT_WIFI_CONN, TXT_NTP_CONN);
  Serial.println("NTP SYNC START");

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
    Serial.println("NTP SYNC OK");

    struct tm timeinfo;
    time_t now = time(nullptr);
    localtime_r(&now, &timeinfo);

    char ora[32];
    snprintf(ora, sizeof(ora), TXT_TIME_LABEL,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_mday,
             timeinfo.tm_mon + 1,
             timeinfo.tm_year + 1900);
    showMessage(ora, TXT_NTP_OK);

    onTimeSynced();
    return;
  }

  if (!ntpFailShown && millis() - ntpStartMs > ntpTimeoutMs) {
    ntpFailShown = true;
    showMessage(TXT_WIFI_CONN, TXT_NTP_FAIL);
    Serial.println("NTP SYNC FAIL, still waiting");
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

  showMessage(TXT_WIFI_CONN, msg);
  Serial.print("WIFI TRY: ");
  Serial.println(ssid);
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
        showMessage(TXT_WIFI_CONN, TXT_WIFI_OK_HOME);
        Serial.println("WIFI CONNECTED: HOME");
        onWifiConnected();
      } else if (millis() - wifiAttemptStart > wifiTimeoutMs) {
        Serial.println("WIFI HOME TIMEOUT, trying OFFICE");
        wifiStart(ssid_office, pass_office, WIFI_CONNECTING_OFFICE, TXT_TRY_OFFICE);
      }
      break;

    case WIFI_CONNECTING_OFFICE:
      if (st == WL_CONNECTED) {
        wifiState = WIFI_CONNECTED;
        showMessage(TXT_WIFI_OK_OFFICE, "");
        Serial.println("WIFI CONNECTED: OFFICE");
        onWifiConnected();
      } else if (millis() - wifiAttemptStart > wifiTimeoutMs) {
        Serial.println("WIFI OFFICE TIMEOUT, trying HOTSPOT");
        wifiStart(ssid_hotspot, pass_hotspot, WIFI_CONNECTING_HOTSPOT, TXT_TRY_HOTSPOT);
      }
      break;

    case WIFI_CONNECTING_HOTSPOT:
      if (st == WL_CONNECTED) {
        wifiState = WIFI_CONNECTED;
        showMessage(TXT_WIFI_OK_HOTSPOT, "");
        Serial.println("WIFI CONNECTED: HOTSPOT");
        onWifiConnected();
      } else if (millis() - wifiAttemptStart > wifiTimeoutMs) {
        wifiState = WIFI_FAIL;
        lastWifiRetry = millis();
        showMessage(TXT_WIFI_CONN, TXT_WIFI_FAIL);
        Serial.println("WIFI HOTSPOT TIMEOUT, all attempts FAILED");
      }
      break;

    case WIFI_CONNECTED:
      if (st != WL_CONNECTED) {
        wifiState = WIFI_FAIL;
        lastWifiRetry = millis();
        showMessage(TXT_WIFI_CONN, TXT_WIFI_LOST);
        setLedState(LED_STATE_WIFI);
        Serial.println("WIFI CONNECTION LOST");
      }
      break;

    case WIFI_FAIL:
      if (st == WL_CONNECTED) {
        wifiState = WIFI_CONNECTED;
        Serial.println("WIFI RECONNECTED (auto)");
        onWifiConnected();
      } else if (millis() - lastWifiRetry > wifiRetryDelayMs) {
        Serial.println("WIFI RETRY: HOME");
        wifiStart(ssid_home, pass_home, WIFI_CONNECTING_HOME, TXT_TRY_HOME);
      }
      break;
  }
}

// Decides whether the current hour must be skipped, after pending readings are resolved
void prepareHourlySlot() {
  if (hourlySlotReady) return;
  if (!timeIsValid()) return;
  if (hasProvisionalReadings()) return;

  static unsigned long lastAttemptMs = 0;
  static int attempts = 0;

  time_t lastDbEpoch = 0;

  if (WiFi.status() == WL_CONNECTED) {
    if (lastAttemptMs != 0 && millis() - lastAttemptMs < slotRetryMs) return;
    lastAttemptMs = millis();

    if (!fetchLastSensorEpoch(lastDbEpoch)) {
      attempts++;
      if (attempts < slotMaxAttempts) return;
      lastDbEpoch = 0;
    }
  }

  time_t nowEpoch = time(nullptr);
  struct tm nowInfo;
  localtime_r(&nowEpoch, &nowInfo);

  time_t currentHourStart = nowEpoch - (nowEpoch % 3600);

  char currentTs[25];
  snprintf(currentTs, sizeof(currentTs), "%04d-%02d-%02dT%02d:00:00Z",
           nowInfo.tm_year + 1900,
           nowInfo.tm_mon + 1,
           nowInfo.tm_mday,
           nowInfo.tm_hour);

  bool skipCurrentHour = (lastDbEpoch >= currentHourStart) ||
                         bufferHasHour(currentTs) ||
                         (bootWasBlackout && nowInfo.tm_min >= 5);

  if (skipCurrentHour) {
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

// Used only while the clock is not valid and the reboot was not a power loss
void provisionalTelemetryTask() {
  if (bootWasBlackout) return;
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
    Serial.println("PROVISIONAL READING BUFFERED");
  } else {
    Serial.println("PROVISIONAL READING BUFFER FULL");
  }
}

void checkClimateDisplayTask() {
  #if defined(MODULE_TELEMETRY_ACTIVE) && defined(MODULE_DISPLAY_ACTIVE)
    static unsigned long lastRun = 0;
    static bool wasActive = false;
    const unsigned long climateDisplayIntervalMs = 60000;

    bool activeNow  = isDisplayActive();
    bool justWokeUp = activeNow && !wasActive;
    wasActive = activeNow;

    if (!justWokeUp && (millis() - lastRun < climateDisplayIntervalMs)) return;
    lastRun = millis();

    float temp  = readTemperature();
    float hum   = readHumidity();
    float press = readPressure();

    refreshClimateDisplay(temp, hum, press);
  #endif
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("BOOT START");

  esp_reset_reason_t resetReason = esp_reset_reason();
  bootWasBlackout = (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_BROWNOUT);
  Serial.print("RESET REASON: ");
  Serial.println((int)resetReason);

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

  checkClimateDisplayTask();

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
