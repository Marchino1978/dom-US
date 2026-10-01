#pragma once

#include "../../../config.h"
#include <Arduino.h>
#include <DHT.h>

// ======================================================
// Write your code and declarations below this line
// ======================================================

static DHT dht(PIN_DHT, DHT22);
 
inline void initClimateHardware() {
  dht.begin();
}
 
// DHT library caches readings internally (min interval 2 s)
inline float readTemperatureValue() {
  return dht.readTemperature();
}
 
inline float readHumidityValue() {
  return dht.readHumidity();
}