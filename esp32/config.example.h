#ifndef CONFIG_H
#define CONFIG_H

// #######################################################
// 1. WI-FI CREDENTIALS (Multi-Network)
// #######################################################
// Enter your credentials here and rename this file to config.h
// NOTE: if you only have 1 or 2 Wi-Fi networks, just enter the same credentials in the remaining fields.
const char* ssid_home    = "YOUR_HOME_SSID";
const char* pass_home    = "YOUR_HOME_PASSWORD";

const char* ssid_office  = "YOUR_OFFICE_SSID";
const char* pass_office  = "YOUR_OFFICE_PASSWORD";

const char* ssid_hotspot = "YOUR_HOTSPOT_SSID";
const char* pass_hotspot = "YOUR_HOTSPOT_PASSWORD";

// #######################################################
// 2. LANGUAGE SELECTION
// #######################################################
// Uncomment ONLY one language
#define LANG_IT
//#define LANG_EN
//#define LANG_ES
//#define LANG_DE
//#define LANG_FR

// #######################################################
// 3. HARDWARE SELECTION 
// #######################################################
// Uncomment ONLY connected hardware

// --- Display (Uncomment ONLY ONE) ---
#define DISPLAY_LCD_16X2
//#define DISPLAY_LCD_20X4
//#define DISPLAY_OLED_SSD1306
//#define DISPLAY_OLED_SH1106
//#define DISPLAY_TFT_ST7789
//#define DISPLAY_TFT_ILI9341

// --- Display Addons (Uncomment ONLY ONE) ---
//#define ADDON_IR_TE174            // IR Obstacle / Proximity sensor (Digital HIGH/LOW)
#define ADDON_LASER_VL53L0X       // Time-of-Flight Laser sensor (I2C 3.3V)
//#define ADDON_PIR_HCSR501         // Passive Infrared (5V VCC, 3.3V Native Out)

// --- SENSORS

// --- Climate sensor (Uncomment ONLY ONE) ---
//#define SENSOR_DHT11              // Legacy Temp + Humidity
//#define SENSOR_DHT22              // Legacy Temp + Humidity
#define SENSOR_BME280             // Temp + Humidity + Pressure (I2C 3.3V)
//#define SENSOR_BMP280             // Temp + Pressure ONLY (I2C 3.3V)
//#define SENSOR_SHT31              // High Precision Temp + Humidity (I2C 3.3V)
//#define SENSOR_AHT20              // Compact & Precise Temp + Humidity (I2C 3.3V)
//#define SENSOR_DS18B20            // Waterproof Temp sensor (OneWire)

// --- Motion sensor (Uncomment ONLY ONE) ---
#define SENSOR_PIR_HCSR501        // Passive Infrared (5V VCC, 3.3V Native Out)
//#define SENSOR_RADAR_RCWL         // Microwave Radar (detects through plastic)
//#define SENSOR_MMWAVE_LD2410      // Human Static Presence Radar (UART)
//#define SENSOR_IR_TE174           // IR Obstacle / Proximity sensor (Digital HIGH/LOW)

// --- Distance sensor (Uncomment ONLY ONE) ---
//#define SENSOR_ULTRASONIC_HCSR04  // Standard Ultrasonic (REQUIRES 5V Logic Conversion)
#define SENSOR_ULTRASONIC_US100   // Dual-mode Ultrasonic (3.3V Native - UART or Trigger/Echo)
//#define SENSOR_LASER_VL53L0X      // Time-of-Flight Laser sensor (I2C 3.3V)

// --- Ambient sensor (Uncomment ONLY ONE) ---
//#define SENSOR_LIGHT_BH1750       // Lux meter (I2C 3.3V)
//#define SENSOR_LIGHT_LDR          // Basic Photoresistor (Analog 3.3V)
#define SENSOR_COLOR_TCS34725     // RGB Color & Ambient Light sensor (I2C 3.3V)
//#define SENSOR_GAS_MQ135          // Air Quality / Gas sensor (Analog)

// #######################################################
// 4. PIN MAPPING (Defaults for Waveshare ESP32-C3-Zero)
// #######################################################
#define PIN_I2C_SDA      8        // SDA pin for I2C Displays & I2C Sensors
#define PIN_I2C_SCL      9        // SCL pin for I2C Displays & I2C Sensors

#define PIN_DHT          4        // Data pin for DHT11/DHT22/DS18B20
#define PIN_MOTION       5        // Signal pin for PIR/Radar
#define PIN_TRIG         6        // Trigger pin for Ultrasonic sensor
#define PIN_ECHO         7        // Echo pin for Ultrasonic sensor
#define PIN_ANALOG       1        // Analog input (LDR / MQ135)

// #######################################################
// 5. URL, TOKEN, SECTRET, PASSWORD, ID, ecc.
// #######################################################

// Telegram Config
#define TELEGRAM_TOKEN "YOUR_BOT_TOKEN"
#define TELEGRAM_CHAT_ID   "YOUR_CHAT_ID"

// Supabase Config
#define SUPABASE_URL       "https://your-project.supabase.co"
#define SUPABASE_KEY       "your_publishable_key"

// Heartbeat / Blackout Monitor Config
#define HEARTBEAT_URL      "https://api.cron-job.org/ping-or-your-endpoint"

#endif