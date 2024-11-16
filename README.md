# EleksTubeHAX - An aftermarket custom firmware for the desk clock


![EleksTube IPS clock](/documentation/ImagesMD/EleksTube_IPS_Classic_Edition.jpg)

### This is the "Home Assistant Edition" with extendent MQTT support, more info see below!

## Supported hardware models

### Original EleksTube Models

1. **EleksTube IPS Clock (Original Version)**
2. **EleksTube IPS Clock Gen2** 
   - EleksTube IPS Classic Edition/Pro/PR1/PR2

### Other Supported Models

3. **SI HAI IPS Clock**
4. **NovelLife SE Clock**
   - With gesture sensor
   - Without gesture sensor
5. **PunkCyber IPS Clock f/k/a RGB Glow Tube DIY Clock**
6. **IPSTUBE Clock - Model H401 and H402**
   - With bottom LED stripe
   - Without bottom LED stripe

### Notes

- **EleksTube IPS Clock** is the original model created by the inventor in 2021. There are now many similar designs and clones on the market with varying hardware modifications.
- All "Original EleksTube" clocks sold after July 2022 are "Gen2" versions. Refer to the [Blog post on EleksTube website](https://elekstube.com/blogs/news/instructions-on-elekstube-clock-for-gen2-systems) for more details.
- Newer versions from EleksTube, such as PR1 and PR2, Pro and special editions (Pink etc.), are based on the base Gen2 version and sometimes called Gen3, because the original firmware version is already at Version 3.x for them. The "basic" version is now officially called "**EleksTube IPS Classic Edition**"
- In 2024 the IPSTUBE clocks become the cheapest and most available "clone" variant of the EleksTube clocks on the market. They are offered by a wide range of seller and under a lot of different names, but they all have the "IPSTUBE" logo printed in front. Most of the offers have words like Nixie, Tube, Clock, LED, Light, RGB, IPS and Glow in description.

### Purchasing Information

You can buy the "EleksTube IPS Clock" or its clones from eBay, Banggood, AliExpress, the EleksTube website, and other retailers.

Ensure the seller offers at least a 30-day guarantee to avoid purchasing fake products or beeing tricked.

### Firmware Compatibility

- This firmware supports and has been tested on various clock models listed above.
- If you discover a new clone or version of these types of clocks, please report it by creating an issue in the "Issues" section of the GitHub project. Provide as many details as possible to help expand compatibility.

## Mainboard/PCB views

EleksTube IPS - Orginal Version - with hardware modification
![EleksTube IPS clock](/documentation/ImagesMD/EleksTube_original_PCB.jpg)
EleksTube IPS - Gen2 (EleksTube IPS Classic Edition/Pro/PR1/PR2)
![EleksTube IPS clock - Gen2](/documentation/ImagesMD/EleksTube_Gen2_PCB.jpg)
SI HAI IPS
![SI HAI IPS clock](/documentation/ImagesMD/SI_HAI_ips_clock.jpg)
NovelLife SE
![NovelLife SE clock](/documentation/ImagesMD/NovelLife_SE.jpg)
PunkCyber IPS
![PunkCyber / RGB Glow tube](/documentation/ImagesMD/PunkCyber_IPS_clock_PCB.jpg)
IPSTUBE - H401
![IPSTUBE clock - Model H401](/documentation/ImagesMD/IPSTUBE_H401_PCB.jpg)
IPSTUBE - H402
![IPSTUBE clock - Model H402](/documentation/ImagesMD/IPSTUBE_H402_PCB.jpg)

For detailed pictures for most of the clocks see the `documentation` subfolder.

## Main clock features

- Show the actual time on the LCDs of the clock with the selected clock face
- Multiple clock faces can be loaded into the clocks flash memory. Switchable via clock menu
- 12/24 hour view (switchable via clock menu)
- On-Screen menu to change settings/configuration of the clock
- WiFi connectivity with NTP server synchronization
- Supports either WPS connection or hardcoded WiFi credentials
- Manual time zone adjustment in 1 h and 15 minute slots
- RGB backlights (wall lights) for nice ambient light with multiple modes ("Off", "Test", "Constant", "Rainbow", "Pulse" and "Breath")
- Dimming of the displays (via alpha blending) and backlights (hardware) during the night time (start and end time configurable in code)
- Turning displays on and off (not supported on all clock versions)
- Keeping time even when power is off by using battery-powered real-time clock (not supported on all clocks)
- Saving and loading clock configuration from the flash, so storing all settings, even when power is off
- Supports various bitmap image file variants (classic or palletized BMP) and proprietary compressed files (CLK)
- Maximum image size is 135 x 240 (Width x Height) pixels
- Supports smaller images – they will be automatically centered
- Advanced error handling for best user experience
- WiFi and MQTT errors are displayed below the digits
- Supports Home Assistant integration with extensive MQTT support (see below for details)
- Debug outputs via the serial port
- Optional MQTT client for remote control - Switch clock faces and turn displays on/off can be controlled via MQTT messages
- With a MQTT broker (SmartNest, SmartThings, Mosquitto etc.), this can also be integrated via a mobile phones app, a website or into an existing home automation network (and can be controlled via Google assistant, Alexa, etc.)
- Optional IP-based geolocation for automatic timezone and DST adjustments (only supported geolocation provider is "[Abstract](https://www.abstractapi.com/)")

## Home Assistant Edition with extensive MQTT features

If activated in the code, this version of the firmware can be remote controlled via Home Assistant. One of the leading free home automation solutions (see [Home Assistant Homepage](https://www.home-assistant.io)).

Features:

- Device detected as two different lights - main and back
- Turn displays on/off
- Control brightness level
- Change modes/effects of the LEDs (main and back)
- Change the color of the LEDs (in constant mode)
- Switching 12/24 hours mode
- Switching blank zeroes setting
- Modes/Effects speed change for backlight
- All options are discoverable via Home Assistant MQTT (if switched on)

Note: Main light clock faces may be named and needs to be changed, if other clock faces are used. See `clockfaces.txt` in the `data` folder.

### Detailed description:

Interactions between the firmware of the clock and Home Assistant is possible like descriped in the MQTT integration documentations:

- [MQTT Discovery](https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery)
- [MQTT Light integration](https://www.home-assistant.io/integrations/light.mqtt/)
- [MQTT Switch integration](https://www.home-assistant.io/integrations/switch.mqtt/)
- [MQTT Number integration](https://www.home-assistant.io/integrations/number.mqtt/)

It is preferable (but not mandatory) to use MQTT Discovery (uncomment `#define MQTT_HOME_ASSISTANT_DISCOVERY` and the following define statements), in which case the device and all entities will be found by the MQTT integration without user intervention. Otherwise, all settings will need to be done manually.

`#define MQTT_CLIENT` is used as a unique device name (i.e. it should be different if you have several IPS clocks) and is the "root" part of the topic for all entities that will be interacted with via MQTT.

Further analysis is easier to perform using an MQTT message reader tool, for example, "MQTT Explorer", a common HA Add-On for users who prefer manual configuration. See [HA forum post for MQTT Explorer Add-On](https://community.home-assistant.io/t/addon-mqtt-explorer-new-version/603739). It can be also used as a stand-alone application. See [Homepage of MQTT Explorer project](https://mqtt-explorer.com/)

# How to use this firmware
If you just want to use new firmware without setting up all the tools and libraries and everything, navigate to folder `\pre-built-firmware\` and modify `_ESP32 write flash.cmd` to upload selected version to your clock. If you want more features, continue reading below.

## Clock specific features

Some clock models have specific functionatlities or hardware specials which are only available for this model

#### NovelLife SE Clock with gesture sensor

- No buttons on the clock, only a gesture sensor
- Gesture sensor is supported by simulating the buttons like the other clocks have

|  gesture | button |
|--|--|
| down, near | Power |
| up, far | Mode |
| left | Left |
| right | Right |

- The movement of the finger/hand from behind the glass tubes of the watch, over the glass tubes, directly and closely over the sensor to the front of the watch is the “down” gesture.
- The movement of the finger/hand from in front of the watch, directly and closely over the sensor, further over the glass tubes and behind it is the “up” gesture.
- The movement of the finger/hand starting left side from the sensor and moving over the sensor to the right side is the “right” gesture.
- The movement of the finger/hand starting right side from the sensor and moving over the sensor to the left side is the “left” gesture.
- Moving the finger/hand from directly above the sensor (from 5-8 cm away) toward the sensor (up to about 1 cm away) is the “near” gesture.
- Moving from close by the sensor (coming from the front and putting the finger/hand in 1cm distance over the sensor) to a bit more far away (5-7cm distance) is the "far" gesture.

#### IPSTUBE Clock - Model H401 and H402

- This model has a 8MB flash memory, so either more clock faces can be stored on the clock or in a better quality (i.e. no palettization/conversion needed).

##### One button menu

- The clock has only one button.
- So there is a special menu mode active for this model.
- Short pressing the button brings up the menu.
- Short pressing while in the menu changes the menu scope.
- Long pressing (1-1.5 sec at least) changes the value of the actual selected menu scope.
- This makes navigating through the menus a bit "unhandy".
- The values always changes "to the right", because the long button press is emulating a "right button" press in the menu.
- This limits the selection of values (like in for timezone values or absolut color values).
- The selection is looping to the first value after reaching the last value.

##### LCD stripe

- Some versions of this model have a LED stripe with 28 RGB LEDs installed on the bottom.
- The stripe is a continuation of the 6 LED on the bottom of the LCDs, called "backlight".
- In the moment, the stripe is following the configuration of the backlight (modes, colour etc.).
- All models have a 3-pin socket on the board, so theoratically the stripe is retrofittable with any LED stripe made of WS2812B LEDs and some work.

## How to use this firmware

### Pre-Build images

If you just want to use this firmware without setting up all the tools and libraries and everything, navigate to folder `pre-built-firmware` and modify `_ESP32 write flash.cmd` to upload the firmware version for your clock.

See also the `README.MD` in the `pre-built-firmware` subfolder of the repo.

If you want more features and configure the firmware, check also section "How to build this firmware".

### Backup first!

**Always backup YOUR clocks firmware version as first step!**

If you mess-up your clock, it's only your fault!

Note for original EleksTube clocks: Backup images from other users **DO NOT WORK** as the original EleksTube firmware is locked to the MAC address of the ESP32.

For other clocks it MAY work, but don't assume it!
Most clocks have MAC address binding!
Surely known for EleksTubes and IPSTUBEs!

### Install the USB Serial Port Device Driver

Windows

- On Windows, plug-in the cable into the clock and connect it to an USB port of your PC. Then run Windows Update. It will find and install the driver and generate an virtual COM port.<br>
Windows device manager COM port example:<br>
![Windows device manager COM port example](/documentation/ImagesMD/WindowsDeviceManagerCOMport.png)
- Only if Windows Update is not working for you, see the chinese manufacturers website for newest [CH340 driver](https://www.wch-ic.com/downloads/CH341SER_ZIP.html) for Windows and install them manually.

Linux

- On an up-to-date Linux it works out of the box. COM port should be something like `/dev/ttyUSB0`.
- If it is not working, see this [tutorial](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/linux) and see the chinese manufacturers website for newest [CH340 driver](https://www.wch-ic.com/downloads/CH341SER_ZIP.html) for Linux.

### Save your original firmware

To read from (or write to) the clock, it needs to be in the "download mode". Most clocks will go into this mode automatically, when the ESPTool tries to read or write to them.
Some clocks needs a button pressed while the powering phase (plugging the USB cable), like the IPSTUBEs.

Windows users:

- In the folder "pre-built-firmware" of this repo you have the `esptool.exe`, which is used to talk to the ESP32 in your clock over the USB-Serial chip on the board of the clock.
- Open Device Manager and find out which virtual COM port represents your clock.
- Modify file `_ESP32 save flash 4MB.cmd` with your COM port number (or 8MB version for the IPSTUBE).
- Run the CMD file.
- It will generate a file named `backup1.bin`. Save it to a safe place!
- This is your precious backup.

Linux users:

- You probably already know where to get `ESPTool` and how to use it. :)
- If not, this guide from the Tasmota project is very easy to follow: [ESPTool](https://tasmota.github.io/docs/Esptool/#put-device-in-firmware-upload-mode)
- Adopt the settings for your connected clock and save the firmware file to your device.

### Restore backup

You can write back your original firmware by modyfing the file `_ESP32 write flash.cmd`.

- Set correct COM port and the name of your firmware backup file
- If needed, set the size to be written
- Run the CMD file and flash the firmware file
- Check if clock is working

### Known backup problems

#### COM port problems

There are a lot of possible issues known with so called "virtual COM ports".
Some common ones are listed below.

- On Windows: No COM port at all, but "unknown device" in the device manager -> Try to install the CH340 drivers manually.
- ESPTool fails to read or write all of the firmware -> Check your cable and if possible try another and a shorter one. Try to reduce the baud rate of the connection (like to 115200 or even below). Try to use an external powered USB hub, to avoid under powering.
- ESPTool is giving an error like "device is not in the download mode" -> Try to hold the power button while plugging the USB-C (power) cable. If this doesn't help, you may have a timing problem with the USB-UART chip. This is known for some combinations of USB controllers (especially 3.x) and the CH340. Try to use a USB2.x HUB with external USB power supply or, if nothing helps, try to use another computer.

For more info ask your favorite search engine or AI Chatbot :)

#### Lost firmware file

If you lost your orginal firmware file and wants to restore the orginal firmware you normaly have a problem, because even if you get a backup file from another user, it is locked to the MAC address of the other ESP32 MAC.
Under some conditions, it should be possible to change the encoded MAC address in the firmware. This worked for EleksTube Gen1 clocks. See [Issue 8]('https://github.com/SmittyHalibut/EleksTubeHAX/issues/8').

Note: There is no gurantee, that you are able to change the MAC address in the firmware file successfully! If the orginal manufacturer decides to encode the MAC address or hide it, the descriped method will not work!

So remember:
**Always backup your original firmware!**

## How to build this firmware

Unfortunately, it's not simple plug-and-play.

You need to do some things.

These instructions assume you already know how to use the **Visual Studio Code** and the **PlatformIO IDE** extension for it, and just need to know what to do.

### Download this code

You're either reading this file after downloading/cloning it already, or you're reading it on GitHub.

If the last point applies, then we assume you can figure out how to get the code from GitHub and put it somewhere on your local machine.

### Visual Studio Code & PlatformIO IDE

Follow this guide here: [Install PlatformIO IDE](https://platformio.org/install/ide?install=vscode)

In short:

- Download, install and run VSCode
- Go to Extensions, search for "PlatformIO IDE" and install it (it will take a while - observe status messages in the bottom right corner).
If you don't have Python already installed it will be automatically added by PlatformIO. In case of issues, install Python 3.x manually.

#### ESP32 platform support

The EspressIF 32 development platform for PlatformIO is required to support the ESP32 microcontroller. It will be installed automatically when this project is opened in VSCode/PlatformIO or if the first build is triggered. It will take a while - observe status messages in the bottom right corner.

Tested on version 6.0.9 from the [PlatformIO registry](https://registry.platformio.org/platforms/platformio/espressif32).

The default environment for this project (and most clocks) are using the "board" definition of the orginal "Espressif ESP32 Dev Module" named "esp32dev".
The IPSTUBE needs to use the "esp32dev8MB" environment with a custom board definition ("Espressif ESP32 Dev Module 8MB").

Flash size settings are already configured in the following files.

| filename | environment | flash size | app part size | data part size |
|----------|-------------|------------|---------------|----------------|
| `partition_noOta_1Mapp_3Mspiffs.csv` | esp32dev | 4.0 MB | 1.2 MB | 2.8 MB |
| `partition_noOta_1Mapp_7Mspiffs.csv` | esp32dev8MB | 8.0 MB | 1.2 MB | 6.8 MB |

No OTA partition, one app partition, one data partition as SPIFFS to store the images of the clock faces.

These files are used by PlatformIO to create the partions on the flash of the ESP32 when uploading.

Upload port is set to 921600 baud in the `platformio.ini` file.
**Note**: Some clocks do not support such high speed, if you have issues, reduce this to 512000 baud or even lower.

#### Libraries in use

All the listed libraries are in use (see `platform.ini` file).

The most recent versions are automatically installed from the [PlatformIO registry](https://registry.platformio.org) (or the given source location) as soon as the project is opened or before first compilation.

It will take a while to initally install them - observe status messages in build log screen.

Compiles and works fine with listed library versions below, as of 2024-02-03. Newer (or possibly older) versions should be fine too.

If you have issues with automatic installation, here are locations of the original source code.

| Library | Author | Version | Link |
| ------ | ------ | ------ | ------ |
| NTPClient | Fabrice Weinberg | 3.2.1 | https://github.com/arduino-libraries/NTPClient |
| Adafruit NeoPixel | Adafruit | 1.12.0 |  https://github.com/adafruit/Adafruit_NeoPixel |
| DS1307RTC | Paul Stoffregen | 1.4.1 | https://github.com/PaulStoffregen/DS1307RTC |
| Time | Paul Stoffregen  | 1.6.1 | https://github.com/PaulStoffregen/Time |
| TFT_eSPI | Bodmer | 2.5.34 | https://github.com/Bodmer/TFT_eSPI |
| PubSubClient | Nick O'Leary  | 2.8.0 | https://www.arduinolibraries.info/libraries/pub-sub-client |
| ArduinoJson | Benoit Blanchon  | 7.0.2 | https://github.com/bblanchon/ArduinoJson.git |
| RTC by Makuna | Michael C.Miller  | 2.4.2 | https://github.com/Makuna/Rtc/wiki |
| SparkFun APDS9960 RGB and Gesture Sensor | SparkFun  | 1.4.3 | https://github.com/sparkfun/SparkFun_APDS-9960_Sensor_Arduino_Library |

**Notes**:
`RTC by Makuna` is only required for "SI HAI clock".

`sparkfun/SparkFun APDS9960 RGB and Gesture Sensor` is only required by NovelLife SE clocks with gesture sensor.

`IPgeolocation` and `NTPclient` libraries were copied into the project and heavily updated (mostly bug fixes and error-catching).

`DS1307RTC` is only available in version "0.0.0-alpha+sha.c2590c0033" from the PlatformIO registry. This version is working and should be compiled from the latest code version in the original repo of the lib. But to have a "nicer" version number (1.4.1), add `https://github.com/PaulStoffregen/DS1307RTC.git#1.4.1` instead of `paulstoffregen/DS1307RTC` into the `platform.ini`.

#### Configure the `TFT_eSPI` library

The supplied `script_configure_tft_lib.py` takes care of the library configuration. It copies two files (`_USER_DEFINES.h` and `GLOBAL_DEFINES.h`) into the library folder before building. This makes sure, the TFT_eSPI library is initalized with the correct values for each clock type.

If you have issues with the scripts, copy the files manually every time the `TFT_eSPI` library is updated.

#### Configure the `APDS9960` library

The supplied `script_adjust_gesture_sensor_lib.py` modifies some files of the APDS9960 library before building. It adds the support for the ID of the used (cloned) gesture chip (needed for NovelLife SE with gesture sensor only).

### Configure, Build and Upload new firmware

Make sure you configured everything in `_USER_DEFINES.h`:

- Rename/Copy `_USER_DEFINES - empty.h` to `_USER_DEFINES.h`
- Select the target hardware platform (Elekstube, NovelLife, SI_HAI, PunkCyber, IPSTUBE) by uncommenting the appropriate hardware define (only ONE clock type can be defined at a time, so comment out the unwanted)
- Select if you prefer WPS or hardcoded credentials for WiFi (comment '#define WIFI_USE_WPS' line and enter your WiFi network credentials to the other lines and uncomment them)
- Select image type for the clock faces to store: Bitmap files (.BMP) or .CLK files (uncomment #define USE_CLK_FILES)

Optionally:

- Enable integrated MQTT service (uncomment `#define MQTT_ENABLED` line and enter your MQTT credentials. From your local broker or from an internet-based broker.
E.g. register on [SmartNest.cz](https://www.smartnest.cz/), create a Thermostat device, copy your username, API key and Thermostat Device ID.
- Use IP-based geolocation by Abstract (uncomment `#define GEOLOCATION_ENABLED` and enter your geolocation API key: Register on [Abstract API](https://www.abstractapi.com/), select Geolocation API and copy your API key.
- Enable temperature sensor (uncomment and define pin for external DS18B20 temperature sensor) - not available for most of the clocks.
- Enable Home Assistant support by uncomment `#define MQTT_HOME_ASSISTANT` and the following block of comments for Auto-Discovery in HA.

Connect the clock to your computer via a USB cable. You'll see, that a new serial port is detected and showing up in the device configuration. If not, check the section "Install the USB Serial Port Device Driver".

PlatformIO will automatically select the right port for uploading (in most cases).

Most clocks will go into to the download mode automatically, when PlatformIO is trying to upload the builded firmware files.
Some clocks needs a button pressed while the powering phase (plugging the USB cable), like the IPSTUBEs.

 **Note**: If you have Bluetooth virtual ports on your machine, it might hang and you must manually select the COM port in the `platformio.ini`, see [Upload options](https://docs.platformio.org/en/latest/projectconf/sections/env/options/upload/index.html).

#### 1st Compile the code and upload the firmware file (to the app partition)

Compile the code via the "Build" command of PlatformIO extension and upload the code via the "Upload" command in the matching environment for your clock (esp32dev is right for all clocks, except IPSTUBE)

![alt text](/documentation/ImagesMD/PlatformIOBuild.png)

At this point, it should build cleanly and upload successfully.

Building:
![alt text](/documentation/ImagesMD/PlatformIOBuildOutput.png)

Uploading:
![alt text](/documentation/ImagesMD/PlatformIOUploadOutput.png)

Note: For IPSTUBE clocks use "esp32dev8MB" environment

**Note**: On auto-reset clocks, you'll see the clock boot up after upload. It will go into the setup routine and ask for WPS or connecting to the configured WiFi network. On some clocks, you need to to a manual reset (power off/on cylce)

**Note**: After the initally flash, your clock will **NOT SHOW ANYTHING** on the displays after the setup phase!
This is, because it doesn't have any bitmaps to display on the flash memory yet!

**The screens will stay blank until you upload data!**

See "2nd Fill data partition (SPIFFS) with images".

#### 2nd Fill data partition (SPIFFS) with images

The repository comes with a "predefined" set of BMP files in the `data` subdirectory of the `EleksTubeHAX_pio` folder.

Each set of 10 images (one image for each digit) is called a "clock face" and can be chosen from the clock menu. Each set represents normally a different design or 'fonts' for the clock.
See below if you want to make your own.

Note: If MQTT is enabled, clock face switching can also be done via the MQTT "set temperature" topic.

Note: All files in the `data` directory will be packed into the SPIFFS flash image! Make sure to tidy it up regulary.

##### Generate and upload

- In PlatformIO extension go to "Project Tasks" and expand: esp32dev -> Platform
- Select "Build Filesystem Image" first 
- Then connect the clock
- Click "Upload Filesystem Image"

![alt text](/documentation/ImagesMD/PlatformIOBuildFilesystem.png)

This will upload the files to the SPIFFS filesystem on the ESP32 (flash of the clock).

Note: The data will stay there, even if you re-upload the real firmware to the app partition, because the data partition is not overriden or modified by that.

Note: For IPSTUBE clocks use "esp32dev8MB" environment

#### Custom clock faces from Bitmaps

If you want to change the uploaded clock faces for the clock:

- Create your own set of BMP files or select and copy some from the provided sets in the `data - other graphics` folder of this repo or download some from the internet (see below).
- Maximum resolution of each image is 135 x 240 pixels (HxW). They can be smaller, then the picture will be centered on the display.
- Maximum color depth is 24 bit RGB. But recommended is palettized Bitmaps with 256 colors palette.
- Name them `10.bmp` (for digit Zero) through `19.bmp` (for digit Nine); `20.bmp` to `29.bmp`, and so on. Note: There is no set 00-09.bmp!
- You can add max 8 clock face sets in the moment (Due to a problem in the detection meachanism - will be fixed soon).

Tips:

- Cut away any black border, this only eats away valuable flash storage space!
- Run your preferred image editor and play with reduced bit depths / palettization of the image!
- Very good results are with Dithering and 256-color palette. Size reduction is approx 70%.
- With very simple images (like 7-segment digits) even 16-color palette is enough and reduces size even further.

#### CLK files as alternative

Before supporting palettized Bitmaps, there was a special format used to store images, to safe some space on the flash of the clock. You can still switch to use CLK (Clock) files only, but there is no space saving anymore, if palattized Bitmaps are used!

To convert existing image files to CLK format:

- Run the tool `\Prepare_images\Convert_BMP_to_CLK.exe` (Windows only)
- You can select all BMP files to be converted at once. It will create CLK files from it.
- For a 24 bit depth Bitmap, size reduction is approx 30%.
- If needed, rename the generated files and put them in the `data` directory.
- Then do the "Build Filesystem Image & Upload Filesystem Image" dance again.

Note: It is either Bitmap or CLK! No mixing, so make sure to "clean" the `data` folder before switching.

#### Clock faces

Here are links to some good 3rd party sets out there:

- <https://github.com/upiir/ips_clock_100x_themes>
- <https://github.com/upiir/elekstube_ips_custom_theme>
- <https://github.com/upiir/rgb_glow_tube_clock>

If you have your own clock face that'll work and want it listed here, please file an Issue and/or Pull Request.

#### Configure your WiFi network

- For WPS: When prompted by the clock, press WPS button on your router (or in web-interface of your router). Clock will automatically connect to the WiFi and save data for future use. No need to input your credentials anywhere in the source code. The clock will remember WiFi connection details even if you unplug the clock.
- Without WPS: Add your WiFi credentials into `_USER_DEFINES.h` file before building the firmware.

Note: The `_USER_DEFINES.h` is included in the default `.gitignore` file, so that your personal credentials will not be pushed to a git repo by default, if you are using a forked git repo.

## Known problems/limitations

##### No RTC for SI HAI IPS Clock

There is no battery on the SI HAI IPS clock, so the clock will loose the time, if powered off.

##### One Button menu for IPSTUBE clocks

The value always changes "to the right", on a long button press.
Values smaller then the starting value ("to the left") can never be seleted (like in for timezone values or absolut color values).

##### No display turn off for some IPSTUBE clocks

Depending on the board version of the IPSTUBEs models H401 and H402, the transistor Q1 is present on the board or not.
If the transistor (marked A19TF, seems a 3401) is present, the displays can be turned on and off like on other clocks.
If the transistor is not present, the TFT LCDs can NOT turned on or off by software without modifing the hardware!

The ground pin for the LCDs is connected directly to ground on these boards.

It should be possible to solder a transistor on position Q1, to bring the switching funcionality to the boards.
Not tried yet!

In general, the pin 1 and pin 7 of each FPC for each TFT LCD are connected together directly to the +V3.3 line of the PCB.
Pin 1 is the general VCC power (LED Anode) and pin 7 is the VDD (Power Supply for Analog).
So there is no way to control the VDD on pin 7 seperately (real dimming) or to turn on or turn off power to pin 1, even if the transistor is present.
But turning on an off via ground pin is possible.

##### Precision gesture sensor

The accuracy of the gesture sensor on the Novellife clock is not very good. It needs some 'training' to be able to control the clock.

## Development Process/History

See [Old Readme File](/README_OLD.md) for details.

[Reddit discussion on the original hack is here.](https://www.reddit.com/r/arduino/comments/mq5td9/hacking_the_elekstube_ips_clock_anyone_tried_it/)

[Original documentation and software from EleksMaker.](https://wiki.eleksmaker.com/doku.php?id=ips)

For Elekstube OV and SI HAI:
Hardware pinout and notes are in the document `Hardware pinout.xlsx` in the `documentation` folder

There was an offical "manual" and software package avaiable for the original Elekstube clock.
The website is not maintained anymore, so just for archive purpose:
See [EleksTube instructions](https://wiki.eleksmaker.com/doku.php?id=ips)

### Hardware modifications

#### PunkCyber IPS clock: No CH340 chip soldered on PCB of newer editions

##### Problem

Short story: CH340 chip is missing.

Long story: The original version of the "PunkCyber IPS Clock" was buyable from PCBWay as "RGB Glow Tube DIY Clock" and is no longer available from there (see [PCBWay | RGB Glow Clock]('https://www.pcbway.com/project/gifts_detail/RGB_Glow_Tube_Clock_907ad35c.html')). This version had a CH340 chip soldered and two USB-C ports, where the right one was working, to communicate with a PC via the USB-UART chip.

The clock is still available to buy from other resellers. It is already assembled and in a box, sold as "PunkCyber IPS Clock".

All these newer versions of the clock do NOT have the CH340 chip and the components for the "auto download mode circuit" soldered! Just empty soldering header.
So both USB-C ports are only for giving power to the clock.

A communication (like downloading or uploading firmware) is not possible easily without this chip!

The original manufacturer has an 'interesting' way to update the firmware "over-the-air", by connecting your mobile phone to the Wifi-Access-Point of the clock and joining a WeChat channel and let them control the clock from there (see the discussion on the PCBWay site).

##### Solution

If you want to bring the EleksTubeHAX firmware to the clock, we need either to solder a CH340 chip and the transistors and resistors on there designated places, or use an "external" USB-UART bridge/board.

The "easiser" way is the external board solution IMO.

Below the missing chip are some soldering headers:

![PunkCyber / RGB Glow tube](/documentation/ImagesMD/PunkCyber_PCB_CH340_header.jpg)

BOOT is connected to GPIO0 (pin 25) on the ESP32
EN is connected to EN (pin 3) on the ESP32
G is connected to Ground (pins 1/38/15) on the ESP32
R is connected to GPIO1 / TxD (pin 35) on the ESP32
T is connected to GPIO3 / RxD (pin 34) on the ESP32

So we just need to connect
G on the PCB to Ground on the UART bridge/board
R on the PCB to Rx on the UART bridge/board
T on the PCB to Tx on the UART bridge/board

| Pin PCB | Pin UART |
| ------ | ------ |
| G | GND |
| R | Rx |
| T | Tx |

We also need a way to put the clock into the download mode, so we need some kind of a switch between
left header of BOOT 1 (to ESP32) and
right header of BOOT 1 (Ground).

Best way is to solder "sockets" to the existing headers, so that there longer pin side is pointing downwards from the not printed side of the PCB. Reason is easy, there is enough space between the backplate and the PCB to leave them soldered on for re-usage and they are not visible from the upper side.

![soldered sockets with backplate](/documentation/ImagesMD/PunkCyber_soldered_sockets.jpg)
Soldered sockets

![soldered sockets with backplate](/documentation/ImagesMD/PunkCyber_soldered_sockets_with_backplate.jpg)
Soldered sockets with backplate

BOOT needs to be shorted, while the USB-C cable is connected to one of the USB-C ports (while powering) and then released to go into the download mode.

Normally, all USB-UART bridges/boards should work, but for me, the UART bridge mode of the Flipper Zero (STM chip) didn't work properly! I have no idea why.

So I successfully used an old FT232 board, which was still flying around (with Mini-USB :)) to read/backup the orginal firmware and write the EleksTubeHAX version.

To get one on your own, search for "USB to UART" on your favorite seller platform, like the ones with A from US or PRC. Costs should not exceed 2-7 EUR/USD.

![PunkCyber connected via FT232](/documentation/ImagesMD/PunkCyber_connected_UART-USB_bridge_and_reset_button.jpg)
Working solution

![PunkCyber successfull backup](/documentation/ImagesMD/PunkCyber_backup_successful.jpg)
Successful backup

Thanks to @Fastdruid for finding a good way to overcome this problem! (see [Issue 62]('https://github.com/SmittyHalibut/EleksTubeHAX/issues/62'))

#### EleksTube Gen1: 5V on CH340 and ESP32

##### Problem

The "Original Version" (Gen1) of the Elekstube clock has a few issues in the hardware design. Most notably, it forces 5V signals from the USB-UART chip into the ESP32, which is not happy about it. This is outside of the safe operating limits and significantly reduces the lifetime of the ESP32. Mine (from the original author) died because of this...

##### Solution

The CH340 chip, used for USB-UART conversion, can operate both on 5V and 3.3V. On the Gen1 board it is powered by 5V.
We can switch it to 3.3V.
Cut one trace on the bottom side of the board that supplies the chip with 5V and route the 3.3V over the resistors / capacitors to VDD and VREF of the CH340.

![EleksStube Gen1 hardware modification](/documentation/ImagesMD/EleksTube_IPS_CH340C_mod.jpg)

Note: This problem does not appear on Gen2 hardware from Elekstube! All clocks sold today (Autumn 2024) do not require any hardware modification!

## Main Contributors

- Mark Smith, aka Smitty ... @SmittyHalibut on GitHub, Twitter, and YouTube.
- Aljaz Ogrin, aka aly-fly ... @aly-fly on GitHub and Instagram
- Misc code snips either commited by or copied from: @icebreaker-ch, @meddle99, @OggyP, @bitrot-alpha
- Home Assistant support by @victorvuelma and @gamba69
- in future (on to-do list) also from: @RedNax67, @wfdudley, @judge2005

_Happy hacking!_
