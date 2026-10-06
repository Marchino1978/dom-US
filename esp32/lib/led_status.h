#pragma once

#include "../config.h"

#include <Arduino.h>

#ifndef PIN_LED_RGB
  #define PIN_LED_RGB 10
#endif

#ifndef LED_BRIGHTNESS
  #define LED_BRIGHTNESS 40
#endif

#define LED_BLINK_INTERVAL_MS 250
#define LED_BREATHE_PERIOD_MS 2000
#define LED_WORK_DEFAULT_MS   1000

enum LedState {
  LED_STATE_OFF,
  LED_STATE_WIFI,
  LED_STATE_NTP,
  LED_STATE_IDLE,
  LED_STATE_WORK,
  LED_STATE_ERROR
};

static LedState ledState = LED_STATE_OFF;
static LedState ledPreviousState = LED_STATE_IDLE;
static unsigned long ledWorkEndTime = 0;
static unsigned long ledLastToggle = 0;
static bool ledBlinkOn = false;
static bool ledStatusEnabled = true;

inline void ledWriteRaw(uint8_t r, uint8_t g, uint8_t b) {
  if (!ledStatusEnabled) {
    r = 0;
    g = 0;
    b = 0;
  }
  r = (uint16_t)r * LED_BRIGHTNESS / 255;
  g = (uint16_t)g * LED_BRIGHTNESS / 255;
  b = (uint16_t)b * LED_BRIGHTNESS / 255;
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    rgbLedWrite(PIN_LED_RGB, r, g, b);
  #else
    neopixelWrite(PIN_LED_RGB, r, g, b);
  #endif
}

inline void setLedStatusEnabled(bool enabled) {
  ledStatusEnabled = enabled;
  ledBlinkOn = false;
  ledWriteRaw(0, 0, 0);
}

inline void initLed() {
  ledState = LED_STATE_OFF;
  ledBlinkOn = false;
  ledLastToggle = millis();
  ledWriteRaw(0, 0, 0);
}

inline void setLedState(LedState state) {
  if (state == ledState) return;
  if (ledState != LED_STATE_WORK) ledPreviousState = ledState;
  ledState = state;
  ledBlinkOn = false;
  ledLastToggle = millis();
}

inline void ledWork(unsigned long durationMs = LED_WORK_DEFAULT_MS) {
  if (ledState == LED_STATE_ERROR) return;
  if (ledState != LED_STATE_WORK) ledPreviousState = ledState;
  ledState = LED_STATE_WORK;
  ledWorkEndTime = millis() + durationMs;
  ledBlinkOn = true;
  ledLastToggle = millis();
  ledWriteRaw(0, 255, 0);
}

inline void ledBlinkStep(uint8_t r, uint8_t g, uint8_t b) {
  unsigned long now = millis();
  if (now - ledLastToggle >= LED_BLINK_INTERVAL_MS) {
    ledLastToggle = now;
    ledBlinkOn = !ledBlinkOn;
    if (ledBlinkOn) ledWriteRaw(r, g, b);
    else ledWriteRaw(0, 0, 0);
  }
}

inline void ledBreatheStep(uint8_t r, uint8_t g, uint8_t b) {
  unsigned long phase = millis() % LED_BREATHE_PERIOD_MS;
  unsigned long half = LED_BREATHE_PERIOD_MS / 2;
  uint16_t level = (phase < half) ? (phase * 255UL / half)
                                  : ((LED_BREATHE_PERIOD_MS - phase) * 255UL / half);
  ledWriteRaw((uint16_t)r * level / 255,
              (uint16_t)g * level / 255,
              (uint16_t)b * level / 255);
}

inline void updateLed() {
  switch (ledState) {
    case LED_STATE_OFF:
      break;

    case LED_STATE_WIFI:
      ledBlinkStep(0, 0, 255);
      break;

    case LED_STATE_NTP:
      ledBlinkStep(255, 60, 0);
      break;

    case LED_STATE_IDLE:
      ledBreatheStep(255, 180, 0);
      break;

    case LED_STATE_WORK:
      if ((long)(millis() - ledWorkEndTime) >= 0) {
        ledState = ledPreviousState;
        ledBlinkOn = false;
        ledLastToggle = millis();
        break;
      }
      ledBlinkStep(0, 255, 0);
      break;

    case LED_STATE_ERROR:
      if (!ledBlinkOn) {
        ledBlinkOn = true;
        ledWriteRaw(255, 0, 0);
      }
      break;
  }
}

inline void ledDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    updateLed();
    delay(5);
  }
}