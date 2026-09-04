#ifndef DISPLAY_ADDONS_H
#define DISPLAY_ADDONS_H

#include "../config.h"

#define LCD_BL 22
#define DISPLAY_TIMEOUT_MS 30000

static unsigned long displayTimerStart = 0;
static bool screenActive = false;

#if defined(ADDON_PIR_AM312)
  #include "displays/addons/AM312.h"
#elif defined(ADDON_RADAR_RCWL0516)
  #include "displays/addons/RCWL-0516.h"
#elif defined(ADDON_LASER_VL53L0X)
  #include "displays/addons/VL53L0X.h"
#elif defined(ADDON_LASER_VL53L1X)
  #include "displays/addons/VL53L1X.h"
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

  #if defined(ADDON_PIR_AM312) || defined(ADDON_RADAR_RCWL0516) || defined(ADDON_LASER_VL53L0X) || defined(ADDON_LASER_VL53L1X)
    initAddonHardware();
  #endif
}

inline bool checkAddonDisplayTriggered() {
  #if defined(ADDON_PIR_AM312) || defined(ADDON_RADAR_RCWL0516) || defined(ADDON_LASER_VL53L0X) || defined(ADDON_LASER_VL53L1X)
    return checkAddonDisplayLogic();
  #else
    return false;
  #endif
}

inline bool checkAddonAlarmTriggered() {
  #if defined(ADDON_PIR_AM312) || defined(ADDON_RADAR_RCWL0516) || defined(ADDON_LASER_VL53L0X) || defined(ADDON_LASER_VL53L1X)
    return checkAddonAlarmLogic();
  #else
    return false;
  #endif
}

inline void handleDisplayAutoWake() {
  if (checkAddonDisplayTriggered()) {
    triggerDisplayWake();
  }

  if (screenActive && (millis() - displayTimerStart > DISPLAY_TIMEOUT_MS)) {
    setDisplayPower(false);
  }
}

#endif