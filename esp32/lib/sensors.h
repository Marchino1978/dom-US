#pragma once

#include "../config.h"
#include "checks.h"

#ifdef MODULE_TELEMETRY_ACTIVE
  #include "sensors/sensor_climate.h"
#endif

#if defined(SENSOR_PIR_AM312) || defined(SENSOR_PIR_HCSR501) || \
    defined(SENSOR_RADAR_RCWL0516) || defined(SENSOR_MMWAVE_LD2410) || \
    defined(SENSOR_IR_TE174)
  #include "sensors/sensor_motion_presence.h"
#endif

#if defined(SENSOR_ULTRASONIC_HCSR04) || defined(SENSOR_ULTRASONIC_HCSR04P) || \
    defined(SENSOR_ULTRASONIC_RCWL1601) || defined(SENSOR_ULTRASONIC_US100) || \
    defined(SENSOR_LASER_VL53L0X) || defined(SENSOR_LASER_VL53L1X)
  #include "sensors/sensor_distance.h"
#endif

#if defined(SENSOR_LIGHT_BH1750) || defined(SENSOR_LIGHT_LDR) || \
    defined(SENSOR_COLOR_TCS34725) || defined(SENSOR_GAS_MQ135) || \
    defined(SENSOR_LIGHT_VEML7700)
  #include "sensors/sensor_ambient.h"
#endif

inline void initSensors() {
  #ifdef MODULE_TELEMETRY_ACTIVE
    initClimate();
  #endif

  #if defined(SENSOR_PIR_AM312) || defined(SENSOR_PIR_HCSR501) || \
      defined(SENSOR_RADAR_RCWL0516) || defined(SENSOR_MMWAVE_LD2410) || \
      defined(SENSOR_IR_TE174)
    initMotion();
  #endif

  #if defined(SENSOR_ULTRASONIC_HCSR04) || defined(SENSOR_ULTRASONIC_HCSR04P) || \
      defined(SENSOR_ULTRASONIC_RCWL1601) || defined(SENSOR_ULTRASONIC_US100) || \
      defined(SENSOR_LASER_VL53L0X) || defined(SENSOR_LASER_VL53L1X)
    initDistance();
  #endif

  #if defined(SENSOR_LIGHT_BH1750) || defined(SENSOR_LIGHT_LDR) || \
      defined(SENSOR_COLOR_TCS34725) || defined(SENSOR_GAS_MQ135) || \
      defined(SENSOR_LIGHT_VEML7700)
    initAmbient();
  #endif
}

// ======================================================
// UNIFIED SENSOR INTERFACES FOR ALARM SYSTEM
// ======================================================

inline bool checkMotionTriggered() {
  #if defined(SENSOR_PIR_AM312) || defined(SENSOR_PIR_HCSR501) || \
      defined(SENSOR_RADAR_RCWL0516) || defined(SENSOR_IR_TE174) || \
      defined(SENSOR_MMWAVE_LD2410)
    return isMotionDetected();
  #else
    return false;
  #endif
}

inline bool checkDistanceTriggered() {
  #if defined(SENSOR_ULTRASONIC_HCSR04) || defined(SENSOR_ULTRASONIC_HCSR04P) || \
      defined(SENSOR_ULTRASONIC_RCWL1601) || defined(SENSOR_ULTRASONIC_US100) || \
      defined(SENSOR_LASER_VL53L0X) || defined(SENSOR_LASER_VL53L1X)
    float distance = readDistanceCM();
    return (distance > 0 && distance < 200.0);
  #else
    return false;
  #endif
}