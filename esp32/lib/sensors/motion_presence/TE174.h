#pragma once

#include "../../../config.h"

#include <Arduino.h>

// ======================================================
// Write your code and declarations below this line
// ======================================================

#define TE174_DEBOUNCE_MS 50
 
static unsigned long te174LastChange = 0;
static bool te174RawState = false;
static bool te174StableState = false;
 
inline void initMotionHardware() {
  pinMode(PIN_OBSTACLE, INPUT_PULLUP);
  te174RawState = (digitalRead(PIN_OBSTACLE) == LOW);
  te174StableState = te174RawState;
  te174LastChange = millis();
}
 
// Output is active LOW: LOW means beam interrupted (obstacle detected)
inline bool readMotionState() {
  bool raw = (digitalRead(PIN_OBSTACLE) == LOW);
 
  if (raw != te174RawState) {
    te174RawState = raw;
    te174LastChange = millis();
  }
 
  if (millis() - te174LastChange >= TE174_DEBOUNCE_MS) {
    te174StableState = te174RawState;
  }
 
  return te174StableState;
}