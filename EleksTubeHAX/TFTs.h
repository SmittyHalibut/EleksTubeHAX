#ifndef TFTS_H
#define TFTS_H

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
  void showDigit(uint8_t digit);

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
};



#endif // TFTS_H
