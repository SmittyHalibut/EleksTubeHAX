# EleksTubeHAX - An aftermarket custom firmware for the EleksTube IPS clock
Buy your own clock under the name "EleksTube IPS Clock" on eBay, Banggood, etc. Look for places that offer 30-day guarantee, otherwise it can be a fake product!

[Reddit discussion on the original hack is here.](https://www.reddit.com/r/arduino/comments/mq5td9/hacking_the_elekstube_ips_clock_anyone_tried_it/)

[Original documentation and software from EleksMaker.](https://wiki.eleksmaker.com/doku.php?id=ips)

## Differences from original repositiory:
- Partitions changed to 1MB / 3 MB for more space for images
- Changed menu to add support for changing clock faces in the menu
- BMP files (24 BPP) converted to 16BPP custom (.CLK) files which only have a header, size and pre-processed pixel data.
- Added pre-loading of next image into the buffer for faster refresh
- Loading time is down from original 150 ms to approx 30 ms
- Supports smaller images which are centered on displays
- Approx 5-7 clock faces can be loaded onto the clock
- Catching invalid NTP responses (does not initialize the clock with invalid time)
- Power button is used to exit the menu (same as on original clock)
- WPS Button mode is used to connect to the WiFi / router (no need to insert credentials into source code)
- Night time mode: dims displays and backlight during night time hours
- MQTT (IoT) support added - clock faces and on/off can be controlled with mobile phone (SmartNest, SmartThings, Google assistant, Alexa, etc.) or included in home automation network
- WiFi and MQTT errors are displayed below clock faces
- Updated and cleaned the original clock faces for a better user experience
- Location detection and automatic TimeZone and DST selection.
- Automatic counting number of clock faces loaded onto the clock (no need to rebuild firmware)
- Added support for original "EleksTube IPS clock" and "SI HAI clock" (chinese cknockoff)
- Added support for NovelLife SE clock -- without gesture sensor thus far
- Added option to use hard-coded WIFI credentials instead of WPS
- (in works: ) Integrated web server to input MQTT connection details (or maybe load new clock faces)


My coding does not keep up with objects, it is more "old school" inline style; please don't complain about that. I tried to write it quite clean. And most importantly: it works fine! :) If someone has courage and time to smash new code into boxes and objects, I'm fine with that.

# Hardware modification
Original EleksTube has a few problems in the hardware design. Most notably it forces 5V signals into ESP32 which is not happy about it. And it is outside of safe operating limits. 
This will extend the lifetime of ESP32. Mine died because of this...
## Conversion
CH340 chip, used for USB-UART conversion can operate both on 5V and 3.3V. On the board it is powered by 5V. Cut one trace on the bottom side of the board that supplies the chip with 5V and route 3.3V over the resistors / capacitors to VDD and VREF.
See folder "Hardware modification" for the photo.

# Backup first!
If you mess-up your clock, it's only your fault. Backup images from other uses DO NOT WORK as the firmware is locked by MAC address of ESP32.
### Install the USB Serial Port Device Driver
[EleksTube instructions](https://wiki.eleksmaker.com/doku.php?id=ips) instruct for installing a serial port driver.
On Windows 10, plug-in the cable and run Windows Update. It will find and install the driver. 
On Linux it works out of the box.
### Save your original FW
* Install ESP32 support (see below)
* Navigate to folder `\Arduino\..\packages\esp32\tools\esptool_py\3.0.0` or `\original-firmware\`
* Copy file `\original-firmware\_ESP32 save flash 4MB.cmd` and `_ESP32 write flash.cmd` to this folder
* Change COM port number inside both files
* Run "save flash" file and wait until it saves the contents of your flash to `backup1.bin`
* if you want to restore your original firmware run `_ESP32 write flash.cmd`

# How to build this firmware
Unfortunately, it's not simple plug-and-play.  You need to do some things.  These instructions assume you already know how to use the Arduino IDE, and just need to know WHAT to do.

If you just want to use new firmware without setting up all the tools and libraries and everything, just navigate to folder `\original-firmware\` and modify `_ESP32 write flash.cmd` to upload latest version (BUILD_ELEKSTUBE_xxxxxxx.bin) or (BUILD_SI_HAI_xxxxxxx.bin) to your clock. 

Note that you will use my geolocation API key which has limited usage. If we all together exceed their free plan, it will stop working.

## Download this code
You're either reading this file after downloading it already, or you're reading it on github.  I'll assume you can figure out how to get the code from github and put it somewhere on your local machine.  This is your preference.

## Setup Arduino IDE
Development was done on Arduino 1.8.13.  It might work on earlier or later versions, I don't know.

### Install ESP32 board support from Espressif
File -> Preferences, add `https://dl.espressif.com/dl/package_esp32_index.json` to your Board Manager URLs.  (Comma separated if there's already something there.)

Tools -> Board -> Board Manager, add the `esp32` boards by `Espressif Systems`.  Development was done on v1.0.6.

Tools -> Board -> ESP32 Arduino -> ESP32 Dev Module

The default configs in the Tools menu should be fine. The important ones are:
* Flash Size: 4MB
* Partition Scheme: No OTA (1 MB app, 3 MB SPIFFS). To fit as many images as possible.
* Port: Set it to whatever serial port your clock shows up as when plugged in.
See screenshot here: https://github.com/aly-fly/EleksTubeHAX/blob/main/EleksTubeHAX/_build_settings.png

### Install Libraries
All these libraries are in Library Manager.  Several libraries have very similar names, so make sure you select the correct one based on the author.
The listed "developed on" versions are just the versions I had installed while developing this code.  Newer (or possibly older) versions should be fine too.

Sketch -> Include Library -> Library Manager
* `NTPClient` by Fabrice Weinberg (developed on v3.2.0)
* `Adafruit NeoPixel` by Adafruit (developed on v1.8.0)
* `DS1307RTC` by Michael Margolis (developed on v1.4.1)
* `TFT_eSPI` by Bodmer (developed on v2.3.61)
* `Time` by Michael Margolis (developed on v1.6.0)
* `PubSubClient` by Nick O'Leary (developed on v2.8.0)  https://www.arduinolibraries.info/libraries/pub-sub-client
* `ArduinoJson` by Benoit Blanchon (worked on 6.19.4)

For "SI HAI clock" also add:
* RTC by Makuna (developed on 2.3.5) https://github.com/Makuna/Rtc/wiki

IPgeolocation and NTPclient libraries were coped into the project and heavily updated (mostly bug fixes and error-catching).

### Configure the `TFT_eSPI` library
**IMPORTANT** You have to do this after every time you install or update the `TFT_eSPI` library!  **IMPORTANT**

* Edit `Arduino\..\libraries\TFT_eSPI\User_Setup_Select.h`
* Comment out all `#include` lines.  (The only one that comes from install is `#include <User_Setup.h>`.)
* Add a `#include` line pointing to `GLOBAL_DEFINES.h` in this code.
  * eg: `#include "D:\<personal folder>\EleksTubeHAX\GLOBAL_DEFINES.h"`
  * Obviously, update the path to point to where ever you keep your code.  Mac and Linux paths will look very different.

### Install SPIFFS uploader
The code is [here](https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/), and instructions to install it are [here](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/).

tl,dr: Download and unzip [this file](https://github.com/lorol/arduino-esp32fs-plugin/releases/download/2.0.7/esp32fs.zip) ([old file, reportedly not working](https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/download/1.0/ESP32FS-1.0.zip)) into your `Arduino/tools/` directory.  There might not be a `tools/` subdirectory yet, but if you see `Arduino/libraries/` that's the right place.

### Restart Arduino
After installing the ESP32 support, all the libraries, and the SPIFFS uploader, restart Arduino to make sure it knows its all there.
 
## Upload New Firmware
Make sure you configured everything in `USER_DEFINES.h`.
* Your MQTT credentials :: Register on [SmartNest.cz](https://www.smartnest.cz/), create a Thermostat device, copy your username, API key and Thermostat Device ID.
* Uncomment MQTT service (if in use)
* Your Geolocation API :: Register on [Abstract API](https://www.abstractapi.com/), select Geolocation API and copy your API key.
* Select hardware platform (Elekstube, NovelLife, or SI_HAI) :: un-comment appropriate hardware define 
* Select if you prefer WPS or hardcoded credentials for WIFI (comment 'WIFI_USE_WPS' line and add credentials if desired)
* Point `User_Setup_Select.h` in the TFT_eSPI library to `GLOBAL_DEFINES.h`

Connect the clock to your computer with USB.  You'll see a new serial port pop up.  Make sure that's the serial port selected in Tools.

### Compile and Upload the Code
Compile and upload the code.  At this point, it should upload cleanly and successfully.  You'll see the clock boot up and ask for WPS.  But it doesn't have any bitmaps to display on the screen yet.

### Upload Bitmaps
The repository comes with a set of CLK and BMP files, modified images from the original firmware, in the `data/` directory. See below if you want to make your own.

Tools -> ESP32 Sketch Data Upload, will upload the files to the SPIFFS filesystem on the micro.  They'll stay there, even if you re-upload the firmware multiple times.

### Custom Bitmaps (Optional)
If you want to change these:
* Create your own BMP files.  Resolution must be max 135x240 pixels, 24bit RGB. Can be smaller, it will be centered on the display. Cut away and black border, this only eats away valuable Flash storage space!
* Name them `10.bmp` through `19.bmp`; `20.bmp` to `29.bmp`, and so on. You can add as many as you can fit into SPIFFS space.
* Run the tool `\Prepare_images\Convert_BMP_to_CLK.exe`
* Select all prepared BMP files at once. It will create CLK files with smaller size.
* Put them in the `\data` directory.
* Then do the "Tools -> ESP32 Sketch Data Upload" dance again.
Each set (1x, 2x, etc) can be chosen in the menu.

If you don't want to make your own bitmaps, there are some good 3rd party sets out there:
* https://github.com/upiir/ips_clock_100x_themes
* https://github.com/upiir/elekstube_ips_custom_theme
* https://github.com/upiir/rgb_glow_tube_clock

If you have your own "font" that'll work and want it listed here, please file an Issue and/or Pull Request.

### Configure your WiFi network
For WPS:  When prompted by the clock, press WPS button on your router (or in web-interface of your router). Clock will automatically connect to the WiFi and save data for future use. No need to input your credentials anywhere in the source code.  THe clock will remember WiFi connection details even if you unplug the clock.

# Development Process:
See SmittyHalibut on GitHub (original author of this alternative FW) for details.

My notes are in the document  `Hardware pinout.xlsx`

*Happy hacking!*
   - Aljaz
