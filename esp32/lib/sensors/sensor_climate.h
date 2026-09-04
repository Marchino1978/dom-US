#ifndef SENSOR_CLIMATE_H
#define SENSOR_CLIMATE_H

#include "../../config.h"

#if defined(SENSOR_BME280)
  #include <Adafruit_BME280.h>
  Adafruit_BME280 bme;
#elif defined(SENSOR_BMP280)
  #include <Adafruit_BMP280.h>
  Adafruit_BMP280 bmp;
#elif defined(SENSOR_SHT31)
  #include <Adafruit_SHT31.h>
  Adafruit_SHT31 sht31 = Adafruit_SHT31();
#elif defined(SENSOR_AHT20)
  #include <Adafruit_AHTX0.h>
  Adafruit_AHTX0 aht;
#elif defined(SENSOR_DHT11) || defined(SENSOR_DHT22)
  #include <DHT.h>
  #if defined(SENSOR_DHT11)
    DHT dht(PIN_DHT, DHT11);
  #else
    DHT dht(PIN_DHT, DHT22);
  #endif
#elif defined(SENSOR_DS18B20)
  #include <OneWire.h>
  #include <DallasTemperature.h>
  OneWire oneWire(PIN_DHT);
  DallasTemperature ds18b20(&oneWire);
#endif

inline void initClimate() {
  #if defined(SENSOR_BME280)
    if (!bme.begin(0x76)) {
      bme.begin(0x77);
    }
  #elif defined(SENSOR_BMP280)
    if (!bmp.begin(0x76)) {
      bmp.begin(0x77);
    }
  #elif defined(SENSOR_SHT31)
    sht31.begin(0x44);
  #elif defined(SENSOR_AHT20)
    aht.begin();
  #elif defined(SENSOR_DHT11) || defined(SENSOR_DHT22)
    dht.begin();
  #elif defined(SENSOR_DS18B20)
    ds18b20.begin();
  #endif
}

inline float readTemperature() {
  #if defined(SENSOR_BME280)
    return bme.readTemperature();
  #elif defined(SENSOR_BMP280)
    return bmp.readTemperature();
  #elif defined(SENSOR_SHT31)
    return sht31.readTemperature();
  #elif defined(SENSOR_AHT20)
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    return temp.temperature;
  #elif defined(SENSOR_DHT11) || defined(SENSOR_DHT22)
    return dht.readTemperature();
  #elif defined(SENSOR_DS18B20)
    ds18b20.requestTemperatures();
    return ds18b20.getTempCByIndex(0);
  #else
    return 0.0;
  #endif
}

inline float readHumidity() {
  #if defined(SENSOR_BME280)
    return bme.readHumidity();
  #elif defined(SENSOR_SHT31)
    return sht31.readHumidity();
  #elif defined(SENSOR_AHT20)
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    return humidity.relative_humidity;
  #elif defined(SENSOR_DHT11) || defined(SENSOR_DHT22)
    return dht.readHumidity();
  #else
    return 0.0;
  #endif
}

#endif