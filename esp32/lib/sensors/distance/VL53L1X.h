#pragma once

#include "../../../config.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L1X.h>

#define VL53L1X_I2C_ADDRESS        0x29
#define VL53L1X_TIMING_BUDGET_MS   50
#define VL53L1X_MAX_CM             400.0f
 
static Adafruit_VL53L1X vl53 = Adafruit_VL53L1X();
static bool vl53Ready = false;
static float vl53Distance = NAN;
 
inline void initDistanceHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  vl53Ready = vl53.begin(VL53L1X_I2C_ADDRESS, &Wire);
  if (!vl53Ready) {
    Serial.println("VL53L1X NOT FOUND");
    return;
  }
  vl53.startRanging();
  vl53.setTimingBudget(VL53L1X_TIMING_BUDGET_MS);
}
 
// Non-blocking: returns the last completed measurement in cm
inline float readDistanceValue() {
  if (!vl53Ready) return NAN;
  if (vl53.dataReady()) {
    int16_t mm = vl53.distance();
    vl53.clearInterrupt();
    vl53Distance = (mm < 0 || mm / 10.0f > VL53L1X_MAX_CM) ? NAN : mm / 10.0f;
  }
  return vl53Distance;
}