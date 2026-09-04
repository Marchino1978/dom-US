#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>
#include "../config.h"
#include "sensors.h"
#include "storage_cloud.h"

extern bool alarmEnabled;
extern Preferences preferences;

inline void getCurrentIsoTimestamp(char* buffer, size_t maxLen) {
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
    snprintf(buffer, maxLen, "");
  }
}

inline void sendTelegramMessage(String message) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) + "/sendMessage";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<512> doc;
  doc["chat_id"] = TELEGRAM_CHAT_ID;
  doc["text"] = message;
  doc["parse_mode"] = "Markdown";

  String requestBody;
  serializeJson(doc, requestBody);

  http.POST(requestBody);
  http.end();
}

inline void checkTelegramUpdates() {
  if (WiFi.status() != WL_CONNECTED) return;

  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 3000) return;
  lastCheck = millis();

  static long lastUpdateId = 0;
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) + 
               "/getUpdates?offset=" + String(lastUpdateId + 1) + "&timeout=0";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    JsonArray result = doc["result"].as<JsonArray>();
    for (JsonObject update : result) {
      lastUpdateId = update["update_id"];
      String text = update["message"]["text"].as<String>();
      text.toLowerCase();
      text.trim();

char ts[25];

      if (text == "/on" || text == "on") {
        if (alarmEnabled) {
          sendTelegramMessage("ℹ️ *ALARM IS ALREADY ON*");
          getCurrentIsoTimestamp(ts, sizeof(ts));
          sendLogToSupabase(ts, "neutral", "ALARM IS ALREADY ON");
        } else {
          alarmEnabled = true;
          preferences.putBool("alarm_state", true);
          
          sendTelegramMessage("🟢 *ALARM ON*");
          
          getCurrentIsoTimestamp(ts, sizeof(ts));
          sendLogToSupabase(ts, "neutral", "ALARM ARMED by user");
        }
      } 
      else if (text == "/off" || text == "off") {
        if (!alarmEnabled) {
          sendTelegramMessage("ℹ️ *ALARM IS ALREADY OFF*");
          getCurrentIsoTimestamp(ts, sizeof(ts));
          sendLogToSupabase(ts, "neutral", "ALARM IS ALREADY OFF");
        } else {
          alarmEnabled = false;
          preferences.putBool("alarm_state", false);
          
          sendTelegramMessage("🔴 *ALARM OFF*");
          
          getCurrentIsoTimestamp(ts, sizeof(ts));
          sendLogToSupabase(ts, "neutral", "ALARM DISARMED by user");
        }
      } 
      else if (text == "/status" || text == "status") {
        getCurrentIsoTimestamp(ts, sizeof(ts));
        sendLogToSupabase(ts, "neutral", "ALARM STATUS requested by user");

        float temp = readTemperature();
        float hum = readHumidity();
        float press = readPressure();
        float lux = readAmbientLux();

        String icona_stato = alarmEnabled ? "🟢" : "🔴";
        String stato = alarmEnabled ? "ON" : "OFF";

        String statusMsg = "*ALARM STATUS:* " + icona_stato + " " + stato + "\n";
        statusMsg += "`🌡️ Temp  :   " + String(temp, 0) + " °C`\n";
        statusMsg += "`💧 Hum   :   " + String(hum, 0) + " %`\n";
        statusMsg += "`🌀 Press : " + String(press, 0) + " hPa`\n";
        statusMsg += "`💡 Light :   " + String(lux, 0) + " Lux`";

        sendTelegramMessage(statusMsg);
        
        getCurrentIsoTimestamp(ts, sizeof(ts));
        sendLogToSupabase(ts, "warning", "ALARM STATUS sent to user");
      }
    }
  }
  http.end();
}

#endif