<p align="center">
  <img src="https://capsule-render.vercel.app/api?type=waving&color=0:ff0080,100:7928ca&fontColor=00f5ff&height=100&section=header&text=DOM-US%20PROJECT&fontSize=50"
  alt="DOM-US project" />
</p>

<div align="center">

A lightweight, serverless smart home IoT ecosystem built with ESP32-C3 mini and Supabase.

Architecture & Smart Features

* Smart Power Outage Detection: The ESP32-C3 constantly sends timestamped pings to the Supabase SQL database. After a blackout, the device reboots, compares the current time with the last saved ping, and sends an alert via a Telegram Bot stating exactly how long the power was lost.
* Offline Sensor Data Buffering:** Continuous environmental logging. Data is buffered offline if Wi-Fi drops and synced to the cloud upon reconnection.
* Web Dashboard & Alarm: Real-time data visualization via interactive charts and integrated security alarm logic.

</div>

<p align="center">
  <img src="img/splash.png" alt="XYZ" width="600">
</p>

<p align="center">
  <strong>Developed on Raspberry Pi 400 | <s>Hosted on</s> Serverless | Uptime: running since ---</strong>
</p>

<br><br>

<p align="center">
  <img src="https://capsule-render.vercel.app/api?type=rect&animation=blinking&color=00000000&fontColor=FF0000&height=60&section=header&text=***WORK%20IN%20PROGRESS***&fontSize=32"
  alt="WORK IN PROGRESS" />
</p>

<br><br>

<table align="center" cellspacing="0" cellpadding="0">
  <tr>
    <td>
      <a href="./gallery/">
        <img src="https://capsule-render.vercel.app/api?type=pulse&height=750&width=750&color=7928ca&text=go%20to%20GALLERY&fontSize=75&fontColor=00f5ff" width="250">
      </a>
    </td>
    <td>
      <a href="./esp32/">
        <img src="https://capsule-render.vercel.app/api?type=pulse&height=750&width=750&color=7928ca&text=go%20to%20ESP32&fontSize=75&fontColor=00f5ff" width="250">
      </a>
    </td>
  </tr>
</table>

<!-- 
HTML, supabase, github, raspberry, esp32, telegram, dashboard, sql, charts, iot, smarthome-iot, home-automation, supabase-postgresql-integration, raspberry-pi-400-project, domotic, arduino-ide, data-logger, embedded-cpp, iot-dashboard, power-outage-monitoring, security-alarm, smart-home, telegram-bot, esp32-c3, esp32-c3-zero
-->