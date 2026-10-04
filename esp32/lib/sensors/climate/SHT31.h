#pragma once

#include "../../../config.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>

#define SHT31_I2C_ADDRESS_PRIMARY   0x44
#define SHT31_I2C_ADDRESS_SECONDARY 0x45
 
static Adafruit_SHT31 sht31 = Adafruit_SHT31(&Wire);
static bool sht31Ready = false;
 
inline void initClimateHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  sht31Ready = sht31.begin(SHT31_I2C_ADDRESS_PRIMARY);
  if (!sht31Ready) {
    sht31Ready = sht31.begin(SHT31_I2C_ADDRESS_SECONDARY);
  }
  if (!sht31Ready) {
    Serial.println("SHT31 NOT FOUND");
  }
}
 
inline float readTemperatureValue() {
  if (!sht31Ready) return NAN;
  return sht31.readTemperature();
}
 
inline float readHumidityValue() {
  if (!sht31Ready) return NAN;
  return sht31.readHumidity();
}