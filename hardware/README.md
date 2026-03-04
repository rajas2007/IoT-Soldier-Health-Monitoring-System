<h2 align="center">Hardware Design</h2>

<p align="center">
Hardware architecture and component integration for the IoT Soldier Health Monitoring System.
</p>

<br>

## Overview

This directory documents the hardware components and system architecture used in the IoT Soldier Health Monitoring System.

The hardware platform is built around an **ESP32 microcontroller**, which collects health and environmental data from multiple sensors and transmits the information to a Python monitoring application. The system also includes local display capability and emergency alert mechanisms.

<br>

## Main Controller

**ESP32 Development Board**

The ESP32 serves as the central processing unit of the system. It performs the following functions:

* Reads sensor data from connected modules
* Processes threshold conditions for emergency alerts
* Displays readings on the LCD display
* Communicates with the Python application through Bluetooth
* Handles manual and automatic SOS triggers

<br>

## Sensors and Modules

### DHT11 Temperature and Humidity Sensor

Used to monitor environmental conditions surrounding the soldier. The sensor periodically provides temperature and humidity readings.

### MAX30100 Heart Rate and SpO₂ Sensor

Measures physiological health parameters including:

* Heart rate
* Blood oxygen saturation (SpO₂)

These parameters help determine the soldier's physical condition.

### NEO-6M GPS Module

Provides real-time location data. The GPS coordinates are transmitted along with sensor readings so that the monitored individual can be located during emergency situations.

### 16×2 LCD Display (I2C Interface)

Displays sensor values locally on the device. The firmware cycles through different readings so that users can monitor the system status directly from the hardware unit.

### SIM800L GSM Module

The GSM module is integrated as a **backup communication option** for transmitting alerts remotely.

During development and testing, Bluetooth communication between the ESP32 and the Python monitoring application was used as the primary communication method. The SIM800L module remains part of the hardware design to demonstrate the possibility of future GSM-based alert transmission.

<br>

## SOS Trigger Mechanisms

The system supports three independent methods for triggering an emergency alert:

1. **Software Trigger** – SOS initiated from the Python monitoring application
2. **Hardware Trigger** – SOS triggered using a physical button connected to the ESP32
3. **Automatic Trigger** – SOS automatically activated when sensor readings exceed predefined threshold limits

When an SOS event occurs, the system immediately transmits all sensor readings and displays the alert status on the LCD screen.

<br>

## Power System

The hardware system is powered using a **12V 2A DC adapter** connected to a **buck converter**.

The buck converter steps the voltage down from **12V to 5V**, which is used to power the sensors and peripheral modules.

During development and testing, the ESP32 board is powered separately using a **USB data connection**, allowing both power supply and communication with the monitoring application.

<br>

## System Integration

All sensors and modules are connected to the ESP32 and operate together as a unified monitoring system. The ESP32 collects data from the sensors, processes it, displays key readings locally, and transmits the information to the monitoring application for visualization and alert management.
