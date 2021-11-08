# ESP8266 Gemini Server

A simple gemini server implementation for the ESP8266.
Demo: gemini://astrra.space

## Requirements
* Arduino IDE set up for your ESP8266 board.
* [This](https://github.com/earlephilhower/arduino-esp8266littlefs-plugin) LittleFS plugin installed.

## Usage
1. Copy ```example_config.h``` to ```config.h```, set up your wifi name and password, add your RSA certicate and key.
2. Open the sketch in Arduino IDE.
3. Compile and upload the sketch to the board.
4. Run ```Arduino IDE>Tools>ESP8266 LittleFS Data Upload``` to upload the contents of the data folder to the flash.
5. Check if the board has connected to your wifi network and connect to it using your gemini browser of choice.
