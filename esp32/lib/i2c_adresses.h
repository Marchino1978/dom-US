#pragma once

#include "../config.h"

#include <Arduino.h>
#include <Wire.h>

#define I2C_NO_WHOAMI          0xFFFF
#define I2C_SCAN_ADDR_FIRST    0x08
#define I2C_SCAN_ADDR_LAST     0x77
#define I2C_SCAN_MAX_RESULTS   16

#define I2C_STATUS_UNRECOGNIZED 0
#define I2C_STATUS_CANDIDATE    1
#define I2C_STATUS_CONFIRMED    2

struct I2cDeviceInfo {
  uint8_t     address;
  const char* name;
  const char* info;
  uint16_t    whoAmIReg;
  bool        whoAmIReg16;
  uint8_t     whoAmIValue;
};

struct I2cScanResult {
  uint8_t              address;
  uint8_t              status;
  uint8_t              readValue;
  const I2cDeviceInfo* entry;
};

// Entries sharing an address are checked in table order; WHO_AM_I entries are probed first.
static const I2cDeviceInfo i2cDatabase[] = {
  { 0x23, "BH1750",                  "Digital light sensor (lux)",              I2C_NO_WHOAMI, false, 0x00 },
  { 0x5C, "BH1750 (ADDR high)",      "Digital light sensor (lux)",              I2C_NO_WHOAMI, false, 0x00 },

  { 0x29, "VL53L0X",                 "Laser ToF distance sensor (2 m)",         0x00C0,        false, 0xEE },
  { 0x29, "VL53L1X",                 "Laser ToF distance sensor (4 m)",         0x010F,        true,  0xEA },
  { 0x29, "TCS34725",                "RGB color and color temperature sensor",  0x0092,        false, 0x44 },

  { 0x10, "VEML7700",                "High precision ambient light sensor",     0x0007,        false, 0x81 },

  { 0x76, "BME280",                  "Temperature, humidity, pressure sensor",  0x00D0,        false, 0x60 },
  { 0x76, "BMP280",                  "Temperature and pressure sensor",         0x00D0,        false, 0x58 },
  { 0x77, "BME280",                  "Temperature, humidity, pressure sensor",  0x00D0,        false, 0x60 },
  { 0x77, "BMP280",                  "Temperature and pressure sensor",         0x00D0,        false, 0x58 },

  { 0x38, "AHT20",                   "Temperature and humidity sensor",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x44, "SHT31",                   "Temperature and humidity sensor",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x45, "SHT31 (ADDR high)",       "Temperature and humidity sensor",         I2C_NO_WHOAMI, false, 0x00 },

  { 0x27, "PCF8574 LCD backpack",    "I2C adapter for 16x2 / 20x4 LCD",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x3F, "PCF8574A LCD backpack",   "I2C adapter for 16x2 / 20x4 LCD",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x3E, "DFRobot RGB LCD",         "LCD controller of RGB backlight display", I2C_NO_WHOAMI, false, 0x00 },
  { 0x2D, "DFRobot RGB backlight",   "RGB backlight driver (new revision)",     I2C_NO_WHOAMI, false, 0x00 },
  { 0x60, "DFRobot RGB backlight",   "RGB backlight driver (old revision)",     I2C_NO_WHOAMI, false, 0x00 },

  { 0x3C, "SSD1306 / SH1106 OLED",   "0.96\" / 1.3\" graphic display",          I2C_NO_WHOAMI, false, 0x00 },
  { 0x3D, "SSD1306 OLED (ADDR high)","0.96\" graphic display",                  I2C_NO_WHOAMI, false, 0x00 },

  { 0x68, "MPU6050",                 "6-axis gyroscope and accelerometer",      0x0075,        false, 0x68 },
  { 0x69, "MPU6050 (AD0 high)",      "6-axis gyroscope and accelerometer",      0x0075,        false, 0x68 },
  { 0x68, "DS3231",                  "Real time clock",                         I2C_NO_WHOAMI, false, 0x00 },

  { 0x48, "ADS1115",                 "16-bit analog to digital converter",      I2C_NO_WHOAMI, false, 0x00 },
  { 0x40, "INA219",                  "Current and power monitor",               I2C_NO_WHOAMI, false, 0x00 },
  { 0x50, "AT24C EEPROM",            "Serial EEPROM",                           I2C_NO_WHOAMI, false, 0x00 }
};

static const size_t i2cDatabaseSize = sizeof(i2cDatabase) / sizeof(i2cDatabase[0]);

static I2cScanResult i2cResults[I2C_SCAN_MAX_RESULTS];
static int  i2cResultCount = 0;
static int  i2cTotalFound = 0;
static bool i2cResultsValid = false;

inline bool i2cReadRegister(uint8_t address, uint16_t reg, bool reg16, uint8_t& value) {
  Wire.beginTransmission(address);
  if (reg16) Wire.write((uint8_t)(reg >> 8));
  Wire.write((uint8_t)(reg & 0xFF));
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint16_t)address, (uint8_t)1) != 1) return false;
  value = Wire.read();
  return true;
}

inline bool i2cAddressResponds(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

inline void runI2cScan() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  i2cResultCount = 0;
  i2cTotalFound = 0;
  i2cResultsValid = false;

  for (uint8_t addr = I2C_SCAN_ADDR_FIRST; addr <= I2C_SCAN_ADDR_LAST; addr++) {
    if (!i2cAddressResponds(addr)) {
      yield();
      continue;
    }

    i2cTotalFound++;
    if (i2cResultCount >= I2C_SCAN_MAX_RESULTS) continue;

    const I2cDeviceInfo* confirmed = nullptr;
    const I2cDeviceInfo* candidate = nullptr;
    const I2cDeviceInfo* anyEntry  = nullptr;
    uint8_t readValue = 0;

    for (size_t i = 0; i < i2cDatabaseSize; i++) {
      const I2cDeviceInfo& d = i2cDatabase[i];
      if (d.address != addr) continue;
      if (!anyEntry) anyEntry = &d;

      if (d.whoAmIReg != I2C_NO_WHOAMI) {
        uint8_t v;
        if (i2cReadRegister(addr, d.whoAmIReg, d.whoAmIReg16, v)) {
          readValue = v;
          if (v == d.whoAmIValue) {
            confirmed = &d;
            break;
          }
        }
      } else if (!candidate) {
        candidate = &d;
      }
    }

    if (!confirmed && !candidate) candidate = anyEntry;

    I2cScanResult& r = i2cResults[i2cResultCount++];
    r.address = addr;
    r.readValue = readValue;
    if (confirmed) {
      r.status = I2C_STATUS_CONFIRMED;
      r.entry = confirmed;
    } else if (candidate) {
      r.status = I2C_STATUS_CANDIDATE;
      r.entry = candidate;
    } else {
      r.status = I2C_STATUS_UNRECOGNIZED;
      r.entry = nullptr;
    }

    yield();
  }

  i2cResultsValid = true;
}

inline void buildI2cSummaryMessage(char* out, size_t maxLen) {
  if (i2cTotalFound == 0) {
    snprintf(out, maxLen, "🔍 *I2C BUS SCAN RESULT*\n\n`No devices found`");
    return;
  }
  snprintf(out, maxLen,
           "🔍 *I2C BUS SCAN RESULT*\n\n🔢 `Total devices found: %d`\n\n❓ `Show detailed info?`",
           i2cTotalFound);
}

inline void buildI2cDeviceMessage(int index, char* out, size_t maxLen) {
  const I2cScanResult& r = i2cResults[index];

  const char* name = "Unknown";
  const char* info = "Not in the database";
  char statusStr[44];

  if (r.status == I2C_STATUS_CONFIRMED) {
    name = r.entry->name;
    info = r.entry->info;
    snprintf(statusStr, sizeof(statusStr), "🟢 Confirmed (ID 0x%02X)", r.readValue);
  } else if (r.status == I2C_STATUS_CANDIDATE) {
    name = r.entry->name;
    info = r.entry->info;
    if (r.entry->whoAmIReg == I2C_NO_WHOAMI) {
      snprintf(statusStr, sizeof(statusStr), "🟡 Candidate (no ID register)");
    } else {
      snprintf(statusStr, sizeof(statusStr), "🟡 Candidate (ID mismatch)");
    }
  } else {
    snprintf(statusStr, sizeof(statusStr), "🔴 Unrecognized");
  }

  snprintf(out, maxLen,
           "📍 `Address: 0x%02X`\n├─ `Device : %s`\n├─ `Status : %s`\n└─ `Info   : %s`",
           r.address, name, statusStr, info);
}