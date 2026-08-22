#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../config.h"
#include "sensors.h"

extern bool alarmEnabled;

inline void sendTelegramMessage(String message) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) + "/sendMessage";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
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
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) + 
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
      text.toUpperCase();
      text.trim();

      if (text == "ALARM ON") {
        alarmEnabled = true;
        sendTelegramMessage("🛡️ *Sistema Allarme ATTIVATO*");
      } 
      else if (text == "ALARM OFF") {
        alarmEnabled = false;
        sendTelegramMessage("🔓 *Sistema Allarme DISATTIVATO*");
      } 
      else if (text == "ALARM STATUS") {
        float temp = readTemperature();
        float hum = readHumidity();
        float lux = readAmbientLux();

        String statusMsg = "📊 *STATO SISTEMA*\n\n";
        statusMsg += "• Stato Allarme: " + String(alarmEnabled ? "🟢 ATTIVO" : "🔴 DISATTIVATO") + "\n";
        statusMsg += "• Temperatura: " + String(temp, 1) + " °C\n";
        statusMsg += "• Umidità: " + String(hum, 0) + " %\n";
        statusMsg += "• Luce Ambiente: " + String(lux, 0) + " Lux";

        sendTelegramMessage(statusMsg);
      }
    }
  }
  http.end();
}

#endif