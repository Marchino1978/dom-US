#pragma once

#include "../../../config.h"
#include "../../debug.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
 
#define AHT20_MIN_INTERVAL_MS 2000
 
static Adafruit_AHTX0 aht;
static bool ahtReady = false;
static bool ahtHasRead = false;
static unsigned long ahtLastRead = 0;
static float ahtTemperature = NAN;
static float ahtHumidity = NAN;
 
inline void initClimateHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  ahtReady = aht.begin(&Wire);
  if (!ahtReady) {
    DEBUG_LOG("AHT20 NOT FOUND");
  }
}
 
// One measurement feeds both temperature and humidity; repeated calls reuse it
inline void ahtUpdate() {
  if (!ahtReady) return;
  if (ahtHasRead && millis() - ahtLastRead < AHT20_MIN_INTERVAL_MS) return;
 
  sensors_event_t humidityEvent, temperatureEvent;
  if (aht.getEvent(&humidityEvent, &temperatureEvent)) {
    ahtTemperature = temperatureEvent.temperature;
    ahtHumidity = humidityEvent.relative_humidity;
  } else {
    ahtTemperature = NAN;
    ahtHumidity = NAN;
  }
  ahtLastRead = millis();
  ahtHasRead = true;
}
 
inline float readTemperatureValue() {
  ahtUpdate();
  return ahtTemperature;
}
 
inline float readHumidityValue() {
  ahtUpdate();
  return ahtHumidity;
}