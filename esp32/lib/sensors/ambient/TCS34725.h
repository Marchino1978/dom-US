#pragma once

#include "../../../config.h"
#include "../../debug.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// To turn off the sensor's onboard white LED, connect the LED pin to the GND pin.
// With the LED off the sensor measures incident (ambient) light.

// Fixed 154 ms integration, gain adjusted automatically to avoid saturation and to stay sensitive in the dark
static Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_16X);
static bool tcsReady = false;

// ams DN40 lux formula. TCS_GLASS_ATT is 1.0 in open air, greater than 1.0 behind a cover (1 / transmittance)
#define TCS_ATIME_MS          153.6f
#define TCS_GLASS_ATT         1.0f
#define TCS_DF                310.0f
#define TCS_R_COEF            0.136f
#define TCS_G_COEF            1.0f
#define TCS_B_COEF            -0.444f

#define TCS_GAIN_STEPS        4
#define TCS_SATURATION_COUNT  60000
#define TCS_LOW_COUNT         2000
#define TCS_TARGET_MAX_COUNT  50000.0f

static const tcs34725Gain_t tcsGainReg[TCS_GAIN_STEPS] = {
  TCS34725_GAIN_1X, TCS34725_GAIN_4X, TCS34725_GAIN_16X, TCS34725_GAIN_60X
};
static constexpr float tcsGainX[TCS_GAIN_STEPS] = {1.0f, 4.0f, 16.0f, 60.0f};

// Counts per lux for each gain step, precomputed at compile time
static constexpr float tcsCpl[TCS_GAIN_STEPS] = {
  (TCS_ATIME_MS * 1.0f)  / (TCS_GLASS_ATT * TCS_DF),
  (TCS_ATIME_MS * 4.0f)  / (TCS_GLASS_ATT * TCS_DF),
  (TCS_ATIME_MS * 16.0f) / (TCS_GLASS_ATT * TCS_DF),
  (TCS_ATIME_MS * 60.0f) / (TCS_GLASS_ATT * TCS_DF)
};

static uint8_t tcsGainIndex = 2;

inline void initAmbientHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  tcsReady = tcs.begin();
  if (!tcsReady) {
    DEBUG_LOG("TCS34725 NOT FOUND");
  }
}

// getRawData blocks for the integration time (154 ms); a gain change needs two extra cycles to settle
inline float readAmbientLuxValue() {
  if (!tcsReady) return NAN;

  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  for (uint8_t i = 0; i < TCS_GAIN_STEPS; i++) {
    uint8_t next = tcsGainIndex;

    if (c >= TCS_SATURATION_COUNT && tcsGainIndex > 0) {
      next = tcsGainIndex - 1;
    } else if (c < TCS_LOW_COUNT && tcsGainIndex < TCS_GAIN_STEPS - 1) {
      float predicted = (float)c * tcsGainX[tcsGainIndex + 1] / tcsGainX[tcsGainIndex];
      if (predicted < TCS_TARGET_MAX_COUNT) next = tcsGainIndex + 1;
    }

    if (next == tcsGainIndex) break;

    tcsGainIndex = next;
    tcs.setGain(tcsGainReg[tcsGainIndex]);
    tcs.getRawData(&r, &g, &b, &c);
    tcs.getRawData(&r, &g, &b, &c);
  }

  if (c == 0) return 0.0f;

  float ir = ((float)r + g + b - c) / 2.0f;
  if (ir < 0.0f) ir = 0.0f;

  float rComp = r - ir;
  float gComp = g - ir;
  float bComp = b - ir;
  if (rComp < 0.0f) rComp = 0.0f;
  if (gComp < 0.0f) gComp = 0.0f;
  if (bComp < 0.0f) bComp = 0.0f;

  float lux = (TCS_R_COEF * rComp + TCS_G_COEF * gComp + TCS_B_COEF * bComp) / tcsCpl[tcsGainIndex];
  if (lux < 0.0f) lux = 0.0f;

  DEBUG_LOG("TCS34725 gain %.0fx r=%u g=%u b=%u c=%u lux=%.1f", tcsGainX[tcsGainIndex], r, g, b, c, lux);
  return lux;
}