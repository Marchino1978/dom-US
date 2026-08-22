#ifndef DISPLAY_ADDONS_H
#define DISPLAY_ADDONS_H

#include "../config.h"

#define LCD_BL 22
#define DISPLAY_TIMEOUT_MS 30000

static unsigned long displayTimerStart = 0;
static bool screenActive = false;

#if defined(ADDON_LASER_VL53L0X)
  #include <Adafruit_VL53L0X.h>
  static Adafruit_VL53L0X loxAddon = Adafruit_VL53L0X();
#endif

inline void setDisplayPower(bool state) {
  screenActive = state;
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, state ? HIGH : LOW);
}

inline void triggerDisplayWake() {
  displayTimerStart = millis();
  if (!screenActive) {
    setDisplayPower(true);
  }
}

inline void initDisplayAddons() {
  pinMode(LCD_BL, OUTPUT);
  setDisplayPower(true);
  displayTimerStart = millis();

  #if defined(ADDON_LASER_VL53L0X)
    loxAddon.begin();
  #endif
}

inline void handleDisplayAutoWake() {
  #if defined(ADDON_LASER_VL53L0X)
    VL53L0X_RangingMeasurementData_t measure;
    loxAddon.rangingTest(&measure, false);
    
    if (measure.RangeStatus != 4 && (measure.RangeMilliMeter / 10.0) < 15.0) {
      triggerDisplayWake();
    }
  #endif

  if (screenActive && (millis() - displayTimerStart > DISPLAY_TIMEOUT_MS)) {
    setDisplayPower(false);
  }
}

#endif