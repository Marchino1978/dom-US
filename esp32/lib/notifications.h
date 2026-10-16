#pragma once

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Preferences.h>

#include "../config.h"
#include "sensors.h"
#include "storage_cloud.h"
#include "led_status.h"
#include "i2c_addresses.h"

extern bool alarmEnabled;
extern bool alarmTriggered;
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
  ledWork();

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) + "/sendMessage";
  
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<1024> doc;
  doc["chat_id"] = TELEGRAM_CHAT_ID;
  doc["text"] = message;
  doc["parse_mode"] = "Markdown";

  String requestBody;
  serializeJson(doc, requestBody);

  int httpCode = http.POST(requestBody);
  http.end();
  DEBUG_LOG("TELEGRAM sendMessage code: %d heap: %u detail: %s", httpCode, ESP.getFreeHeap(), HTTPClient::errorToString(httpCode).c_str());
}

inline void sendTelegramYesNo(String message, const char* yesData, const char* noData) {
  if (WiFi.status() != WL_CONNECTED) return;
  ledWork();

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) + "/sendMessage";

  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<1024> doc;
  doc["chat_id"] = TELEGRAM_CHAT_ID;
  doc["text"] = message;
  doc["parse_mode"] = "Markdown";

  JsonObject markup = doc.createNestedObject("reply_markup");
  JsonArray keyboard = markup.createNestedArray("inline_keyboard");
  JsonArray row = keyboard.createNestedArray();

  JsonObject btnYes = row.createNestedObject();
  btnYes["text"] = "YES";
  btnYes["callback_data"] = yesData;

  JsonObject btnNo = row.createNestedObject();
  btnNo["text"] = "NO";
  btnNo["callback_data"] = noData;

  String requestBody;
  serializeJson(doc, requestBody);

  http.POST(requestBody);
  http.end();
}

inline void answerTelegramCallback(const String& callbackId) {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) + "/answerCallbackQuery";

  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<256> doc;
  doc["callback_query_id"] = callbackId;

  String requestBody;
  serializeJson(doc, requestBody);

  http.POST(requestBody);
  http.end();
}

inline void handleI2cCallback(const String& data) {
  char ts[25];
  getCurrentIsoTimestamp(ts, sizeof(ts));

  if (data == "i2c_yes") {
    if (!i2cResultsValid) {
      sendTelegramMessage("ℹ️ *I2C SCAN DATA NOT AVAILABLE* - run /i2c\\_scan again");
      return;
    }

    char buf[300];
    for (int i = 0; i < i2cResultCount; i++) {
      buildI2cDeviceMessage(i, buf, sizeof(buf));
      sendTelegramMessage(String(buf));
    }
    i2cResultsValid = false;
    sendLogToSupabase(ts, "⚪", "I2C SCAN details sent to user");
  } else if (data == "i2c_no") {
    i2cResultsValid = false;
  }
}

inline const char* wifiSignalQuality(int rssi) {
  if (rssi > -55) return "excellent";
  if (rssi > -67) return "good";
  if (rssi > -75) return "fair";
  return "poor";
}

inline void appendMemoryRow(String& msg, const char* label, uint32_t bytes) {
  char row[40];
  snprintf(row, sizeof(row), "`%-5s : %4lu KB`\n", label, (unsigned long)(bytes / 1024));
  msg += row;
}

inline String buildSystemInfoMessage() {
  // getFreeSketchSpace() returns the size of the next OTA slot, used here as the app partition size
  uint32_t romUsed  = ESP.getSketchSize();
  uint32_t romTot   = ESP.getFreeSketchSpace();
  uint32_t romFree  = (romTot > romUsed) ? (romTot - romUsed) : 0;
  uint32_t ramTot   = ESP.getHeapSize();
  uint32_t ramFree  = ESP.getFreeHeap();
  uint32_t ramUsed  = (ramTot > ramFree) ? (ramTot - ramFree) : 0;
  uint32_t ramMin   = ESP.getMinFreeHeap();
  uint32_t ramBlock = ESP.getMaxAllocHeap();

  String msg;
  msg.reserve(520);
  msg = "*SYSTEM INFO*\n\n";

  msg += "💾 *ROM*\n";
  appendMemoryRow(msg, "Tot",  romTot);
  appendMemoryRow(msg, "Used", romUsed);
  appendMemoryRow(msg, "Free", romFree);

  msg += "\n🧠 *RAM*\n";
  appendMemoryRow(msg, "Tot",   ramTot);
  appendMemoryRow(msg, "Used",  ramUsed);
  appendMemoryRow(msg, "Free",  ramFree);
  appendMemoryRow(msg, "Min",   ramMin);
  appendMemoryRow(msg, "Block", ramBlock);

  char row[40];
  msg += "\n📦 *BUFFER*\n";
  snprintf(row, sizeof(row), "`%-5s : %d/%d`\n", "Sens", bufferCount, MAX_OFFLINE_READINGS);
  msg += row;
  snprintf(row, sizeof(row), "`%-5s : %d/%d`\n", "Log", logBufferCount, MAX_OFFLINE_LOGS);
  msg += row;

  msg += "\n📶 *WIFI*\n";
  if (WiFi.status() == WL_CONNECTED) {
    int rssi = WiFi.RSSI();
    msg += "`SSID   : " + WiFi.SSID() + "`\n";
    msg += "`Signal : " + String(wifiSignalQuality(rssi)) + " (" + String(rssi) + " dBm)`\n";
    msg += "`IP     : " + WiFi.localIP().toString() + "`\n";
  } else {
    msg += "`Status : offline`\n";
  }
  msg += "`MAC    : " + WiFi.macAddress() + "`";

  return msg;
}

inline void checkTelegramUpdates() {
  if (WiFi.status() != WL_CONNECTED) return;

  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 3000) return;
  lastCheck = millis();

  static long lastUpdateId = 0;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) + 
               "/getUpdates?offset=" + String(lastUpdateId + 1) + "&timeout=0";

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    http.end();

    StaticJsonDocument<256> filter;
    filter["result"][0]["update_id"] = true;
    filter["result"][0]["message"]["text"] = true;
    filter["result"][0]["callback_query"]["id"] = true;
    filter["result"][0]["callback_query"]["data"] = true;

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload, DeserializationOption::Filter(filter));

    JsonArray result = doc["result"].as<JsonArray>();
    for (JsonObject update : result) {
      lastUpdateId = update["update_id"];
      ledWork();

      JsonObject callback = update["callback_query"];
      if (!callback.isNull()) {
        String callbackId = callback["id"].as<String>();
        String callbackData = callback["data"].as<String>();
        answerTelegramCallback(callbackId);
        handleI2cCallback(callbackData);
        continue;
      }

      String text = update["message"]["text"].as<String>();
      text.toLowerCase();
      text.trim();
      DEBUG_LOG("TELEGRAM RX: '%s'", text.c_str());

      char ts[25];

      if (text == "/on" || text == "on") {
        if (alarmEnabled) {
          sendTelegramMessage("ℹ️ *ALARM IS ALREADY ON*");
          getCurrentIsoTimestamp(ts, sizeof(ts));
          sendLogToSupabase(ts, "⚪", "ALARM IS ALREADY ON");
        } else {
          alarmEnabled = true;
          preferences.putBool("alarm_state", true);
          setLedStatusEnabled(false);
          
          sendTelegramMessage("🟢 *ALARM ON*");
          
          getCurrentIsoTimestamp(ts, sizeof(ts));
          sendLogToSupabase(ts, "⚪", "ALARM ARMED by user");
        }
      } 
      else if (text == "/off" || text == "off") {
        if (!alarmEnabled) {
          sendTelegramMessage("ℹ️ *ALARM IS ALREADY OFF*");
          getCurrentIsoTimestamp(ts, sizeof(ts));
          sendLogToSupabase(ts, "⚪", "ALARM IS ALREADY OFF");
        } else {
          alarmEnabled = false;
          preferences.putBool("alarm_state", false);
          setLedStatusEnabled(true);
          
          sendTelegramMessage("🔴 *ALARM OFF*");
          
          getCurrentIsoTimestamp(ts, sizeof(ts));
          sendLogToSupabase(ts, "⚪", "ALARM DISARMED by user");
        }
      } 
      else if (text == "/reset" || text == "reset") {
        getCurrentIsoTimestamp(ts, sizeof(ts));

        if (!alarmEnabled) {
          sendTelegramMessage("ℹ️ *ALARM IS OFF* - nothing to reset");
          sendLogToSupabase(ts, "⚪", "RESET requested but ALARM is OFF");
        } else if (!alarmTriggered) {
          sendTelegramMessage("ℹ️ *ALARM NOT TRIGGERED* - nothing to reset");
          sendLogToSupabase(ts, "⚪", "RESET requested but ALARM not triggered");
        } else {
          alarmTriggered = false;
          sendTelegramMessage("🟢 *ALARM RESET* by user - system re-armed");
          sendLogToSupabase(ts, "⚪", "ALARM RESET by user - system re-armed");
        }
      }
      else if (text == "/led" || text == "led") {
        getCurrentIsoTimestamp(ts, sizeof(ts));

        if (alarmEnabled) {
          sendTelegramMessage("🎚️ *LED STATUS TOGGLED IGNORED*");
          sendLogToSupabase(ts, "⚪", "🎚️ LED STATUS TOGGLED ignored");
        } else {
          setLedStatusEnabled(!ledStatusEnabled);
          if (ledStatusEnabled) {
            sendTelegramMessage("🎚️ *LED STATUS TOGGLED TO ON*");
            sendLogToSupabase(ts, "⚪", "🎚️ LED STATUS TOGGLED TO ON by user");
          } else {
            sendTelegramMessage("🎚️ *LED STATUS TOGGLED TO OFF*");
            sendLogToSupabase(ts, "⚪", "🎚️ LED STATUS TOGGLED TO OFF by user");
          }
        }
      }
      else if (text == "/i2c_scan" || text == "i2c_scan") {
        getCurrentIsoTimestamp(ts, sizeof(ts));

        runI2cScan();

        char summary[160];
        buildI2cSummaryMessage(summary, sizeof(summary));

        if (i2cTotalFound > 0) {
          sendTelegramYesNo(String(summary), "i2c_yes", "i2c_no");
        } else {
          sendTelegramMessage(String(summary));
        }

        sendLogToSupabase(ts, "⚪", "I2C SCAN requested by user");
      }
      else if (text == "/info" || text == "info") {
        getCurrentIsoTimestamp(ts, sizeof(ts));

        sendTelegramMessage(buildSystemInfoMessage());

        sendLogToSupabase(ts, "⚪", "SYSTEM INFO requested by user");
      }
      else if (text == "/status" || text == "status") {
        getCurrentIsoTimestamp(ts, sizeof(ts));

        float temp = readTemperature();
        float hum = readHumidity();
        float press = readPressure();
        float lux = readAmbientLux();
        DEBUG_LOG("STATUS RAW: temp=%.2f hum=%.2f press=%.2f lux=%.2f", temp, hum, press, lux);

        String tempStr  = isnan(temp)  ? "---" : String(temp, 0);
        String humStr   = isnan(hum)   ? "---" : String(hum, 0);
        String pressStr = isnan(press) ? "---" : String(press, 0);
        String luxStr   = isnan(lux)   ? "---" : String(lux, 0);
        String luceStato = "";
        if (!isnan(lux)) luceStato = isAmbientLightOn(lux) ? " 🟡 ON" : " ⚫ OFF";

        String icona_stato = alarmEnabled ? "🟢" : "🔴";
        String stato = alarmEnabled ? "ON" : "OFF";

        char rigaTemp[25], rigaHum[25], rigaPress[25], rigaLux[25];

        snprintf(rigaTemp,  sizeof(rigaTemp),  "`%-5s : %4s %-3s`", "Temp",  tempStr.c_str(),  "°C");
        snprintf(rigaHum,   sizeof(rigaHum),   "`%-5s : %4s %-3s`", "Hum",   humStr.c_str(),   "%");
        snprintf(rigaPress, sizeof(rigaPress), "`%-5s : %4s %-3s`", "Press", pressStr.c_str(), "hPa");
        snprintf(rigaLux,   sizeof(rigaLux),   "`%-5s : %4s %-3s`", "Light", luxStr.c_str(),   AMBIENT_UNIT);

        String statusMsg = "*ALARM STATUS:* " + icona_stato + " *" + stato + "*\n\n";
        statusMsg += "🌡️ " + String(rigaTemp) + "\n";
        statusMsg += "💧 " + String(rigaHum) + "\n";
        statusMsg += "🌀 " + String(rigaPress) + "\n";
        statusMsg += "💡 " + String(rigaLux) + luceStato;

        sendTelegramMessage(statusMsg);
    
        sendLogToSupabase(ts, "⚪", "ALARM STATUS requested by user");
        getCurrentIsoTimestamp(ts, sizeof(ts));
        sendLogToSupabase(ts, "⚪", "ALARM STATUS sent to user");
      }
    }
  }
  http.end();
}