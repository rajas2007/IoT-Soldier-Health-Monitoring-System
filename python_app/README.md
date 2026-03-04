<h2 align="center">Python Monitoring Application</h2>

<p align="center">
Desktop application for real-time monitoring, system interaction, and emergency alert handling in the IoT Soldier Health Monitoring System.
</p>

<br>

## Overview

This directory contains the **Python-based monitoring application** used to interact with the ESP32 device.
The application receives sensor data via Bluetooth, displays real-time readings, and provides control features for triggering emergency alerts and viewing system thresholds.

The application acts as the **user interface layer** of the system, allowing operators to monitor health parameters and respond quickly to abnormal conditions.

<br>

## Core Features

* Receive real-time sensor data from the ESP32
* Display current health and environmental readings
* Provide manual **SOS alert triggering**
* Display configured sensor threshold values
* Provide help and usage guidance for system operators

<br>

## Graphical User Interface

The application contains four primary control buttons:

### Current Readings

Displays the latest sensor values received from the ESP32, including temperature, humidity, heart rate, SpO₂, and GPS location.

### Help

Provides instructions on how to use the monitoring system and explains the functionality of each control option.

### Threshold Values

Displays the predefined safety limits for each sensor parameter used for automatic SOS detection.

### Trigger SOS

Allows the user to manually activate an emergency alert.
When triggered, the system immediately sends all available sensor data and displays the alert on both the monitoring application and the device LCD.

<br>

## Communication with ESP32

The Python application communicates with the ESP32 using **Bluetooth serial communication**.

The ESP32 periodically transmits sensor readings to the application, where they are processed and displayed in the interface.

Data transmitted includes:

* Temperature
* Humidity
* Heart rate
* Blood oxygen saturation (SpO₂)
* GPS location coordinates

<br>

## SOS Alert Handling

The monitoring application works together with the ESP32 firmware to support multiple emergency alert triggers:

1. **Application Trigger** – SOS initiated directly from the GUI
2. **Hardware Trigger** – SOS activated using the physical button connected to the ESP32
3. **Automatic Trigger** – SOS triggered when sensor readings exceed predefined thresholds

When an SOS event occurs, the application displays the alert and shows the complete set of current sensor readings.

<br>

## Role in the System

The Python monitoring application serves as the **central monitoring interface**, enabling users to view real-time health data, manage alerts, and interact with the IoT device during operation.

<br>

## Running the Application

1. Install Python 3
2. Install dependencies:

pip install -r requirements.txt

3. Run the program:

python health_monitor_gui.py
