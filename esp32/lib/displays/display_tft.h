#ifndef DISPLAY_TFT_H
#define DISPLAY_TFT_H

#include "../config.h"
#include <Adafruit_GFX.h>

#if defined(DISPLAY_TFT_ILI9341)
  #include <Adafruit_ILI9341.h>
  #define TFT_CS   15
  #define TFT_DC    2
  #define TFT_RST   4
  Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
#else
  #include <Adafruit_ST7789.h>
  #define TFT_CS   15
  #define TFT_DC    2
  #define TFT_RST   4
  Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
#endif

inline void initTFT() {
  #if defined(DISPLAY_TFT_ILI9341)
    tft.begin();
  #else
    tft.init(240, 240);
  #endif
  tft.setRotation(1);
  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF);
}

inline void printTFT(const String& line1, const String& line2 = "") {
  tft.fillScreen(0x0000);
  tft.setCursor(10, 30);
  tft.setTextSize(2);
  tft.println(line1);
  tft.setCursor(10, 70);
  tft.println(line2);
}

#endif