# EleksTubeHAX - An aftermarket custom firmware for the EleksTube IPS clock
Buy your own clock here: [EleksTube IPS Clock](https://www.banggood.com/Pseudo-glow-Tube-Programmable-Display-IPS-Screen-RGB-Clock-Desktop-Creative-Ornaments-Digital-Clock-Colorful-LED-Picture-Display-p-1789259.html)

[Reddit discussion on the hack is here.](https://www.reddit.com/r/arduino/comments/mq5td9/hacking_the_elekstube_ips_clock_anyone_tried_it/)

[Original documentation and software from EleksMaker.](https://wiki.eleksmaker.com/doku.php?id=ips)

# How to build this firmware
Unfortunately, it's not simple plug-and-play.  You need to do some things.  These instructions assume you already know how to use the Arduino IDE, and just need to know WHAT to do.

## Download this code
You're either reading this file after downloading it already, or you're reading it on github.  I'll assume you can figure out how to get the code from github and put it somewhere on your local machine.  This is your preference.

### Configure your WiFi network
In the source code directory (where ever you just installed it), copy `wifi_creds-example.h` to `wifi_creds.h` and edit it for your WiFi SSID and Password.

Note that `wifi_creds.h` is in `.gitignore`, so you can safely put credentials in that file and still submit pull requests, push code, whatever.  git will never store your credentials in a repository.

## Setup Arduino IDE
Development was done on Arduino 1.8.13.  It might work on earlier or later versions, I don't know.

### Windows Only: Install the USB Serial Port Device Driver (??)
I'm not a Windows user, but the [EleksTube instructions](https://wiki.eleksmaker.com/doku.php?id=ips) have you installing a driver to get the serial port to work.  So I assume that's necessary.  It seemed to work out of the box on my Ubuntu 16.04 machine.

### Install ESP32 board support from Espressif
File -> Preferences, add `https://dl.espressif.com/dl/package_esp32_index.json` to your Board Manager URLs.  (Comma separated if there's already something there.)

Tools -> Board -> Board Manager, add the `esp32` boards by `Espressif Systems`.  Development was done on v1.0.6.

Tools -> Board -> ESP32 Arduino -> ESP32 Dev Module

The default configs in the Tools menu should be fine. The important ones are:
* Flash Size: 4MB
* Partition Scheme: Any that includes at least 1MB for SPIFFS.  I use the "Default 4MB: 1.2MB App, 1.5MB SPIFFS" one.
* Port: Set it to whatever serial port your clock shows up as when plugged in.

### Install Libraries
All these libraries are in Library Manager.  Several libraries have very similar names, so make sure you select the correct one based on the author.
The listed "developed on" versions are just the versions I had installed while developing this code.  Newer (or possibly older) versions should be fine too.

Sketch -> Include Library -> Library Manager
* `NTPClient` by Fabrice Weinberg (developed on v3.2.0)
* `Adafruit NeoPixel` by Adafruit (developed on v1.8.0)
* `DS1307RTC` by Michael Margolis (developed on v1.4.1)
* `TFT_eSPI` by Bodmer (developed on v2.3.61)
* `Time` by Michael Margolis (developed on v1.6.0)

### Configure the `TFT_eSPI` library
**IMPORTANT** You have to do this after every time you install or update the `TFT_eSPI` library!  **IMPORTANT**

The full documentation for this is in the `TFT_eSPI` library, but tl,dr:
* Edit `Arduino/libraries/TFT_eSPI/User_Setup_Select.h`
* Comment out all `#include` lines.  (The only one that comes from install is `#include <User_Setup.h>`.)
* Add a `#include` line pointing to `User_Setup.h` in this code.
  * eg: `#include </home/foo/src/EleksTubeHAX/EleksTubeHAX/User_Setup.h>`
  * Obviously, update the path to point to where ever you keep your code.  Mac and Windows paths will look very different.

### Install SPIFFS uploader
The code is [here](https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/), and instructions to install it are [here](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/).

tl,dr: Download and unzip [this file](https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/download/1.0/ESP32FS-1.0.zip) into your `Arduino/tools/` directory.  There might not be a `tools/` subdirectory yet, but if you see `Arduino/libraries/` that's the right place.

### Restart Arduino
After installing the ESP32 support, all the libraries, and the SPIFFS uploader, restart Arduino to make sure it knows its all there.
 
## Upload New Firmware
Make sure you configured everything:
* Put your wifi credentials in `wifi_creds.h`
* Pointed `User_Setup_Select.h` in the TFT_eSPI library to our `User_Setup.h`

Connect the clock to your computer with USB.  You'll see a new serial port pop up.  Make sure that's the serial port selected in Tools.

### Compile and Upload the Code
Compile (Ctrl-R) and Upload (Ctrl-U) the code.  At this point, it should upload cleanly and successfully.  You'll see the clock boot up and connect to WiFi.  But it doesn't have any bitmaps to display on the screen yet.

### Upload Bitmaps
The repository comes with a set of BMP files, nixie tubes from the original firmware, in the `data/` directory. See below if you want to make your own.

Tools -> ESP32 Sketch Data Upload, will upload the files to the SPIFFS filesystem on the micro.  They'll stay there, even if you re-upload the firmware multiple times.

### Custom Bitmaps (Optional)
If you want to change these:
* Create your own BMP files.  Resolution must be 135x240 pixels, 24bit RGB.
* Name them `0.bmp` through `9.bmp` and put them in the `data/` directory.

Then do the "Tools -> ESP32 Sketch Data Upload" dance again.

## Setup the Arduino IDE (CLI)

Install the [arduino-cli](https://arduino.github.io/arduino-cli/latest/installation/).

`$ make setup`

You'll still need to [configure the `TFT_eSPI` library](#configure-the-tft_espi-library).

### Compile and Upload

```
export SERIAL_DEV=/dev/ttyUSB0
$ make compile
$ make upload_bin upload_spiffs
```

or simply

`$ make SERIAL_DEV=/dev/ttyUSB0``

# Development Process:
## Original Firmware
Check in [original-firmware/](original-firmware/) for a direct dump of the firmware as I received my clock, and instructions for how to restore it to the clock.  This is useful if you're hacking around and get some non-working code, and just want to restore it to original.

## Unpacking BMPs from original firmware
Download and unpack the original software (link above).  It contains a directory called `IPSimages/` which contains several pre-made SPIFFS images full of the BMPs available in the original software.  You can see what they all look like in the `gallery/` directory, same numbers.

To unpack one of these SPIFFS images into the original BMPs:
* Make a destination directory, eg: `unpacked/`
* Run: `mkspiffs -u unpacked/ [image].bin`
  * This assumes you've already installed ESP32 support in Arduino. `mkspiffs` comes with the ESP32 tools.  On my Linux system, it's in `~/.arduino15/packages/esp32/tools/mkspiffs/0.2.3/mkspiffs`.
  * If you're on Windows, the IPS software also comes with `mkspiffs.exe` which I assume works the same way, but I haven't confirmed.

This puts 12 files in `unpacked/`:
* `0.bmp` through `9.bmp` which are 135x240px 24 bit BMPs for the 10 digits
* `month.bmp` and `date.bmp` another couple BMPs, but I'm not sure where they're ever used.  We won't need these in our firmware, so they can be deleted.

# Documentation
## Hardware
* Microcontroller: ESP32-WROOM-32D
  * [Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf)
  * [Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
* Displays: [TFT 1.14" 135x240](https://www.aliexpress.com/item/32564432870.html)  (includes pinout of ribbon connector)
  * ST7789 controller [Datasheet](https://www.rhydolabz.com/documents/33/ST7789.pdf)
* RGB LED: SK6812 side emmitting
  * [Datasheet](http://www.ledlightinghut.com/files/SK6812%20side%204020.pdf)
  * NeoPixel/WS2812 compatible.

## Libraries
* Talking to displays: [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
* Real Time Clock: [DS3231 RTC](https://circuitdigest.com/microcontroller-projects/esp32-real-time-clock-using-ds3231-module)
* NeoPixel library for RGB LEDs (link coming)

# Hardware 
My (SmittyHalibut) notes from reverse engineering the board

## Display boards
The card edge connector has 13 connection points on each side, but both sides are tied together, so there are only 13 unique pins.

The socket on the board is oriented so that Pin 1 is on the RIGHT SIDE (look for the little arrow on the top of the socket.)  So 
the documentation below is RIGHT TO LEFT.

1. WS2812 In
2. WS2812 Out
3. TFT pin 5, SDA, ESP32 pin 37, IO23, VSPID
4. TFT pin 6, SCL, ESP32 pin 30, IO18, VSPICLK
5. TFT pin 4, RS (Register Select, or DC Data Command), ESP32 pin 10, IO25
6. TFT pin 3, RESET (active low),  ESP32 pin 11, IO26
7. TFT pin 8, Chip Select, driven by 74HC595
8. WS2812, GND (Tied to 13)
9. N/C
10. TFT pin 1 and 7, Vdd and LEDA (Tied to 12)
11. TFT pin 2, GND 
  * Tied to system ground through a MOSFET, controlled by ESP32 pin 12, IO27, so the displays can be completely turned off.
  * (If I (SmittyHalibut) were doing this, I'd have used a P channel MOSFET and controlled LEDA, which would allow dimming as well as completely shutting it off. Oh well.)
12. TFT pin 1 and 7, Vdd and LEDA (Tied to 10)
13. WS2812, GND  (Tied to 8)

## Chip Select Shift Register
There's a [74HC595](https://www.arduino.cc/en/Tutorial/Foundations/ShiftOut)
([datasheet](https://www.arduino.cc/en/uploads/Tutorial/595datasheet.pdf))
that drives the 6 SPI Chip Select lines on the displays.  Chip Select lines are Active Low,
so write 1s to the displays you do NOT want to update, a 0 to the display you want to update.

Q0 is the most recent bit written to the shift register, Q7 is the oldest bit written.

Outputs:
* Q0: Hour Tens
* Q1: Hour Ones
* Q2: Minute Tens
* Q3: Minute Ones
* Q4: Seconds Tens
* Q5: Seconds Ones
* Q6 and Q7: Unused, not connected.

Inputs:
* Ds (Data In): ESP32 pin 13, IO14, GPIO14
* /OE (Output Enable): Strapped to Ground, always enabled.
* STcp (Storage Register Clock Input): ESP32 pin 28, IO17, GPIO17
* SHcp (Shift Register Clock Input): ESP32 pin 27, IO16, GPIO16

## WS2812(?) RGB LEDs
They only require a single GPIO pin to drive all 6 LEDs.  They are driven in reverse order, right to left.  The first LED is Seconds Ones, the sixth LED is Hours Tens.

* ESP32 pin 14, IO12

## Buttons
All 4 buttons are externally pulled up (an actual 10k resistor!) and shorted to ground by the button.

* `<<<`: ESP32 pin 9, IO33
* `MODE`: ESP32 pin 8, IO32
* `>>>`: ESP pin 7, IO35
* `POWER`: ESP pin 6, IO34

## RTC DS3231
The DS3231 ([datasheet](https://datasheets.maximintegrated.com/en/ds/DS3231.pdf)) is an I2C device, and a very common one at that. Lots of good documentation and libraries already.

* SCL: ESP32 pin 36, IO22
* SDA: ESP32 pin 33, IO21

# Contributers
Mark Smith, aka Smitty
@SmittyHalibut on GitHub, Twitter, and YouTube.


