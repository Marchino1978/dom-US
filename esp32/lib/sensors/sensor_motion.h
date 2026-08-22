#ifndef SENSOR_MOTION_H
#define SENSOR_MOTION_H

#include "../config.h"

inline void initMotion() {
  #if defined(SENSOR_PIR_HCSR501) || defined(SENSOR_RADAR_RCWL)
    pinMode(PIN_MOTION, INPUT);
  #elif defined(SENSOR_IR_TE174)
    pinMode(PIN_MOTION, INPUT_PULLUP);
  #elif defined(SENSOR_MMWAVE_LD2410)
  #endif
}

inline bool isMotionDetected() {
  #if defined(SENSOR_PIR_HCSR501) || defined(SENSOR_RADAR_RCWL)
    return (digitalRead(PIN_MOTION) == HIGH);

  #elif defined(SENSOR_IR_TE174)
    return (digitalRead(PIN_MOTION) == LOW);

  #elif defined(SENSOR_MMWAVE_LD2410)
    return false; 

  #else
    return false;
  #endif
}

#endif