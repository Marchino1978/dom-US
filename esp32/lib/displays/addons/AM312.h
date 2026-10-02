#pragma once

#include "../../../config.h"

#include <Arduino.h>

inline void initAddonHardware() {
  pinMode(PIN_MOTION, INPUT);
}
 
// Used by handleDisplayAutoWake() while the alarm is disarmed
inline bool checkAddonDisplayLogic() {
  return digitalRead(PIN_MOTION) == HIGH;
}
 
// Used by checkAlarmSystem() while the alarm is armed
inline bool checkAddonAlarmLogic() {
  return digitalRead(PIN_MOTION) == HIGH;
}