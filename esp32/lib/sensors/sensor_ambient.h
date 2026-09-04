#ifndef SENSOR_AMBIENT_H
#define SENSOR_AMBIENT_H

#include "../../config.h"

#if defined(SENSOR_LIGHT_BH1750)
  #include "ambient/BH1750.h"
#elif defined(SENSOR_LIGHT_LDR)
  #include "ambient/LDR.h"
#elif defined(SENSOR_GAS_MQ135)
  #include "ambient/MQ135.h"
#elif defined(SENSOR_COLOR_TCS34725)
  #include "ambient/TCS34725.h"
#elif defined(SENSOR_LIGHT_VEML7700)
  #include "ambient/VEML7700.h"
#endif

inline void initAmbient() {
  #if defined(SENSOR_LIGHT_BH1750) || defined(SENSOR_LIGHT_LDR) || defined(SENSOR_GAS_MQ135) || defined(SENSOR_COLOR_TCS34725) || defined(SENSOR_LIGHT_VEML7700)
    initAmbientHardware();
  #endif
}

inline float readAmbientLux() {
  #if defined(SENSOR_LIGHT_BH1750) || defined(SENSOR_LIGHT_LDR) || defined(SENSOR_GAS_MQ135) || defined(SENSOR_COLOR_TCS34725) || defined(SENSOR_LIGHT_VEML7700)
    return readAmbientLuxValue();
  #else
    return 0.0;
  #endif
}

#endif