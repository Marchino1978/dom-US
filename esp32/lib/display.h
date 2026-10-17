#pragma once

#include "../config.h"
#include "checks.h"
#include "language.h"

#include <Arduino.h>
#include <time.h>

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
  #define DISPLAY_KIND_LCD
#endif

#if defined(DISPLAY_KIND_LCD)
#include "displays/display_lcd.h"
#elif defined(DISPLAY_OLED_SSD1306) || \
      defined(DISPLAY_OLED_SH1106)
#include "displays/display_oled.h"
#elif defined(DISPLAY_TFT_ST7789) || \
      defined(DISPLAY_TFT_ILI9341)
#include "displays/display_tft.h"
#endif

inline void initDisplay() {
#if defined(DISPLAY_KIND_LCD)
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
#if defined(DISPLAY_KIND_LCD)
    printLCD(line1, line2);
#elif defined(DISPLAY_OLED_SSD1306) || \
      defined(DISPLAY_OLED_SH1106)
    printOLED(line1, line2);
#elif defined(DISPLAY_TFT_ST7789) || \
      defined(DISPLAY_TFT_ILI9341)
    printTFT(line1, line2);
#endif
}

#if defined(DISPLAY_KIND_LCD)
  #define DISPLAY_DEGREE_SYMBOL "\xDF"
#else
  #define DISPLAY_DEGREE_SYMBOL ""
#endif

// Draws the climate values already read elsewhere.
// - Stealth: never draws while the alarm is armed.
// - Only draws while the screen is actually on.
// - LCD 16x2: time on row 1, values on row 2. LCD 20x4: time with weekday on row 1, one value per row.
inline void refreshClimateDisplay(float temp, float hum, float press) {
  if (alarmEnabled) return;
  if (!isDisplayActive()) return;

  time_t nowEpoch = time(nullptr);
  bool timeOk = nowEpoch > 1700000000;
  struct tm nowInfo;
  if (timeOk) localtime_r(&nowEpoch, &nowInfo);

  char tempStr[12];
  char humStr[12];
  char pressStr[12];

  if (isnan(temp)) snprintf(tempStr, sizeof(tempStr), "--");
  else snprintf(tempStr, sizeof(tempStr), "%d", (int)roundf(temp));

  if (isnan(hum)) snprintf(humStr, sizeof(humStr), "--");
  else snprintf(humStr, sizeof(humStr), "%02d", (int)roundf(hum));

  if (isnan(press)) snprintf(pressStr, sizeof(pressStr), "----");
  else snprintf(pressStr, sizeof(pressStr), "%d", (int)roundf(press));

#if defined(DISPLAY_KIND_LCD) && LCD_ROWS >= 4
  static const char* const dayNames[7] = {
    TXT_SUN_DAY, TXT_MON_DAY, TXT_TUE_DAY, TXT_WED_DAY, TXT_THU_DAY, TXT_FRI_DAY, TXT_SAT_DAY
  };

  char line[LCD_COLS + 1];
  char value[12];

  if (timeOk) {
    snprintf(line, sizeof(line), "%02d:%02d %.3s %02d/%02d/%04d",
             nowInfo.tm_hour,
             nowInfo.tm_min,
             dayNames[nowInfo.tm_wday],
             nowInfo.tm_mday,
             nowInfo.tm_mon + 1,
             nowInfo.tm_year + 1900);
  } else {
    snprintf(line, sizeof(line), "--:-- --- --/--/----");
  }
  printLCDRow(0, String(line));

  snprintf(value, sizeof(value), "%s%sC", tempStr, DISPLAY_DEGREE_SYMBOL);
  snprintf(line, sizeof(line), "%-11.11s:%8s", TXT_TEMP, value);
  printLCDRow(1, String(line));

  snprintf(value, sizeof(value), "%s%%", humStr);
  snprintf(line, sizeof(line), "%-11.11s:%8s", TXT_HUM, value);
  printLCDRow(2, String(line));

  snprintf(value, sizeof(value), "%shPa", pressStr);
  snprintf(line, sizeof(line), "%-11.11s:%8s", TXT_PRESS, value);
  printLCDRow(3, String(line));
#elif defined(DISPLAY_KIND_LCD)
  char timeLine[LCD_COLS + 1];

  if (timeOk) {
    snprintf(timeLine, sizeof(timeLine), "%02d:%02d %02d/%02d/%04d",
             nowInfo.tm_hour,
             nowInfo.tm_min,
             nowInfo.tm_mday,
             nowInfo.tm_mon + 1,
             nowInfo.tm_year + 1900);
  } else {
    snprintf(timeLine, sizeof(timeLine), "--:-- --/--/----");
  }

  printLCDRow(0, String(timeLine));
  printLCDRow(1, String(tempStr) + DISPLAY_DEGREE_SYMBOL "C " + humStr + "% " + pressStr + "hPa");
#else
  char timeLine[40];

  if (timeOk) {
    snprintf(timeLine, sizeof(timeLine), "%02d:%02d %02d/%02d/%04d",
             nowInfo.tm_hour,
             nowInfo.tm_min,
             nowInfo.tm_mday,
             nowInfo.tm_mon + 1,
             nowInfo.tm_year + 1900);
  } else {
    snprintf(timeLine, sizeof(timeLine), "--:-- --/--/----");
  }

  showMessage(String(timeLine), String(tempStr) + DISPLAY_DEGREE_SYMBOL "C " + humStr + "% " + pressStr + "hPa");
#endif
}

#ifdef DISPLAY_BOOT_MESSAGES

// Boot messages are queued and shown one frame at a time, each held for a minimum time,
// so the main loop never waits for the display. The boot phase ends after the last queued frame.
#ifndef DISPLAY_BOOT_STEP_MS
  #define DISPLAY_BOOT_STEP_MS 1500
#endif

#define DISPLAY_BOOT_QUEUE_SIZE 8

#if defined(DISPLAY_KIND_LCD)
  #define DISPLAY_BOOT_ROWS LCD_ROWS
  #define DISPLAY_BOOT_COLS LCD_COLS
#else
  #define DISPLAY_BOOT_ROWS 2
  #define DISPLAY_BOOT_COLS 24
#endif

// First row of the NTP step: below the Wi-Fi rows on 4-row displays, same rows on 2-row displays
#if DISPLAY_BOOT_ROWS >= 4
  #define DISPLAY_BOOT_NTP_ROW 2
#else
  #define DISPLAY_BOOT_NTP_ROW 0
#endif

struct DisplayBootFrame {
  char rows[DISPLAY_BOOT_ROWS][DISPLAY_BOOT_COLS + 1];
};

static DisplayBootFrame displayBootStaged;
static DisplayBootFrame displayBootLast;
static DisplayBootFrame displayBootQueue[DISPLAY_BOOT_QUEUE_SIZE];
static uint8_t displayBootHead = 0;
static uint8_t displayBootCount = 0;
static bool displayBootHasLast = false;
static bool displayBootRunning = true;
static bool displayBootEnding = false;
static bool displayBootShowing = false;
static unsigned long displayBootShownAt = 0;

inline bool displayBootActive() {
  return displayBootRunning;
}

// Stages the text of one row; nothing is shown until displayBootCommit()
inline void displayBootRow(uint8_t row, const char* text) {
  if (!displayBootRunning || displayBootEnding) return;
  if (row >= DISPLAY_BOOT_ROWS) return;
  strncpy(displayBootStaged.rows[row], text, DISPLAY_BOOT_COLS);
  displayBootStaged.rows[row][DISPLAY_BOOT_COLS] = '\0';
}

// Queues the staged rows as one frame; when the queue is full the newest frame is overwritten
inline void displayBootCommit() {
  if (!displayBootRunning || displayBootEnding) return;
  if (displayBootHasLast && memcmp(&displayBootLast, &displayBootStaged, sizeof(DisplayBootFrame)) == 0) return;

  uint8_t index;
  if (displayBootCount < DISPLAY_BOOT_QUEUE_SIZE) {
    index = (displayBootHead + displayBootCount) % DISPLAY_BOOT_QUEUE_SIZE;
    displayBootCount++;
  } else {
    index = (displayBootHead + displayBootCount - 1) % DISPLAY_BOOT_QUEUE_SIZE;
  }
  displayBootQueue[index] = displayBootStaged;
  displayBootLast = displayBootStaged;
  displayBootHasLast = true;
}

// Frames already queued are still shown before the boot phase terminates
inline void displayBootEnd() {
  displayBootEnding = true;
}

inline void displayBootRender(const DisplayBootFrame& frame) {
#if defined(DISPLAY_KIND_LCD)
  for (uint8_t r = 0; r < DISPLAY_BOOT_ROWS; r++) {
    printLCDRow(r, String(frame.rows[r]));
  }
#else
  showMessage(String(frame.rows[0]), String(frame.rows[1]));
#endif
}

// Non-blocking; returns true once, when the boot phase has just terminated
inline bool displayBootUpdate() {
  if (!displayBootRunning) return false;

  unsigned long now = millis();
  if (displayBootShowing && (now - displayBootShownAt < DISPLAY_BOOT_STEP_MS)) return false;

  if (displayBootCount > 0) {
    displayBootRender(displayBootQueue[displayBootHead]);
    displayBootHead = (displayBootHead + 1) % DISPLAY_BOOT_QUEUE_SIZE;
    displayBootCount--;
    displayBootShowing = true;
    displayBootShownAt = now;
    return false;
  }

  if (displayBootEnding) {
    displayBootRunning = false;
    return true;
  }
  return false;
}

#endif // DISPLAY_BOOT_MESSAGES

#else

inline void showMessage(const String& line1, const String& line2 = "") {}

#endif // MODULE_DISPLAY_ACTIVE

#if !defined(MODULE_DISPLAY_ACTIVE) || !defined(DISPLAY_BOOT_MESSAGES)

#define DISPLAY_BOOT_NTP_ROW 0

inline bool displayBootActive() { return false; }
inline void displayBootRow(uint8_t, const char*) {}
inline void displayBootCommit() {}
inline void displayBootEnd() {}
inline bool displayBootUpdate() { return false; }

#endif