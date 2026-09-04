// ======================================================
//  DEMO SKETCH Tested and fully working on: 
//  Waveshare ESP32-C3 mini
//  https://www.waveshare.com/esp32-c3-zero.htm
// ======================================================
#include <WiFi.h>
#include <Preferences.h>
#include <time.h>

#include "config.h"
#include "lib/language.h"
#include "lib/display.h"
#include "lib/sensors.h"
#include "lib/notifications.h"
#include "lib/storage_cloud.h"
#include "lib/alarm.h"

// ======================================================
//  GLOBAL ALARM & SYSTEM VARIABLES
// ======================================================
bool alarmEnabled = false;
bool alarmTriggered = false;
Preferences preferences;

// ======================================================
//  WIFI STATE MACHINE AND TIMERS
// ======================================================
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
const unsigned long wifiTimeoutMs    = 15000;
const unsigned long wifiRetryDelayMs = 30000;

// ======================================================
//  NTP SYNCHRONIZATION
// ======================================================
void syncNtp() {
  showMessage(TXT_WIFI_CONN, TXT_NTP_CONN);
  delay(1000);
  
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  int tentativi = 0;
  while (!getLocalTime(&timeinfo) && tentativi < 20) {
    delay(200);
    tentativi++;
  }

  if (!getLocalTime(&timeinfo)) {
    showMessage(TXT_WIFI_CONN, TXT_NTP_FAIL);
    delay(3000);
    return;
  }

  showMessage(TXT_WIFI_CONN, TXT_NTP_OK);
  delay(1500);

  char ora[32];
  snprintf(ora, sizeof(ora), TXT_TIME_LABEL, 
           timeinfo.tm_hour, 
           timeinfo.tm_min, 
           timeinfo.tm_mday, 
           timeinfo.tm_mon + 1, 
           timeinfo.tm_year + 1900);
           
  showMessage(ora, "");
  delay(2000);
}

// ======================================================
//  WIFI START
// ======================================================
void wifiStart(const char* ssid, const char* pass, WifiState nextState, const char* msg) {
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, pass);

  wifiAttemptStart = millis();
  wifiState = nextState;

  showMessage(TXT_WIFI_CONN, msg);
  delay(1000);
}

// ======================================================
//  WIFI STATE MACHINE UPDATE
// ======================================================
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
        delay(2000);
        syncNtp();
        handleBootSequence();
      } else if (millis() - wifiAttemptStart > wifiTimeoutMs) {
        wifiStart(ssid_office, pass_office, WIFI_CONNECTING_OFFICE, TXT_TRY_OFFICE);
      }
      break;

    case WIFI_CONNECTING_OFFICE:
      if (st == WL_CONNECTED) {
        wifiState = WIFI_CONNECTED;
        showMessage(TXT_WIFI_OK_OFFICE, "");
        delay(2000);
        syncNtp();
        handleBootSequence();
      } else if (millis() - wifiAttemptStart > wifiTimeoutMs) {
        wifiStart(ssid_hotspot, pass_hotspot, WIFI_CONNECTING_HOTSPOT, TXT_TRY_HOTSPOT);
      }
      break;

    case WIFI_CONNECTING_HOTSPOT:
      if (st == WL_CONNECTED) {
        wifiState = WIFI_CONNECTED;
        showMessage(TXT_WIFI_OK_HOTSPOT, "");
        delay(2000);
        syncNtp();
        handleBootSequence();
      } else if (millis() - wifiAttemptStart > wifiTimeoutMs) {
        wifiState = WIFI_FAIL;
        lastWifiRetry = millis();
        showMessage(TXT_WIFI_CONN, TXT_WIFI_FAIL);
      }
      break;

    case WIFI_CONNECTED:
      if (st != WL_CONNECTED) {
        wifiState = WIFI_FAIL;
        lastWifiRetry = millis();
        showMessage(TXT_WIFI_CONN, TXT_WIFI_LOST);
      }
      break;

    case WIFI_FAIL:
      if (millis() - lastWifiRetry > wifiRetryDelayMs) {
        wifiStart(ssid_home, pass_home, WIFI_CONNECTING_HOME, TXT_TRY_HOME);
      }
      break;
  }
}

// ======================================================
//  HOURLY TELEMETRY TASK
// ======================================================
void checkHourlyTask(struct tm* timeinfo) {
  static int lastExecutedHour = -1;

  if (timeinfo->tm_min == 5 && timeinfo->tm_hour != lastExecutedHour) {
    lastExecutedHour = timeinfo->tm_hour;

    float temp  = readTemperature();
    float hum   = readHumidity();
    float press = readPressure();

    if (!alarmEnabled) {
      triggerDisplayWake();

      char riga1[20];
      char riga2[20];
      snprintf(riga1, sizeof(riga1), "T:%.1fC H:%.0f%%", temp, hum);
      snprintf(riga2, sizeof(riga2), "P:%.0fhPa", press);
      showMessage(riga1, riga2);
    }

    saveTelemetryData(timeinfo, temp, hum, press);
  }
}

// ======================================================
//  ALARM VERIFICATION LOGIC
// ======================================================
void checkAlarmSystem() {
  if (!alarmEnabled) {
    return;
  }
}

// ======================================================
//  SETUP AND MAIN LOOP
// ======================================================
void setup() {
  Serial.begin(115200);

  preferences.begin("domus-alarm", false);
  alarmEnabled = preferences.getBool("alarm_state", false);

  initDisplay();
  initDisplayAddons();
  initSensors();

  wifiState = WIFI_IDLE;
}

void loop() {
  wifiUpdateState();

  if (wifiState != WIFI_CONNECTED) {
    delay(20);
    return;
  }

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    checkHourlyTask(&timeinfo);
  }

  if (!alarmEnabled) {
    handleDisplayAutoWake();
  }

  checkTelegramUpdates();
  checkAlarmSystem();
  sendHeartbeat();
}