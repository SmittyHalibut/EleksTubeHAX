#/bin/bash

# This is Linux script to restore the flash from this backup.
# You'll want to update for your location of files, and if you're running Windows
# But the interesting bits (the paraleters to `esptool.py`) should be the same.

PYTHON=python
ESPTOOL=~/.arduino15/packages/esp32/tools/esptool_py/3.0.0/esptool.py

$PYTHON $ESPTOOL --baud 115200 --port /dev/ttyUSB1 write_flash 0x0 fw-backup-4M.bin
