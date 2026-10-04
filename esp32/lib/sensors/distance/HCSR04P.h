#pragma once

#include "../../../config.h"

#include <Arduino.h>

#define HCSR04P_MIN_CM       2.0f
#define HCSR04P_MAX_CM       400.0f
#define HCSR04P_TIMEOUT_US   30000UL
#define HCSR04P_INTERVAL_MS  60
 
static volatile unsigned long hcsr04pEchoStartUs = 0;
static volatile unsigned long hcsr04pEchoEndUs = 0;
static volatile bool hcsr04pEchoDone = false;
static bool hcsr04pPending = false;
static unsigned long hcsr04pTriggerUs = 0;
static unsigned long hcsr04pLastTriggerMs = 0;
static float hcsr04pDistance = NAN;
 
static void IRAM_ATTR hcsr04pEchoIsr() {
  if (digitalRead(PIN_ECHO) == HIGH) {
    hcsr04pEchoStartUs = micros();
  } else {
    hcsr04pEchoEndUs = micros();
    hcsr04pEchoDone = true;
  }
}
 
inline void initDistanceHardware() {
  pinMode(PIN_TRIG, OUTPUT);
  digitalWrite(PIN_TRIG, LOW);
  pinMode(PIN_ECHO, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ECHO), hcsr04pEchoIsr, CHANGE);
}
 
inline void hcsr04pSendTrigger() {
  hcsr04pEchoDone = false;
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  hcsr04pTriggerUs = micros();
  hcsr04pLastTriggerMs = millis();
  hcsr04pPending = true;
}
 
// Returns the last completed measurement in cm; a new one is triggered when idle
inline float readDistanceValue() {
  if (hcsr04pPending) {
    if (hcsr04pEchoDone) {
      float cm = (hcsr04pEchoEndUs - hcsr04pEchoStartUs) / 58.0f;
      hcsr04pDistance = (cm >= HCSR04P_MIN_CM && cm <= HCSR04P_MAX_CM) ? cm : NAN;
      hcsr04pPending = false;
    } else if (micros() - hcsr04pTriggerUs > HCSR04P_TIMEOUT_US) {
      hcsr04pDistance = NAN;
      hcsr04pPending = false;
    }
  }
 
  if (!hcsr04pPending && millis() - hcsr04pLastTriggerMs >= HCSR04P_INTERVAL_MS) {
    hcsr04pSendTrigger();
  }
  return hcsr04pDistance;
}