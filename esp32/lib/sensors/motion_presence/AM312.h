#pragma once

#include "../../../config.h"

#include <Arduino.h>

inline void initMotionHardware() {
  pinMode(PIN_MOTION, INPUT);
}
 
// Output is active HIGH while motion is detected
inline bool readMotionState() {
  return digitalRead(PIN_MOTION) == HIGH;
}