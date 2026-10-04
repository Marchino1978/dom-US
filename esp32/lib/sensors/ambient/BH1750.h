#pragma once

#include "../../../config.h"

#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>

#define BH1750_I2C_ADDRESS_PRIMARY   0x23
#define BH1750_I2C_ADDRESS_SECONDARY 0x5C
 
static BH1750 lightMeter;
static bool bh1750Ready = false;
 
inline void initAmbientHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  bh1750Ready = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, BH1750_I2C_ADDRESS_PRIMARY, &Wire);
  if (!bh1750Ready) {
    bh1750Ready = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, BH1750_I2C_ADDRESS_SECONDARY, &Wire);
  }
  if (!bh1750Ready) {
    Serial.println("BH1750 NOT FOUND");
  }
}
 
inline float readAmbientLuxValue() {
  if (!bh1750Ready) return NAN;
  float lux = lightMeter.readLightLevel();
  return (lux < 0) ? NAN : lux;
}
 