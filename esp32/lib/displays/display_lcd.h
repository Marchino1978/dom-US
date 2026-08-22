#ifndef DISPLAY_LCD_H
#define DISPLAY_LCD_H

#include "../config.h"
#include <LiquidCrystal_I2C.h>

#if defined(DISPLAY_LCD_20X4)
  LiquidCrystal_I2C lcd(0x27, 20, 4);
#else
  LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif

inline void initLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

inline void printLCD(const String& line1, const String& line2 = "") {
  lcd.setCursor(0, 0);
  lcd.print(line1 + "                ");
  
  lcd.setCursor(0, 1);
  lcd.print(line2 + "                ");
}

inline void setLCDBacklight(bool state) {
  if (state) lcd.backlight();
  else lcd.noBacklight();
}

#endif