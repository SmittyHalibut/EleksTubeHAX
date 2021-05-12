#include <stdint.h>
#include "Buttons.h"
#include "Backlights.h"
#include "TFTs.h"
#include "Clock.h"

// put two lines in here defining `const char *ssid` and `const char *password`.
// The file wifi_creds.h is in .gitignore so we don't risk accidentally sharing our creds with the world.
#include "wifi_creds.h"

Backlights backlights;
Buttons buttons;
TFTs tfts;
Clock uclock;


void updateClockDisplay(TFTs::show_t show=TFTs::yes) {
  tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), show);
  tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), show);
  tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), show);
  tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), show);
  tfts.setDigit(SECONDS_TENS, uclock.getSecondsTens(), show);
  tfts.setDigit(SECONDS_ONES, uclock.getSecondsOnes(), show);
}


void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("");
  Serial.println("In setup().");  

  // Setup TFTs
  backlights.begin();
  buttons.begin();
  tfts.begin();
  tfts.fillScreen(TFT_BLACK);
  tfts.setTextColor(TFT_WHITE, TFT_BLACK);
  tfts.setCursor(0, 0, 2);
  tfts.println("setup()");

  // Setup WiFi connection. Must be done before setting up Clock.
  // This is done outside Clock so the network can be used for other things.
  tfts.print("Joining wifi.");
  
  // ssid and password are defined in `wifi_creds.h`
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tfts.print(".");
  }
  tfts.println("\nDone.");

  // Setup the clock.  It needs WiFi to be established already.
  uclock.begin();

  // Leave boot up messages on screen for a few seconds.
  for (uint8_t ndx=0; ndx < 5; ndx++) {
    tfts.print(".");
    delay(1000);
  }

  // Setup initial clock display.
  updateClockDisplay(TFTs::force);
  
  Serial.println("Done with setup().");
}

void loop() {
  // Do all the maintenance work
  buttons.loop();
  backlights.loop();
  uclock.loop();

  // Update the clock.
  updateClockDisplay();
  
  /*
  // Backlight pattern debug
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
  */
  
  /* 
  // Buttons debug
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
