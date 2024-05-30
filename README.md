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
- [X] Remove extra code
- [X] Clean up the OLED output
- [ ] ~~Data processing code~~
- [X] Wiring diagram
- [X] Make a case
- [ ] Arange the code so the screen is setup first and outputs any errors

## Parts
- [ESP32](https://www.amazon.com/gp/product/B08246MCL5/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1)
- [Grove Dust sensor](https://www.seeedstudio.com/Grove-Dust-Sensor-PPD42NS.html)
- [SD Card Module for Arduino](https://www.amazon.com/gp/product/B07BJ2P6X6/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
- [64x128 OLED](https://www.amazon.com/gp/product/B085WCRS7C/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
- [ENS160 from Sparkfun](https://www.sparkfun.com/products/20844)
- [AHT20 from Adafruit](https://www.adafruit.com/product/4566)

## Links To More Info
- Video: https://youtu.be/t2bbnQ1g2lM
- Instructable: https://www.instructables.com/Air-Quality-Monitor-2/
- Printables: https://www.printables.com/model/895121-air-quailty-monitor-case
