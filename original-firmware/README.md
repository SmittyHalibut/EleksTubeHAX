# Restore the original firmware
This directory is useful for if you just want to restore the original firmware, 
or if you hack your clock into a state you don't know how to recover from.

The file `fw-backup-4M.bin` is a full dump of the entire 4MB flash as I 
(`SmittyHalibut`) received it.  It's similar to the output of `dd if=/dev/sda of=file.img`
in that it includes the ENTIRE flash: boot loader, partition table, all the partitions, etc.

## Firmware Images Unique To Each Clock?
See https://github.com/SmittyHalibut/EleksTubeHAX/issues/8

Looks like the MAC address of the device is in the Firmware and needs updating.  Copied
from that github issue:
```
 neptune2 commented on May 29 •

Hi Jens @koshisan, I have found how to convert @SmittyHalibut original firmware in this repo to work on my clock!

The discovery is that the MAC address in the firmware must match the factory programmed MAC address in your clock.

Here are the steps (in Windows):

    Find the MAC address in your clock:
    esptool flash_id
    Look for the line that starts MAC:

    Replace all 42 instances of the MAC address in your copy of fw-backup-4M.bin.
    Note that the 6 MAC bytes in the firmware are in reverse byte order from the MAC: xx:xx:xx:xx:xx:xx format in step 1.
    I used WinMerge https://winmerge.org/ to edit the firmware binary with the following steps:

2a. Make a copy of fw-backup-4M.bin (I named it edited_fw-backup-4M.bin)
2b. Open both bin files side-by-side in WinMerge (they should be identical)
2c. Select the edited_fw-backup-4M.bin pane in WinMerge
2d. Edit->Replace <bh:80><bh:93><bh:2f><bh:84><bh:db><bh:e8> with <bh:xx><bh:xx><bh:xx><bh:xx><bh:xx><bh:xx>
     Click Replace all following occurrences (it may report 43 differences - there are actually 42)
     where the xx bytes are replaced with your clock MAC address bytes (reverse order from Step1. format). 
     The last 2-3 bytes should be the same.
2e. Save the edited_fw-backup-4M.bin file

    Restore the edited_fw-backup-4M.bin firmware to your clock:
    esptool --baud 115200 --port [COM port] write_flash 0x0 edited_fw-backup-4M.bin

Hopefully this also works on your clock.
Please reply with you results. 頑張って
```

## Writing Firmware To Your Clock
You can resture this image using `esptool.py`, which you have to get yourself.  I provide
a script `restore-firmware.sh` which uses `esptool.py` from my install of Arduino 1.8, 
but you can update it to point to where ever you have `esptool.py` installed.  (Similarly,
update for Python if needed.)

`fw-strings.txt` is the output of `strings fw-backup-4M.bin`. It might contain some useful
information about what was in the original firmware, such as what libraries were used, etc.

-Mark
