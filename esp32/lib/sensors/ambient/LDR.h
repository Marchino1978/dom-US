#pragma once

#include "../../../config.h"

#include <Arduino.h>

// Wiring: 3.3V -> LDR -> PIN_ANALOG node -> 10k resistor -> GND
// Value rises with brightness. Not calibrated: result is a percentage (0-100), not lux.
#define LDR_SAMPLES    8
#define LDR_ADC_MAX    4095.0f
 
inline void initAmbientHardware() {
  pinMode(PIN_ANALOG, INPUT);
  analogSetPinAttenuation(PIN_ANALOG, ADC_11db);
}
 
// Returns brightness percentage through the readAmbientLuxValue() interface
inline float readAmbientLuxValue() {
  uint32_t sum = 0;
  for (int i = 0; i < LDR_SAMPLES; i++) {
    sum += analogRead(PIN_ANALOG);
  }
  float raw = (float)sum / LDR_SAMPLES;
  return raw * 100.0f / LDR_ADC_MAX;
}
 