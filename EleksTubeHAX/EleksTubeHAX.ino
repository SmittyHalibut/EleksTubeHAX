#include <stdint.h>
#include "Buttons.h"
#include "Backlights.h"

Backlights backlights;
Buttons buttons;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("");
  Serial.println("In setup().");  

  backlights.begin();
  //backlights.setPattern(Backlights::constant);
  buttons.begin();

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
