# Restore the original firmware
This directory is useful for if you just want to restore the original firmware, 
or if you hack your clock into a state you don't know how to recover from.

The file `fw-backup-4M.bin` is a full dump of the entire 4MB flash as I 
(`SmittyHalibut`) received it.  It's similar to the output of `dd if=/dev/sda of=file.img`
in that it includes the ENTIRE flash: boot loader, partition table, all the partitions, etc.

You can resture this image using `esptool.py`, which you have to get yourself.  I provide
a script `restore-firmware.sh` which uses `esptool.py` from my install of Arduino 1.8, 
but you can update it to point to where ever you have `esptool.py` installed.  (Similarly,
update for Python if needed.)

`fw-strings.txt` is the output of `strings fw-backup-4M.bin`. It might contain some useful
information about what was in the original firmware, such as what libraries were used, etc.

-Mark
