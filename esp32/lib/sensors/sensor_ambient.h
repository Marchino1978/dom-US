#pragma once

#include "../../config.h"

#if defined(SENSOR_LIGHT_BH1750)
  #include "ambient/BH1750.h"
#elif defined(SENSOR_LIGHT_LDR)
  #include "ambient/LDR.h"
#elif defined(SENSOR_COLOR_TCS34725)
  #include "ambient/TCS34725.h"
#elif defined(SENSOR_LIGHT_VEML7700)
  #include "ambient/VEML7700.h"
#endif

// LDR returns a percentage (0-100), every other ambient sensor returns lux
#if defined(SENSOR_LIGHT_LDR)
  #define AMBIENT_UNIT "%"
  #ifndef AMBIENT_LIGHT_ON_THRESHOLD
    #define AMBIENT_LIGHT_ON_THRESHOLD 25.0f
  #endif
#else
  #define AMBIENT_UNIT "Lux"
  #ifndef AMBIENT_LIGHT_ON_THRESHOLD
    #define AMBIENT_LIGHT_ON_THRESHOLD 50.0f
  #endif
#endif

inline bool isAmbientLightOn(float value) {
  return !isnan(value) && value >= AMBIENT_LIGHT_ON_THRESHOLD;
}

inline void initAmbient() {
  #if defined(SENSOR_LIGHT_BH1750) || \
      defined(SENSOR_LIGHT_LDR) || \
      defined(SENSOR_COLOR_TCS34725) || \
      defined(SENSOR_LIGHT_VEML7700)
    initAmbientHardware();
  #endif
}

inline float readAmbientLux() {
  #if defined(SENSOR_LIGHT_BH1750) || \
      defined(SENSOR_LIGHT_LDR) || \
      defined(SENSOR_COLOR_TCS34725) || \
      defined(SENSOR_LIGHT_VEML7700)
    return readAmbientLuxValue();
  #else
    return NAN;
  #endif
}