# EleksTubeHAX - An aftermarket custom firmware for the desk clock

## Pre-built firmware files

In this folder you can find pre-built firmware image files that you can directly upload to your clock.

These firmwares may have limited functionality compared to a self-built firmware, because not all available features are enabled or can be pre-configured.

These image files are fixed and can't be changed without development tools!

## Pre-configured functionality

- WiFi connectivity via WPS (While clock is in boot phase, press WPS button on your router for WiFi setup).
- Night time dimming enabled - from 22:00h (10 pm) in the evening to 07:00h (7 am) in the morning.
- IP-based geolocation disabled - time zone & DST must be set manually via the clocks menu.
- MQTT disabled - no 'remote control'.
- External thermometer sensor disabled.
- DEBUG_OUTPUT enabled - This produces diagnostic messages from the firmware over the serial port.
- DCORE_DEBUG_LEVEL=5 - This produces diagnostic messages from the ESP32 operating system in case of an error over the serial port.

### Available image files

| clock model | firmware image file |
|--|--|  
| EleksTube IPS - Orginal Version | `FW_Elekstube_HAX_1.0_original.bin` |
| EleksTube IPS - Gen2 models | `FW_Elekstube_HAX_1.0_Gen2-1.bin` |
| SI HAI IPS | `FW_SI_HAI_CLOCK_HAX_1.0.bin` |
| NovelLife SE version | `FW_NovelLife_SE_HAX_1.0.bin` |
| PunkCyber/RGB Glow Tube DIY | `FW_PunkCyber_Glow_PCBway_HAX_1.0.bin` |
| IPSTUBE - Model H401 | TBD |

Note: All "Original" EleksTube clocks, sold after July 2022 are "Gen2" versions. See [Note on EleksTube website](https://elekstube.com/blogs/news/instructions-on-elekstube-clock-for-gen2-systems). But always check the PCB version of your clock!

## Backup your original firmware

**Always backup YOUR clocks firmware version as first step!**

Note for original EleksTube clocks: Backup images from other users **DO NOT WORK** as the original EleksTube firmware is locked to the MAC address of the ESP32.

For other clocks it MAY work, but don't assume it!

Save your original firmware using the `_ESP32 save flash 4MB.cmd` (or 8MB version for the IPSTUBE) by changing the COM port to the number, your clock uses.

Rename and store the `backup1.bin` on a save location.

See also the section "Backup first" and following in the `README.MD` file in the root.

## Write the EleksTubeHAX firmware file

- Choose the right pre-built firmware file for your clock.
- Edit the file `_ESP32 write flash.cmd` with an editor.
- Write the correct COM port of your clock.
- Write the correct firmware file name for your clock.
- Run the CMD file.

Note: Most clocks will go into to the "download mode" automatically when esptool is trying to write to it.
Some clocks needs a button pressed while the powering phase (plugging the USB cable) to enter this mode, like the IPSTUBE ones. 

## There is no warranty of any type

Use at your own risk!

If you mess-up your clock, it's only your fault!
