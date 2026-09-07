#pragma once
#include "../config.h"

#if defined(DISPLAY_LCD_16X2_RGB) || defined(DISPLAY_LCD_20X4_RGB)
#include <DFRobot_RGBLCD1602.h>
#if defined(DISPLAY_LCD_20X4_RGB)
DFRobot_RGBLCD1602 lcd(20, 4);
#else
DFRobot_RGBLCD1602 lcd(16, 2);
#endif
#else
#include <LiquidCrystal_I2C.h>
#if defined(DISPLAY_LCD_20X4)
LiquidCrystal_I2C lcd(0x27, 20, 4);
#else
LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif
#endif

inline void initLCD() {
#if defined(DISPLAY_LCD_16X2_RGB) || defined(DISPLAY_LCD_20X4_RGB)
    lcd.init();
    lcd.setPWM(255, 255, 255);
#else
    lcd.init();
    lcd.backlight();
#endif
    lcd.clear();
}

inline void printLCD(const String& line1, const String& line2 = "") {
    lcd.setCursor(0, 0);
    lcd.print(line1 + "                ");
    lcd.setCursor(0, 1);
    lcd.print(line2 + "                ");
}

inline void setLCDBacklight(bool state) {
#if defined(DISPLAY_LCD_16X2_RGB) || defined(DISPLAY_LCD_20X4_RGB)
    if (state) {
        lcd.setPWM(255, 255, 255);
    } else {
        lcd.setPWM(0, 0, 0);
    }
#else
    if (state) lcd.backlight();
    else lcd.noBacklight();
#endif
}