#pragma once

#include "../config.h"

// ======================================================
//  SECURITY BLOCK & CONFLICT RESOLUTION
// ======================================================

// Cross-category conflict check: ADDON vs SENSOR
#if defined(ADDON_PIR_AM312) && defined(SENSOR_PIR_AM312)
#error "⚠️ BUILD BLOCKED: Do not enable the same sensor multiple times as both ADDON and SENSOR."
#endif
#if defined(ADDON_RADAR_RCWL0516) && defined(SENSOR_RADAR_RCWL0516)
#error "⚠️ BUILD BLOCKED: Do not enable the same sensor multiple times as both ADDON and SENSOR."
#endif
#if defined(ADDON_LASER_VL53L0X) && defined(SENSOR_LASER_VL53L0X)
#error "⚠️ BUILD BLOCKED: Do not enable the same sensor multiple times as both ADDON and SENSOR."
#endif
#if defined(ADDON_LASER_VL53L1X) && defined(SENSOR_LASER_VL53L1X)
#error "⚠️ BUILD BLOCKED: Do not enable the same sensor multiple times as both ADDON and SENSOR."
#endif

// Display uniqueness check
#if (defined(DISPLAY_LCD_16X2) + defined(DISPLAY_LCD_20X4) + \
     defined(DISPLAY_LCD_16X2_RGB) + defined(DISPLAY_LCD_20X4_RGB) + \
     defined(DISPLAY_OLED_SSD1306) + defined(DISPLAY_OLED_SH1106) + \
     defined(DISPLAY_TFT_ST7789) + defined(DISPLAY_TFT_ILI9341)) > 1
#error "⚠️ BUILD BLOCKED: You can select ONLY ONE display configuration in config.h."
#endif

// Addon uniqueness check
#if (defined(ADDON_PIR_AM312) + defined(ADDON_RADAR_RCWL0516) + \
     defined(ADDON_LASER_VL53L0X) + defined(ADDON_LASER_VL53L1X)) > 1
#error "⚠️ BUILD BLOCKED: You can select ONLY ONE addon configuration in config.h."
#endif

// Climate sensor uniqueness check
#if (defined(SENSOR_AHT20) + defined(SENSOR_BME280) + defined(SENSOR_BMP280) + \
     defined(SENSOR_DHT11) + defined(SENSOR_DHT22) + defined(SENSOR_DS18B20) + \
     defined(SENSOR_SHT31)) > 1
#error "⚠️ BUILD BLOCKED: You can select ONLY ONE climate sensor in config.h."
#endif

// Ambient sensor uniqueness check
#if (defined(SENSOR_LIGHT_BH1750) + defined(SENSOR_LIGHT_LDR) + \
     defined(SENSOR_GAS_MQ135) + defined(SENSOR_COLOR_TCS34725) + \
     defined(SENSOR_LIGHT_VEML7700)) > 1
#error "⚠️ BUILD BLOCKED: You can select ONLY ONE ambient sensor in config.h."
#endif

// Motion/Presence sensor uniqueness check
#if (defined(SENSOR_PIR_AM312) + defined(SENSOR_PIR_HCSR501) + \
     defined(SENSOR_MMWAVE_LD2410) + defined(SENSOR_RADAR_RCWL0516) + \
     defined(SENSOR_IR_TE174)) > 1
#error "⚠️ BUILD BLOCKED: You can select ONLY ONE motion/presence sensor in config.h."
#endif

// Distance sensor uniqueness check
#if (defined(SENSOR_ULTRASONIC_HCSR04) + defined(SENSOR_ULTRASONIC_HCSR04P) + \
     defined(SENSOR_ULTRASONIC_RCWL1601) + defined(SENSOR_ULTRASONIC_US100) + \
     defined(SENSOR_LASER_VL53L0X) + defined(SENSOR_LASER_VL53L1X)) > 1
#error "⚠️ BUILD BLOCKED: You can select ONLY ONE distance sensor in config.h."
#endif

// ======================================================
//  MACRO-MODULE AUTO-DETECTION (GLOBAL SWITCHES)
// ======================================================

// Display module: active if any display is configured
#if defined(DISPLAY_LCD_16X2) || defined(DISPLAY_LCD_20X4) || \
    defined(DISPLAY_LCD_16X2_RGB) || defined(DISPLAY_LCD_20X4_RGB) || \
    defined(DISPLAY_OLED_SSD1306) || defined(DISPLAY_OLED_SH1106) || \
    defined(DISPLAY_TFT_ST7789) || defined(DISPLAY_TFT_ILI9341)
  #define MODULE_DISPLAY_ACTIVE
#endif

// Telemetry module: active if any climate sensor is configured
#if defined(SENSOR_AHT20) || defined(SENSOR_BME280) || defined(SENSOR_BMP280) || \
    defined(SENSOR_DHT11) || defined(SENSOR_DHT22) || defined(SENSOR_DS18B20) || \
    defined(SENSOR_SHT31)
  #define MODULE_TELEMETRY_ACTIVE
#endif

// Alarm module: active if any motion/presence, distance sensor OR addon is configured.
// An addon requires a display (see HAS_WAKEUP_ADDON check below), so it never stands alone.
#if defined(SENSOR_PIR_AM312) || defined(SENSOR_PIR_HCSR501) || defined(SENSOR_MMWAVE_LD2410) || \
    defined(SENSOR_RADAR_RCWL0516) || defined(SENSOR_IR_TE174) || \
    defined(SENSOR_ULTRASONIC_HCSR04) || defined(SENSOR_ULTRASONIC_HCSR04P) || \
    defined(SENSOR_ULTRASONIC_RCWL1601) || defined(SENSOR_ULTRASONIC_US100) || \
    defined(SENSOR_LASER_VL53L0X) || defined(SENSOR_LASER_VL53L1X) || \
    defined(ADDON_PIR_AM312) || defined(ADDON_RADAR_RCWL0516) || \
    defined(ADDON_LASER_VL53L0X) || defined(ADDON_LASER_VL53L1X)
  #define MODULE_ALARM_ACTIVE
#endif

// ======================================================
//  COUNTERS AND SMART FILTERS
// ======================================================

// Total count of physical sensors contributing to the alarm logic (dedicated + addons).
// Addons always count here: their alarm-trigger function is gated by alarmEnabled at runtime.
// At most one device per category is allowed (uniqueness checks above),
// so each category contributes 0 or 1 to the total.
#ifdef MODULE_ALARM_ACTIVE
  #if defined(SENSOR_PIR_AM312) || defined(SENSOR_PIR_HCSR501) || \
      defined(SENSOR_MMWAVE_LD2410) || defined(SENSOR_RADAR_RCWL0516) || \
      defined(SENSOR_IR_TE174)
    #define ALARM_COUNT_MOTION 1
  #else
    #define ALARM_COUNT_MOTION 0
  #endif

  #if defined(SENSOR_ULTRASONIC_HCSR04) || defined(SENSOR_ULTRASONIC_HCSR04P) || \
      defined(SENSOR_ULTRASONIC_RCWL1601) || defined(SENSOR_ULTRASONIC_US100) || \
      defined(SENSOR_LASER_VL53L0X) || defined(SENSOR_LASER_VL53L1X)
    #define ALARM_COUNT_DISTANCE 1
  #else
    #define ALARM_COUNT_DISTANCE 0
  #endif

  #if defined(ADDON_PIR_AM312) || defined(ADDON_RADAR_RCWL0516) || \
      defined(ADDON_LASER_VL53L0X) || defined(ADDON_LASER_VL53L1X)
    #define ALARM_COUNT_ADDON 1
  #else
    #define ALARM_COUNT_ADDON 0
  #endif

  #define ACTIVE_ALARM_SENSORS (ALARM_COUNT_MOTION + ALARM_COUNT_DISTANCE + ALARM_COUNT_ADDON)
#endif

// Light sensor presence (drives display dimming). Gas sensor (MQ135) excluded on purpose:
// it has no influence on display, alarm or telemetry logic.
#if defined(SENSOR_LIGHT_BH1750) || defined(SENSOR_LIGHT_LDR) || \
    defined(SENSOR_COLOR_TCS34725) || defined(SENSOR_LIGHT_VEML7700)
  #define HAS_LIGHT_SENSOR
#endif

// Wake-up addon presence (drives display wake behavior)
#if defined(ADDON_PIR_AM312) || defined(ADDON_RADAR_RCWL0516) || \
    defined(ADDON_LASER_VL53L0X) || defined(ADDON_LASER_VL53L1X)
  #define HAS_WAKEUP_ADDON
#endif

#if defined(HAS_WAKEUP_ADDON) && !defined(MODULE_DISPLAY_ACTIVE)
#error "BUILD BLOCKED: An addon requires a display to be selected in config.h."
#endif

// Automatic display profile assignment
#ifdef MODULE_DISPLAY_ACTIVE
  #if defined(HAS_WAKEUP_ADDON) && defined(HAS_LIGHT_SENSOR)
    #define DISPLAY_MODE_WAKE_AND_DIM
  #elif defined(HAS_WAKEUP_ADDON) && !defined(HAS_LIGHT_SENSOR)
    #define DISPLAY_MODE_WAKE_ONLY
  #elif !defined(HAS_WAKEUP_ADDON) && defined(HAS_LIGHT_SENSOR)
    #define DISPLAY_MODE_ALWAYS_ON_DIM
  #else
    #define DISPLAY_MODE_ALWAYS_ON_FIXED
  #endif
#endif