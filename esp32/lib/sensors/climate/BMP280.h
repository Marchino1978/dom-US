#pragma once

#include "../../../config.h"
#include "../../debug.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define BMP280_I2C_ADDRESS 0x77
 
static Adafruit_BMP280 bmp(&Wire);
static bool bmpReady = false;
 
inline void initClimateHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  bmpReady = bmp.begin(BMP280_I2C_ADDRESS);
  if (!bmpReady) {
    DEBUG_LOG("BMP280 NOT FOUND");
    return;
  }
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_OFF,
                  Adafruit_BMP280::STANDBY_MS_1);
}
 
inline float readTemperatureValue() {
  if (!bmpReady) return NAN;
  if (!bmp.takeForcedMeasurement()) return NAN;
  return bmp.readTemperature();
}
 
// BMP280 has no humidity channel; required by sensor_climate.h
inline float readHumidityValue() {
  return NAN;
}
 
// Station pressure (Pa to hPa) reduced to sea level with the Bosch barometric formula
inline float readPressureValue() {
  if (!bmpReady) return NAN;
  if (!bmp.takeForcedMeasurement()) return NAN;
  float stationHpa = bmp.readPressure() / 100.0f;
  return stationHpa / powf(1.0f - ((float)SENSOR_ALTITUDE / 44330.0f), 5.255f);
}