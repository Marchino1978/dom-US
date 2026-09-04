#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "../config.h"
#include "sensors.h"
#include "display.h"
#include "notifications.h"
#include "storage_cloud.h"

extern bool alarmEnabled;
extern bool alarmTriggered;

enum AlarmState {
  STATE_IDLE,
  STATE_PRE_ALARM
};

static AlarmState currentAlarmState = STATE_IDLE;
static unsigned long preAlarmStartTime = 0;
const unsigned long preAlarmWindowMs = 5000;

void getFormattedTimestamp(char* buffer, size_t maxLen) {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    snprintf(buffer, maxLen, "%04d-%02d-%02dT%02d:%02d:%02dZ",
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec);
  } else {
    snprintf(buffer, maxLen, "2026-09-01T00:00:00Z");
  }
}

inline int countActiveMotionSensors() {
  int count = 0;
  
  #if defined(SENSOR_PIR_HCSR501) || defined(SENSOR_RADAR_RCWL) || defined(SENSOR_MMWAVE_LD2410) || defined(SENSOR_IR_TE174)
    count++;
  #endif

  #if defined(SENSOR_ULTRASONIC_HCSR04) || defined(SENSOR_ULTRASONIC_US100) || defined(SENSOR_LASER_VL53L0X)
    count++;
  #endif

  #if defined(ADDON_IR_TE174) || defined(ADDON_LASER_VL53L0X) || defined(ADDON_PIR_HCSR501)
    if (alarmEnabled) {
      count++;
    }
  #endif

  return count;
}

// ======================================================
//  MAIN ALARM SYSTEM LOGIC
// ======================================================
void checkAlarmSystem() {
  char ts[25];
  getFormattedTimestamp(ts, sizeof(ts));

  bool trigMotion   = checkMotionTriggered();
  bool trigDistance = checkDistanceTriggered();
  bool trigAddon    = alarmEnabled ? checkAddonAlarmTriggered() : false;

  int activeSensorsCount = countActiveMotionSensors();
  bool anyTriggered = (trigMotion || trigDistance || trigAddon);

  // ----------------------------------------------------
  // CASE 1: ALARM DISABLED (OFF)
  // ----------------------------------------------------
  if (!alarmEnabled) {
    currentAlarmState = STATE_IDLE;
    alarmTriggered = false;

    #if defined(ADDON_IR_TE174) || defined(ADDON_LASER_VL53L0X) || defined(ADDON_PIR_HCSR501)
      if (checkAddonDisplayTriggered()) {
        triggerDisplayWake();
      }
    #endif

    static bool loggedOffMotion = false;
    if (anyTriggered && !loggedOffMotion) {
      loggedOffMotion = true;
      sendLogToSupabase(ts, "green", "ℹ️ NOTICE - MOTION DETECTED ignored");
    } else if (!anyTriggered) {
      loggedOffMotion = false;
    }
    return;
  }

  // ----------------------------------------------------
  // CASE 2: ALARM ENABLED (ON)
  // ----------------------------------------------------
  if (alarmTriggered) return;

  int triggeredCount = (trigMotion ? 1 : 0) + (trigDistance ? 1 : 0) + (trigAddon ? 1 : 0);

  switch (currentAlarmState) {
    case STATE_IDLE:
      if (anyTriggered) {
        if (activeSensorsCount <= 1) {
          alarmTriggered = true;
          sendLogToSupabase(ts, "red", "🚨 ALARM - INTRUSION DETECTED");
          sendTelegramMessage("🚨 *ALARM* - INTRUSION DETECTED");
          sendLogToSupabase(ts, "yellow", "📁 ALARM sent to user");
        } else {
          currentAlarmState = STATE_PRE_ALARM;
          preAlarmStartTime = millis();
          sendLogToSupabase(ts, "yellow", "⚠️ PRE-ALARM - MOTION DETECTED awaiting confirmation");
        }
      }
      break;

    case STATE_PRE_ALARM:
      if (triggeredCount >= 2 || (activeSensorsCount == 1 && triggeredCount >= 1)) {
        currentAlarmState = STATE_IDLE;
        alarmTriggered = true;
        
        sendLogToSupabase(ts, "red", "🚨 ALARM - INTRUSION DETECTED");
        sendTelegramMessage("🚨 *ALARM* - INTRUSION DETECTED");
        sendLogToSupabase(ts, "yellow", "📁 ALARM sent to user");
      } 
      else if (millis() - preAlarmStartTime > preAlarmWindowMs) {
        currentAlarmState = STATE_IDLE;
        sendLogToSupabase(ts, "green", "🔍 PRE-ALARM TIMEOUT - INTRUSION NOT CONFIRMED system secured");
      }
      break;
  }
}

#endif