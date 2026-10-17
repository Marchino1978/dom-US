#pragma once

#include "../../../config.h"

#include <Arduino.h>
#include <Wire.h>
#include <DFRobot_RGBLCD1602.h>

#define LCD_COLS 16
#define LCD_ROWS 2

DFRobot_RGBLCD1602 lcd(LCD_COLS, LCD_ROWS);

inline void initLCDHardware() {
  lcd.init();
  lcd.setPWM(255, 255, 255);
  lcd.clear();
}

inline void setLCDBacklightValue(bool state) {
  if (state) {
    lcd.setPWM(255, 255, 255);
  } else {
    lcd.setPWM(0, 0, 0);
  }
}