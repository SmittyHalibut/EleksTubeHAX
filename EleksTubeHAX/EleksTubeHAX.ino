#include <stdint.h>
#include "Buttons.h"
#include "Backlights.h"
#include "TFTs.h"
#include "Clock.h"
#include "Menu.h"
#include "StoredConfig.h"

// put two lines in here defining `const char *ssid` and `const char *password`.
// The file wifi_creds.h is in .gitignore so we don't risk accidentally sharing our creds with the world.
#include "wifi_creds.h"

Backlights backlights;
Buttons buttons;
TFTs tfts;
Clock uclock;
Menu menu;
StoredConfig stored_config;

// Helper function, defined below.
void updateClockDisplay(TFTs::show_t show=TFTs::yes);
void setupMenu();

void setup() {
  Serial.begin(115200);
  delay(1000);  // Waiting for serial monitor to catch up.
  Serial.println("");
  Serial.println("In setup().");  

  stored_config.begin();
  stored_config.load();

  backlights.begin(&stored_config.config.backlights);
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
  // TODO Once we've added a way to SET the SSID and Password from the menu, use
  // stored_config.config.wifi to store and recall them.
  // For now, we're still pulling them from wifi_creds.h
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tfts.print(".");
  }
  tfts.println("\nDone.");

  // Setup the clock.  It needs WiFi to be established already.
  uclock.begin(&stored_config.config.uclock);
  
  // Leave boot up messages on screen for a few seconds.
  for (uint8_t ndx=0; ndx < 2; ndx++) {
    tfts.print(".");
    delay(1000);
  }
  tfts.println("\nDone with setup().");

  // Start up the clock displays.
  tfts.fillScreen(TFT_BLACK);
  uclock.loop();
  updateClockDisplay(TFTs::force);
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
    backlights.togglePower();
  }

  // Update the clock.
  updateClockDisplay();


  // Menu
  if (menu.stateChanged() && tfts.isEnabled()) {
    Menu::states menu_state = menu.getState();
    int8_t menu_change = menu.getChange();

    if (menu_state == Menu::idle) {
      // We just changed into idle, so force redraw everything, and save the config.
      updateClockDisplay(TFTs::force);
      Serial.print("Saving config.");
      stored_config.save();
      Serial.println(" Done.");
    }
    else {
      // Backlight Pattern
      if (menu_state == Menu::backlight_pattern) {
        if (menu_change != 0) {
          backlights.setNextPattern(menu_change);
        }

        setupMenu();
        tfts.println("Pattern:");
        tfts.println(backlights.getPatternStr());
      }
      // Backlight Color
      else if (menu_state == Menu::pattern_color) {
        if (menu_change != 0) {
          backlights.adjustColorPhase(menu_change*16);
        }
        setupMenu();
        tfts.println("Color:");
        tfts.printf("%06X\n", backlights.getColor()); 
      }
      // Backlight Intensity
      else if (menu_state == Menu::backlight_intensity) {
        if (menu_change != 0) {
          backlights.adjustIntensity(menu_change);
        }
        setupMenu();
        tfts.println("Intensity:");
        tfts.println(backlights.getIntensity());
      }
      // 12 Hour or 24 Hour mode?
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
      // Blank leading zeros on the hours?
      else if (menu_state == Menu::blank_hours_zero) {
        if (menu_change != 0) {
          uclock.toggleBlankHoursZero();
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::force);
        }
        
        setupMenu();
        tfts.println("Blank zero?");
        tfts.println(uclock.getBlankHoursZero() ? "yes" : "no");
      }
      // UTC Offset, hours
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
      // UTC Offset, 15 minutes
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
      // Exit the menu?
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
  tfts.setCursor(0, 124, 4);
}

void updateClockDisplay(TFTs::show_t show) {
  tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), show);
  tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), show);
  tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), show);
  tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), show);
  tfts.setDigit(SECONDS_TENS, uclock.getSecondsTens(), show);
  tfts.setDigit(SECONDS_ONES, uclock.getSecondsOnes(), show);
}
