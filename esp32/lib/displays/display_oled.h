#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include "../config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>

#if defined(DISPLAY_OLED_SH1106)
  #include <Adafruit_SH1106.h>
  Adafruit_SH1106 oled(-1);
#else
  #include <Adafruit_SSD1306.h>
  Adafruit_SSD1306 oled(128, 64, &Wire, -1);
#endif

inline void initOLED() {
  #if defined(DISPLAY_OLED_SH1106)
    oled.begin(SH1106_SWITCHCAPVCC, 0x3C);
  #else
    oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  #endif
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(1);
  oled.display();
}

inline void printOLED(const String& line1, const String& line2 = "") {
  oled.clearDisplay();
  oled.setCursor(0, 10);
  oled.setTextSize(1);
  oled.println(line1);
  oled.setCursor(0, 30);
  oled.println(line2);
  oled.display();
}

inline void setOLEDPower(bool state) {
  if (state) oled.ssd1306_command(SSD1306_DISPLAYON);
  else oled.ssd1306_command(SSD1306_DISPLAYOFF);
}

#endif