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
- [Software & Setup Guide](#software--setup-guide)
- [Data Output Format](#data-output-format)
- [Repository Structure](#repository-structure)

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

- **Code**: [`/firmware/environmental_sensor_code.ino`](./firmware/environmental_sensor_code.ino)  
- **Sensors**: SHT31 (Temp/Humidity), MH-Z19C (CO2), MQ-4 (Methane), Pyranometer (Solar Radiation)

### Finished Circuit & Connections
This image shows the final assembled circuit for the Environmental Hub.

![Environmental Hub Finished Circuit](./images/environmental_hub_finished_circuit.jpg)

| Sensor | Interfacing Photo | Datasheet |
|-------|--------------------|-----------|
| **SHT31** | ![SHT31 Wiring](./images/sht31_wiring.jpg) | [SHT31 Datasheet](./datasheets/SHT31-datasheet.pdf) |
| **MH-Z19C** | ![MH-Z19C Wiring](./images/mhz19c_wiring.jpg) | [MH-Z19C Datasheet](./datasheets/MH-Z19C-datasheet.pdf) |
| **MQ-4** | ![MQ-4 Wiring](./images/mq4_wiring.jpg) | [MQ-4 Datasheet](./datasheets/MQ-4-datasheet.pdf) |
| **Pyranometer** | ![Pyranometer Wiring](./images/pyranometer_wiring.jpg) | [Pyranometer Datasheet](./datasheets/pyranometer-datasheet.pdf) |

---

## Unit 2: Soil & Data Aggregator
Gathers soil data and aggregates it with data from the Environmental Hub.  
Outputs a combined **CSV string** to its serial port.

- **Code**: [`/firmware/Teros10_and_22_sensor_code.ino`](./firmware/Teros10_and_22_sensor_code.ino)  
- **Sensors**: 6x TEROS 10 (VWC), 6x TEROS 22 (VWC, Temp, EC), DS3231 RTC

### Finished Circuit & Connections
![Soil Aggregator Finished Circuit](./images/soil_aggregator_finished_circuit.jpg)

| Sensor | Interfacing Photo | Datasheet |
|-------|--------------------|-----------|
| **TEROS 10** | ![TEROS 10 Wiring](./images/teros10_wiring.jpg) | [TEROS 10 Datasheet](./datasheets/TEROS-10-datasheet.pdf) |
| **TEROS 22** | ![TEROS 22 Wiring](./images/teros22_wiring.jpg) | [TEROS 22 Datasheet](./datasheets/TEROS-22-datasheet.pdf) |

---

## Unit 3: ATMOS 41 Weather Station
A standalone, all-in-one weather station that logs detailed meteorological data directly to an **SD card**.

- **Code**: [`/firmware/Atmos41_Gen_2_Weather_station_code.ino`](./firmware/Atmos41_Gen_2_Weather_station_code.ino)  
- **Sensors**: METER ATMOS 41, DS3231 RTC

### Finished Circuit & Connections
![ATMOS 41 Finished Circuit](./images/atmos41_finished_circuit.jpg)

| Sensor | Interfacing Photo | Datasheet |
|-------|--------------------|-----------|
| **ATMOS 41** | ![ATMOS 41 Wiring](./images/atmos41_wiring.jpg) | [ATMOS 41 Datasheet](./datasheets/ATMOS-41-datasheet.pdf) |

---

## Software & Setup Guide

### 1. Required Libraries
Install these libraries via the **Arduino IDE Library Manager** (`Sketch > Include Library > Manage Libraries...`).

- **Environmental Hub**:
  - Adafruit SHT31 Library
  - Adafruit ADS1X15
- **Soil & Data Aggregator**:
  - Adafruit ADS1X15
  - RTClib
  - ESP32-SDI12 by Manuel Haim
  - Arduino_JSON
- **ATMOS 41 Weather Station**:
  - SDI12 by EnviroDIY
  - RTClib
  - SD

### 2. Setup Instructions
1. **Assemble Hardware**: Connect all sensors and modules to their respective ESP32 units as shown in the images above and as defined by the pin constants in each code file.  
2. **Configure Code**:
   - **Environmental Hub**:  
     Default Wi-Fi → `SSID: Environment_Sensors`, `PASS: password123`  
     Calibrate the `RO_IN_CLEAN_AIR` value for your MQ-4 sensor by running it in clean air for several hours and updating the code.
   - **Soil Aggregator**:  
     Ensure the `ssid` and `password` variables match the credentials set in the Environmental Hub code.
3. **Upload Firmware**: Flash each `.ino` file to its corresponding ESP32 board using the Arduino IDE.
4. **Deploy System**:
   - Power on the **Environmental Hub** first.
   - Power on the **Soil & Data Aggregator**. Open the Serial Monitor at `115200` baud to verify it connects to the Hub and starts outputting CSV data.
   - Power on the **ATMOS 41 Weather Station**. It will automatically begin logging data to the SD card.
5. **Connect to Pi Zero**: Connect the Soil Aggregator via USB to your Raspberry Pi.  
   Use a Python script with the `pyserial` library to read the serial port and process the data.

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

## Repository Structure
```
.
├─ firmware/
│  ├─ environmental_sensor_code.ino
│  ├─ Teros10_and_22_sensor_code.ino
│  └─ Atmos41_Gen_2_Weather_station_code.ino
├─ images/
│  ├─ environmental_hub_finished_circuit.jpg
│  ├─ soil_aggregator_finished_circuit.jpg
│  ├─ atmos41_finished_circuit.jpg
│  ├─ sht31_wiring.jpg
│  ├─ mhz19c_wiring.jpg
│  ├─ mq4_wiring.jpg
│  ├─ pyranometer_wiring.jpg
│  ├─ teros10_wiring.jpg
│  ├─ teros22_wiring.jpg
│  └─ atmos41_wiring.jpg
├─ datasheets/
│  ├─ SHT31-datasheet.pdf
│  ├─ MH-Z19C-datasheet.pdf
│  ├─ MQ-4-datasheet.pdf
│  ├─ pyranometer-datasheet.pdf
│  ├─ TEROS-10-datasheet.pdf
│  ├─ TEROS-22-datasheet.pdf
│  └─ ATMOS-41-datasheet.pdf
└─ README.md
```
