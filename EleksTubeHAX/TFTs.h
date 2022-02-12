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
    { for (uint8_t digit=0; digit < NUM_DIGITS; digit++) digits[digit] = 0; }

  // no == Do not send to TFT. yes == Send to TFT if changed. force == Send to TFT.
  enum show_t { no, yes, force };
  // A digit of 0xFF means blank the screen.
  const static uint8_t blanked = 255;

  uint8_t dimming = 255; // amount of dimming graphics
  uint8_t current_graphic = 0;
  
  void begin();
  void clear();
  void showNoWifiStatus();

  void setDigit(uint8_t digit, uint8_t value, show_t show=yes);
  uint8_t getDigit(uint8_t digit)                 { return digits[digit]; }

  void showAllDigits()               { for (uint8_t digit=0; digit < NUM_DIGITS; digit++) showDigit(digit); }
  void showDigit(uint8_t digit);

  // Controls the power to all displays
  void enableAllDisplays()           { digitalWrite(TFT_ENABLE_PIN, HIGH); enabled = true; }
  void disableAllDisplays()          { digitalWrite(TFT_ENABLE_PIN, LOW); enabled = false; }
  void toggleAllDisplays()           { if (enabled) disableAllDisplays(); else enableAllDisplays(); }
  bool isEnabled()                   { return enabled; }

  // Making chip_select public so we don't have to proxy all methods, and the caller can just use it directly.
  ChipSelect chip_select;

private:
  uint8_t digits[NUM_DIGITS];
  bool enabled;

  // These BMP functions are stolen straight from TFT_eSPI example code, the TFT_SPIFFS_BMP example.
  // Unfortunately, it's not part of the library.
  bool drawBmp(const char *filename, int16_t x, int16_t y);
  uint16_t read16(fs::File &f);
  uint32_t read32(fs::File &f);

  static uint16_t output_buffer[TFT_HEIGHT][TFT_WIDTH];

};

#endif // TFTS_H
