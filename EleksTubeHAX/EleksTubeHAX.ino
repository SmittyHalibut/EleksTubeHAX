/*
 * Author: Aljaz Ogrin
 * Project: Alternative firmware for EleksTube IPS clock
 * Original location: https://github.com/aly-fly/EleksTubeHAX
 * Hardware: ESP32
 * Based on: https://github.com/SmittyHalibut/EleksTubeHAX
 */

#include <stdint.h>
#include "GLOBAL_DEFINES.h"
#include "Buttons.h"
#include "Backlights.h"
#include "TFTs.h"
#include "Clock.h"
#include "Menu.h"
#include "StoredConfig.h"
#include "WiFi_WPS.h"
#include "mqtt_client_ips.h"

Backlights backlights;
Buttons buttons;
TFTs tfts;
Clock uclock;
Menu menu;
StoredConfig stored_config;

uint8_t hour_old = 255;

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
  tfts.println("setup...");

  // Setup WiFi connection. Must be done before setting up Clock.
  // This is done outside Clock so the network can be used for other things.
//  WiFiBegin(&stored_config.config.wifi);
  tfts.println("WiFi start");
  WifiBegin();
  
  // wait for a bit before querying NTP
  for (uint8_t ndx=0; ndx < 10; ndx++) {
    tfts.print(">");
    delay(200);
  }
  tfts.println("");

  // Setup the clock.  It needs WiFi to be established already.
  tfts.println("Clock start");
  uclock.begin(&stored_config.config.uclock);

  // Setup MQTT
  tfts.println("MQTT start");
  MqttStart();
  
  tfts.println("Done with setup.");

  // Leave boot up messages on screen for a few seconds.
  for (uint8_t ndx=0; ndx < 10; ndx++) {
    tfts.print(">");
    delay(200);
  }

  tfts.current_graphic = uclock.getActiveGraphicIdx();

  // Start up the clock displays.
  tfts.fillScreen(TFT_BLACK);
  uclock.loop();
  updateClockDisplay(TFTs::force);
  Serial.println("Setup finished.");
}

void loop() {
  uint32_t millis_at_top = millis();
  // Do all the maintenance work
  WifiReconnect(); // if not connected attempt to reconnect

  MqttStatusPower = tfts.isEnabled();
  MqttStatusState = uclock.getActiveGraphicIdx() * 10; 
  MqttLoop();
  if (MqttCommandPowerReceived) {
    MqttCommandPowerReceived = false;
    if (MqttCommandPower) {
      tfts.enableAllDisplays();
      backlights.PowerOn();
    } else {
      tfts.disableAllDisplays();
      backlights.PowerOff();
    }
  }

  if (MqttCommandStateReceived) {
    MqttCommandStateReceived = false;
    uclock.setClockGraphicsIdx(MqttCommandState / 10);
    tfts.current_graphic = uclock.getActiveGraphicIdx();
    updateClockDisplay(TFTs::force);   // redraw everything
    Serial.print("Saving config...");
    stored_config.save();
    Serial.println(" Done.");
  }

  buttons.loop();

  // Power button: If in menu, exit menu. Else turn off displays and backlight.
  if (buttons.power.isDownEdge() && (menu.getState() == Menu::idle)) {
    tfts.toggleAllDisplays();
    backlights.togglePower();
  }
 
  menu.loop(buttons);  // Must be called after buttons.loop()
  backlights.loop();
  uclock.loop();

  // Update the clock.
  updateClockDisplay();
  
  // Menu
  if (menu.stateChanged() && tfts.isEnabled()) {
    Menu::states menu_state = menu.getState();
    int8_t menu_change = menu.getChange();

    if (menu_state == Menu::idle) {
      // We just changed into idle, so force redraw everything, and save the config.
      updateClockDisplay(TFTs::force);
      Serial.print("Saving config...");
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
      // select clock "font"
      else if (menu_state == Menu::selected_graphic) {
        if (menu_change != 0) {
          uclock.adjustClockGraphicsIdx(menu_change);
          tfts.current_graphic = uclock.getActiveGraphicIdx();
          updateClockDisplay(TFTs::force);   // redraw everything
        }

        setupMenu();
        tfts.println("Selected");
        tfts.println(" graphic:");
        tfts.printf("    %d\n", uclock.getActiveGraphicIdx());
      }
      // connect to WiFi using wps pushbutton mode
      else if (menu_state == Menu::start_wps) {
        if (menu_change != 0) { // button was pressed
          if (menu_change < 0) { // left button
            Serial.println("WiFi WPS start request");
            tfts.clear();
            tfts.fillScreen(TFT_BLACK);
            tfts.setTextColor(TFT_WHITE, TFT_BLACK);
            tfts.setCursor(0, 0, 4);
            WiFiStartWps();
          }
        }
        
        setupMenu();
        tfts.println("Connect to WiFi?");
        tfts.println("Left=WPS");
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

  // dim the clock at night
  uint8_t current_hour = uclock.getHour24();
  if (current_hour != hour_old) {
  Serial.print("current hour = ");
  Serial.println(current_hour);
    if ((current_hour >= 22) || (current_hour < 6)) {
      Serial.println("Setting night mode (dimmed)");
      tfts.dimming = TFT_DIMMED_INTENSITY;
      backlights.dimming = true;
      if (menu.getState() == Menu::idle) { // otherwise erases the menu
        show = TFTs::force; // update all
      }
    } else {
      tfts.dimming = 255; // 0..255
      backlights.dimming = false;
      if (menu.getState() == Menu::idle) { // otherwise erases the menu
        show = TFTs::force; // update all
      }
      Serial.println("Setting daytime mode (normal brightness)");
    }
    hour_old = current_hour;
  } 
    
  tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), show);
  tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), show);
  tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), show);
  tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), show);
  tfts.setDigit(SECONDS_TENS, uclock.getSecondsTens(), show);
  tfts.setDigit(SECONDS_ONES, uclock.getSecondsOnes(), show);
}
