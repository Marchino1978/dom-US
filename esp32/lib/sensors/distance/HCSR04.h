#pragma once

#include "../../../config.h"

#include <Arduino.h>

// ECHO output is 5V: a voltage divider is required before the ESP32 input
#define HCSR04_MIN_CM       2.0f
#define HCSR04_MAX_CM       400.0f
#define HCSR04_TIMEOUT_US   30000UL
#define HCSR04_INTERVAL_MS  60
 
static volatile unsigned long hcsr04EchoStartUs = 0;
static volatile unsigned long hcsr04EchoEndUs = 0;
static volatile bool hcsr04EchoDone = false;
static bool hcsr04Pending = false;
static unsigned long hcsr04TriggerUs = 0;
static unsigned long hcsr04LastTriggerMs = 0;
static float hcsr04Distance = NAN;
 
static void IRAM_ATTR hcsr04EchoIsr() {
  if (digitalRead(PIN_ECHO) == HIGH) {
    hcsr04EchoStartUs = micros();
  } else {
    hcsr04EchoEndUs = micros();
    hcsr04EchoDone = true;
  }
}
 
inline void initDistanceHardware() {
  pinMode(PIN_TRIG, OUTPUT);
  digitalWrite(PIN_TRIG, LOW);
  pinMode(PIN_ECHO, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ECHO), hcsr04EchoIsr, CHANGE);
}
 
inline void hcsr04SendTrigger() {
  hcsr04EchoDone = false;
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  hcsr04TriggerUs = micros();
  hcsr04LastTriggerMs = millis();
  hcsr04Pending = true;
}
 
// Returns the last completed measurement in cm; a new one is triggered when idle
inline float readDistanceValue() {
  if (hcsr04Pending) {
    if (hcsr04EchoDone) {
      float cm = (hcsr04EchoEndUs - hcsr04EchoStartUs) / 58.0f;
      hcsr04Distance = (cm >= HCSR04_MIN_CM && cm <= HCSR04_MAX_CM) ? cm : NAN;
      hcsr04Pending = false;
    } else if (micros() - hcsr04TriggerUs > HCSR04_TIMEOUT_US) {
      hcsr04Distance = NAN;
      hcsr04Pending = false;
    }
  }
 
  if (!hcsr04Pending && millis() - hcsr04LastTriggerMs >= HCSR04_INTERVAL_MS) {
    hcsr04SendTrigger();
  }
  return hcsr04Distance;
}