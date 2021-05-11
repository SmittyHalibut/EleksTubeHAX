#include "TFTs.h"

void TFTs::begin() {
  // Start with all displays selected.
  chip_select.begin();
  chip_select.setAll();

  // Turn power on to displays.
  pinMode(TFT_ENABLE_PIN, OUTPUT);
  enableAllDisplays();

  // Initialize the super class.
  init();
}

/* 
 * Where the rubber meets the road.  Displays the bitmap for the value to the given digit. 
 */
void TFTs::showDigit(uint8_t digit) {
  // Dummy code, to just write text to the given display, for testing.
  chip_select.setDigit(digit);
  
  fillScreen(TFT_BLUE);
  setCursor(40, 80, 8);
  setTextColor(TFT_WHITE, TFT_BLUE);
  //print("Digit: ");
  //println(digit);
  //print("Value: ");
  println(digits[digit]);
}
