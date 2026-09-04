#ifndef SENSOR_MOTION_PRESENCE_H
#define SENSOR_MOTION_PRESENCE_H

#include "../../config.h"

#if defined(SENSOR_PIR_AM312)
  #include "motion_presence/AM312.h"
#elif defined(SENSOR_PIR_HCSR501)
  #include "motion_presence/HCSR501.h"
#elif defined(SENSOR_RADAR_RCWL0516)
  #include "motion_presence/RCWL-0516.h"
#elif defined(SENSOR_IR_TE174)
  #include "motion_presence/TE174.h"
#elif defined(SENSOR_MMWAVE_LD2410)
  #include "motion_presence/LD2410.h"
#endif

inline void initMotion() {
  #if defined(SENSOR_PIR_AM312) || defined(SENSOR_PIR_HCSR501) || defined(SENSOR_RADAR_RCWL0516) || defined(SENSOR_IR_TE174) || defined(SENSOR_MMWAVE_LD2410)
    initMotionHardware();
  #endif
}

inline bool isMotionDetected() {
  #if defined(SENSOR_PIR_AM312) || defined(SENSOR_PIR_HCSR501) || defined(SENSOR_RADAR_RCWL0516) || defined(SENSOR_IR_TE174) || defined(SENSOR_MMWAVE_LD2410)
    return readMotionState();
  #else
    return false;
  #endif
}

#endif