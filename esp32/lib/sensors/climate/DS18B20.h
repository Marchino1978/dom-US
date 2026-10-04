#pragma once

#include "../../../config.h"

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// External 4.7k pull-up between data line and 3.3V is required
#define DS18B20_RESOLUTION_BITS 10
 
static OneWire oneWire(PIN_DHT);
static DallasTemperature ds18b20(&oneWire);
static bool ds18b20Ready = false;
 
inline void initClimateHardware() {
  ds18b20.begin();
  ds18b20Ready = ds18b20.getDeviceCount() > 0;
  if (!ds18b20Ready) {
    Serial.println("DS18B20 NOT FOUND");
    return;
  }
  ds18b20.setResolution(DS18B20_RESOLUTION_BITS);
}
 
// Blocks for the conversion time (about 190 ms at 10 bit)
inline float readTemperatureValue() {
  if (!ds18b20Ready) return NAN;
  ds18b20.requestTemperatures();
  float t = ds18b20.getTempCByIndex(0);
  if (t == DEVICE_DISCONNECTED_C) return NAN;
  return t;
}
 
// DS18B20 has no humidity channel; required by sensor_climate.h
inline float readHumidityValue() {
  return NAN;
}