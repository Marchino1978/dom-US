#ifndef DISPLAY_ADDONS_H
#define DISPLAY_ADDONS_H

#include "../config.h"

#if defined(ADDON_IR_TE174)
  #define PIN_ADDON_IR 10
#elif defined(ADDON_PIR_HCSR501)
  #define PIN_ADDON_PIR 11
#elif defined(ADDON_LASER_VL53L0X)
  #include <Adafruit_VL53L0X.h>
  static Adafruit_VL53L0X loxAddon = Adafruit_VL53L0X();
#endif

inline void initDisplayAddons() {
  #if defined(ADDON_IR_TE174)
    pinMode(PIN_ADDON_IR, INPUT_PULLUP);
  #elif defined(ADDON_PIR_HCSR501)
    pinMode(PIN_ADDON_PIR, INPUT);
  #elif defined(ADDON_LASER_VL53L0X)
    loxAddon.begin();
  #endif
}

inline void handleDisplayAutoWake() {
  #if defined(ADDON_IR_TE174) || defined(ADDON_LASER_VL53L0X) || defined(ADDON_PIR_HCSR501)
    static unsigned long lastActivityTime = 0;
    static bool screenActive = true;
    bool motionDetected = false;

    #if defined(ADDON_IR_TE174)
      motionDetected = (digitalRead(PIN_ADDON_IR) == LOW);
    #elif defined(ADDON_PIR_HCSR501)
      motionDetected = (digitalRead(PIN_ADDON_PIR) == HIGH);
    #elif defined(ADDON_LASER_VL53L0X)
      VL53L0X_RangingMeasurementData_t measure;
      loxAddon.rangingTest(&measure, false);
      if (measure.RangeStatus != 4 && (measure.RangeMilliMeter / 10.0) < 15.0) {
        motionDetected = true;
      }
    #endif

    if (motionDetected) {
      lastActivityTime = millis();
      if (!screenActive) {
        screenActive = true;
      }
    }

    if (screenActive && (millis() - lastActivityTime > 30000)) {
      screenActive = false;
    }
  #endif
}

#endif