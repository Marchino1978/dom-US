#pragma once

#include "../../config.h"

// Not a valid GPIO on ESP32-C3: check and reassign before use, only needed by displays driven by a backlight pin
#define LCD_BL 22
#define DISPLAY_TIMEOUT_MS 30000

static unsigned long displayTimerStart = 0;
static bool screenActive = false;

#if defined(ADDON_RADAR_RCWL0516)
  #include "addons/RCWL-0516.h"
#elif defined(ADDON_LASER_VL53L0X)
  #include "addons/VL53L0X.h"
#elif defined(ADDON_LASER_VL53L1X)
  #include "addons/VL53L1X.h"
#endif

// Defined in display.h, after the display driver is included
inline void applyDisplayPower(bool state);

inline void setDisplayPower(bool state) {
  screenActive = state;
  applyDisplayPower(state);
}

inline bool isDisplayActive() {
  return screenActive;
}

inline void triggerDisplayWake() {
  displayTimerStart = millis();
  if (!screenActive) {
    setDisplayPower(true);
  }
}

inline void initDisplayAddons() {
  setDisplayPower(true);
  displayTimerStart = millis();

  #if defined(ADDON_RADAR_RCWL0516) || \
      defined(ADDON_LASER_VL53L0X) || \
      defined(ADDON_LASER_VL53L1X)
    initAddonHardware();
  #endif
}

inline bool checkAddonDisplayTriggered() {
  #if defined(ADDON_RADAR_RCWL0516) || \
      defined(ADDON_LASER_VL53L0X) || \
      defined(ADDON_LASER_VL53L1X)
    return checkAddonDisplayLogic();
  #else
    return false;
  #endif
}

inline bool checkAddonAlarmTriggered() {
  #if defined(ADDON_RADAR_RCWL0516) || \
      defined(ADDON_LASER_VL53L0X) || \
      defined(ADDON_LASER_VL53L1X)
    return checkAddonAlarmLogic();
  #else
    return false;
  #endif
}

inline void handleDisplayAutoWake() {
  #ifdef HAS_WAKEUP_ADDON
    if (checkAddonDisplayTriggered()) {
      triggerDisplayWake();
    }

    if (screenActive && (millis() - displayTimerStart > DISPLAY_TIMEOUT_MS)) {
      setDisplayPower(false);
    }
  #else
    if (!screenActive) triggerDisplayWake();
  #endif
}