# EleksTubeHAX - An aftermarket custom firmware for the desk clock
![EleksTube IPS clock](/Photos/Clock1.jpg)

Supported hardware models:
### "EleksTube IPS clock", "SI HAI IPS clock", "NovelLife SE clock", "PunkCyber clock", "RGB Glow Tube DIY clock"

In this folder you can find pre-built firmware images that you can upload to your clock.

Note that these firmwares may have limited functionality.

## Enabled functions:
- WPS connectivity (press WPS button on your router for WiFi setup).
- DEBUG_OUTPUT - it will produce diagnostic messages on the serial port.
- DCORE_DEBUG_LEVEL=5 - it will produce diagnostic messages from the operating system.
- Night time from 22h in the evening to 7h in the morning.
- No geolocation (time zone & DST must be set manually).
- No MQTT (without remote control).
- No thermometer.
- Image files are fixed and can't be changed without development tools.

### Select appropriate .bin file that fits your hardware and upload it using the `_ESP32 write flash.cmd` file. 
Available files:
- original Elekstube clock - `FW_Elekstube_HAX_1.0_original.bin`
- original Elekstube clock Gen2.1 (ESP32 Pico D4 Chip) - `FW_Elekstube_HAX_1.0_Gen2-1.bin`
- SI HAI copy of the clock - `FW_SI_HAI_CLOCK_HAX_1.0.bin`
- NovelLife SE version (non-SE not tested) - `FW_NovelLife_SE_HAX_1.0.bin`
- PunkCyber / RGB Glow tube / PCBway clock - `FW_PunkCyber_Glow_PCBway_HAX_1.0.bin`

Make sure to edit it and write correct COM port and file name into it before running.

## Save your original firmware using the `_ESP32 save flash 4MB.cmd` before tinkering.

## There is no warranty of any type.
