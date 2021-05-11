#include <stdint.h>
#include "Buttons.h"
#include "Backlights.h"
#include "TFTs.h"

Backlights backlights;
Buttons buttons;
TFTs tfts;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("");
  Serial.println("In setup().");  

  backlights.begin();
  buttons.begin();
  tfts.begin();
  tfts.fillScreen(TFT_BLACK);
  tfts.setTextColor(TFT_WHITE, TFT_BLACK);
  tfts.setCursor(0, 0, 4);
  tfts.println("setup()");
  tfts.setTextColor(TFT_RED, TFT_BLACK);
  tfts.println("Red Text");
  tfts.setTextColor(TFT_GREEN, TFT_BLACK);
  tfts.println("Green Text");
  tfts.setTextColor(TFT_BLUE, TFT_BLACK);
  tfts.println("Blue Text");
  for (uint8_t ndx=0; ndx < 5; ndx++) {
    tfts.print(".");
    delay(1000);
  }
  tfts.showAllDigits();
  Serial.println("Done with setup().");
}

void loop() {
  buttons.loop();
  backlights.loop();

  if (buttons.left.isDownEdge()) {
    backlights.setPrevPattern();
    Serial.print("Pattern: ");
    Serial.println(backlights.getPatternStr());
  }
  if (buttons.right.isDownEdge()) {
    backlights.setNextPattern();
    Serial.print("Pattern: ");
    Serial.println(backlights.getPatternStr());
  }
  if (buttons.mode.isDownEdge()) {
    tfts.setDigit(0, tfts.getDigit(0)+1);
  }
  if (buttons.power.isDownEdge()) {
    tfts.setDigit(0, tfts.getDigit(0)-1);
  }
  
  /*
  if (buttons.left.stateChanged()) {
    Serial.print("Left: ");
    Serial.println(buttons.left.getStateStr());
  }
  if (buttons.right.stateChanged()) {
    Serial.print("Right: ");
    Serial.println(buttons.right.getStateStr());
    backlights.setNextPattern();
  }
  if (buttons.mode.stateChanged()) {
    Serial.print("Mode: ");
    Serial.println(buttons.mode.getStateStr());
  }
  if (buttons.power.stateChanged()) {
    Serial.print("Power: ");
    Serial.println(buttons.power.getStateStr());
  }
*/
  delay(20);
}
