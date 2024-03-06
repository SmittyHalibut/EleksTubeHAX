# EleksTubeHAX - An aftermarket custom firmware for the desk clock
![EleksTube IPS clock](/Photos/Clock1.jpg)

Supported hardware models:
### "EleksTube IPS clock", "SI HAI IPS clock", "NovelLife SE clock", "PunkCyber clock", "RGB Glow Tube DIY clock"

Buy your own clock under the name "EleksTube IPS Clock" on eBay, Banggood, Aliexpress, etc. Look for places that offer 30-day guarantee, otherwise it can be a fake product!

[Reddit discussion on the original hack is here.](https://www.reddit.com/r/arduino/comments/mq5td9/hacking_the_elekstube_ips_clock_anyone_tried_it/)

[Original documentation and software from EleksMaker.](https://wiki.eleksmaker.com/doku.php?id=ips)

Firmware supports and was tested on different clock versions. Note that "EleksTube IPS" is the original, from the inventor of this type of the clock in 2021. Later appeared more and more similarly looking clock designs, copies of the original with more or less hardware modifications. In this repository we try to support most popular models. You can identify them by looking closely of the main board design:
![EleksTube IPS clock](/Photos/EleksTube_original_PCB.jpg)
![SI HAI IPS clock](/Photos/SI_HAI_ips_clock.jpg)
![NovelLife SE clock](/Photos/NovelLife_SE.jpg)
![PunkCyber / RGB Glow tube](/Photos/PunkCyber_ips_clock.jpg)

# Main clock features

- Up to 7 clock faces loaded onto the clock simultaneously; selected in menu or over MQTT
- WiFi connectivity with NTP server synchronization
- Supported either WPS connection or hardcoded WiFi credentials
- Optional IP geolocation for autiomatic Timezone and DST adjustment
- Manual time zone adjust in 15-minute increments
- Optional MQTT client for remote control - clock faces and on/off can be controlled with mobile phone (SmartNest, SmartThings, Google assistant, Alexa, etc.) or included into existing home automation network
- RGB baclights (wall lights) for nice ambient with multiple modes
- Optional DS18B20 temperature sensor 
- Dimming of the clock and backlights during the night time
- Different image files supported (BMP classic or paletized) and proprietary compressed files 
- Supports smaller images which are centered on displays
- Advanced error handling for best user experience
- WiFi and MQTT errors are displayed below clock faces
- Supported hardware: original "EleksTube IPS clock"; "SI HAI clock" (chinese cknockoff); NovelLife SE clock (without gesture sensor); "PunkCyber" or "RGB Glow Tube DIY" clock (from pcbway). NOTE: EleksTube IPS Gen 2 was not tested. If someone owns it, please test this firmware and contact us (best to open Issue on GitHub and report back)
- (in works: ) Integrated web server to input configuration (or maybe load new clock faces)

# How to use this firmware
If you just want to use new firmware without setting up all the tools and libraries and everything, navigate to folder `\pre-built-firmware\` and modify `_ESP32 write flash.cmd` to upload selected version to your clock. If you want more features, continue reading below.

### Hardware modification
Original EleksTube has a few problems in the hardware design. Most notably it forces 5V signals into ESP32 which is not happy about it. And it is outside of safe operating limits. This will extend the lifetime of ESP32. Mine died because of this...
_Conversion:_
CH340 chip, used for USB-UART conversion can operate both on 5V and 3.3V. On the board it is powered by 5V. Cut one trace on the bottom side of the board that supplies the chip with 5V and route 3.3V over the resistors / capacitors to VDD and VREF.
See folder "Hardware modification" for the photo.

# Backup first!
If you mess-up your clock, it's only your fault. Backup images from other uses DO NOT WORK as the original EleksTube firmware is locked by MAC address of ESP32.

### Install the USB Serial Port Device Driver
[EleksTube instructions](https://wiki.eleksmaker.com/doku.php?id=ips) instruct for installing a serial port driver.
On Windows 10, plug-in the cable and run Windows Update. It will find and install the driver. 
On Linux it works out of the box.

### Save your original FW
Windows users:
* In this folder you have `esptool.exe` which is used to talk to the ESP32 in your clock over the USB-Serial chip on the board.
* Open Device Manager and find out which COM port represents your clock.
* Modify file `_ESP32 save flash 4MB.cmd` with your COM port number.
* Run this file. It will generate `backup1.bin`. Save it to a safe place. This is your precious backup.

Linux users:
* You probably already know where to get Esptool and how to use it. :)

# How to build this firmware
Unfortunately, it's not simple plug-and-play.  You need to do some things.  These instructions assume you already know how to use the Visual Studio Code & PlatformIO plugin, and just need to know WHAT to do.

## Download this code
You're either reading this file after downloading it already, or you're reading it on github.  I'll assume you can figure out how to get the code from github and put it somewhere on your local machine.  This is your preference.

## Setup Visual Studio Code & PlatformIO plugin
Follow guide here: https://platformio.org/install/ide?install=vscode
* Download, install and run VSCode
* Go to Extensions, Search for Platformio IDE and install it (it will take a while - observe status messages in the bottom right corner).
If you don't have Python already installed it will be automatically added by Platformio. In case of issues, install it manually.

### Development board support
ESP32 environment from Espressif is required. It will be installed automatically when this project is opened in VSCode & Platformio. Or before first compilation. It will take a while - observe status messages in the bottom right corner.
Developed and tested on version 2.0.11.
Flash size settings that are already configured in `partition_noOta_1Mapp_3Mspiffs.csv`: Flash Size: 4MB; Partition Scheme: No OTA (1 MB app, 3 MB SPIFFS). To fit as many images as possible.
Upload port is set to 921600 baud in the `platformio.ini` file Some clocks do not support such high speed, if you have issues, reduce this to 512000 baud.

### Libraries in use
All these libraries are in use. The most recent versions are automatically installed from the Platformio servers as soon as the project is opened or before first compilation. It will take a while - observe status messages in build log screen.
Compiles and works fine with listed library versions, as of 2024-02-03. Newer (or possibly older) versions should be fine too. If you have issues with automatic install, here are locations of the originals.

| Library | Author | Version | Link |
| ------ | ------ | ------ | ------ |
| NTPClient | Fabrice Weinberg | 3.2.1 | https://github.com/arduino-libraries/NTPClient |
| Adafruit NeoPixel | Adafruit | 1.12.0 |  https://github.com/adafruit/Adafruit_NeoPixel |
| DS1307RTC | Paul Stoffregen | 1.14.1 | http://playground.arduino.cc/code/time |
| Time | Paul Stoffregen  | 1.6.1 | http://playground.arduino.cc/Code/Time/ |
| TFT_eSPI | Bodmer | 2.5.34 | https://github.com/Bodmer/TFT_eSPI |
| PubSubClient | Nick O'Leary  | 2.8.0 | https://www.arduinolibraries.info/libraries/pub-sub-client |
| ArduinoJson | Benoit Blanchon  | 7.0.2 | https://github.com/bblanchon/ArduinoJson.git |
| RTC by Makuna | Michael C.Miller  | 2.4.2 | https://github.com/Makuna/Rtc/wiki |

Note: `RTC by Makuna` is only required for "SI HAI clock".
IPgeolocation and NTPclient libraries were coped into the project and heavily updated (mostly bug fixes and error-catching).

### Configure the `TFT_eSPI` library
Supplied `script_configure_tft_lib.py` takes care for library configuration. Copies both files `_USER_DEFINES.h` and `GLOBAL_DEFINES.h` into the library folder before building. Tested on Windows.
If for some reason you have issues, manually fopy the files every time the `TFT_eSPI` library is updated.

## Configure, Build and Upload New Firmware
Make sure you configured everything in `_USER_DEFINES.h`:
* Rename `_USER_DEFINES - empty.h` to `_USER_DEFINES.h`
* Select hardware platform (Elekstube, NovelLife, SI_HAI, PunkCyber/RGB Glow tube) :: un-comment appropriate hardware define 
* Select if you prefer WPS or hardcoded credentials for WIFI (comment 'WIFI_USE_WPS' line and add credentials if desired)
* Select image type: .BMP files (default) or .CLK files

Optionally:
* Uncomment MQTT service (if in use)
* Your MQTT credentials :: Register on [SmartNest.cz](https://www.smartnest.cz/), create a Thermostat device, copy your username, API key and Thermostat Device ID.
* Uncomment Geolocation (if in use)
* Your Geolocation API :: Register on [Abstract API](https://www.abstractapi.com/), select Geolocation API and copy your API key.
* Uncomment and define pin for external DS18B20 temperature sensor (if connected)

Connect the clock to your computer with USB.  You'll see a new serial port pop up.  Platformio will automatically select the port. If you have Bluetooth virtal ports on your machine, it might hang and you must manually select the COM port in the `platformio.ini`.

### Compile and Upload the Code
Compile and upload the code.  At this point, it should build cleanly and upload successfully.  You'll see the clock boot up and ask for WPS.  But it doesn't have any bitmaps to display on the screen yet.

### Upload Bitmaps
The repository comes with a set of CLK and BMP files in the `data/` directory. See below if you want to make your own.
In Platformio extension go to Project tasks and expand: Esp32 -> Platform -> Build Filesystem image & Upload filesystem image.
This will upload the files to the SPIFFS filesystem on the micro.  They'll stay there, even if you re-upload the firmware multiple times.

### Custom Bitmaps
If you want to change clock faces / fonts:
* Create your own BMP files or select from the provided folder.  Resolution must be max 135 x 240 pixels, 24 bit RGB. Can be smaller, it will be centered on the display. Cut away any black border, this only eats away valuable Flash storage space!
* Name them `10.bmp` through `19.bmp`; `20.bmp` to `29.bmp`, and so on. You can add as many as you can fit into SPIFFS space.
* Run your preferred image editor and play with reduced bit depths / paletization of the image. Very good results are with Dithering and 256-color palette. Size reduction is approx 70%. With very simple images (like 7-segment) even 16-color palette is enough and reduces size even further.

Alternatively:
* Run the tool `\Prepare_images\Convert_BMP_to_CLK.exe`
* Select all prepared BMP files at once. It will create CLK files with smaller size. Size reduction is approx 30%.

* Put files in the `\data` directory.
* Then do the "Build Filesystem image & Upload filesystem image" dance again.
Each set of images can be chosen in the menu or via the MQTT "set temperature".

Here are links to some good 3rd party sets out there:
* https://github.com/upiir/ips_clock_100x_themes
* https://github.com/upiir/elekstube_ips_custom_theme
* https://github.com/upiir/rgb_glow_tube_clock

If you have your own "font" that'll work and want it listed here, please file an Issue and/or Pull Request.


### Configure your WiFi network
* For WPS: When prompted by the clock, press WPS button on your router (or in web-interface of your router). Clock will automatically connect to the WiFi and save data for future use. No need to input your credentials anywhere in the source code. The clock will remember WiFi connection details even if you unplug the clock.
* Without WPS: Add your WiFi credentials into `_USER_DEFINES.h` file before building the firmware.

# Development Process
See [Old Readme File](/README_OLD.md) for details.

Hardware pinout and notes are in the document  `Hardware pinout.xlsx`

# Main Contributors
- Mark Smith, aka Smitty ... @SmittyHalibut on GitHub, Twitter, and YouTube.
- Aljaz Ogrin, aka aly-fly ... @aly-fly on GitHub and Instagram
- Misc code snips either commited by or copied from: @icebreaker-ch, @meddle99, @OggyP, @bitrot-alpha
- in future (on to-do list) also from: @RedNax67, @wfdudley, @judge2005

*Happy hacking!*

