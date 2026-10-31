#pragma once

#include "../config.h"
#include "checks.h"

#ifdef MODULE_ALARM_ACTIVE

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "sensors.h"
#include "display.h"
#include "notifications.h"
#include "storage_cloud.h"

extern bool alarmEnabled;
extern bool alarmTriggered;
extern bool alarmPreAlert;

enum AlarmState {
  STATE_IDLE,
  STATE_PRE_ALARM
};

static AlarmState currentAlarmState = STATE_IDLE;
static unsigned long preAlarmStartTime = 0;
static unsigned long triggeredStartTime = 0;
static bool latchedMotion = false;
static bool latchedDistance = false;
static bool latchedAddon = false;
static bool preAlarmWaitRelease = false;
const unsigned long preAlarmWindowMs = 5000;
#ifndef ALARM_AUTO_RESET_MS
  #define ALARM_AUTO_RESET_MS 900000UL
#endif
const unsigned long autoResetMs = ALARM_AUTO_RESET_MS;

void getFormattedTimestamp(char* buffer, size_t maxLen) {
  if (timeIsValid()) {
    struct tm timeinfo;
    time_t now = time(nullptr);
    localtime_r(&now, &timeinfo);
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

// ======================================================
//  MAIN ALARM SYSTEM LOGIC
// ======================================================
void checkAlarmSystem() {
  char ts[25];
  getFormattedTimestamp(ts, sizeof(ts));

  if (alarmEnabled && alarmTriggered) {
    if (millis() - triggeredStartTime > autoResetMs) {
      alarmTriggered = false;
      callVoiceMonkey(VOICEMONKEY_RESET_URL);
      sendTelegramMessage("🟢 *ALARM AUTO-RESET* - system re-armed");
      sendLogToSupabase(ts, "⚪", "ALARM AUTO-RESET - system re-armed");
    }
    return;
  }

  bool trigMotion   = checkMotionTriggered();
  bool trigDistance = checkDistanceTriggered();
  #ifdef HAS_WAKEUP_ADDON
    bool trigAddon  = alarmEnabled ? checkAddonAlarmTriggered() : false;
  #else
    bool trigAddon  = false;
  #endif

  int activeSensorsCount = ACTIVE_ALARM_SENSORS;
  bool anyTriggered = (trigMotion || trigDistance || trigAddon);

// ----------------------------------------------------
// CASE 1: ALARM DISABLED (OFF)
// ----------------------------------------------------
  if (!alarmEnabled) {
    currentAlarmState = STATE_IDLE;
    alarmTriggered = false;
    alarmPreAlert = false;

    static bool loggedOffMotion = false;
    if (anyTriggered && !loggedOffMotion) {
      loggedOffMotion = true;
      #ifdef DEBUG_INFO_LOG
        sendLogToSupabase(ts, "🟢", "ℹ️ NOTICE - MOTION DETECTED ignored");
      #endif
    } else if (!anyTriggered) {
      loggedOffMotion = false;
    }
    return;
  }

// ----------------------------------------------------
// CASE 2: ALARM ENABLED (ON)
// ----------------------------------------------------
  int triggeredCount = (trigMotion ? 1 : 0) + (trigDistance ? 1 : 0) + (trigAddon ? 1 : 0);

  switch (currentAlarmState) {
    case STATE_IDLE:
      if (preAlarmWaitRelease) {
        if (!anyTriggered) preAlarmWaitRelease = false;
        break;
      }
      if (anyTriggered) {
        if (activeSensorsCount <= 1) {
          alarmTriggered = true;
          callVoiceMonkey(VOICEMONKEY_PRE_ALERT_URL);
          callVoiceMonkey(VOICEMONKEY_ALARM_URL);
          triggeredStartTime = millis();
          sendTelegramMessage("🚨 *ALARM* - INTRUSION DETECTED");
          sendLogToSupabase(ts, "🔴", "🚨 ALARM - INTRUSION DETECTED");
          #ifdef DEBUG_INFO_LOG
            sendLogToSupabase(ts, "⚪", "📁 ALARM sent to user");
          #endif
        } else {
          currentAlarmState = STATE_PRE_ALARM;
          alarmPreAlert = true;
          latchedMotion = trigMotion;
          latchedDistance = trigDistance;
          latchedAddon = trigAddon;
          callVoiceMonkey(VOICEMONKEY_PRE_ALERT_URL);
          sendLogToSupabase(ts, "🟡", "⚠️ PRE-ALARM - MOTION DETECTED awaiting confirmation");
          preAlarmStartTime = millis();
        }
      }
      break;

    case STATE_PRE_ALARM:
      latchedMotion   = latchedMotion   || trigMotion;
      latchedDistance = latchedDistance || trigDistance;
      latchedAddon    = latchedAddon    || trigAddon;
      triggeredCount  = (latchedMotion ? 1 : 0) + (latchedDistance ? 1 : 0) + (latchedAddon ? 1 : 0);
      if (triggeredCount >= 2) {
        currentAlarmState = STATE_IDLE;
        alarmPreAlert = false;
        alarmTriggered = true;
        callVoiceMonkey(VOICEMONKEY_ALARM_URL);
        triggeredStartTime = millis();
        sendTelegramMessage("🚨 *ALARM* - INTRUSION DETECTED");
        sendLogToSupabase(ts, "🔴", "🚨 ALARM - INTRUSION DETECTED");
        #ifdef DEBUG_INFO_LOG
          sendLogToSupabase(ts, "⚪", "📁 ALARM sent to user");
        #endif
      }
      else if (millis() - preAlarmStartTime > preAlarmWindowMs) {
        currentAlarmState = STATE_IDLE;
        alarmPreAlert = false;
        preAlarmWaitRelease = true;
        callVoiceMonkey(VOICEMONKEY_PRE_ALERT_URL);
        sendLogToSupabase(ts, "🟢", "🔍 PRE-ALARM TIMEOUT - INTRUSION NOT CONFIRMED system secured");
      }
      break;
  }
}

#endif