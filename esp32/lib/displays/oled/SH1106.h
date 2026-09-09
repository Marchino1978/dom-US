#pragma once
 
#include "../../../config.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
 
Adafruit_SH1106 oled(-1);
 
inline void initOLEDHardware() {
  oled.begin(SH1106_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(1);
  oled.display();
}
 
inline void printOLEDValue(const String& line1, const String& line2 = "") {
  oled.clearDisplay();
  oled.setCursor(0, 10);
  oled.setTextSize(1);
  oled.println(line1);
  oled.setCursor(0, 30);
  oled.println(line2);
  oled.display();
}
 
inline void setOLEDPowerValue(bool state) {
  if (state) oled.ssd1306_command(SSD1306_DISPLAYON);
  else oled.ssd1306_command(SSD1306_DISPLAYOFF);
}