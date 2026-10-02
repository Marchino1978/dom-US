#pragma once

#include "../../../config.h"

#include <Arduino.h>

// UART mode: jumper on the back of the module must be installed.
// ESP32 TX -> sensor Trig/TX, ESP32 RX <- sensor Echo/RX.
#define US100_PIN_TX        PIN_TRIG
#define US100_PIN_RX        PIN_ECHO
#define US100_BAUD          9600
#define US100_TIMEOUT_MS    100
#define US100_CMD_DISTANCE  0x55
#define US100_CMD_TEMP      0x50
#define US100_MAX_MM        4500
 
enum Us100State {
  US100_IDLE,
  US100_WAIT_DISTANCE,
  US100_WAIT_TEMP
};
 
static Us100State us100State = US100_IDLE;
static unsigned long us100RequestTime = 0;
static bool us100TempWanted = false;
static float us100Distance = NAN;
static float us100Temperature = NAN;
 
inline void initDistanceHardware() {
  Serial1.begin(US100_BAUD, SERIAL_8N1, US100_PIN_RX, US100_PIN_TX);
}
 
inline void us100Request(uint8_t command, Us100State waitState) {
  while (Serial1.available()) Serial1.read();
  Serial1.write(command);
  us100State = waitState;
  us100RequestTime = millis();
}
 
inline void us100Service() {
  switch (us100State) {
    case US100_IDLE:
      break;
 
    case US100_WAIT_DISTANCE:
      if (Serial1.available() >= 2) {
        uint16_t mm = ((uint16_t)Serial1.read() << 8) | Serial1.read();
        us100Distance = (mm > 0 && mm <= US100_MAX_MM) ? mm / 10.0f : NAN;
        us100State = US100_IDLE;
      } else if (millis() - us100RequestTime > US100_TIMEOUT_MS) {
        us100Distance = NAN;
        us100State = US100_IDLE;
      }
      break;
 
    case US100_WAIT_TEMP:
      if (Serial1.available() >= 1) {
        int raw = Serial1.read();
        us100Temperature = (raw > 1 && raw < 130) ? (float)(raw - 45) : NAN;
        us100State = US100_IDLE;
      } else if (millis() - us100RequestTime > US100_TIMEOUT_MS) {
        us100Temperature = NAN;
        us100State = US100_IDLE;
      }
      break;
  }
}
 
// Returns the last completed measurement in cm; a new one is requested when idle
inline float readDistanceValue() {
  us100Service();
  if (us100State == US100_IDLE) {
    if (us100TempWanted) {
      us100TempWanted = false;
      us100Request(US100_CMD_TEMP, US100_WAIT_TEMP);
    } else {
      us100Request(US100_CMD_DISTANCE, US100_WAIT_DISTANCE);
    }
  }
  return us100Distance;
}
 
// Returns the last completed temperature in Celsius; refreshed on the next idle slot
inline float readUS100TemperatureValue() {
  us100TempWanted = true;
  us100Service();
  return us100Temperature;
}