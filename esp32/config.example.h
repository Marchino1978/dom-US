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

// --- Display Addons / Wake-up (Uncomment ONLY ONE) ---
//#define ADDON_PIR_AM312         // Mini Passive Infrared (Presence / Wake-up 3.3V)
//#define ADDON_RADAR_RCWL0516    // Doppler Microwave (Room Presence / Alarm 4V-28V)
#define ADDON_LASER_VL53L0X     // Laser ToF (2.8V - 5V / I2C)
//#define ADDON_LASER_VL53L1X       // Laser ToF (Short proximity + 4m Alarm 3.3V / I2C)

// --- SENSORS ---

// --- Climate sensor (Uncomment ONLY ONE) ---
//#define SENSOR_AHT20              // Temp / Hum (3.3V / I2C)
#define SENSOR_BME280             // Temp / Hum / Press (3.3V / I2C)
//#define SENSOR_BMP280             // Temp / Press (3.3V / I2C)
//#define SENSOR_DHT11              // Temp / Hum (3.3V / 5V)
//#define SENSOR_DHT22              // Temp / Hum (3.3V / 5V)
//#define SENSOR_DS18B20            // Temp (3.3V / 5V)
//#define SENSOR_SHT31              // Temp / Hum (3.3V / 5V / I2C)

// --- Motion & Presence sensor (Uncomment ONLY ONE) ---
//#define SENSOR_PIR_AM312          // Mini Passive Infrared (3.3V)
//#define SENSOR_PIR_HCSR501        // Passive Infrared (5V / 3.3V signal)
//#define SENSOR_MMWAVE_LD2410      // Micro-movements (5V)
#define SENSOR_RADAR_RCWL0516     // Doppler Microwave (4V - 28V)
//#define SENSOR_IR_TE174           // IR Beam (3.3V / 5V)

// --- Distance sensor (Uncomment ONLY ONE) ---
//#define SENSOR_ULTRASONIC_HCSR04  // Ultrasound (5V)
//#define SENSOR_ULTRASONIC_HCSR04P // Ultrasound (3.3V / 5V)
//#define SENSOR_ULTRASONIC_RCWL1601// Ultrasound (3V - 5.5V)
//#define SENSOR_ULTRASONIC_US100   // Ultrasound + Temp (UART / PWM 3.3V / 5V)
#define SENSOR_LASER_VL53L0X      // Laser ToF (2.8V - 5V / I2C)
//#define SENSOR_LASER_VL53L1X      // Laser ToF (Up to 4m 3.3V / I2C)

// --- Ambient sensor (Uncomment ONLY ONE) ---
//#define SENSOR_LIGHT_BH1750       // Lux Digital (3.3V / 5V / I2C)
//#define SENSOR_LIGHT_LDR          // Analog Photoresistor (3.3V / 5V)
//#define SENSOR_GAS_MQ135          // Air Quality (5V)
#define SENSOR_COLOR_TCS34725     // RGB + Color Temp (3.3V / 5V / I2C)
//define SENSOR_LIGHT_VEML7700     // Lux High precision / Human eye (3.3V / I2C)

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

#endif