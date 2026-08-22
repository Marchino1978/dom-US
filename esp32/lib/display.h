#ifndef DISPLAY_H
#define DISPLAY_H

#include "../config.h"

// Includes dynamic file based on selection
#if defined(DISPLAY_LCD_16X2) || defined(DISPLAY_LCD_20X4)
  #include "displays/display_lcd.h"
#elif defined(DISPLAY_OLED_SSD1306) || defined(DISPLAY_OLED_SH1106)
  #include "displays/display_oled.h"
#elif defined(DISPLAY_TFT_ST7789) || defined(DISPLAY_TFT_ILI9341)
  #include "displays/display_tft.h"
#endif

// Unified display initialization
inline void initDisplay() {
  #if defined(DISPLAY_LCD_16X2) || defined(DISPLAY_LCD_20X4)
    initLCD();
  #elif defined(DISPLAY_OLED_SSD1306) || defined(DISPLAY_OLED_SH1106)
    initOLED();
  #elif defined(DISPLAY_TFT_ST7789) || defined(DISPLAY_TFT_ILI9341)
    initTFT();
  #endif
}

// Unified print function
inline void showMessage(const String& line1, const String& line2 = "") {
  #if defined(DISPLAY_LCD_16X2) || defined(DISPLAY_LCD_20X4)
    printLCD(line1, line2);
  #elif defined(DISPLAY_OLED_SSD1306) || defined(DISPLAY_OLED_SH1106)
    printOLED(line1, line2);
  #elif defined(DISPLAY_TFT_ST7789) || defined(DISPLAY_TFT_ILI9341)
    printTFT(line1, line2);
  #endif
}

#endif#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "../config.h"

#if defined(LANG_IT)
  #define TXT_WELCOME      "Avvio Sistema..."
  #define TXT_WIFI_CONN    "Connessione WiFi..."
  #define TXT_WIFI_OK      "WiFi Connesso!"
  #define TXT_TEMP         "Temp: "
  #define TXT_HUM          "Umid: "
  #define TXT_MOTION       "Movimento!"
  #define TXT_NO_MOTION    "Nessun Movimento"

#elif defined(LANG_EN)
  #define TXT_WELCOME      "System Booting..."
  #define TXT_WIFI_CONN    "Connecting WiFi..."
  #define TXT_WIFI_OK      "WiFi Connected!"
  #define TXT_TEMP         "Temp: "
  #define TXT_HUM          "Humi: "
  #define TXT_MOTION       "Motion Detected!"
  #define TXT_NO_MOTION    "Clear"

#else // Default Fallback (English)
  #define TXT_WELCOME      "System Booting..."
  #define TXT_WIFI_CONN    "Connecting WiFi..."
  #define TXT_WIFI_OK      "WiFi Connected!"
  #define TXT_TEMP         "Temp: "
  #define TXT_HUM          "Humi: "
  #define TXT_MOTION       "Motion Detected!"
  #define TXT_NO_MOTION    "Clear"
#endif

#endif