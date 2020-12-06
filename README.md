# MRFGS
FANET Groundstation

## References
 * FANET protocol specification https://github.com/3s1d/fanet-stm32/blob/master/Src/fanet/radio/protocol.txt
 * Radio/Packet Code from https://github.com/Betschi/FANET_V2019_02_07

## Features

### receive and process FANET packets
* Type 0 - ACK
* Type 1 - (Air)Tracking
* Type 2 - Name
* Type 3 - Message
* Type 4 - Service - Weather - relay Skytraxx Windstations
* Type 7 - Groundtracking
* Type 8 - Device Info - (old)
* Type 9 - Thermals
* Type A - Device Info - (new)

### send FANET packets
* Type 0 - ACK
* Type 1 - (Air)Tracking
* Type 2 - Name
* Type 3 - Message
* Type 4 - Service (Weatherdata from local Weatherstations)
* Type 5 - Landmarks
* Type 7 - Groundtracking

###

## Dependencies
- aprs-is - weather-submit
- rapidjson
- rapidcsv
- httplib.h - Yuji Hirose
- libcurl
- TDequeConcurrent <christophe@xtof.info>
- wiringPi

## Hardware support
- Raspberry Pi Model 4 / 3 / 0
- Dragino Lora/GPS HAT
- Tindy SX1276/LORA HAT 

## Installation

### Raspian
   - sudo apt-get update
   - activate SPI for SX1276 chip interface
   
####  Tools:
   - sudo apt-get install git
   - sudo apt-get install cmake
   
### Libraries:
   - sudo apt-get install libssl-dev
   - sudo apt-get install libcurl4-openssl-dev
   
   - http://wiringpi.com/wiringpi-updated-to-2-52-for-the-raspberry-pi-4b/
     - cd /tmp
     - wget https://project-downloads.drogon.net/wiringpi-latest.deb
     - sudo dpkg -i wiringpi-latest.deb
     
   - https://raspberry-projects.com/pi/programming-in-c/boost-c-libraries/installing-and-using-boost
     - sudo apt-get install libboost1.67-all
     
### build 
   - in user work directory
     - git clone https://github.com/mreinart/MRFGS.git
     - cd MRFGS
     - cmake CMakeLists.txt
     - make

### configuration
   - edit config/MRFGS.json - some working defaults
   - Todo: options

### execute
   - create empty file ./MRFGS.lock in file where FANET_GS_AIR is executed 
   - start ./MRFGS
   - trace file ./fanet_packets.txt
   - trace file ./aprsLog.txt
   - trace file ./cwopLog.txt
     
### configure automatic start
 - ...
