#pragma once

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "../config.h"

void sendTelegramMessage(String message);

struct OfflineReading {
  char timestamp[25];
  float temp;
  float hum;
  float press;
};

struct OfflineLog {
  char timestamp[25];
  char severity[10];
  char message[100];
};

const int MAX_OFFLINE_READINGS = 72;
const int MAX_OFFLINE_LOGS = 500;

OfflineReading ramBuffer[MAX_OFFLINE_READINGS];
int bufferCount = 0;

OfflineLog logBuffer[MAX_OFFLINE_LOGS];
int logBufferCount = 0;

bool sendToSupabase(const char* ts, float temp, float hum, float press) {
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/sensor_data";
  
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));
  http.addHeader("Prefer", "resolution=merge-duplicates");

  StaticJsonDocument<200> doc;
  doc["created_at"] = ts;
  doc["temp"]        = temp;
  doc["hum"]         = hum;
  doc["press"]       = press;

  String body;
  serializeJson(doc, body);

  int httpCode = http.POST(body);
  http.end();

  Serial.print("SUPABASE sensor_data POST code: ");
  Serial.println(httpCode);

  return (httpCode == 200 || httpCode == 201);
}

bool sendLogToSupabaseDirect(const char* timestamp, const char* severity, const char* message) {
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/logs";
  
  http.begin(client, url);
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

  Serial.print("SUPABASE logs POST code: ");
  Serial.println(httpCode);

  return (httpCode == 200 || httpCode == 201);
}

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

void flushLogBuffer() {
  if (logBufferCount == 0 || WiFi.status() != WL_CONNECTED) return;

  int sentSuccessfully = 0;
  for (int i = 0; i < logBufferCount; i++) {
    bool ok = sendLogToSupabaseDirect(logBuffer[i].timestamp, logBuffer[i].severity, logBuffer[i].message);
    if (ok) {
      sentSuccessfully++;
    } else {
      break;
    }
  }

  if (sentSuccessfully > 0) {
    for (int i = sentSuccessfully; i < logBufferCount; i++) {
      logBuffer[i - sentSuccessfully] = logBuffer[i];
    }
    logBufferCount -= sentSuccessfully;
  }
}

bool sendLogToSupabase(const char* timestamp, const char* severity, const char* message) {
  if (WiFi.status() == WL_CONNECTED) {
    flushLogBuffer();
    return sendLogToSupabaseDirect(timestamp, severity, message);
  } else {
    if (logBufferCount < MAX_OFFLINE_LOGS) {
      strncpy(logBuffer[logBufferCount].timestamp, timestamp, sizeof(logBuffer[logBufferCount].timestamp));
      strncpy(logBuffer[logBufferCount].severity, severity, sizeof(logBuffer[logBufferCount].severity));
      strncpy(logBuffer[logBufferCount].message, message, sizeof(logBuffer[logBufferCount].message));
      logBufferCount++;
      return true;
    }
    return false;
  }
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

void sendHeartbeat() {
  if (WiFi.status() != WL_CONNECTED) return;

  static unsigned long lastPing = 0;
  if (millis() - lastPing < 60000 && lastPing != 0) return;
  lastPing = millis();

  Serial.println("HEARTBEAT: sending ping");

  struct tm timeinfo;
  char ts[25];
  if (getLocalTime(&timeinfo)) {
    snprintf(ts, sizeof(ts), "%04d-%02d-%02dT%02d:%02d:%02dZ",
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec);
  } else {
    strncpy(ts, "2026-09-01T00:00:00Z", sizeof(ts));
  }

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/device_status?id=eq.1";
  
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));

  StaticJsonDocument<100> doc;
  doc["last_ping"] = ts;

  String body;
  serializeJson(doc, body);

  int httpCode = http.PATCH(body);
  http.end();

  Serial.print("HEARTBEAT PATCH code: ");
  Serial.println(httpCode);
}

void handleBootSequence() {
  if (WiFi.status() != WL_CONNECTED) return;

  Serial.println("BOOT SEQUENCE: checking last_ping");

  flushRamBuffer();
  flushLogBuffer();

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/device_status?id=eq.1&select=last_ping";
  
  http.begin(client, url);
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));

  int httpCode = http.GET();
  Serial.print("BOOT SEQUENCE GET code: ");
  Serial.println(httpCode);

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(512);
    deserializeJson(doc, payload);

    if (doc.is<JsonArray>() && doc.size() > 0) {
      String lastPingStr = doc[0]["last_ping"].as<String>();
      Serial.print("last_ping raw: ");
      Serial.println(lastPingStr);
      
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
          if (getLocalTime(&nowInfo)) {
            time_t nowEpoch = mktime(&nowInfo);
            long diffSec = nowEpoch - oldEpoch;

            char nowStr[25];
            snprintf(nowStr, sizeof(nowStr), "%04d-%02d-%02dT%02d:%02d:%02d",
                     nowInfo.tm_year + 1900, nowInfo.tm_mon + 1, nowInfo.tm_mday,
                     nowInfo.tm_hour, nowInfo.tm_min, nowInfo.tm_sec);

            Serial.print("last_ping (parsed): ");
            Serial.println(lastPingStr);
            Serial.print("now (local): ");
            Serial.println(nowStr);
            Serial.print("oldEpoch: ");
            Serial.println((long)oldEpoch);
            Serial.print("nowEpoch: ");
            Serial.println((long)nowEpoch);
            Serial.print("diffSec: ");
            Serial.println(diffSec);

            if (diffSec > 600) {
              Serial.println("BLACKOUT DETECTED");
              int totMinutes = diffSec / 60;
              
              int days = totMinutes / 1440;
              int hours = (totMinutes % 1440) / 60;
              int minutes = totMinutes % 60;

              char fromStr[30], toStr[30], totStr[30], currentTs[30];
              char rigaFrom[35], rigaTo[35], rigaTot[35];
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

              sendLogToSupabase(currentTs, "🔴", supabaseMsg);
              sendTelegramMessage(telegramMsg);
            }
          }
        }
      }
    }
  }
  http.end();
  
  sendHeartbeat();
}