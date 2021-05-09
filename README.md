# EleksTubeHAX - An aftermarket custom firmware for the EleksTube IPS clock
Buy your own clock here: [EleksTube IPS Clock](https://www.banggood.com/Pseudo-glow-Tube-Programmable-Display-IPS-Screen-RGB-Clock-Desktop-Creative-Ornaments-Digital-Clock-Colorful-LED-Picture-Display-p-1789259.html)

[Reddit discussion on the hack is here.](https://www.reddit.com/r/arduino/comments/mq5td9/hacking_the_elekstube_ips_clock_anyone_tried_it/)

## Original Firmware
Check in [original-firmware/](original-firmware/) for a direct dump of the firmware as I received my clock, and instructions for how to restore it to the clock.  This is useful if you're hacking around and get some non-working code, and just want to restore it to original.

# Documentation
## Hardware
* Microcontroller: [ESP32-WROOM-32D](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32d_esp32-wroom-32u_datasheet_en.pdf)
* Displays: [TFT 1.14" 135x240](https://drive.google.com/file/d/1yNx8xv3QHelXeaZiQ4Ia_Et1136Wh8VI/view)
  * This isn't the exact module, but it looks like the same display.

## Libraries
* Talking to displays: [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
* Real Time Clock: [DS3231 RTC](https://circuitdigest.com/microcontroller-projects/esp32-real-time-clock-using-ds3231-module)
* [Driving RGB LEDs using 74HC595](https://www.youtube.com/watch?v=hqVpxCPFaQk)

# Contributers
Mark Smith, aka Smitty
@SmittyHalibut on GitHub, Twitter, and YouTube.


