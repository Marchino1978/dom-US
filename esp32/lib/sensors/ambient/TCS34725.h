#pragma once

#include "../../../config.h"
#include "../../debug.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// To turn off the sensor's onboard white LED, connect the LED pin to the GND pin.
// With the LED off the sensor measures incident (ambient) light.

// Fixed gain and integration time: the clear channel is scaled to a percentage of its full range
#define TCS_SAMPLES        5
#define TCS_SAMPLE_GAP_MS  5
#define TCS_CLEAR_MAX      10240.0f

static Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_60X);
static bool tcsReady = false;

inline void initAmbientHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  tcsReady = tcs.begin();
  if (!tcsReady) {
    DEBUG_LOG("TCS34725 NOT FOUND");
  }
}

// Blocks for about TCS_SAMPLES * (24 ms + gap); returns the averaged clear channel as a percentage (0-100)
inline float readAmbientLuxValue() {
  if (!tcsReady) return NAN;

  uint32_t sum = 0;
  for (uint8_t i = 0; i < TCS_SAMPLES; i++) {
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    DEBUG_LOG("TCS34725 sample %u r=%u g=%u b=%u c=%u", i, r, g, b, c);
    sum += c;
    if (i < TCS_SAMPLES - 1) delay(TCS_SAMPLE_GAP_MS);
  }

  float percent = ((float)sum / TCS_SAMPLES) * 100.0f / TCS_CLEAR_MAX;
  if (percent > 100.0f) percent = 100.0f;
  return percent;
}