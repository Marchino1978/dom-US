#pragma once
#include "../config.h"
 
#if defined(DISPLAY_LCD_16X2)
  #include "lcd/16x2.h"
#elif defined(DISPLAY_LCD_20X4)
  #include "lcd/20x4.h"
#elif defined(DISPLAY_LCD_16X2_RGB)
  #include "lcd/16x2_RGB.h"
#elif defined(DISPLAY_LCD_20X4_RGB)
  #include "lcd/20x4_RGB.h"
#endif
 
inline void initLCD() {
  #if defined(DISPLAY_LCD_16X2) || defined(DISPLAY_LCD_20X4) || \
      defined(DISPLAY_LCD_16X2_RGB) || defined(DISPLAY_LCD_20X4_RGB)
    initLCDHardware();
  #endif
}
 
inline void printLCD(const String& line1, const String& line2 = "") {
  #if defined(DISPLAY_LCD_16X2) || defined(DISPLAY_LCD_20X4) || \
      defined(DISPLAY_LCD_16X2_RGB) || defined(DISPLAY_LCD_20X4_RGB)
    printLCDValue(line1, line2);
  #endif
}
 
inline void setLCDBacklight(bool state) {
  #if defined(DISPLAY_LCD_16X2) || defined(DISPLAY_LCD_20X4) || \
      defined(DISPLAY_LCD_16X2_RGB) || defined(DISPLAY_LCD_20X4_RGB)
    setLCDBacklightValue(state);
  #endif
}