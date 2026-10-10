#pragma once

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include "../config.h"
#include "led_status.h"
#include "debug.h"

void sendTelegramMessage(String message);

struct OfflineReading {
  char timestamp[25];
  float temp;
  float hum;
  float press;
  bool provisional;
  uint32_t bootSeconds;
};

struct OfflineLog {
  char timestamp[25];
  char severity[10];
  char message[150];
  bool provisional;
  uint32_t bootSeconds;
};

const int MAX_OFFLINE_READINGS = 72;
const int MAX_OFFLINE_LOGS = 500;
const int BATCH_READINGS = 24;
const int BATCH_LOGS = 50;
const unsigned long UPLOAD_RETRY_INTERVAL_MS = 30000;
const time_t MIN_VALID_EPOCH = 1700000000;
const int32_t HTTP_CONNECT_TIMEOUT_MS = 4000;
const uint16_t HTTP_READ_TIMEOUT_MS = 4000;
const unsigned long TLS_HANDSHAKE_TIMEOUT_S = 4;

OfflineReading ramBuffer[MAX_OFFLINE_READINGS];
int bufferCount = 0;

OfflineLog logBuffer[MAX_OFFLINE_LOGS];
int logBufferCount = 0;

static unsigned long lastUploadAttemptMs = 0;
static bool bootCheckPassed = false;

inline bool timeIsValid() {
  return time(nullptr) > MIN_VALID_EPOCH;
}

// Telegram requests in notifications.h still use the default timeouts
inline void setNetworkTimeouts(WiFiClientSecure& client, HTTPClient& http) {
  client.setHandshakeTimeout(TLS_HANDSHAKE_TIMEOUT_S);
  http.setConnectTimeout(HTTP_CONNECT_TIMEOUT_MS);
  http.setTimeout(HTTP_READ_TIMEOUT_MS);
}

inline void copyField(char* dst, size_t dstSize, const char* src) {
  strncpy(dst, src, dstSize - 1);
  dst[dstSize - 1] = '\0';
}

inline bool bufferReading(const char* ts, float temp, float hum, float press) {
  if (bufferCount >= MAX_OFFLINE_READINGS) return false;
  copyField(ramBuffer[bufferCount].timestamp, sizeof(ramBuffer[bufferCount].timestamp), ts);
  ramBuffer[bufferCount].temp  = temp;
  ramBuffer[bufferCount].hum   = hum;
  ramBuffer[bufferCount].press = press;
  ramBuffer[bufferCount].provisional = false;
  ramBuffer[bufferCount].bootSeconds = 0;
  bufferCount++;
  return true;
}

// Reading taken without a valid clock: the real timestamp is rebuilt after NTP sync
inline bool bufferProvisionalReading(uint32_t bootSeconds, float temp, float hum, float press) {
  if (bufferCount >= MAX_OFFLINE_READINGS) return false;
  ramBuffer[bufferCount].timestamp[0] = '\0';
  ramBuffer[bufferCount].temp  = temp;
  ramBuffer[bufferCount].hum   = hum;
  ramBuffer[bufferCount].press = press;
  ramBuffer[bufferCount].provisional = true;
  ramBuffer[bufferCount].bootSeconds = bootSeconds;
  bufferCount++;
  return true;
}

inline bool hasProvisionalReadings() {
  for (int i = 0; i < bufferCount; i++) {
    if (ramBuffer[i].provisional) return true;
  }
  return false;
}

inline bool bufferHasHour(const char* hourTimestamp) {
  for (int i = 0; i < bufferCount; i++) {
    if (!ramBuffer[i].provisional && strcmp(ramBuffer[i].timestamp, hourTimestamp) == 0) return true;
  }
  return false;
}

inline bool bufferLog(const char* ts, const char* severity, const char* message) {
  if (logBufferCount >= MAX_OFFLINE_LOGS) return false;
  copyField(logBuffer[logBufferCount].timestamp, sizeof(logBuffer[logBufferCount].timestamp), ts);
  copyField(logBuffer[logBufferCount].severity, sizeof(logBuffer[logBufferCount].severity), severity);
  copyField(logBuffer[logBufferCount].message, sizeof(logBuffer[logBufferCount].message), message);
  logBuffer[logBufferCount].provisional = false;
  logBuffer[logBufferCount].bootSeconds = 0;
  logBufferCount++;
  return true;
}

// Log written without a valid clock: the real timestamp is rebuilt after NTP sync
inline bool bufferProvisionalLog(uint32_t bootSeconds, const char* severity, const char* message) {
  if (logBufferCount >= MAX_OFFLINE_LOGS) return false;
  logBuffer[logBufferCount].timestamp[0] = '\0';
  copyField(logBuffer[logBufferCount].severity, sizeof(logBuffer[logBufferCount].severity), severity);
  copyField(logBuffer[logBufferCount].message, sizeof(logBuffer[logBufferCount].message), message);
  logBuffer[logBufferCount].provisional = true;
  logBuffer[logBufferCount].bootSeconds = bootSeconds;
  logBufferCount++;
  return true;
}

inline bool hasProvisionalLogs() {
  for (int i = 0; i < logBufferCount; i++) {
    if (logBuffer[i].provisional) return true;
  }
  return false;
}

inline void resolveProvisionalLogs() {
  if (!timeIsValid()) return;

  time_t nowEpoch = time(nullptr);
  uint32_t nowSeconds = millis() / 1000;

  for (int i = 0; i < logBufferCount; i++) {
    if (!logBuffer[i].provisional) continue;

    time_t epoch = nowEpoch - (time_t)(nowSeconds - logBuffer[i].bootSeconds);
    struct tm info;
    localtime_r(&epoch, &info);
    snprintf(logBuffer[i].timestamp, sizeof(logBuffer[i].timestamp), "%04d-%02d-%02dT%02d:%02d:%02dZ",
             info.tm_year + 1900,
             info.tm_mon + 1,
             info.tm_mday,
             info.tm_hour,
             info.tm_min,
             info.tm_sec);
    logBuffer[i].provisional = false;
  }
}

bool sendToSupabase(const char* ts, float temp, float hum, float press) {
  if (WiFi.status() != WL_CONNECTED) return false;
  ledWork();

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/sensor_data";
  
  http.begin(client, url);
  setNetworkTimeouts(client, http);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));
  http.addHeader("Prefer", "resolution=merge-duplicates");

  StaticJsonDocument<200> doc;
  doc["created_at"] = ts;
  doc["temp"]       = temp;
  doc["hum"]        = hum;
  doc["press"]      = press;

  String body;
  serializeJson(doc, body);

  int httpCode = http.POST(body);
  http.end();

  DEBUG_LOG("SUPABASE sensor_data POST code: %d", httpCode);

  return (httpCode == 200 || httpCode == 201);
}

bool sendLogToSupabaseDirect(const char* timestamp, const char* severity, const char* message) {
  if (WiFi.status() != WL_CONNECTED) return false;
  ledWork();

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/logs";
  
  http.begin(client, url);
  setNetworkTimeouts(client, http);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));

  StaticJsonDocument<250> doc;
  doc["created_at"]    = timestamp;
  doc["severity"]      = severity;
  doc["event_message"] = message;

  String body;
  serializeJson(doc, body);

  int httpCode = http.POST(body);
  http.end();

  DEBUG_LOG("SUPABASE logs POST code: %d", httpCode);

  return (httpCode == 200 || httpCode == 201);
}

bool sendReadingsBatch(const OfflineReading* items, int count) {
  if (WiFi.status() != WL_CONNECTED) return false;
  ledWork();

  DynamicJsonDocument doc(JSON_ARRAY_SIZE(BATCH_READINGS) + BATCH_READINGS * JSON_OBJECT_SIZE(4));
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < count; i++) {
    JsonObject o = arr.createNestedObject();
    o["created_at"] = items[i].timestamp;
    o["temp"]       = items[i].temp;
    o["hum"]        = items[i].hum;
    o["press"]      = items[i].press;
  }
  if (doc.overflowed()) return false;

  String body;
  serializeJson(doc, body);

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, String(SUPABASE_URL) + "/rest/v1/sensor_data");
  setNetworkTimeouts(client, http);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));
  http.addHeader("Prefer", "resolution=merge-duplicates");

  int httpCode = http.POST(body);
  http.end();

  DEBUG_LOG("SUPABASE sensor_data BATCH POST code: %d", httpCode);

  return (httpCode == 200 || httpCode == 201);
}

bool sendLogsBatch(const OfflineLog* items, int count) {
  if (WiFi.status() != WL_CONNECTED) return false;
  ledWork();

  DynamicJsonDocument doc(JSON_ARRAY_SIZE(BATCH_LOGS) + BATCH_LOGS * JSON_OBJECT_SIZE(3));
  JsonArray arr = doc.to<JsonArray>();
  for (int i = 0; i < count; i++) {
    JsonObject o = arr.createNestedObject();
    o["created_at"]    = items[i].timestamp;
    o["severity"]      = items[i].severity;
    o["event_message"] = items[i].message;
  }
  if (doc.overflowed()) return false;

  String body;
  serializeJson(doc, body);

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, String(SUPABASE_URL) + "/rest/v1/logs");
  setNetworkTimeouts(client, http);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));

  int httpCode = http.POST(body);
  http.end();

  DEBUG_LOG("SUPABASE logs BATCH POST code: %d", httpCode);

  return (httpCode == 200 || httpCode == 201);
}

// Requires a SELECT policy on sensor_data for the publishable key, otherwise the result is always empty.
// Returns false only on request/parse failure; outEpoch is 0 when the table has no rows.
bool fetchLastSensorEpoch(time_t& outEpoch) {
  if (WiFi.status() != WL_CONNECTED) return false;
  ledWork();

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, String(SUPABASE_URL) + "/rest/v1/sensor_data?select=created_at&order=created_at.desc&limit=1");
  setNetworkTimeouts(client, http);
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));

  int httpCode = http.GET();
  DEBUG_LOG("SUPABASE last sensor_data GET code: %d", httpCode);

  if (httpCode != 200) {
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, payload)) return false;

  outEpoch = 0;
  if (doc.is<JsonArray>() && doc.size() > 0) {
    const char* raw = doc[0]["created_at"] | "";
    int y, m, d, h, mi, s;
    if (sscanf(raw, "%d-%d-%dT%d:%d:%d", &y, &m, &d, &h, &mi, &s) != 6) return false;

    struct tm t = {0};
    t.tm_year  = y - 1900;
    t.tm_mon   = m - 1;
    t.tm_mday  = d;
    t.tm_hour  = h;
    t.tm_min   = mi;
    t.tm_sec   = s;
    t.tm_isdst = -1;
    outEpoch = mktime(&t);
  }
  return true;
}

// Rebuilds timestamps of provisional readings; drops those already covered by the database
// or belonging to an hour that the regular hourly task will still read.
bool resolveProvisionalReadings() {
  if (!hasProvisionalReadings()) return true;
  if (!timeIsValid()) return false;

  time_t lastDbEpoch = 0;
  if (!fetchLastSensorEpoch(lastDbEpoch)) return false;

  time_t nowEpoch = time(nullptr);
  uint32_t nowSeconds = millis() / 1000;

  struct tm nowInfo;
  localtime_r(&nowEpoch, &nowInfo);

  time_t currentHourStart = nowEpoch - (nowEpoch % 3600);
  time_t limitHour = (nowInfo.tm_min >= 5) ? currentHourStart : currentHourStart - 3600;
  time_t previousHour = lastDbEpoch - (lastDbEpoch % 3600);

  int kept = 0;
  for (int i = 0; i < bufferCount; i++) {
    OfflineReading r = ramBuffer[i];

    if (r.provisional) {
      time_t epoch = nowEpoch - (time_t)(nowSeconds - r.bootSeconds);
      time_t hour = epoch - (epoch % 3600);
      if (hour <= previousHour) continue;
      if (hour > limitHour) continue;

      struct tm hourInfo;
      localtime_r(&hour, &hourInfo);
      snprintf(r.timestamp, sizeof(r.timestamp), "%04d-%02d-%02dT%02d:00:00Z",
               hourInfo.tm_year + 1900,
               hourInfo.tm_mon + 1,
               hourInfo.tm_mday,
               hourInfo.tm_hour);
      r.provisional = false;
      previousHour = hour;
    }

    ramBuffer[kept++] = r;
  }
  bufferCount = kept;
  return true;
}

void flushRamBuffer() {
  lastUploadAttemptMs = millis();
  if (hasProvisionalReadings() && !resolveProvisionalReadings()) return;

  while (bufferCount > 0) {
    if (WiFi.status() != WL_CONNECTED) return;
    int n = (bufferCount < BATCH_READINGS) ? bufferCount : BATCH_READINGS;
    if (!sendReadingsBatch(ramBuffer, n)) return;
    memmove(&ramBuffer[0], &ramBuffer[n], (bufferCount - n) * sizeof(OfflineReading));
    bufferCount -= n;
  }
}

void flushLogBuffer() {
  lastUploadAttemptMs = millis();
  if (hasProvisionalLogs()) {
    if (!timeIsValid()) return;
    resolveProvisionalLogs();
  }

  while (logBufferCount > 0) {
    if (WiFi.status() != WL_CONNECTED) return;
    int n = (logBufferCount < BATCH_LOGS) ? logBufferCount : BATCH_LOGS;
    if (!sendLogsBatch(logBuffer, n)) return;
    memmove(&logBuffer[0], &logBuffer[n], (logBufferCount - n) * sizeof(OfflineLog));
    logBufferCount -= n;
  }
}

// Called from loop(): retries pending uploads that failed right after reconnection
void retryPendingUploads() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (!timeIsValid()) return;
  if (bufferCount == 0 && logBufferCount == 0) return;
  if (millis() - lastUploadAttemptMs < UPLOAD_RETRY_INTERVAL_MS) return;

  flushRamBuffer();
  flushLogBuffer();
}

bool sendLogToSupabase(const char* timestamp, const char* severity, const char* message) {
  if (!timeIsValid()) {
    return bufferProvisionalLog(millis() / 1000, severity, message);
  }

  if (WiFi.status() == WL_CONNECTED) {
    flushLogBuffer();
    if (sendLogToSupabaseDirect(timestamp, severity, message)) return true;
  }
  return bufferLog(timestamp, severity, message);
}

void saveTelemetryData(struct tm* timeinfo, float temp, float hum, float press) {
  char ts[25];
  snprintf(ts, sizeof(ts), "%04d-%02d-%02dT%02d:00:00Z",
           timeinfo->tm_year + 1900,
           timeinfo->tm_mon + 1,
           timeinfo->tm_mday,
           timeinfo->tm_hour);

  if (WiFi.status() == WL_CONNECTED) {
    flushRamBuffer();
    flushLogBuffer();
    if (!sendToSupabase(ts, temp, hum, press)) {
      bufferReading(ts, temp, hum, press);
    }
  } else {
    bufferReading(ts, temp, hum, press);
  }
}

// No heartbeat until the boot check succeeded, otherwise last_ping would be overwritten
void sendHeartbeat() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (!timeIsValid()) return;
  if (!bootCheckPassed) return;

  static unsigned long lastPing = 0;
  if (millis() - lastPing < 60000 && lastPing != 0) return;
  lastPing = millis();

  DEBUG_LOG("HEARTBEAT: sending ping");
  ledWork();

  struct tm timeinfo;
  char ts[25];
  if (getLocalTime(&timeinfo, 0)) {
    snprintf(ts, sizeof(ts), "%04d-%02d-%02dT%02d:%02d:%02dZ",
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec);
  } else {
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/device_status?id=eq.1";
  
  http.begin(client, url);
  setNetworkTimeouts(client, http);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));

  StaticJsonDocument<100> doc;
  doc["last_ping"] = ts;

  String body;
  serializeJson(doc, body);

  int httpCode = http.PATCH(body);
  http.end();

  DEBUG_LOG("HEARTBEAT PATCH code: %d", httpCode);
}

// Returns true only when last_ping was read successfully; the caller retries otherwise
bool handleBootSequence() {
  if (WiFi.status() != WL_CONNECTED) return false;
  if (!timeIsValid()) return false;

  DEBUG_LOG("BOOT SEQUENCE: checking last_ping");

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/device_status?id=eq.1&select=last_ping";
  
  http.begin(client, url);
  setNetworkTimeouts(client, http);
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));

  int httpCode = http.GET();
  DEBUG_LOG("BOOT SEQUENCE GET code: %d", httpCode);

  if (httpCode != 200) {
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  bool blackoutDetected = false;
  char fromStr[30], toStr[30], totStr[30], currentTs[30];
  char rigaFrom[35], rigaTo[35], rigaTot[35];
  char telegramMsg[250], supabaseMsg[150];

  DynamicJsonDocument doc(512);
  deserializeJson(doc, payload);

  if (doc.is<JsonArray>() && doc.size() > 0) {
    String lastPingStr = doc[0]["last_ping"].as<String>();
    DEBUG_LOG("last_ping raw: %s", lastPingStr.c_str());
    
    if (lastPingStr.length() > 10) {
      struct tm oldTime = {0};
      int y, m, d, h, min, s;
      if (sscanf(lastPingStr.c_str(), "%d-%d-%dT%d:%d:%d", &y, &m, &d, &h, &min, &s) == 6) {
        oldTime.tm_year = y - 1900;
        oldTime.tm_mon  = m - 1;
        oldTime.tm_mday = d;
        oldTime.tm_hour = h;
        oldTime.tm_min  = min;
        oldTime.tm_sec  = s;
        oldTime.tm_isdst = -1;

        time_t oldEpoch = mktime(&oldTime);
        
        struct tm nowInfo;
        if (getLocalTime(&nowInfo, 0)) {
          time_t nowEpoch = mktime(&nowInfo);
          long diffSec = nowEpoch - oldEpoch;

          char nowStr[25];
          snprintf(nowStr, sizeof(nowStr), "%04d-%02d-%02dT%02d:%02d:%02d",
                   nowInfo.tm_year + 1900, nowInfo.tm_mon + 1, nowInfo.tm_mday,
                   nowInfo.tm_hour, nowInfo.tm_min, nowInfo.tm_sec);

          DEBUG_LOG("last_ping (parsed): %s", lastPingStr.c_str());
          DEBUG_LOG("now (local): %s", nowStr);
          DEBUG_LOG("oldEpoch: %ld", (long)oldEpoch);
          DEBUG_LOG("nowEpoch: %ld", (long)nowEpoch);
          DEBUG_LOG("diffSec: %ld", diffSec);

          if (diffSec > 600) {
            DEBUG_LOG("BLACKOUT DETECTED");
            int totMinutes = diffSec / 60;
            
            int days = totMinutes / 1440;
            int hours = (totMinutes % 1440) / 60;
            int minutes = totMinutes % 60;

            snprintf(fromStr, sizeof(fromStr), "%02d-%02d-%04d %02d:%02d", d, m, y, h, min);
            snprintf(toStr, sizeof(toStr), "%02d-%02d-%04d %02d:%02d", 
                     nowInfo.tm_mday, nowInfo.tm_mon + 1, nowInfo.tm_year + 1900, 
                     nowInfo.tm_hour, nowInfo.tm_min);

            if (totMinutes < 60) {
              snprintf(totStr, sizeof(totStr), "%dm", totMinutes);
            } else if (totMinutes < 1440) {
              snprintf(totStr, sizeof(totStr), "%dh %dm", hours, minutes);
            } else {
              snprintf(totStr, sizeof(totStr), "%dd %dh %dm", days, hours, minutes);
            }

            snprintf(currentTs, sizeof(currentTs), "%04d-%02d-%02dT%02d:%02d:%02dZ",
                     nowInfo.tm_year + 1900, nowInfo.tm_mon + 1, nowInfo.tm_mday,
                     nowInfo.tm_hour, nowInfo.tm_min, nowInfo.tm_sec);

            snprintf(rigaFrom, sizeof(rigaFrom), "`%-5s: %s`", "From", fromStr);
            snprintf(rigaTo,   sizeof(rigaTo),   "`%-5s: %s`", "To",   toStr);
            snprintf(rigaTot,  sizeof(rigaTot),  "`%-5s: %s`", "TOT",  totStr);

            snprintf(telegramMsg, sizeof(telegramMsg),
              "⚡ *BLACKOUT DETECTED*\n%s\n%s\n%s",
              rigaFrom, rigaTo, rigaTot
            );

            snprintf(supabaseMsg, sizeof(supabaseMsg),
              "⚡ BLACKOUT DETECTED - %s",
              totStr
            );

            blackoutDetected = true;
          }
        }
      }
    }
  }
  bootCheckPassed = true;

  if (blackoutDetected) {
    sendTelegramMessage(telegramMsg);
    sendLogToSupabase(currentTs, "🔴", supabaseMsg);
  }

  flushRamBuffer();
  flushLogBuffer();
  sendHeartbeat();
  setLedState(LED_STATE_IDLE);
  return true;
}