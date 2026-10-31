///////////7////////
// DA VERIFICARE //
///////////////////

#pragma once

#include "../../../config.h"
#include "../../debug.h"

#include <Arduino.h>
#include <HLK-LD2410S.h>

// ESP32 TX -> sensor RX, ESP32 RX <- sensor TX (same pins as US100, shared Serial1)
#define LD2410S_PIN_TX   PIN_TRIG
#define LD2410S_PIN_RX   PIN_ECHO
#define LD2410S_BAUD     115200
 
static HLK_LD2410S radar;
static bool ld2410sReady = false;
 
inline void initMotionHardware() {
  Serial1.begin(LD2410S_BAUD, SERIAL_8N1, LD2410S_PIN_RX, LD2410S_PIN_TX);
  radar.begin(Serial1); 
  ld2410sReady = true; 
}
 
// Non-blocking: read() only parses frames already in the UART buffer
inline bool readMotionState() {
  if (!ld2410sReady) return false;
  radar.read();
  return radar.isTargetDetected();
}