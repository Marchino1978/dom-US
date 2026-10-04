#pragma once

#include "../../../config.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

#define VL53L0X_I2C_ADDRESS  0x29
#define VL53L0X_MAX_CM       200.0f

#ifndef ADDON_WAKE_DISTANCE_CM
  #define ADDON_WAKE_DISTANCE_CM   5.0f
#endif
#ifndef ADDON_ALARM_DISTANCE_CM
  #define ADDON_ALARM_DISTANCE_CM  VL53L0X_MAX_CM
#endif
 
static Adafruit_VL53L0X lox;
static bool loxReady = false;
static float loxDistance = NAN;
 
inline void initAddonHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  loxReady = lox.begin(VL53L0X_I2C_ADDRESS, false, &Wire);
  if (!loxReady) {
    Serial.println("VL53L0X NOT FOUND");
    return;
  }
  lox.startRangeContinuous();
}
 
// Non-blocking: returns the last completed measurement in cm
inline float addonReadDistance() {
  if (!loxReady) return NAN;
  if (lox.isRangeComplete()) {
    uint16_t mm = lox.readRangeResult();
    loxDistance = (mm == 0xFFFF || mm / 10.0f > VL53L0X_MAX_CM) ? NAN : mm / 10.0f;
  }
  return loxDistance;
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