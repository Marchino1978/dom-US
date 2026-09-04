#ifndef SENSOR_AMBIENT_H
#define SENSOR_AMBIENT_H

#include "../../config.h"

#if defined(SENSOR_LIGHT_BH1750)
  #include <Wire.h>
  #include <BH1750.h>
  BH1750 lightMeter;
#elif defined(SENSOR_LIGHT_LDR)
#elif defined(SENSOR_COLOR_TCS34725)
  #include <Wire.h>
  #include <Adafruit_TCS34725.h>
  Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
#elif defined(SENSOR_GAS_MQ135)
#endif

inline void initAmbient() {
  #if defined(SENSOR_LIGHT_BH1750)
    lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  #elif defined(SENSOR_LIGHT_LDR)
    pinMode(PIN_LDR, INPUT);
  #elif defined(SENSOR_COLOR_TCS34725)
    tcs.begin();
  #elif defined(SENSOR_GAS_MQ135)
    pinMode(PIN_MQ135, INPUT);
  #endif
}

inline float readAmbientLux() {
  #if defined(SENSOR_LIGHT_BH1750)
    return lightMeter.readLightLevel();
  #elif defined(SENSOR_LIGHT_LDR)
    return (float)analogRead(PIN_LDR);
  #elif defined(SENSOR_COLOR_TCS34725)
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    return (float)c;
  #else
    return 0.0;
  #endif
}

#endif