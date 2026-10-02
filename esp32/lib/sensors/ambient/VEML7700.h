#pragma once

#include "../../../config.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VEML7700.h>

static Adafruit_VEML7700 veml;
static bool vemlReady = false;
 
inline void initAmbientHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  vemlReady = veml.begin(&Wire);
  if (!vemlReady) {
    Serial.println("VEML7700 NOT FOUND");
  }
}
 
// Auto-ranging may block for a few hundred ms in the worst case
inline float readAmbientLuxValue() {
  if (!vemlReady) return NAN;
  return veml.readLux(VEML_LUX_AUTO);
}