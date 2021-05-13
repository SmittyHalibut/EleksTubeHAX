#include <stdint.h>
#include "Buttons.h"
#include "Backlights.h"
#include "TFTs.h"
#include "Clock.h"
#include "Menu.h"

// put two lines in here defining `const char *ssid` and `const char *password`.
// The file wifi_creds.h is in .gitignore so we don't risk accidentally sharing our creds with the world.
#include "wifi_creds.h"

Backlights backlights;
Buttons buttons;
TFTs tfts;
Clock uclock;
Menu menu;

// Helper function, defined below.
void updateClockDisplay(TFTs::show_t show=TFTs::yes);
void setupMenu();

void setup() {
  Serial.begin(115200);
  delay(1000);  // Waiting for serial monitor to catch up.
  Serial.println("");
  Serial.println("In setup().");  

  backlights.begin();
  buttons.begin();
  menu.begin();

  // Setup TFTs
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
  tfts.print("\nSet 12h clock ");
  tfts.println(twelvehour);
  uclock.setTwelveHour(twelvehour);
  tfts.print("Set UTC offset ");
  tfts.println(UTCoffset);
  uclock.setTimeZoneOffset(UTCoffset);

  // Leave boot up messages on screen for a few seconds.
  for (uint8_t ndx=0; ndx < 2; ndx++) {
    tfts.print(".");
    delay(1000);
  }
  tfts.println("\nDone with setup().");

  // Start up the clock displays.
  //tfts.fillScreen(TFT_BLACK);
  //updateClockDisplay(TFTs::force);
}

void loop() {
  uint32_t millis_at_top = millis();
  // Do all the maintenance work
  buttons.loop();
  menu.loop(buttons);  // Must be called after buttons.loop()
  backlights.loop();
  uclock.loop();

  // Power button
  if (buttons.power.isDownEdge()) {
    tfts.toggleAllDisplays();
  }

  // Update the clock.
  updateClockDisplay();


  // Menu
  if (menu.stateChanged()) {
    Menu::states menu_state = menu.getState();
    int8_t menu_change = menu.getChange();

    Serial.print(menu_state);
    Serial.print(" ");
    Serial.println(menu_change);
    
    if (menu_state == Menu::idle) {
      // We just changed into idle, so redraw the hours tens digit
      tfts.showDigit(HOURS_TENS);
    }
    else {
      // backlight_pattern, pattern_color, twelve_hour, utc_offset_hour, utc_offset_15m
      if (menu_state == Menu::backlight_pattern) {
        backlights.setNextPattern(menu_change);

        setupMenu();
        tfts.println("Pattern:");
        tfts.println(backlights.getPatternStr());
      }
      else if (menu_state == Menu::pattern_color) {
        // TODO Adjust the color through the color wheel.  Reuse this code for the Rainbow pattern?

        setupMenu();
        tfts.println("Color:");
        tfts.println(backlights.getPatternColor(), HEX); 
      }
      else if (menu_state == Menu::twelve_hour) {
        if (menu_change != 0) {
          uclock.toggleTwelveHour();
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::force);
          tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::force);
        }
        
        setupMenu();
        tfts.println("12 Hour?");
        tfts.println(uclock.getTwelveHour() ? "12 hour" : "24 hour"); 
      }
      else if (menu_state == Menu::utc_offset_hour) {
        if (menu_change != 0) {
          uclock.adjustTimeZoneOffset(menu_change * 3600);
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::force);
          tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::force);
        }

        setupMenu();
        tfts.println("UTC Offset");
        tfts.println(" +/- Hour");
        time_t offset = uclock.getTimeZoneOffset();
        int8_t offset_hour = offset/3600;
        int8_t offset_min = (offset%3600)/60;
        tfts.printf("%d:%02d\n", offset_hour, offset_min);
      }
      else if (menu_state == Menu::utc_offset_15m) {
        if (menu_change != 0) {
          uclock.adjustTimeZoneOffset(menu_change * 900);
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::force);
          tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::force);
          tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), TFTs::force);
          tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), TFTs::force);
        }

        setupMenu();
        tfts.println("UTC Offset");
        tfts.println(" +/- 15m");
        time_t offset = uclock.getTimeZoneOffset();
        int8_t offset_hour = offset/3600;
        int8_t offset_min = (offset%3600)/60;
        tfts.printf("%d:%02d\n", offset_hour, offset_min);
      }
      else if (menu_state == Menu::exit_menu) {
        setupMenu();
        tfts.println("Exit Menu?");
      }
    }
  }

  // Sleep for up to 20ms, less if we've spent time doing stuff above.
  uint32_t time_in_loop = millis() - millis_at_top;
  /*
  if (time_in_loop == 0) {
    Serial.print(".");
  }
  else {
    Serial.println(time_in_loop);
  }
  */
  if (time_in_loop < 20) {
    delay(20 - time_in_loop);
  }
}

void setupMenu() {
  tfts.chip_select.setHoursTens();
  tfts.setTextColor(TFT_WHITE, TFT_BLACK);
  tfts.fillRect(0, 120, 135, 120, TFT_BLACK);
  tfts.setCursor(0, 120, 4);
}

void updateClockDisplay(TFTs::show_t show) {
  tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), show);
  tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), show);
  tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), show);
  tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), show);
  tfts.setDigit(SECONDS_TENS, uclock.getSecondsTens(), show);
  tfts.setDigit(SECONDS_ONES, uclock.getSecondsOnes(), show);
}
