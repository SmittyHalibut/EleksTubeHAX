#ifndef TFTS_H
#define TFTS_H

// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>
#include "SPIFFS.h"  // For ESP32 only

#include <TFT_eSPI.h>
#include "Hardware.h"
#include "ChipSelect.h"

class TFTs : public TFT_eSPI {
public:
  TFTs() : TFT_eSPI(), chip_select(), enabled(false)
    { for (uint8_t digit=0; digit < NUM_DIGITS; digit++) digits[digit] = 0; }

  // no == Do not send to TFT. yes == Send to TFT if changed. force == Send to TFT.
  enum show_t { no, yes, force };
  void begin();
  void showAllDigits()  { for (uint8_t digit=0; digit < NUM_DIGITS; digit++) showDigit(digit); }
  void showDigit(uint8_t digit, bool debug=false);

  // Controls the power to all displays
  void enableAllDisplays()    { digitalWrite(TFT_ENABLE_PIN, HIGH); enabled = true; }
  void disableAllDisplays()   { digitalWrite(TFT_ENABLE_PIN, LOW); enabled = false; }
  bool isEnabled()            { return enabled; }

  void setDigit(uint8_t digit, uint8_t value, show_t show=yes);
  uint8_t getDigit(uint8_t digit)                             { return digits[digit]; }

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

};

#endif // TFTS_H
