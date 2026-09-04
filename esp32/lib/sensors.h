#ifndef SENSORS_H
#define SENSORS_H

#include "../config.h"

#if defined(SENSOR_DHT11) || defined(SENSOR_DHT22) || defined(SENSOR_BMP280) || \
    defined(SENSOR_BME280) || defined(SENSOR_SHT31) || defined(SENSOR_AHT20) || \
    defined(SENSOR_DS18B20)
  #include "sensors/sensor_climate.h"
#endif

#if defined(SENSOR_PIR_HCSR501) || defined(SENSOR_RADAR_RCWL) || \
    defined(SENSOR_MMWAVE_LD2410) || defined(SENSOR_IR_TE174)
  #include "sensors/sensor_motion_presence.h"
#endif

#if defined(SENSOR_ULTRASONIC_HCSR04) || defined(SENSOR_ULTRASONIC_US100) || \
    defined(SENSOR_LASER_VL53L0X)
  #include "sensors/sensor_distance.h"
#endif

#if defined(SENSOR_LIGHT_BH1750) || defined(SENSOR_LIGHT_LDR) || \
    defined(SENSOR_COLOR_TCS34725) || defined(SENSOR_GAS_MQ135)
  #include "sensors/sensor_ambient.h"
#endif

inline void initSensors() {
  #if defined(SENSOR_DHT11) || defined(SENSOR_DHT22) || defined(SENSOR_BMP280) || \
      defined(SENSOR_BME280) || defined(SENSOR_SHT31) || defined(SENSOR_AHT20) || \
      defined(SENSOR_DS18B20)
    initClimate();
  #endif

  #if defined(SENSOR_PIR_HCSR501) || defined(SENSOR_RADAR_RCWL) || \
      defined(SENSOR_MMWAVE_LD2410) || defined(SENSOR_IR_TE174)
    initMotion();
  #endif

  #if defined(SENSOR_ULTRASONIC_HCSR04) || defined(SENSOR_ULTRASONIC_US100) || \
      defined(SENSOR_LASER_VL53L0X)
    initDistance();
  #endif

  #if defined(SENSOR_LIGHT_BH1750) || defined(SENSOR_LIGHT_LDR) || \
      defined(SENSOR_COLOR_TCS34725) || defined(SENSOR_GAS_MQ135)
    initAmbient();
  #endif
}

// ======================================================
// UNIFIED SENSOR INTERFACES FOR ALARM SYSTEM
// ======================================================

inline bool checkMotionTriggered() {
  #if defined(SENSOR_PIR_HCSR501) || defined(SENSOR_RADAR_RCWL) || defined(SENSOR_IR_TE174) || defined(SENSOR_MMWAVE_LD2410)
    return isMotionDetected();
  #else
    return false;
  #endif
}

inline bool checkDistanceTriggered() {
  #if defined(SENSOR_ULTRASONIC_HCSR04) || defined(SENSOR_ULTRASONIC_US100) || defined(SENSOR_LASER_VL53L0X)
    float distance = readDistanceCM();
    return (distance > 0 && distance < 200.0);
  #else
    return false;
  #endif
}

#endif