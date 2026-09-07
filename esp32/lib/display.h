#pragma once
#include "../config.h"

#if defined(DISPLAY_LCD_16X2) || defined(DISPLAY_LCD_20X4) || defined(DISPLAY_LCD_16X2_RGB) || defined(DISPLAY_LCD_20X4_RGB)
#include "displays/display_lcd.h"
#elif defined(DISPLAY_OLED_SSD1306) || defined(DISPLAY_OLED_SH1106)
#include "displays/display_oled.h"
#elif defined(DISPLAY_TFT_ST7789) || defined(DISPLAY_TFT_ILI9341)
#include "displays/display_tft.h"
#endif

#include "displays/display_addons.h"

inline void initDisplay() {
#if defined(DISPLAY_LCD_16X2) || defined(DISPLAY_LCD_20X4) || defined(DISPLAY_LCD_16X2_RGB) || defined(DISPLAY_LCD_20X4_RGB)
    initLCD();
#elif defined(DISPLAY_OLED_SSD1306) || defined(DISPLAY_OLED_SH1106)
    initOLED();
#elif defined(DISPLAY_TFT_ST7789) || defined(DISPLAY_TFT_ILI9341)
    initTFT();
#endif
    initDisplayAddons();
}

inline void showMessage(const String& line1, const String& line2 = "") {
#if defined(DISPLAY_LCD_16X2) || defined(DISPLAY_LCD_20X4) || defined(DISPLAY_LCD_16X2_RGB) || defined(DISPLAY_LCD_20X4_RGB)
    printLCD(line1, line2);
#elif defined(DISPLAY_OLED_SSD1306) || defined(DISPLAY_OLED_SH1106)
    printOLED(line1, line2);
#elif defined(DISPLAY_TFT_ST7789) || defined(DISPLAY_TFT_ILI9341)
    printTFT(line1, line2);
#endif
}