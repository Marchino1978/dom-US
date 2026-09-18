<p align="center">
  <img src="https://capsule-render.vercel.app/api?type=waving&color=0:ff0080,100:7928ca&fontColor=00f5ff&height=100&section=header&text=DIAGRAMS&fontSize=50"
  alt="DIAGRAMS" />
</p>

<div align="center">

Logic Diagrams, Schematics and Wiring

</div>

<style>
  /* Forza la pagina a usare più spazio orizzontale per i diagrammi */
  .page-content, .wrapper, main, article {
    max-width: 1200px !important;
    width: 95% !important;
  }
  /* Rimpicciolisce leggermente il font dell'ASCII art per farlo stare negli schermi più piccoli */
  pre code {
    font-size: 13px !important;
    white-space: pre !important;
    overflow-x: auto !important;
  }
</style>

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

```
{% endraw %}

<!-- 
HTML, supabase, github, raspberry, esp32, telegram, dashboard, sql, charts, iot, smarthome-iot, home-automation, supabase-postgresql-integration, raspberry-pi-400-project, domotic, arduino-ide, data-logger, embedded-cpp, iot-dashboard, power-outage-monitoring, security-alarm, smart-home, telegram-bot, esp32-c3, esp32-c3-zero
-->