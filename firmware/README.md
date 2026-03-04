<h2 align="center">ESP32 Firmware</h2>

<p align="center">
Embedded firmware for sensor acquisition, system monitoring, and emergency alert handling in the IoT Soldier Health Monitoring System.
</p>

<br>

## Overview

This directory contains the firmware running on the **ESP32 microcontroller**, which acts as the central controller of the IoT Soldier Health Monitoring System.

The firmware is responsible for collecting sensor data, displaying system information locally, handling emergency alerts, and transmitting data to the Python monitoring application using Bluetooth communication.

<br>

## Core Responsibilities

* Acquire sensor readings at fixed intervals
* Display system readings on the **16×2 I2C LCD**
* Transmit data to the Python monitoring application via **Bluetooth**
* Monitor sensor values against predefined safety thresholds
* Detect manual and automatic **SOS triggers**
* Manage system status and emergency alerts

<br>

## Hardware Interfaces

### Microcontroller

* ESP32 development board

### Connected Sensors and Modules

* **DHT11** – temperature and humidity monitoring
* **MAX30100** – heart rate and SpO₂ measurement
* **NEO-6M GPS** – real-time location tracking
* **16×2 I2C LCD display** – local monitoring of system data
* **SIM800L GSM module** – integrated as a backup communication module
* **Physical SOS button** – manual emergency trigger connected to ESP32

<br>

## Firmware Functionalities

### Sensor Data Acquisition

The ESP32 periodically reads environmental and physiological data from the connected sensors. These readings include temperature, humidity, heart rate, and blood oxygen saturation.

### GPS Location Tracking

Location coordinates are obtained from the **NEO-6M GPS module** and included in the transmitted monitoring data.

### Local Display System

Sensor readings are cycled and displayed on the **16×2 I2C LCD display**, allowing users to monitor system status directly from the device.

### Bluetooth Communication

The ESP32 transmits collected data to the **Python desktop monitoring application** using its built-in Bluetooth capability.

### Emergency Alert System

The firmware supports three independent SOS trigger mechanisms:

1. **Application Trigger** – SOS initiated from the Python monitoring application
2. **Hardware Trigger** – SOS trigger
