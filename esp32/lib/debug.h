#pragma once

#include "../config.h"

#include <Arduino.h>

// Enabled by uncommenting DEBUG_SERIAL in config.h; when disabled every call compiles to nothing
#ifdef DEBUG_SERIAL
  #define DEBUG_LOG(fmt, ...) Serial.printf("[%lu] " fmt "\n", millis(), ##__VA_ARGS__)
#else
  #define DEBUG_LOG(...) do {} while (0)
#endif