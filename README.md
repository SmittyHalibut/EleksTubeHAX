# EleksTubeHAX - An aftermarket custom firmware for the EleksTube IPS clock
Buy your own clock here: [EleksTube IPS Clock](https://www.banggood.com/Pseudo-glow-Tube-Programmable-Display-IPS-Screen-RGB-Clock-Desktop-Creative-Ornaments-Digital-Clock-Colorful-LED-Picture-Display-p-1789259.html)

[Reddit discussion on the hack is here.](https://www.reddit.com/r/arduino/comments/mq5td9/hacking_the_elekstube_ips_clock_anyone_tried_it/)

## Original Firmware
Check in [original-firmware/](original-firmware/) for a direct dump of the firmware as I received my clock, and instructions for how to restore it to the clock.  This is useful if you're hacking around and get some non-working code, and just want to restore it to original.

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
5. TFT pin 4, RS, ESP32 pin 10, IO25
6. TFT pin 3, RESET,  ESP32 pin 11, IO26
7. TFT pin 8, Chip Select, driven by 74HC595
8. WS2812, GND (Tied to 13)
  * Separate from TFT GND, not sure why, but here we are.
9. N/C
10. TFT pin 1 and 7, Vdd and LEDA (Tied to 12)
11. TFT pin 2, GND 
12. TFT pin 1 and 7, Vdd and LEDA (Tied to 10)
13. WS2812, GND  (Tied to 8)

## Chip Select Shift Register
There's a [74HC595](https://www.arduino.cc/en/Tutorial/Foundations/ShiftOut)
([datasheet](https://www.arduino.cc/en/uploads/Tutorial/595datasheet.pdf))
that drives the 6 SPI Chip Select lines on the displays.  Thankfully, they're in order:

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


