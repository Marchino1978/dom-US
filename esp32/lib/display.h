#pragma once

#include "../config.h"
#include "checks.h"

// display_addons.h provides generic backlight/wake handling (needed by any
// display, with or without an addon) AND checkAddonAlarmTriggered() (needed
// by lib/alarm.h even with an addon alone, no display selected).
#if defined(MODULE_DISPLAY_ACTIVE) || defined(HAS_WAKEUP_ADDON)
  #include "displays/display_addons.h"
#endif

#ifdef MODULE_DISPLAY_ACTIVE

extern bool alarmEnabled;

#if defined(DISPLAY_LCD_16X2) || \
    defined(DISPLAY_LCD_20X4) || \
    defined(DISPLAY_LCD_16X2_RGB) || \
    defined(DISPLAY_LCD_20X4_RGB)
#include "displays/display_lcd.h"
#elif defined(DISPLAY_OLED_SSD1306) || \
      defined(DISPLAY_OLED_SH1106)
#include "displays/display_oled.h"
#elif defined(DISPLAY_TFT_ST7789) || \
      defined(DISPLAY_TFT_ILI9341)
#include "displays/display_tft.h"
#endif

inline void initDisplay() {
#if defined(DISPLAY_LCD_16X2) || \
    defined(DISPLAY_LCD_20X4) || \
    defined(DISPLAY_LCD_16X2_RGB) || \
    defined(DISPLAY_LCD_20X4_RGB)
    initLCD();
#elif defined(DISPLAY_OLED_SSD1306) || \
      defined(DISPLAY_OLED_SH1106)
    initOLED();
#elif defined(DISPLAY_TFT_ST7789) || \
      defined(DISPLAY_TFT_ILI9341)
    initTFT();
#endif
}

inline void showMessage(const String& line1, const String& line2 = "") {
#if defined(DISPLAY_LCD_16X2) || \
    defined(DISPLAY_LCD_20X4) || \
    defined(DISPLAY_LCD_16X2_RGB) || \
    defined(DISPLAY_LCD_20X4_RGB)
    printLCD(line1, line2);
#elif defined(DISPLAY_OLED_SSD1306) || \
      defined(DISPLAY_OLED_SH1106)
    printOLED(line1, line2);
#elif defined(DISPLAY_TFT_ST7789) || \
      defined(DISPLAY_TFT_ILI9341)
    printTFT(line1, line2);
#endif
}

// ======================================================
//  CLIMATE DISPLAY REFRESH (continuous reading support)
// ======================================================
// Decides WHEN to redraw the climate values already read elsewhere.
// - Stealth: never draws while the alarm is armed.
// - Only draws while the screen is actually on (covers both ALWAYS_ON_*
//   modes, where the screen stays on, and WAKE_* modes, where the screen
//   is only on during the addon-triggered wake window).
// - LCD always redraws (cheap full-text refresh).
// - OLED/TFT redraw only when a value actually changed, to reduce
//   unnecessary panel writes (true partial/pixel-level redraw is left to
//   the individual driver files, see oled/*.h and tft/*.h).
#if defined(DISPLAY_LCD_16X2) || \
    defined(DISPLAY_LCD_20X4) || \
    defined(DISPLAY_LCD_16X2_RGB) || \
    defined(DISPLAY_LCD_20X4_RGB)
  #define DISPLAY_DEGREE_SYMBOL "\xDF"
#else
  #define DISPLAY_DEGREE_SYMBOL ""
#endif

inline void refreshClimateDisplay(float temp, float hum, float press) {
  if (alarmEnabled) return;
  if (!isDisplayActive()) return;

  char timeLine[40];
  time_t nowEpoch = time(nullptr);
  if (nowEpoch > 1700000000) {
    struct tm nowInfo;
    localtime_r(&nowEpoch, &nowInfo);
    snprintf(timeLine, sizeof(timeLine), "%02d:%02d %02d/%02d/%04d",
             nowInfo.tm_hour,
             nowInfo.tm_min,
             nowInfo.tm_mday,
             nowInfo.tm_mon + 1,
             nowInfo.tm_year + 1900);
  } else {
    snprintf(timeLine, sizeof(timeLine), "--:-- --/--/----");
  }

  char tempStr[12];
  char humStr[12];
  char pressStr[12];

  if (isnan(temp)) snprintf(tempStr, sizeof(tempStr), "--");
  else snprintf(tempStr, sizeof(tempStr), "%d", (int)roundf(temp));

  if (isnan(hum)) snprintf(humStr, sizeof(humStr), "--");
  else snprintf(humStr, sizeof(humStr), "%02d", (int)roundf(hum));

  if (isnan(press)) snprintf(pressStr, sizeof(pressStr), "----");
  else snprintf(pressStr, sizeof(pressStr), "%d", (int)roundf(press));

  showMessage(String(timeLine), String(tempStr) + DISPLAY_DEGREE_SYMBOL "C " + humStr + "% " + pressStr + "hPa");
}

#else

inline void showMessage(const String& line1, const String& line2 = "") {}

#endif // MODULE_DISPLAY_ACTIVE