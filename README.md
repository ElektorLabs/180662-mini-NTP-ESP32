# 180622 mini NTP with ESP32

Code for the GPS based NTP with ESP32. This software uses a Wemos LoLin ESP32 OLED as base.
The OLED used is a SSD1307 based one attached to the i²C Bus. This board can be purchased 
at ( https://www.elektor.com/wemos-lolin-esp32-oled-module-with-wifi ). As GPS receiver this one is used in the build ( https://www.elektor.com/open-smart-gps-serial-gps-module-for-arduino-apm2-5-flight-control )

## Software 

The software is written using the arduino framwork for the ESP32. Also you need to compile the code following libraries to be present:
 *  U8G2 by oliver
 *  Time by Michael Magolis
 *  Ticker by Bert Melis
 *  TinyGPS++ ( https://github.com/mikalhart/TinyGPSPlus )  
 *  RTCLib by Adafruit
 *  ArduinoJson 6.10.0
 *  CRC32 by Christopher Baker

 Compile and upload the code to your ESP32. Also upload the webpages.

 ### GPIO Mapping
 For the GPIOs used these are not the arduino default ones, as they needed to be modified for the OLED. The following pins are used:

## I²C:
| GPIO PIN  | Function  |
|-----------|-----------|
|  GPIO04   |  SCL      |
|  GPIO05   |  SDA      |
|           |           |

## GPS:
| GPIO PIN  | Function      |
|-----------|---------------|
| GPIO13    | UART RX       |
| GPIO15    | UART TX       |
| GPIO25    | PPS Interrupt |

For more inforamtion have a look at: https://www.elektormagazine.com/labs/admin-preview/mini-ntp-server-with-gps