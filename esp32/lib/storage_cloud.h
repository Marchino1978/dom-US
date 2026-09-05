#ifndef STORAGE_CLOUD_H
#define STORAGE_CLOUD_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "../config.h"

struct OfflineReading {
  char timestamp[25];
  float temp;
  float hum;
  float press;
};

const int MAX_OFFLINE_READINGS = 48;
OfflineReading ramBuffer[MAX_OFFLINE_READINGS];
int bufferCount = 0;

// ======================================================
// 1. SEND SINGLE RECORD TO SUPABASE (sensor_data)
// ======================================================
bool sendToSupabase(const char* ts, float temp, float hum, float press) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/sensor_data";
  
  http.begin(url);
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

  return (httpCode == 200 || httpCode == 201);
}

// ======================================================
// 2. FLUSH RAM BUFFER WHEN WIFI IS BACK
// ======================================================
void flushRamBuffer() {
  if (bufferCount == 0 || WiFi.status() != WL_CONNECTED) return;

  int sentSuccessfully = 0;
  for (int i = 0; i < bufferCount; i++) {
    bool ok = sendToSupabase(ramBuffer[i].timestamp, ramBuffer[i].temp, ramBuffer[i].hum, ramBuffer[i].press);
    if (ok) {
      sentSuccessfully++;
    } else {
      break;
    }
  }

  if (sentSuccessfully > 0) {
    for (int i = sentSuccessfully; i < bufferCount; i++) {
      ramBuffer[i - sentSuccessfully] = ramBuffer[i];
    }
    bufferCount -= sentSuccessfully;
  }
}

// ======================================================
// 3. SAVE TELEMETRY (NORMALIZED TO HH:00:00)
// ======================================================
void saveTelemetryData(struct tm* timeinfo, float temp, float hum, float press) {
  char ts[25];
  snprintf(ts, sizeof(ts), "%04d-%02d-%02dT%02d:00:00Z",
           timeinfo->tm_year + 1900,
           timeinfo->tm_mon + 1,
           timeinfo->tm_mday,
           timeinfo->tm_hour);

  if (WiFi.status() == WL_CONNECTED) {
    flushRamBuffer();
    sendToSupabase(ts, temp, hum, press);
  } else {
    if (bufferCount < MAX_OFFLINE_READINGS) {
      strncpy(ramBuffer[bufferCount].timestamp, ts, sizeof(ramBuffer[bufferCount].timestamp));
      ramBuffer[bufferCount].temp  = temp;
      ramBuffer[bufferCount].hum   = hum;
      ramBuffer[bufferCount].press = press;
      bufferCount++;
    }
  }
}

// ======================================================
//  4. BLACKOUT HEARTBEAT
// ======================================================
void sendHeartbeat() {
  if (WiFi.status() != WL_CONNECTED) return;

  static unsigned long lastPing = 0;
  if (millis() - lastPing < 60000 && lastPing != 0) return;
  lastPing = millis();

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/device_status?id=eq.1";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));

  StaticJsonDocument<100> doc;
  doc["last_ping"] = "now()";

  String body;
  serializeJson(doc, body);

  http.PATCH(body);
  http.end();
}

// ======================================================
//  5. SEND EVENT LOG TO SUPABASE
// ======================================================
bool sendLogToSupabase(const char* timestamp, const char* severity, const char* message) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/logs";
  
  http.begin(url);
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

  return (httpCode == 200 || httpCode == 201);
}

// ======================================================
//  7. BOOT SEQUENCE & BLACKOUT CHECK HANDLER
// ======================================================
void handleBootSequence() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/device_status?id=eq.1&select=last_ping";
  
  http.begin(url);
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));

  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(512);
    deserializeJson(doc, payload);

    if (doc.is<JsonArray>() && doc.size() > 0) {
      String lastPingStr = doc[0]["last_ping"].as<String>();
      
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

          time_t oldEpoch = mktime(&oldTime);
          
          struct tm nowInfo;
          if (getLocalTime(&nowInfo)) {
            time_t nowEpoch = mktime(&nowInfo);
            long diffSec = nowEpoch - oldEpoch;

            if (diffSec > 600) {
              int totMinutes = diffSec / 60;
              
              int days = totMinutes / 1440;
              int hours = (totMinutes % 1440) / 60;
              int minutes = totMinutes % 60;

              char fromStr[30], toStr[30], totStr[30], currentTs[30];
              char telegramMsg[250], supabaseMsg[150];

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

              snprintf(telegramMsg, sizeof(telegramMsg),
                "⚡ BLACKOUT DETECTED\n"
                "From : %s\n"
                "To   : %s\n"
                "TOT  : %s",
                fromStr, toStr, totStr
              );

              snprintf(supabaseMsg, sizeof(supabaseMsg),
                "⚡ BLACKOUT DETECTED - %s",
                totStr
              );

              sendLogToSupabase(currentTs, "🔴", supabaseMsg);
            }
          }
        }
      }
    }
  }
  http.end();
  
  sendHeartbeat();
}

#endif










