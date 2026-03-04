<h2 align="center">ESP32 Pin Connections</h2>

This document describes the wiring used in the **IoT Soldier Health Monitoring System** prototype.

The ESP32 acts as the central controller and interfaces with multiple sensors, a GPS module, and a local LCD display.

---

## Sensor Connections

### DHT11 – Temperature and Humidity Sensor

| DHT11 Pin | ESP32 Pin |
| --------- | --------- |
| VCC       | 5V        |
| GND       | GND       |
| DATA      | GPIO 4    |

---

### MAX30100 – Heart Rate and SpO₂ Sensor (I2C)

| MAX30100 Pin | ESP32 Pin |
| ------------ | --------- |
| VCC          | 3.3V      |
| GND          | GND       |
| SDA          | GPIO 22   |
| SCL          | GPIO 21   |

*Note: SDA and SCL pins are intentionally swapped compared to the typical ESP32 default configuration.*

---

### 16×2 LCD Display (I2C)

| LCD Pin | ESP32 Pin |
| ------- | --------- |
| VCC     | 5V        |
| GND     | GND       |
| SDA     | GPIO 22   |
| SCL     | GPIO 21   |

The LCD shares the same I2C bus as the MAX30100 sensor.

---

### NEO-6M GPS Module

| GPS Pin | ESP32 Pin |
| ------- | --------- |
| VCC     | 5V        |
| GND     | GND       |
| TX      | GPIO 25   |
| RX      | GPIO 26   |

The GPS communicates with the ESP32 using a UART interface.

---

### SOS Button

| Button Connection | ESP32 Pin |
| ----------------- | --------- |
| Button Output     | GPIO 15   |
| Other Terminal    | GND       |

Pressing the button triggers a **manual SOS alert** handled by the firmware.

---

## Power Architecture

The system is powered using a **12V 2A DC adapter** connected to a **buck converter**, which steps the voltage down to **5V** for sensors and modules.

During development and testing, the **ESP32 is powered separately through USB**, allowing both power supply and serial communication with the monitoring application.

---

## System Communication Summary

Sensors → ESP32 → Bluetooth → Python Monitoring Application

The ESP32 collects sensor readings, processes threshold conditions, and transmits monitoring data to the Python GUI.
