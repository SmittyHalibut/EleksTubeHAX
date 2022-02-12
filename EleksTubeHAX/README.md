
arduino-cli config add board_manager.additional_urls https://dl.espressif.com/dl/package_esp32_index.json


 ======= required libraries =========

NTPClient @3.2.0

Adafruit Neopixel @1.8.0

DS1307RTC @1.4.1

TFT_eSPI @2.3.61

Time @1.6.0

PubSubClient https://www.arduinolibraries.info/libraries/pub-sub-client


========= important ========

copy file "User_Setup.h" to folder where you have TFT_eSPI librry, for example: 
Documents\Arduino\libraries\TFT_eSPI\User_Setup_Select.h
or
Arduino\PORTABLE\sketchbook\libraries\TFT_eSPI

Yiu can make a backup copy of original file before.
Note: when you update this library, you have to copy this file again.
