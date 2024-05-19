# AQI-Sensor
Basic data logging AQI sensor
A basic sensor to log AQI/Dust in the air.

## Goal
The goal of this project was to create a dust sensor/AQI sensor that will be able to track the air quailty over time. 
I wanted to be able to capture the time of each measurment and take readings for a long time, to enable me to test air filters/just keep track.
I designed this around the Grove Dust Sensor from [SEEED Studio](https://www.seeedstudio.com/Grove-Dust-Sensor-PPD42NS.html). The sensor claims to measure dust over 1um.
The brain is an ESP32 which will be able to grab the time from the internet and report that into the reading. 
Two other envrionmental sensors are used to collect temperature and humidity (AHT20) and to collect gas concentrations (ENS160).
The data will all be logged in a text file on an SD card attached to the module. This file can then be converted to a CSV file for processing.
The OLED screen will allow for quick referance of the current reading as well as the average reading over the whole measurment time. 

## To Do
- [ ] Remove extra code
- [X] Clean up the OLED output
- [ ] Data processing code
- [X] Wiring diagram
- [X] Make a case
- [ ] Arange the code so the screen is setup first and outputs any errors

## Parts
- ESP32
- Grove Dust sensor
- SD Card Module for Arduino
- 64x128 OLED
- ENS160 from Sparkfun
- AHT20 from Adafruit

