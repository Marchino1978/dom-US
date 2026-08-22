#ifndef SENSOR_DISTANCE_H
#define SENSOR_DISTANCE_H

#include "../config.h"

#if defined(SENSOR_ULTRASONIC_US100)
  #define US100_SERIAL Serial2
  #define PIN_US100_RX 16
  #define PIN_US100_TX 17
#elif defined(SENSOR_ULTRASONIC_HCSR04)
#elif defined(SENSOR_LASER_VL53L0X)
  #include <Adafruit_VL53L0X.h>
  Adafruit_VL53L0X lox = Adafruit_VL53L0X();
#endif

inline void initDistance() {
  #if defined(SENSOR_ULTRASONIC_US100)
    US100_SERIAL.begin(9600, SERIAL_8N1, PIN_US100_RX, PIN_US100_TX);
  #elif defined(SENSOR_ULTRASONIC_HCSR04)
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    digitalWrite(PIN_TRIG, LOW);
  #elif defined(SENSOR_LASER_VL53L0X)
    lox.begin();
  #endif
}

inline float readDistanceCM() {
  #if defined(SENSOR_ULTRASONIC_US100)
    while (US100_SERIAL.available()) { US100_SERIAL.read(); }
    
    US100_SERIAL.write(0x55);
    
    unsigned long startTime = millis();
    while (US100_SERIAL.available() < 2) {
      if (millis() - startTime > 100) return -1.0;
    }
    
    unsigned int highByte = US100_SERIAL.read();
    unsigned int lowByte  = US100_SERIAL.read();
    float distanceMM = (highByte * 256) + lowByte;
    
    return distanceMM / 10.0;

  #elif defined(SENSOR_ULTRASONIC_HCSR04)
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    
    long duration = pulseIn(PIN_ECHO, HIGH, 30000);
    if (duration == 0) return -1.0;
    return (duration * 0.0343) / 2.0;

  #elif defined(SENSOR_LASER_VL53L0X)
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);
    if (measure.RangeStatus != 4) {
      return measure.RangeMilliMeter / 10.0;
    }
    return -1.0;

  #else
    return -1.0;
  #endif
}

inline float readUS100Temperature() {
  #if defined(SENSOR_ULTRASONIC_US100)
    while (US100_SERIAL.available()) { US100_SERIAL.read(); }
    
    US100_SERIAL.write(0x50);
    
    unsigned long startTime = millis();
    while (US100_SERIAL.available() < 1) {
      if (millis() - startTime > 100) return -99.0;
    }
    
    int tempByte = US100_SERIAL.read();
    return (float)(tempByte - 45);
  #else
    return -99.0;
  #endif
}

#endif