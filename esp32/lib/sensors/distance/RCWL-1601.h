#pragma once

#include "../../../config.h"

#include <Arduino.h>

// Trigger/echo mode (module jumper left open)
#define RCWL1601_MIN_CM       2.0f
#define RCWL1601_MAX_CM       400.0f
#define RCWL1601_TIMEOUT_US   30000UL
#define RCWL1601_INTERVAL_MS  60
 
static volatile unsigned long rcwl1601EchoStartUs = 0;
static volatile unsigned long rcwl1601EchoEndUs = 0;
static volatile bool rcwl1601EchoDone = false;
static bool rcwl1601Pending = false;
static unsigned long rcwl1601TriggerUs = 0;
static unsigned long rcwl1601LastTriggerMs = 0;
static float rcwl1601Distance = NAN;
 
static void IRAM_ATTR rcwl1601EchoIsr() {
  if (digitalRead(PIN_ECHO) == HIGH) {
    rcwl1601EchoStartUs = micros();
  } else {
    rcwl1601EchoEndUs = micros();
    rcwl1601EchoDone = true;
  }
}
 
inline void initDistanceHardware() {
  pinMode(PIN_TRIG, OUTPUT);
  digitalWrite(PIN_TRIG, LOW);
  pinMode(PIN_ECHO, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ECHO), rcwl1601EchoIsr, CHANGE);
}
 
inline void rcwl1601SendTrigger() {
  rcwl1601EchoDone = false;
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  rcwl1601TriggerUs = micros();
  rcwl1601LastTriggerMs = millis();
  rcwl1601Pending = true;
}
 
// Returns the last completed measurement in cm; a new one is triggered when idle
inline float readDistanceValue() {
  if (rcwl1601Pending) {
    if (rcwl1601EchoDone) {
      float cm = (rcwl1601EchoEndUs - rcwl1601EchoStartUs) / 58.0f;
      rcwl1601Distance = (cm >= RCWL1601_MIN_CM && cm <= RCWL1601_MAX_CM) ? cm : NAN;
      rcwl1601Pending = false;
    } else if (micros() - rcwl1601TriggerUs > RCWL1601_TIMEOUT_US) {
      rcwl1601Distance = NAN;
      rcwl1601Pending = false;
    }
  }
 
  if (!rcwl1601Pending && millis() - rcwl1601LastTriggerMs >= RCWL1601_INTERVAL_MS) {
    rcwl1601SendTrigger();
  }
  return rcwl1601Distance;
}