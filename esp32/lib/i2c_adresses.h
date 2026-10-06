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

  // DISPLAYS
  // --- DISPLAYS AND VISUAL INDICATORS ---
  { 0x3C, "SSD1306 / SH1106 OLED",   "0.96\" / 1.3\" graphic display",          I2C_NO_WHOAMI, false, 0x00 },
  { 0x3D, "SSD1306 OLED (ADDR high)","0.96\" graphic display",                  I2C_NO_WHOAMI, false, 0x00 },
  { 0x27, "PCF8574 LCD backpack",    "I2C adapter for 16x2 / 20x4 LCD",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x3F, "PCF8574A LCD backpack",   "I2C adapter for 16x2 / 20x4 LCD",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x70, "HT16K33 / Max7219 I2C",   "LED matrix / 7-segment display driver",   I2C_NO_WHOAMI, false, 0x00 },
  { 0x71, "HT16K33 (ADDR high)",     "LED matrix / 7-segment display driver",   I2C_NO_WHOAMI, false, 0x00 },
  { 0x3E, "DFRobot RGB LCD",         "LCD controller of RGB backlight display", I2C_NO_WHOAMI, false, 0x00 },
  { 0x2D, "DFRobot RGB backlight",   "RGB backlight driver (new revision)",     I2C_NO_WHOAMI, false, 0x00 },
  { 0x60, "DFRobot RGB backlight",   "RGB backlight driver (old revision)",     I2C_NO_WHOAMI, false, 0x00 },

  // CLIMATE
  // --- TEMPERATURE, HUMIDITY AND PRESSURE SENSORS ---
  { 0x38, "AHT10",                   "Temperature and humidity sensor",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x38, "AHT20",                   "Temperature and humidity sensor",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x44, "SHT30",                   "Temperature and humidity sensor",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x45, "SHT30 (ADDR high)",       "Temperature and humidity sensor",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x44, "SHT31",                   "Temperature and humidity sensor",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x45, "SHT31 (ADDR high)",       "Temperature and humidity sensor",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x44, "SHT40",                   "Temperature and humidity sensor",         0x89,          false, 0x00 },
  { 0x45, "SHT40 (ADDR high)",       "Temperature and humidity sensor",         0x89,          false, 0x00 },
  { 0x5C, "DHT12",                   "Temperature and humidity sensor",         I2C_NO_WHOAMI, false, 0x00 },
  { 0x18, "MCP9808",                 "Microchip Precision Temp Sensor",         0x07,          false, 0x04 },
  { 0x48, "LM75A",                   "Digital Temp Sensor & Watchdog",          0x01,          false, 0x00 },
  { 0x76, "BME280",                  "Temperature, humidity, pressure sensor",  0x00D0,        false, 0x60 },
  { 0x76, "BMP280",                  "Temperature and pressure sensor",         0x00D0,        false, 0x58 },
  { 0x77, "BME280",                  "Temperature, humidity, pressure sensor",  0x00D0,        false, 0x60 },
  { 0x77, "BMP280",                  "Temperature and pressure sensor",         0x00D0,        false, 0x58 },

  // MOTION & PRESENCE
  // --- xxx SENSORS ---

  // DISTANCE
  // --- PROXIMITY SENSORS ---
  { 0x29, "VL53L0X",                 "Laser ToF distance sensor (2 m)",         0x00C0,        false, 0xEE },
  { 0x29, "VL53L1X",                 "Laser ToF distance sensor (4 m)",         0x010F,        true,  0xEA },

  // AMBIENT
  // --- LIGHT AND COLOR SENSORS ---
  // --- GAS AND AIR QUALITY SENSORS ---
  { 0x23, "BH1750",                  "Digital light sensor (lux)",              I2C_NO_WHOAMI, false, 0x00 },
  { 0x5C, "BH1750 (ADDR high)",      "Digital light sensor (lux)",              I2C_NO_WHOAMI, false, 0x00 },
  { 0x39, "TSL2561",                 "Dual-diode digital light sensor",         0x000A,        false, 0x50 },
  { 0x29, "TSL2591",                 "High dynamic range digital light sensor", 0x00B2,        false, 0x50 },
  { 0x29, "TCS34725",                "RGB color and color temperature sensor",  0x0092,        false, 0x44 },
  { 0x10, "VEML7700",                "High precision ambient light sensor",     0x0007,        false, 0x81 },
  { 0x39, "APDS-9960",               "Gesture, proximity, light & RGB sensor",  0x0092,        false, 0xAB },
  { 0x76, "BME680",                  "Environmental 4-in-1 Gas Sensor",         0xD0,          false, 0x61 },
  { 0x77, "BME680 (ADDR high)",      "Environmental 4-in-1 Gas Sensor",         0xD0,          false, 0x61 },
  { 0x76, "BME688",                  "Environmental Gas Sensor with AI",        0xD0,          false, 0x61 },
  { 0x77, "BME688 (ADDR high)",      "Environmental Gas Sensor with AI",        0xD0,          false, 0x61 },
  { 0x58, "SGP30",                   "TVOC and eCO2 gas sensor",                I2C_NO_WHOAMI, false, 0x00 },
  { 0x59, "SGP40",                   "VOC index gas sensor",                    I2C_NO_WHOAMI, false, 0x00 },
  { 0x5A, "CCS811",                  "VOC sensor for indoor air quality",       0x0020,        false, 0x81 },
  { 0x5B, "CCS811 (ADDR high)",      "VOC sensor for indoor air quality",       0x0020,        false, 0x81 },
  { 0x61, "SCD30",                   "NDIR CO2, temperature & humidity sensor", I2C_NO_WHOAMI, false, 0x00 },
  { 0x62, "SCD40 / SCD41",           "Photoacoustic CO2 sensor",                I2C_NO_WHOAMI, false, 0x00 },

  // --- GYROSCOPES, ACCELEROMETERS AND COMPASSES (IMU) ---
  { 0x68, "MPU6050",                 "6-axis gyroscope and accelerometer",      0x0075,        false, 0x68 },
  { 0x69, "MPU6050 (AD0 high)",      "6-axis gyroscope and accelerometer",      0x0075,        false, 0x68 },
  { 0x68, "MPU6500",                 "6-axis gyroscope and accelerometer",      0x0075,        false, 0x70 },
  { 0x69, "MPU6500 (AD0 high)",      "6-axis gyroscope and accelerometer",      0x0075,        false, 0x70 },
  { 0x68, "MPU9250",                 "9-axis gyroscope, accel & magnetometer",  0x0075,        false, 0x71 },
  { 0x69, "MPU9250 (AD0 high)",      "9-axis gyroscope, accel & magnetometer",  0x0075,        false, 0x71 },
  { 0x53, "ADXL345",                 "3-axis accelerometer (low power)",        0x0000,        false, 0xE5 },
  { 0x1D, "ADXL345 (ADDR high)",     "3-axis accelerometer (low power)",        0x0000,        false, 0xE5 },
  { 0x0D, "QMC5883L",                "3-axis digital compass / magnetometer",   0x000D,        false, 0xFF },
  { 0x1E, "HMC5883L",                "3-axis digital compass / magnetometer",   0x000A,        false, 0x48 },
  { 0x1E, "LSM303DLHC (Mag)",        "3-axis magnetometer",                     0x000A,        false, 0x48 },
  { 0x32, "LSM303DLHC (Acc)",        "3-axis accelerometer",                    0x000F,        false, 0x33 },

  // --- GPIO EXPANDERS, ADC CONVERTERS AND PWM DRIVERS ---
  { 0x48, "ADS1115",                 "16-bit analog to digital converter",      I2C_NO_WHOAMI, false, 0x00 },
  { 0x49, "ADS1115 (ADDR vcc)",      "16-bit analog to digital converter",      I2C_NO_WHOAMI, false, 0x00 },
  { 0x4A, "ADS1115 (ADDR sda)",      "16-bit analog to digital converter",      I2C_NO_WHOAMI, false, 0x00 },
  { 0x4B, "ADS1115 (ADDR scl)",      "16-bit analog to digital converter",      I2C_NO_WHOAMI, false, 0x00 },
  { 0x20, "PCF8574 / MCP23017 (000)","8-bit or 16-bit I/O Expander",            I2C_NO_WHOAMI, false, 0x00 },
  { 0x21, "PCF8574 / MCP23017 (001)","8-bit or 16-bit I/O Expander",            I2C_NO_WHOAMI, false, 0x00 },
  { 0x22, "PCF8574 / MCP23017 (010)","8-bit or 16-bit I/O Expander",            I2C_NO_WHOAMI, false, 0x00 },
  { 0x23, "PCF8574 / MCP23017 (011)","8-bit or 16-bit I/O Expander",            I2C_NO_WHOAMI, false, 0x00 },
  { 0x24, "PCF8574 / MCP23017 (100)","8-bit or 16-bit I/O Expander",            I2C_NO_WHOAMI, false, 0x00 },
  { 0x25, "PCF8574 / MCP23017 (101)","8-bit or 16-bit I/O Expander",            I2C_NO_WHOAMI, false, 0x00 },
  { 0x26, "PCF8574 / MCP23017 (110)","8-bit or 16-bit I/O Expander",            I2C_NO_WHOAMI, false, 0x00 },
  { 0x27, "PCF8574 / MCP23017 (111)","8-bit or 16-bit I/O Expander",            I2C_NO_WHOAMI, false, 0x00 },
  { 0x30, "PCF8574A (000)",          "8-bit I/O Expander (A-Variant)",          I2C_NO_WHOAMI, false, 0x00 },
  { 0x37, "PCF8574A (111)",          "8-bit I/O Expander (A-Variant)",          I2C_NO_WHOAMI, false, 0x00 },
  { 0x40, "INA219 / PCA9685 (00000)","Current Monitor / 16-ch PWM Driver",      I2C_NO_WHOAMI, false, 0x00 },

  // --- RTC CLOCKS AND MEMORIES ---
  { 0x68, "DS3231 / DS1307 RTC",     "Real time clock",                         I2C_NO_WHOAMI, false, 0x00 },
  { 0x50, "AT24C EEPROM (Base)",     "Serial EEPROM",                           I2C_NO_WHOAMI, false, 0x00 },
  { 0x57, "AT24C32 EEPROM (RTC Mod)","Serial EEPROM (On RTC Board)",            I2C_NO_WHOAMI, false, 0x00 }
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
    snprintf(out, maxLen, "🔍 *I2C BUS SCAN RESULT*\n\n❗ No devices found");
    return;
  }
  snprintf(out, maxLen,
           "🔍 *I2C BUS SCAN RESULT*\n\n🔢 Total devices found: %d\n\n❓ Show detailed info?",
           i2cTotalFound);
}

inline void buildI2cDeviceMessage(int index, char* out, size_t maxLen) {
  const I2cScanResult& r = i2cResults[index];

  const char* name = "Unknown";
  const char* info = "Not in the database";
  const char* statusIcon = "🔴";
  char statusStr[40];

  if (r.status == I2C_STATUS_CONFIRMED) {
    name = r.entry->name;
    info = r.entry->info;
    statusIcon = "🟢";
    snprintf(statusStr, sizeof(statusStr), "Confirmed (ID 0x%02X)", r.readValue);
  } else if (r.status == I2C_STATUS_CANDIDATE) {
    name = r.entry->name;
    info = r.entry->info;
    statusIcon = "🟡";
    if (r.entry->whoAmIReg == I2C_NO_WHOAMI) {
      snprintf(statusStr, sizeof(statusStr), "Candidate (no ID register)");
    } else {
      snprintf(statusStr, sizeof(statusStr), "Candidate (ID mismatch)");
    }
  } else {
    snprintf(statusStr, sizeof(statusStr), "Unrecognized (ID missing)");
  }

  snprintf(out, maxLen,
           "📍 `%-7s : 0x%02X`\n"
           "🛠️ `%-7s : %s`\n"
           "%s `%-7s : %s`\n"
           "🆔 `%-7s : %s`",
           "Address", r.address,
           "Device", name,
           statusIcon, "Status", statusStr,
           "Info", info);
}

// change 📝 with 📦 or 📟 ⚙️ 🛠️
// change ℹ️ with 🗒️ or 🆔