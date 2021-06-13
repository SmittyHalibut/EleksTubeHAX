# Makefile for EleksTubeHAX
#
# Modified from that of Valerio Di Giampietro http://va.ler.io v@ler.io
#
# Please note that:
#
#   1. make setup should only be run once.
#
#   2. the main make targets are:
#      - all     compiles and upload
#      - compile compiles only
#      - upload  upload via serial port, compile if the binary file is
#                not available
#      - clean   clean the build directory
#      - setup   install libraries from "requirements.txt"
#
#      default is "all"
#
#   3. it gets the name of the sketch using the wildcard make command;
#      the name is *.ino; this means that you must have ONLY a file
#      with .ino extension, otherwise this makefile will break.  This
#      also means that you can use this Makefile, almost unmodified,
#      for any sketch as long as you keep a single .ino file in each
#      folder
#
#   4. you can split your project in multiple files, if you wish,
#      using a single .ino file and multiple .h files, that you can
#      include in the .ino file with an '#include "myfile.h"'
#      directive
#
# Optionally some environment variables can be set:
#
#   FQBN        Fully Qualified Board Name; if not set in the environment
#               it will be assigned a value in this makefile
#
#   SERIAL_DEV  Serial device to upload the sketch; if not set in the
#               environment it will be assigned:
#               /dev/ttyUSB0   if it exists, or
#               /dev/ttyACM0   if it exists, or
#               unknown
#
#   SERIAL_BAUD Set the baud rate.
#
#   IOT_NAME    Name of the IOT device; if not set in the environment
#               it will be assigned a value in this makefile. This is
#               very useful for OTA update, the device will be searched
#               on the local subnet using this name
#
#   OTA_PORT    Port used by OTA update; if not set in the environment
#               it will be assigned the default value of 8266 in this
#               makefile
#
#   OTA_PASS    Password used for OTA update; if not set in the environment
#               it will be assigned the default value of an empty string
#
#   V           verbose flag; can be 0 (quiet) or 1 (verbose); if not set
#               in the environment it will be assigned a default value
#               in this makefile


MAKE_DIR   := $(PWD)
#
FQBN        ?= esp32:esp32:esp32
# ----- ---------------------
VFLAG      =

ifeq "$(V)" "1"
VFLAG      =-v
endif

ifndef SERIAL_DEV
  ifneq (,$(wildcard /dev/ttyUSB0))
    SERIAL_DEV = /dev/ttyUSB0
  else ifneq (,$(wildcard /dev/ttyACM0))
    SERIAL_DEV = /dev/ttyACM0
  else
    SERIAL_DEV = unknown
  endif
endif

SERIAL_BAUD := 921600

BUILD_DIR  := build

SRC        := $(wildcard *.ino)
HDRS       := $(wildcard *.h *.cpp)
BIN        := $(BUILD_DIR)/$(SRC).bin
SPIFFS     := $(BUILD_DIR)/spiffs.bin
DATADIR    := data

ARDUINO    := arduino-cli
MKSPIFFS   := $(shell find ~/.arduino* -type f -name mkspiffs | head -n1)
ESPTOOL    := $(shell find ~/.arduino* -type f -name esptool.py | head -n1) --chip esp32 --port $(SERIAL_DEV) --baud $(SERIAL_BAUD)

$(info FQBN        is [${FQBN}])
$(info IOT_NAME    is [${IOT_NAME}])
$(info OTA_PORT    is [${OTA_PORT}])
$(info OTA_PASS    is [${OTA_PASS}])
$(info VFLAG       is [${VFLAG}])
$(info MAKE_DIR    is [${MAKE_DIR}])
$(info BUILD_DIR   is [${BUILD_DIR}])
$(info SRC         is [${SRC}])
$(info HDRS        is [${HDRS}])
$(info BIN         is [${BIN}])
$(info SERIAL_DEV  is [${SERIAL_DEV}])
$(info SERIAL_BAUD is [${SERIAL_BAUD}])
$(info ARDUINO is [${ARDUINO}])
$(info ESPTOOL is [${ESPTOOL}])
$(info MKSPIFFS is [${MKSPIFFS}])

.PHONY: all
all: compile upload

.PHONY: compile
compile: $(BIN) $(SPIFFS)

.PHONY: upload
upload: upload_bin upload_spiffs

.PHONY: upload_bin
upload_bin:
	$(ARDUINO) upload -b $(FQBN) --input-dir $(BUILD_DIR) -p $(SERIAL_DEV) $(VFLAG)

## ota:
## 	@PLAT_PATH=`arduino-cli compile -b $(FQBN) --show-properties | grep '^runtime.platform.path' | awk -F= '{print $$2}'` ; \
## 	   PY_PATH=`arduino-cli compile -b $(FQBN) --show-properties | grep '^runtime.tools.python3.path' | awk -F= '{print $$2}'` ; \
## 	IOT_IP=`avahi-browse _arduino._tcp --resolve --parsable --terminate|grep -i ';$(IOT_NAME);'|grep ';$(OTA_PORT);'| awk -F\; '{print $$8}'|head -1`; \
## 	BINFILE=$(wildcard $(BUILD_DIR)/$(SRC)*bin); \
## 	echo "PLAT_PATH  is [$$PLAT_PATH]" ; \
## 	echo "PY_PATH:   is [$$PY_PATH]"  ; \
## 	echo "IOT_IP:    is [$$IOT_IP]"   ; \
## 	echo "BINFILE:   is [$$BINFILE]"  ; \
## 	if [ "$$IOT_IP" = "" ] ; \
## 	then echo "Unable to find device IP. Check that the IOT_NAME environment variable is correctly set. Use 'make find' to search devices"; \
## 	else echo "---> Uploading Over The Air"; \
## 	$$PY_PATH/python3 $$PLAT_PATH/tools/espota.py -i $$IOT_IP -p $(OTA_PORT) --auth=$(OTA_PASS) -f $$BINFILE ;\
## 	fi

# Offset from menu.PartitionScheme.default
# .arduino15/packages/esp32/hardware/esp32/1.0.6/tools/partitions
.PHONY: upload_spiffs
upload_spiffs: $(SPIFFS)
	$(ESPTOOL) write_flash -z 0x290000 $(SPIFFS)

.PHONY: download_image
download_image:
	$(ESPTOOL) read_flash 0x0 0x400000 fw-backup.bin

$(BIN): $(SRC) $(HDRS)
	$(ARDUINO) compile -b $(FQBN) --output-dir $(BUILD_DIR) $(VFLAG)

$(SPIFFS): $(wildcard $(DATA_DIR)/*)
	$(MKSPIFFS) -c $(DATADIR) -s 1507328 $(SPIFFS)

.PHONY: clean
clean:
	@echo "---> Cleaning $(BUILD_DIR)"
	rm -r $(BUILD_DIR)

.PHONY: setup
setup:
	$(ARDUINO) config add board_manager.additional_urls https://dl.espressif.com/dl/package_esp32_index.json
	@if [ -e requirements.txt ]; \
	then while read -r i ; do echo ; \
	  echo "---> Installing " '"'$$i'"' ; \
	  $(ARDUINO) lib install "$$i" ; \
	done < requirements.txt ; \
	else echo "---> MISSING requirements.txt file"; \
	fi
	$(info Follow README.md for updating the User_Setup.h file.)

