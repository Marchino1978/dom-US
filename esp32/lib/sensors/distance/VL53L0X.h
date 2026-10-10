#pragma once

#include "../../../config.h"
#include "../../debug.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

#define VL53L0X_I2C_ADDRESS  0x29
#define VL53L0X_MAX_CM       200.0f
 
static Adafruit_VL53L0X lox;
static bool loxReady = false;
static float loxDistance = NAN;
 
inline void initDistanceHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  loxReady = lox.begin(VL53L0X_I2C_ADDRESS, false, &Wire);
  if (!loxReady) {
    DEBUG_LOG("VL53L0X NOT FOUND");
    return;
  }
  lox.startRangeContinuous();
}
 
// Non-blocking: returns the last completed measurement in cm
inline float readDistanceValue() {
  if (!loxReady) return NAN;
  if (lox.isRangeComplete()) {
    uint16_t mm = lox.readRangeResult();
    loxDistance = (mm == 0xFFFF || mm / 10.0f > VL53L0X_MAX_CM) ? NAN : mm / 10.0f;
  }
  return loxDistance;
}