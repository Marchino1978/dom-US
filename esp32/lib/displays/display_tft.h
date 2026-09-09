#pragma once
#include "../config.h"

#if defined(DISPLAY_TFT_ST7789)
  #include "tft/ST7789.h"
#elif defined(DISPLAY_TFT_ILI9341)
  #include "tft/ILI9341.h"
#endif

inline void initTFT() {
  #if defined(DISPLAY_TFT_ST7789) || defined(DISPLAY_TFT_ILI9341)
    initTFTHardware();
  #endif
}

inline void printTFT(const String& line1, const String& line2 = "") {
  #if defined(DISPLAY_TFT_ST7789) || defined(DISPLAY_TFT_ILI9341)
    printTFTValue(line1, line2);
  #endif
}