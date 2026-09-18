<p align="center">
  <img src="https://capsule-render.vercel.app/api?type=waving&color=0:ff0080,100:7928ca&fontColor=00f5ff&height=100&section=header&text=DIAGRAMS&fontSize=50"
  alt="DIAGRAMS" />
</p>

<div align="center">

Logic Diagrams, Schematics and Wiring

</div>

{% raw %}
```text

****************************************
GENERAL SCHEMA
****************************************

                            +-----------------------+
                            |       ESP32-C3        |
                            |     BOOT / SETUP      |
                            +-----------+-----------+
                                        |
                                        v
                            +------------------------+
                            |   WIFI STATE MACHINE   |
                            |  HOME->OFFICE->HOTSPOT |
                            +-----------+------------+
                                        |
                           WIFI_CONNECTED
                                        |
              +-------------------------+-------------------------+
              |                         |                         |
              v                         v                         v
  +-----------------------+ +-----------------------+ +-----------------------+
  |       TELEMETRY       | |         ALARM         | |  BLACKOUT/CONNECTION  |
  |    temp/hum/press     | |    motion/distance    | |    heartbeat/ping     |
  +-----------------------+ +-----------+-----------+ +-----------+-----------+
               |                        |                         |
               v                        v                         v
  +-----------------------+ +-----------------------+ +-----------------------+
  |       SUPABASE        | |       TELEGRAM        | |       SUPABASE        |
  |   sensor_data table   | |     notifications     | |  device_status table  |
  +-----------------------+ +-----------------------+ +-----------------------+
                                        |
                                        v
                            +-----------------------+
                            |        DISPLAY        |
                            |  LCD/OLED/TFT status  |
                            +-----------------------+



****************************************
DETAIL SCHEMA - TELEMETRY
****************************************

  +----------------+     every minute **:05     +-----------------------+
  |  loop() timer  +--------------------------->|   checkHourlyTask()   |
  +----------------+                            +-----------+-----------+
                                                            |
                                                            v
                                                +-----------------------+
                                                |   readTemperature()   |
                                                |    readHumidity()     |
                                                |    readPressure()     |
                                                +-----------+-----------+
                                                            |
                                                            v
                                                +-----------------------+
                                                |  saveTelemetryData()  |
                                                +-----------+-----------+
                                                            |
                                            +---------------+---------------+
                                            |                               |
                               WiFi CONNECTED                               WiFi NOT AVAILABLE
                                            |                               |
                                            v                               v
                               +-------------------------+     +-------------------------+
                               |    flushRamBuffer()     |     | ramBuffer[] (72 slots)  |
                               |    flushLogBuffer()     |     | bufferCount++           |
                               |    sendToSupabase()     |     +-------------------------+
                               +-------------------------+                  |
                                                                            v
                                                                     on WiFi restore:        
                                                                 flushRamBuffer() sends
                                                               the whole buffered backlog

 PARALLEL TASK (independent, every 60s or on display wake-up):
 +---------------------------+     +---------------------------+ 
 | checkClimateDisplayTask() +---->|  refreshClimateDisplay()  |
 +---------------------------+     +---------------------------+
                                                 |
                                                 alarmEnabled? ---YES---> no refresh (stealth)
                                                 |
                                              NO |
                                                 |
                                                 v
                                                 |
                                                 isDisplayActive()? ---NO---> no refresh
                                                 |
                                             YES |
                                                 |
                                                 v
                                           showMessage()



****************************************
DETAIL SCHEMA - ALARM
****************************************

                          +---------------------------+
                          |    checkAlarmSystem()     |
                          +-------------+-------------+
                                        |
                       +----------------+----------------+
                       |                                 |
    alarmEnabled = false                                 alarmEnabled = true
                       |                                 |
                       v                                 v
         +---------------------------+     +---------------------------+
         |    STATE_IDLE forced      |     |      alarmTriggered?      |---YES---> see RESET branch below
         |   log NOTICE if trigger   |     +-------------+-------------+
         +---------------------------+                   |
                                                         NO
                                                         |
                                                         v
                                         +-------------------------------+
                                         |  ACTIVE_ALARM_SENSORS count   |
                                         |   trigMotion / trigDistance   |
                                         |  trigAddon (if alarmEnabled)  |
                                         +---------------+---------------+
                                                         |
                                       +-----------------+-----------------+
                                       |                                   |
                                       count <= 1                 count >= 2
                                       |                                   |
                                       v                                   v
                        +-----------------------------+     +-----------------------------+
                        |         STATE_IDLE          |     |  STATE_IDLE                 |
                        |  trigger --> ALARM at once  |     | trigger --> STATE_PRE_ALARM |
                        |    alarmTriggered = true    |     |  preAlarmStartTime = now    |
                        |       log + Telegram        |     +--------------+--------------+
                        +-----------------------------+                    |
                                       |                                   v
                                       |                    +-----------------------------+
                                       |                    |       STATE_PRE_ALARM       |
                        +--------------+                    |       within 5000 ms:       |
                        |                                   +--------------+--------------+
                        |                                                  |
                        |                                +-----------------+-----------------+
                        |                                |                                   |
                        |             2 triggers confirmed                                   5s timeout without confirmation
                        |                                |                                   |
                        |                                v                                   v
                        |                 +-----------------------------+     +-----------------------------+
                        |                 |         STATE_IDLE          |     |         STATE_IDLE          |
                        |                 |       ALARM confirmed       |     |         log SECURED         |
                        |                 |       log + Telegram        |     |          no alarm           |
                        |                 +-----------------------------+     +-----------------------------+
                        |                                |
                        +--------------------------------+
                                                         |
                                                         v
                                          +-----------------------------+
                                          |    alarmTriggered = true    |
                                          |        RESET BRANCH         |
                                          +--------------+--------------+
                                                         |
                                          +--------------+--------------+
                                          |                             |
                            Telegram /reset                             auto-reset timeout elapsed
                                          |                             |
                                          v                             v
                                     +---------------------------------------+
                                     |        alarmTriggered = false         |
                                     | alarmEnabled unchanged (stays armed)  |
                                     |    log RESET + Telegram (optional)    |
                                     +---------------------------------------+
                                                         |
                                                         v
                                        back to STATE_IDLE, sensors re-armed



****************************************
DETAIL SCHEMA - BLACKOUT / CONNECTION
****************************************

                        +-----------------------------+
                        |       WIFI_CONNECTED        |
                        +-----------------------------+
                                       |
                                       v
                        +-----------------------------+
                        |          syncNtp()          |
                        +-----------------------------+
                                       |
                                       v
                        +-----------------------------+
                        |    handleBootSequence()     |
                        +-----------------------------+
                                       |
                                       v
                        +-----------------------------+
                        | GET device_status.last_ping |
                        +--------------+--------------+
                                       |
                                       v
                        +-----------------------------+
                        |  diffSec = now - last_ping  |
                        +--------------+--------------+
                                       |
                   +-------------------+-------------------+
                   |                                       |
                   diffSec <= 600              diffSec > 600
                   |                                       |
                   v                                       v
  +---------------------------------+     +---------------------------------+
  |           no blackout           |     |   compute days/hours/minutes    |
  +---------------------------------+     | sendLogToSupabase() severity R  |
                                          | sendTelegramMessage() BLACKOUT  |
                                          +---------------------------------+

  CONTINUOUS LOOP (main loop, every 60s):
  +-------------------------+
  |     sendHeartbeat()     |
  |   PATCH last_ping=now   |
  +-------------------------+

  ON WIFI DROP:
  +-------------------------+                     +-------------------------+
  |     WIFI_CONNECTED      +-------------------->|        WIFI_FAIL        |
  |    st != WL_CONNECTED   |                     |  retry every 30000 ms   |
  +-------------------------+                     +-------------------------+
                                                               |
                                                               v
                                                  +-------------------------+
                                                  |     wifiStart(home)     |
                                                  | -> WIFI_CONNECTING_HOME |
                                                  +-------------------------+

  OFFLINE LOG BUFFER (parallel to blackout):
  +-------------------------+     WiFi absent     +-------------------------+
  |   sendLogToSupabase()   +-------------------->| logBuffer[] (500 slots) |
  +-------------------------+                     +-------------------------+
             |
             | WiFi present
             |
             v
  +-------------------------+
  |    flushLogBuffer()     |
  |    bulk send backlog    |
  +-------------------------+



****************************************
WIRING COLOR LEGEND
****************************************

╔══════════════════════════════════════════════════════════════════╦═════════════╗
║ USE / CATEGORY                                                   ║ CABLE COLOR ║
╠══════════════════════════════════════════════════════════════════╬═════════════╣
║ POWER LINES                                                      ║             ║
║ Positive Power Supply (VCC / 3.3V / 5V)                          ║         Red ║ 🟥
║ Ground / GND                                                     ║       Black ║ ⬛
╠══════════════════════════════════════════════════════════════════╬═════════════╣
║ SIGNAL LINES                                                     ║             ║
║ Clock signals (SCL / SCK) or PWM signals                         ║      Yellow ║ 🟨
╠══════════════════════════════════════════════════════════════════╬═════════════╣
║ DATA LINES                                                       ║             ║
║ Data lines (SDA / MOSI) or analog signals                        ║       Green ║ 🟩
║ Receive data lines (RX) or negative power lines                  ║        Blue ║ 🟦
║ Transmit data lines (TX) or reset                                ║      Orange ║ 🟧
╠══════════════════════════════════════════════════════════════════╬═════════════╣
║ GENERAL PURPOSE LINES                                            ║             ║
║ Generic data lines (GPIO) or enable/chip select pins (Enable/CS) ║       White ║ ⬜
║ Generic data lines (GPIO) or enable/chip select pins (Enable/CS) ║       Brown ║ 🟫
║ Generic data lines (GPIO) or enable/chip select pins (Enable/CS) ║      Purple ║ 🟪
║ Generic data lines (GPIO) or enable/chip select pins (Enable/CS) ║        Gray ║ ⚫
╚══════════════════════════════════════════════════════════════════╩═════════════╝



****************************************
PLUG/JACK WIRING
****************************************

╔══════════════════════════════════════════╦══════════╦══════════════════╗
║ FUNCTION                                 ║ RJ45 PIN ║ T568B COLOR      ║
╠══════════════════════════════════════════╬══════════╬══════════════════╣
║ Positive Power Supply (VCC 3.3V)         ║ Pin 1    ║ White-Orange     ║ ⬜🟧
║ I2C Data Bus (SDA) - BME280 / VEML7700   ║ Pin 2    ║ Orange           ║ 🟧🟧
║ Ground Reference (GND)                   ║ Pin 3    ║ White-Green      ║ ⬜🟩
║ Serial Receive Line (RX) - US-100        ║ Pin 4    ║ Blue             ║ 🟦🟦
║ Serial Transmit Line (TX) - US-100       ║ Pin 5    ║ White-Blue       ║ ⬜🟦
║ I2C Clock Bus (SCL) - BME280 / VEML7700  ║ Pin 6    ║ Green            ║ 🟩🟩
║ Digital PIR Signal (OUT) - AM312         ║ Pin 7    ║ White-Brown      ║ ⬜🟫
║ Dedicated PIR Ground (GND) - AM312       ║ Pin 8    ║ Brown            ║ 🟫🟫
╚══════════════════════════════════════════╩══════════╩══════════════════╝



****************************************
CABLE PAIRS SCHEME
****************************************

 PAIR 1
 ⬜🟦 Serial Transmit Line (TX) - US-100
 🟦🟦 Serial Receive Line (RX) - US-100

 PAIR 2
 ⬜🟧 Positive Power Supply (VCC 3.3V)
 🟧🟧 I2C Data Bus (SDA) - BME280 / VEML7700

 PAIR 3
 ⬜🟩 Ground Reference (GND)
 🟩🟩 I2C Clock Bus (SCL) - BME280 / VEML7700

 PAIR 4
 ⬜🟫 Digital PIR Signal (OUT) - AM312
 🟫🟫 Dedicated PIR Ground (GND) - AM312

```
{% endraw %}

<!-- 
HTML, supabase, github, raspberry, esp32, telegram, dashboard, sql, charts, iot, smarthome-iot, home-automation, supabase-postgresql-integration, raspberry-pi-400-project, domotic, arduino-ide, data-logger, embedded-cpp, iot-dashboard, power-outage-monitoring, security-alarm, smart-home, telegram-bot, esp32-c3, esp32-c3-zero
-->