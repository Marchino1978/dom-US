#pragma once
 
#include "../../../config.h"
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
 
LiquidCrystal_I2C lcd(0x27, 20, 4);
 
inline void initLCDHardware() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}
 
inline void printLCDValue(const String& line1, const String& line2 = "") {
  lcd.setCursor(0, 0);
  lcd.print(line1 + "                ");
  lcd.setCursor(0, 1);
  lcd.print(line2 + "                ");
}
 
inline void setLCDBacklightValue(bool state) {
  if (state) lcd.backlight();
  else lcd.noBacklight();
}
 