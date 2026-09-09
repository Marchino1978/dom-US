#pragma once

#include "../../../config.h"
#include <Arduino.h>
#include <Wire.h>
#include <DFRobot_RGBLCD1602.h>

DFRobot_RGBLCD1602 lcd(20, 4);

inline void initLCDHardware() {
  lcd.init();
  lcd.setPWM(255, 255, 255);
  lcd.clear();
}

inline void printLCDValue(const String& line1, const String& line2 = "") {
  lcd.setCursor(0, 0);
  lcd.print(line1 + "                ");
  lcd.setCursor(0, 1);
  lcd.print(line2 + "                ");
}

inline void setLCDBacklightValue(bool state) {
  if (state) {
    lcd.setPWM(255, 255, 255);
  } else {
    lcd.setPWM(0, 0, 0);
  }
}