#pragma once
#include "../config.h"

#if defined(DISPLAY_OLED_SSD1306)
  #include "oled/SSD1306.h"
#elif defined(DISPLAY_OLED_SH1106)
  #include "oled/SH1106.h"
#endif

inline void initOLED() {
  #if defined(DISPLAY_OLED_SSD1306) || defined(DISPLAY_OLED_SH1106)
    initOLEDHardware();
  #endif
}

inline void printOLED(const String& line1, const String& line2 = "") {
  #if defined(DISPLAY_OLED_SSD1306) || defined(DISPLAY_OLED_SH1106)
    printOLEDValue(line1, line2);
  #endif
}

inline void setOLEDPower(bool state) {
  #if defined(DISPLAY_OLED_SSD1306) || defined(DISPLAY_OLED_SH1106)
    setOLEDPowerValue(state);
  #endif
}