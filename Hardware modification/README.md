# Hardware modification
Original EleksTube has a few problems in the hardware design. Most notably it forces 5V signals into ESP32 which is not happy about it. And it is outside of safe operating limits. 
This will extend the lifetime of ESP32. Mine died because of this...
## Conversion
CH340 chip, used for USB-UART conversion can operate both on 5V and 3.3V. On the board it is powered by 5V. Cut one trace on the bottom side of the board that supplies the chip with 5V and route 3.3V over the resistors / capacitors to VDD and VREF.
