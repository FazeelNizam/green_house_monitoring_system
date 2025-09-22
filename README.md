# Multi-Unit Environmental and Agricultural Monitoring System

A comprehensive, distributed monitoring system for collecting high-resolution environmental, atmospheric, and agricultural data.  
The system is composed of three distinct ESP32-based units that collaborate to measure everything from soil moisture and conductivity to air quality and meteorological conditions.

The architecture is designed for modularity, allowing sensor units to be placed strategically in the field.  
Data is aggregated wirelessly and logged, ready for processing by a central hub like a **Raspberry Pi Zero** for visualization on a dashboard.

---

## Table of Contents
- [System Architecture](#system-architecture)
- [Key Features](#key-features)
- [Hardware & Components](#hardware--components)
- [Unit 1: Environmental Sensor Hub](#unit-1-environmental-sensor-hub)
- [Unit 2: Soil & Data Aggregator](#unit-2-soil--data-aggregator)
- [Unit 3: ATMOS 41 Weather Station](#unit-3-atmos-41-weather-station)
- [Data Output Format](#data-output-format)

---

## System Architecture
The system operates on a **hub-and-spoke model**.

- The **Environmental Sensor Hub** acts as a standalone Wi-Fi Access Point, serving its data as a JSON payload.
- The **Soil & Data Aggregator** connects to this hub as a client, fetches the environmental data, combines it with its own local soil readings, and outputs the aggregated data via its serial port.
- The **ATMOS 41 Weather Station** works independently, logging detailed meteorological data to an SD card.

### Data Flow Diagram
```
[ Environmental Hub ]      <--- (HTTP GET Request) ---     [ Soil & Data Aggregator ]      ---> (Serial CSV Data) ---> [ Raspberry Pi Zero ] ---> [ Dashboard ]
  (Wi-Fi AP)                                                      (Wi-Fi Client)
  | SHT31 Temp/Humidity                                          | 6x TEROS 10
  | MH-Z19C CO2                                                    | 6x TEROS 22
  | MQ-4 Methane                                                   | DS3231 RTC
  | Pyranometer (Solar)


[ ATMOS 41 Weather Station ] ---> (Logs to atmos_data.csv) ---> [ SD Card ] ---> (Manual/Pi Upload) ---> [ Dashboard ]
  | ATMOS 41 Sensor
  | DS3231 RTC
```

---

## Key Features
- **Comprehensive Data Collection**: Measures over 20 distinct parameters, including soil VWC, temperature, EC, air temperature, humidity, pressure, solar radiation, CO2, and CH4.
- **Distributed & Modular**: Place sensor units where they are most needed for accurate readings.
- **Wireless Data Aggregation**: Uses Wi-Fi to consolidate data from multiple units, reducing complex wiring.
- **Robust Logging**: Features both real-time serial data output for live monitoring and onboard SD card logging for long-term, fail-safe data capture.
- **Accurate Timestamps**: Each data point is precisely timestamped using a DS3231 Real-Time Clock (RTC).
- **Standardized Data Formats**: Outputs data in clean, easy-to-parse CSV and JSON formats.

---

## Hardware & Components

| Component                | Environmental Hub | Soil Aggregator | Weather Station |
|---------------------------|-------------------|------------------|-----------------|
| **ESP32 Microcontroller** | ✅ | ✅ | ✅ |
| Adafruit SHT31            | ✅ |   |   |
| MH-Z19C (CO2)             | ✅ |   |   |
| MQ-4 (Methane)            | ✅ |   |   |
| Pyranometer               | ✅ |   |   |
| Adafruit ADS1115 ADC      | ✅ | ✅ |   |
| TEROS 10 Sensor           |   | ✅ |   |
| TEROS 22 Sensor           |   | ✅ |   |
| METER ATMOS 41 Sensor     |   |   | ✅ |
| DS3231 Real-Time Clock    |   | ✅ | ✅ |
| MicroSD Card Module       |   |   | ✅ |

---

## Unit 1: Environmental Sensor Hub
Measures ambient air quality and environmental data.  
It creates a Wi-Fi Access Point and serves sensor readings as a **JSON object** upon request.

- **Sensors**: SHT31 (Temp/Humidity), MH-Z14A (CO2), MQ-4 (Methane), SP Lite 02 Pyranometer (Solar Radiation)

### Finished Circuit & Connections
This image shows the final assembled circuit for the Environmental Hub.

![Environmental Hub Finished Circuit](./images/environmental_hub_finished_circuit.jpg)

---

## Unit 2: Soil & Data Aggregator
Gathers soil data and aggregates it with data from the Environmental Hub.  
Outputs a combined **CSV string** to its serial port.
 
- **Sensors**: 6x TEROS 10 (VWC), 6x TEROS 22 (VWC, Temp, EC), DS3231 RTC

### Finished Circuit & Connections
![Soil Aggregator Finished Circuit](https://github.com/FazeelNizam/green_house_monitoring_system/blob/main/Images/Logger%20Circuit.jpg)

---

## Unit 3: ATMOS 41 Weather Station
A standalone, all-in-one weather station that logs detailed meteorological data directly to an **SD card**.

- **Sensors**: METER ATMOS 41, DS3231 RTC

### Finished Circuit & Connections
![ATMOS 41 Finished Circuit](https://github.com/FazeelNizam/green_house_monitoring_system/blob/main/Images/Atmos41%20Connected%20To%20Logger.jpg)

---

## Data Output Format

### Soil & Data Aggregator (Serial Output)
The aggregator outputs a single line of **CSV data** every 15 seconds.  
`NaN` is used for failed or out-of-range readings.

**Header Example:**
```csv
DateTime,T10_1_VWC,T10_2_VWC,T10_3_VWC,T10_4_VWC,T10_5_VWC,T10_6_VWC,T22_1_VWC,T22_1_Temp,T22_1_EC,T22_2_VWC,T22_2_Temp,T22_2_EC,T22_3_VWC,T22_3_Temp,T22_3_EC,T22_4_VWC,T22_4_Temp,T22_4_EC,T22_5_VWC,T22_5_Temp,T22_5_EC,T22_6_VWC,T22_6_Temp,T22_6_EC,Ext_Temp,Ext_Humid,Ext_CO2,Ext_SolarRad,Ext_CH4
```

### ATMOS 41 Weather Station (SD Card Log)
The weather station appends a new line of **CSV data** to `atmos_data.csv` on the SD card every 10 seconds.

**Header Example:**
```csv
Timestamp,Solar_Rad,Precip,Lightning_Count,Lightning_Dist,Wind_Speed,Wind_Dir,Gust_Speed,Air_Temp,Pressure,Rel_Humid,EC,Tilt_Angle,Drops,Spoons,RH_Temp,xTilt,yTilt
```

---
