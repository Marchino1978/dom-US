#ifndef SENSOR_CLIMATE_H
#define SENSOR_CLIMATE_H

#include "../../config.h"

#if defined(SENSOR_AHT20)
  #include "climate/AHT20.h"
#elif defined(SENSOR_BME280)
  #include "climate/BMPE80.h"
#elif defined(SENSOR_BMP280)
  #include "climate/BMP280.h"
#elif defined(SENSOR_DHT11)
  #include "climate/DHT11.h"
#elif defined(SENSOR_DHT22)
  #include "climate/DHT22.h"
#elif defined(SENSOR_DS18B20)
  #include "climate/DS18B20.h"
#elif defined(SENSOR_SHT31)
  #include "climate/SHT31.h"
#endif

inline void initClimate() {
  #if defined(SENSOR_AHT20) || defined(SENSOR_BME280) || defined(SENSOR_BMPP280) || defined(SENSOR_DHT11) || defined(SENSOR_DHT22) || defined(SENSOR_DS18B20) || defined(SENSOR_SHT31)
    initClimateHardware();
  #endif
}

inline float readTemperature() {
  #if defined(SENSOR_AHT20) || defined(SENSOR_BME280) || defined(SENSOR_BMPP280) || defined(SENSOR_DHT11) || defined(SENSOR_DHT22) || defined(SENSOR_DS18B20) || defined(SENSOR_SHT31)
    return readTemperatureValue();
  #else
    return 0.0;
  #endif
}

inline float readHumidity() {
  #if defined(SENSOR_AHT20) || defined(SENSOR_BME280) || defined(SENSOR_BMPP280) || defined(SENSOR_DHT11) || defined(SENSOR_DHT22) || defined(SENSOR_DS18B20) || defined(SENSOR_SHT31)
    return readHumidityValue();
  #else
    return 0.0;
  #endif
}

#endif