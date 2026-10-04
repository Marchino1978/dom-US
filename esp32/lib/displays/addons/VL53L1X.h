#pragma once

#include "../../../config.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L1X.h>

#define VL53L1X_I2C_ADDRESS        0x29
#define VL53L1X_TIMING_BUDGET_MS   50
#define VL53L1X_MAX_CM             400.0f

#ifndef ADDON_WAKE_DISTANCE_CM
  #define ADDON_WAKE_DISTANCE_CM   5.0f
#endif
#ifndef ADDON_ALARM_DISTANCE_CM
  #define ADDON_ALARM_DISTANCE_CM  VL53L1X_MAX_CM
#endif
 
static Adafruit_VL53L1X vl53 = Adafruit_VL53L1X();
static bool vl53Ready = false;
static float vl53Distance = NAN;
 
inline void initAddonHardware() {
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
inline float addonReadDistance() {
  if (!vl53Ready) return NAN;
  if (vl53.dataReady()) {
    int16_t mm = vl53.distance();
    vl53.clearInterrupt();
    vl53Distance = (mm < 0 || mm / 10.0f > VL53L1X_MAX_CM) ? NAN : mm / 10.0f;
  }
  return vl53Distance;
}
 
// Used by handleDisplayAutoWake() while the alarm is disarmed (short range)
inline bool checkAddonDisplayLogic() {
  float d = addonReadDistance();
  return !isnan(d) && d < ADDON_WAKE_DISTANCE_CM;
}
 
// Used by checkAlarmSystem() while the alarm is armed (long range)
inline bool checkAddonAlarmLogic() {
  float d = addonReadDistance();
  return !isnan(d) && d < ADDON_ALARM_DISTANCE_CM;
}