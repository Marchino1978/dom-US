#ifndef STORAGE_CLOUD_H
#define STORAGE_CLOUD_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../config.h"

struct OfflineReading {
  char timestamp[25];
  float temp;
  float hum;
};

const int MAX_OFFLINE_READINGS = 48;
OfflineReading ramBuffer[MAX_OFFLINE_READINGS];
int bufferCount = 0;

// ======================================================
// 1. SEND SINGLE RECORD TO SUPABASE
// ======================================================
bool sendToSupabase(const char* ts, float temp, float hum) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = String(SUPABASE_URL) + "/rest/v1/telemetry";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_KEY));
  http.addHeader("Prefer", "resolution=merge-duplicates");

  StaticJsonDocument<200> doc;
  doc["created_at"]  = ts;
  doc["temperature"] = temp;
  doc["humidity"]    = hum;

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
    bool ok = sendToSupabase(ramBuffer[i].timestamp, ramBuffer[i].temp, ramBuffer[i].hum);
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
// 3. SAVE TELEMETRY (ONLINE OR RAM BUFFER)
// ======================================================
void saveTelemetryData(struct tm* timeinfo, float temp, float hum) {
  char ts[25];
  snprintf(ts, sizeof(ts), "%04d-%02d-%02dT%02d:00:00Z",
           timeinfo->tm_year + 1900,
           timeinfo->tm_mon + 1,
           timeinfo->tm_mday,
           timeinfo->tm_hour);

  if (WiFi.status() == WL_CONNECTED) {
    flushRamBuffer();
    sendToSupabase(ts, temp, hum);
  } else {
    if (bufferCount < MAX_OFFLINE_READINGS) {
      strncpy(ramBuffer[bufferCount].timestamp, ts, sizeof(ramBuffer[bufferCount].timestamp));
      ramBuffer[bufferCount].temp = temp;
      ramBuffer[bufferCount].hum  = hum;
      bufferCount++;
    }
  }
}

// ======================================================
// 4. BLACKOUT HEARTBEAT (UPDATES SINGLE ROW)
// ======================================================
void sendHeartbeat() {
  if (WiFi.status() != WL_CONNECTED) return;

  static unsigned long lastPing = 0;
  if (millis() - lastPing < 300000 && lastPing != 0) return;
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

#endif