#pragma once

#include "../../../config.h"
#include "../../debug.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

#define BME280_I2C_ADDRESS_PRIMARY   0x77
#define BME280_I2C_ADDRESS_SECONDARY 0x76
 
static Adafruit_BME280 bme;
static bool bmeReady = false;
 
inline void initClimateHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  bmeReady = bme.begin(BME280_I2C_ADDRESS_PRIMARY, &Wire);
  if (!bmeReady) {
    bmeReady = bme.begin(BME280_I2C_ADDRESS_SECONDARY, &Wire);
  }
  if (!bmeReady) {
    DEBUG_LOG("BME280 NOT FOUND");
    return;
  }
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X2,
                  Adafruit_BME280::SAMPLING_X16,
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::FILTER_OFF);
}
 
inline float readTemperatureValue() {
  if (!bmeReady) return NAN;
  if (!bme.takeForcedMeasurement()) return NAN;
  return bme.readTemperature();
}
 
inline float readHumidityValue() {
  if (!bmeReady) return NAN;
  if (!bme.takeForcedMeasurement()) return NAN;
  return bme.readHumidity();
}
 
// Station pressure (Pa to hPa) reduced to sea level with the Bosch barometric formula
inline float readPressureValue() {
  if (!bmeReady) return NAN;
  if (!bme.takeForcedMeasurement()) return NAN;
  float stationHpa = bme.readPressure() / 100.0f;
  return stationHpa / powf(1.0f - ((float)SENSOR_ALTITUDE / 44330.0f), 5.255f);
}