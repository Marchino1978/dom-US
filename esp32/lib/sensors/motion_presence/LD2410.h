#pragma once

#include "../../../config.h"
#include "../../debug.h"

#include <Arduino.h>
#include <ld2410.h>

// ESP32 TX -> sensor RX, ESP32 RX <- sensor TX (same pins as US100, shared Serial1)
#define LD2410_PIN_TX   PIN_TRIG
#define LD2410_PIN_RX   PIN_ECHO
#define LD2410_BAUD     256000
 
static ld2410 radar;
static bool ld2410Ready = false;
 
inline void initMotionHardware() {
  Serial1.begin(LD2410_BAUD, SERIAL_8N1, LD2410_PIN_RX, LD2410_PIN_TX);
  ld2410Ready = radar.begin(Serial1);
  if (!ld2410Ready) {
    DEBUG_LOG("LD2410 NOT FOUND");
  }
}
 
// Non-blocking: read() only parses frames already in the UART buffer
inline bool readMotionState() {
  if (!ld2410Ready) return false;
  radar.read();
  return radar.presenceDetected();
}