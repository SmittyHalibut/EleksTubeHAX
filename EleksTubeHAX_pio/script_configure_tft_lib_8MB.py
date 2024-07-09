# Import("env")  # should not be required; un-comment if you have issues
import shutil

# print("Current CLI targets", COMMAND_LINE_TARGETS)
# print("Current Build targets", BUILD_TARGETS)

print("===== copying TFT config files ===== ")

# https://stackoverflow.com/questions/123198/how-to-copy-files

# source file location: EleksTubeHAX_pio\src\GLOBAL_DEFINES.h  &  _USER_DEFINES.h
# target file location: EleksTubeHAX_pio\.pio\libdeps\esp32dev8MB\TFT_eSPI\User_Setup.h

# copy using Python libraries
# "copy" changes file timestamp -> lib is always recompiled.
# "copy2" keeps file timestamp -> lib is compiled once
ret = shutil.copy2('./src/_USER_DEFINES.h', './.pio/libdeps/esp32dev8MB/TFT_eSPI')
print("Copied {ret}".format(**locals()))
ret2 = shutil.copy2('./src/GLOBAL_DEFINES.h', './.pio/libdeps/esp32dev8MB/TFT_eSPI/User_Setup.h')
print("Copied {ret2}".format(**locals()))

# copy using Windows command line
# native "copy" command keeps file timestamp -> lib is compiled once
# env.Execute("copy .\\src\\_USER_DEFINES.h .\\.pio\\libdeps\\esp32dev8MB\\TFT_eSPI")
# env.Execute("copy .\\src\\GLOBAL_DEFINES.h .\\.pio\\libdeps\\esp32dev8MB\\TFT_eSPI\\User_Setup.h")

print("Done copying TFT config files!")
