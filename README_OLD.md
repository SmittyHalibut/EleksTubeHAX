Author of the first section:
Mark Smith, aka Smitty
@SmittyHalibut on GitHub, Twitter, and YouTube.

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


=================================================================================================

Author of the following section:
Aljaz Ogrin, aka aly-fly ... @aly-fly on GitHub and Instagram

## Updates from original SmittyHalibut's version:

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
- (in works: ) Integrated web server to input MQTT connection details (or maybe load new clock faces)
