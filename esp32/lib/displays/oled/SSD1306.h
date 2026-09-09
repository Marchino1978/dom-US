1306 · H
#pragma once
 
#include "../../../config.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
 
inline void initOLEDHardware() {
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
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