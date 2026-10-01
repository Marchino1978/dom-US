#pragma once

#include "../../../config.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// ======================================================
// Write your code and declarations below this line
// ======================================================

static Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
static bool tcsReady = false;

// ams DN40 lux formula, open air (GA = 1), values matching 50 ms / 4x gain above
#define TCS_ATIME_MS   50.4f
#define TCS_AGAIN      4.0f
#define TCS_GLASS_ATT  1.0f
#define TCS_DF         310.0f
#define TCS_R_COEF     0.136f
#define TCS_G_COEF     1.0f
#define TCS_B_COEF     -0.444f
 
inline void initAmbientHardware() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  tcsReady = tcs.begin();
  if (!tcsReady) {
    Serial.println("TCS34725 NOT FOUND");
  }
}
 
// getRawData blocks for the integration time (50 ms)
inline float readAmbientLuxValue() {
  if (!tcsReady) return NAN;
 
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  if (c == 0) return 0.0f;

  float ir = ((float)r + g + b - c) / 2.0f;
  if (ir < 0) ir = 0;

  float rComp = r - ir;
  float gComp = g - ir;
  float bComp = b - ir;

  float cpl = (TCS_ATIME_MS * TCS_AGAIN) / (TCS_GLASS_ATT * TCS_DF);
  float lux = (TCS_R_COEF * rComp + TCS_G_COEF * gComp + TCS_B_COEF * bComp) / cpl;

  return (lux < 0) ? 0.0f : lux;
}