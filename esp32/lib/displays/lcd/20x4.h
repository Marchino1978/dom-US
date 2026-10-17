#pragma once
 
#include "../../../config.h"

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_COLS 20
#define LCD_ROWS 4
 
LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS);
 
inline void initLCDHardware() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}
 
inline void setLCDBacklightValue(bool state) {
  if (state) lcd.backlight();
  else lcd.noBacklight();
}