#ifndef SENSOR_DISTANCE_H
#define SENSOR_DISTANCE_H

#include "../../config.h"

#if defined(SENSOR_ULTRASONIC_HCSR04)
  #include "distance/HCSR04.h"
#elif defined(SENSOR_ULTRASONIC_HCSR04P)
  #include "distance/HCSR04P.h"
#elif defined(SENSOR_ULTRASONIC_RCWL1601)
  #include "distance/RCWL-1601.h"
#elif defined(SENSOR_ULTRASONIC_US100)
  #include "distance/US100.h"
#elif defined(SENSOR_LASER_VL53L0X)
  #include "distance/VL53L0X.h"
#elif defined(SENSOR_LASER_VL53L1X)
  #include "distance/VL53L1X.h"
#endif

inline void initDistance() {
  #if defined(SENSOR_ULTRASONIC_HCSR04) || defined(SENSOR_ULTRASONIC_HCSR04P) || defined(SENSOR_ULTRASONIC_RCWL1601) || defined(SENSOR_ULTRASONIC_US100) || defined(SENSOR_LASER_VL53L0X) || defined(SENSOR_LASER_VL53L1X)
    initDistanceHardware();
  #endif
}

inline float readDistanceCM() {
  #if defined(SENSOR_ULTRASONIC_HCSR04) || defined(SENSOR_ULTRASONIC_HCSR04P) || defined(SENSOR_ULTRASONIC_RCWL1601) || defined(SENSOR_ULTRASONIC_US100) || defined(SENSOR_LASER_VL53L0X) || defined(SENSOR_LASER_VL53L1X)
    return readDistanceValue();
  #else
    return -1.0;
  #endif
}

inline float readUS100Temperature() {
  #if defined(SENSOR_ULTRASONIC_US100)
    return readUS100TemperatureValue();
  #else
    return -99.0;
  #endif
}

#endif