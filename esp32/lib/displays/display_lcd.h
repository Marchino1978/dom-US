#pragma once

#include "../../config.h"
 
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
  #if defined(DISPLAY_LCD_16X2) || \
      defined(DISPLAY_LCD_20X4) || \
      defined(DISPLAY_LCD_16X2_RGB) || \
      defined(DISPLAY_LCD_20X4_RGB)
    initLCDHardware();
  #endif
}

// Writes one full row, padding with spaces and truncating to LCD_COLS
inline void printLCDRow(uint8_t row, const String& text) {
  #if defined(DISPLAY_LCD_16X2) || \
      defined(DISPLAY_LCD_20X4) || \
      defined(DISPLAY_LCD_16X2_RGB) || \
      defined(DISPLAY_LCD_20X4_RGB)
    if (row >= LCD_ROWS) return;

    char buf[LCD_COLS + 1];
    uint8_t i = 0;
    for (; i < LCD_COLS && i < text.length(); i++) buf[i] = text[i];
    for (; i < LCD_COLS; i++) buf[i] = ' ';
    buf[LCD_COLS] = '\0';

    lcd.setCursor(0, row);
    lcd.print(buf);
  #endif
}
 
inline void printLCD(const String& line1, const String& line2 = "") {
  printLCDRow(0, line1);
  printLCDRow(1, line2);
}
 
inline void setLCDBacklight(bool state) {
  #if defined(DISPLAY_LCD_16X2) || \
      defined(DISPLAY_LCD_20X4) || \
      defined(DISPLAY_LCD_16X2_RGB) || \
      defined(DISPLAY_LCD_20X4_RGB)
    if (state) {
      lcd.display();
      setLCDBacklightValue(true);
    } else {
      setLCDBacklightValue(false);
      lcd.noDisplay();
    }
  #endif
}