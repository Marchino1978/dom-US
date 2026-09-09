#pragma once
 
#include "../../../config.h"
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
 
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
 
inline void initTFTHardware() {
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(0x0000);
  tft.setTextColor(0xFFFF);
}
 
inline void printTFTValue(const String& line1, const String& line2 = "") {
  tft.fillScreen(0x0000);
  tft.setCursor(10, 30);
  tft.setTextSize(2);
  tft.println(line1);
  tft.setCursor(10, 70);
  tft.println(line2);
}