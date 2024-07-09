#ifndef TFTS_H
#define TFTS_H

#include "GLOBAL_DEFINES.h"

// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>
#include "SPIFFS.h"  // For ESP32 only

#include <TFT_eSPI.h>
#include "ChipSelect.h"


class TFTs : public TFT_eSPI {
public:
  TFTs() : TFT_eSPI(), chip_select(), enabled(false)
  { 
    #ifndef HARDWARE_IPSTUBE_H401_CLOCK
      for (uint8_t digit=0; digit < NUM_DIGITS; digit++) digits[digit] = 0;
    #endif
  }

  // no == Do not send to TFT. yes == Send to TFT if changed. force == Send to TFT.
  enum show_t { no, yes, force };
  // A digit of 0xFF means blank the screen.
  const static uint8_t blanked = 255;

  uint8_t dimming = 255; // amount of dimming graphics
  uint8_t current_graphic = 1;
  
  void begin();
  void reinit();
  void clear();
  void showNoWifiStatus();
  void showNoMqttStatus();
  void showTemperature();

  void setDigit(uint8_t digit, uint8_t value, show_t show=yes);
  uint8_t getDigit(uint8_t digit)                 { return digits[digit]; }

  void showAllDigits()               { for (uint8_t digit=0; digit < NUM_DIGITS; digit++) showDigit(digit); }
  void showDigit(uint8_t digit);

  // Controls the power to all displays
  void enableAllDisplays();
  void disableAllDisplays();
  void toggleAllDisplays();
  bool isEnabled()                   { return enabled; }

  // Making chip_select public so we don't have to proxy all methods, and the caller can just use it directly.
  ChipSelect chip_select;

  uint8_t NumberOfClockFaces = 0;
  void LoadNextImage();
  void InvalidateImageInBuffer(); // force reload from Flash with new dimming settings

private:
  uint8_t digits[NUM_DIGITS];
  bool enabled;

  bool FileExists(const char* path);
  int8_t CountNumberOfClockFaces();
  bool LoadImageIntoBuffer(uint8_t file_index);
  void DrawImage(uint8_t file_index);
  uint16_t read16(fs::File &f);
  uint32_t read32(fs::File &f);

  static uint16_t UnpackedImageBuffer[TFT_HEIGHT][TFT_WIDTH];
  uint8_t FileInBuffer=255; // invalid, always load first image
  uint8_t NextFileRequired = 0; 
};

extern TFTs tfts;


#endif // TFTS_H
