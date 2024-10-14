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
#include "Mqtt_client_ips.h"
#include "TempSensor_inc.h"
#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// #include "Gestures.h"
//TODO put into class
#include <Wire.h>
#include <SparkFun_APDS9960.h>
#endif //NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// Constants

// Global Variables
#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//TODO put into class
SparkFun_APDS9960 apds      = SparkFun_APDS9960();
//interupt signal for gesture sensor
int volatile      isr_flag  = 0;
#endif //NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

Backlights    backlights;
Buttons       buttons;
TFTs          tfts;
Clock         uclock;
Menu          menu;
StoredConfig  stored_config;

bool          FullHour        = false;
uint8_t       hour_old        = 255;
bool          DstNeedsUpdate  = false;
uint8_t       yesterday       = 0;

uint32_t lastMqttCommandExecuted = (uint32_t) -1;

// Helper function, defined below.
void updateClockDisplay(TFTs::show_t show=TFTs::yes);
void setupMenu(void);
void EveryFullHour(bool loopUpdate=false);
void UpdateDstEveryNight(void);
#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void GestureStart();
void HandleGestureInterupt(void); //only for NovelLife SE
void GestureInterruptRoutine(void); //only for NovelLife SE
void HandleGesture(void); //only for NovelLife SE
#endif //NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

void setup() {
  Serial.begin(115200);
  delay(1000);  // Waiting for serial monitor to catch up.
  Serial.println("");
  Serial.println(FIRMWARE_VERSION);
  Serial.println("In setup().");  

  stored_config.begin();
  stored_config.load();

  backlights.begin(&stored_config.config.backlights);
  buttons.begin();
  menu.begin();

  // Setup the displays (TFTs) initaly and show bootup message(s)
  tfts.begin();
  tfts.fillScreen(TFT_BLACK);
  tfts.setTextColor(TFT_WHITE, TFT_BLACK);
  tfts.setCursor(0, 0, 2);  // Font 2. 16 pixel high
  tfts.println("setup...");

#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  //Init the Gesture sensor
  tfts.println("Gesture sensor start");Serial.println("Gesture sensor start");
  GestureStart(); //TODO put into class
#endif

  // Setup WiFi connection. Must be done before setting up Clock.
  // This is done outside Clock so the network can be used for other things.
//  WiFiBegin(&stored_config.config.wifi);
  tfts.println("WiFi start");
  WifiBegin();
  
  // wait for a bit before querying NTP
  for (uint8_t ndx=0; ndx < 5; ndx++) {
    tfts.print(">");
    delay(100);
  }
  tfts.println("");

  // Setup the clock.  It needs WiFi to be established already.
  tfts.println("Clock start");
  uclock.begin(&stored_config.config.uclock);

  // Setup MQTT
  tfts.println("MQTT start");
  MqttStart();

#ifdef GEOLOCATION_ENABLED
  tfts.println("Geoloc query");
  if (GetGeoLocationTimeZoneOffset()) {
    tfts.print("TZ: ");
    tfts.println(GeoLocTZoffset);
    uclock.setTimeZoneOffset(GeoLocTZoffset * 3600);
    Serial.print("Saving config...");
    stored_config.save();
    Serial.println(" Done.");
  } else {
    Serial.println("Geolocation failed.");    
    tfts.println("Geo FAILED");
  }
#endif

  if (uclock.getActiveGraphicIdx() > tfts.NumberOfClockFaces) {
    uclock.setActiveGraphicIdx(tfts.NumberOfClockFaces);
    Serial.println("Last selected index of clock face is larger than currently available number of image sets.");
  }
  if (uclock.getActiveGraphicIdx() < 1) {
    uclock.setActiveGraphicIdx(1);
    Serial.println("Last selected index of clock face is less than 1.");
  }
  tfts.current_graphic = uclock.getActiveGraphicIdx();

  tfts.println("Done with setup.");

  // Leave boot up messages on screen for a few seconds.
  for (uint8_t ndx=0; ndx < 10; ndx++) {
    tfts.print(">");
    delay(200);
  }

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
  
  MqttLoopFrequently();

  bool MqttCommandReceived = 
    MqttCommandPowerReceived || 
    MqttCommandMainPowerReceived ||
    MqttCommandBackPowerReceived ||
    MqttCommandStateReceived ||
    MqttCommandBrightnessReceived ||
    MqttCommandMainBrightnessReceived ||
    MqttCommandBackBrightnessReceived ||
    MqttCommandPatternReceived ||
    MqttCommandBackPatternReceived ||
    MqttCommandBackColorPhaseReceived || 
    MqttCommandGraphicReceived ||
    MqttCommandMainGraphicReceived ||
    MqttCommandUseTwelveHoursReceived ||
    MqttCommandBlankZeroHoursReceived ||
    MqttCommandPulseBpmReceived ||
    MqttCommandBreathBpmReceived ||
    MqttCommandRainbowSecReceived;
  
  if (MqttCommandPowerReceived) {
    MqttCommandPowerReceived = false;
    if (MqttCommandPower) {
#ifndef HARDWARE_SI_HAI_CLOCK
      if (!tfts.isEnabled()) {
        tfts.reinit();  // reinit (original EleksTube HW: after a few hours in OFF state the displays do not wake up properly)
        updateClockDisplay(TFTs::force);
      }
#endif
      tfts.enableAllDisplays();
      backlights.PowerOn();
    } else {
      tfts.disableAllDisplays();
      backlights.PowerOff();
    }
  }

  if (MqttCommandMainPowerReceived) {
    MqttCommandMainPowerReceived = false;
    if (MqttCommandMainPower) {
 #ifndef HARDWARE_SI_HAI_CLOCK
      if (!tfts.isEnabled()) {
        tfts.reinit();  // reinit (original EleksTube HW: after a few hours in OFF state the displays do not wake up properly)
        updateClockDisplay(TFTs::force);
      }
 #endif
      tfts.enableAllDisplays();
    } else {
      tfts.disableAllDisplays();
    }
  }

  if (MqttCommandBackPowerReceived) {
    MqttCommandBackPowerReceived = false;
    if (MqttCommandBackPower) {
      backlights.PowerOn();
    } else {
      backlights.PowerOff();
    }
  }

  if (MqttCommandStateReceived) {
    MqttCommandStateReceived = false;
    randomSeed(millis());
    uint8_t idx;
    if (MqttCommandState >= 90)
      { idx = random(1, tfts.NumberOfClockFaces+1); } else
      { idx = (MqttCommandState / 5) -1; }  // 10..40 -> graphic 1..6
    Serial.print("Graphic change request from MQTT; command: ");
    Serial.print(MqttCommandState);
    Serial.print(", index: ");
    Serial.println(idx);
    uclock.setClockGraphicsIdx(idx);  
    tfts.current_graphic = uclock.getActiveGraphicIdx();
    updateClockDisplay(TFTs::force);   // redraw everything
  }

  if(MqttCommandMainBrightnessReceived) {
    MqttCommandMainBrightnessReceived = false;
    tfts.dimming = MqttCommandMainBrightness;
    tfts.ProcessUpdatedDimming();
    updateClockDisplay(TFTs::force);
  }

  if(MqttCommandBackBrightnessReceived) {
    MqttCommandBackBrightnessReceived = false;
    backlights.setIntensity(uint8_t(MqttCommandBackBrightness));
  }

  if(MqttCommandPatternReceived) {
    MqttCommandPatternReceived = false;

    for(int8_t i = 0; i < Backlights::num_patterns; i++){
        Serial.print("New pattern ");
        Serial.print(MqttCommandPattern);
        Serial.print(", check pattern ");
        Serial.println(Backlights::patterns_str[i]);
      if(strcmp(MqttCommandPattern, (Backlights::patterns_str[i]).c_str()) == 0) {
        backlights.setPattern(Backlights::patterns(i));
        break;
      }
    } 
  }

  if(MqttCommandBackPatternReceived) {
    MqttCommandBackPatternReceived = false;
    for(int8_t i = 0; i < Backlights::num_patterns; i++){
        Serial.print("new pattern ");
        Serial.print(MqttCommandBackPattern);
        Serial.print(", check pattern ");
        Serial.println(Backlights::patterns_str[i]);
      if(strcmp(MqttCommandBackPattern, (Backlights::patterns_str[i]).c_str()) == 0) {
        backlights.setPattern(Backlights::patterns(i));
        break;
      }
    } 
  }

  if(MqttCommandBackColorPhaseReceived) {
    MqttCommandBackColorPhaseReceived = false;
    
    backlights.setColorPhase(MqttCommandBackColorPhase);
  } 

  if(MqttCommandGraphicReceived) {
    MqttCommandGraphicReceived = false;

    uclock.setClockGraphicsIdx(MqttCommandGraphic);
    tfts.current_graphic = uclock.getActiveGraphicIdx();
    updateClockDisplay(TFTs::force);   // redraw everything
  }

  if(MqttCommandMainGraphicReceived) {
    MqttCommandMainGraphicReceived = false;
    uclock.setClockGraphicsIdx(MqttCommandMainGraphic);
    tfts.current_graphic = uclock.getActiveGraphicIdx();
    updateClockDisplay(TFTs::force);   // redraw everything
  }

  if(MqttCommandUseTwelveHoursReceived) {
    MqttCommandUseTwelveHoursReceived = false;
    uclock.setTwelveHour(MqttCommandUseTwelveHours);
  }

  if(MqttCommandBlankZeroHoursReceived) {
    MqttCommandBlankZeroHoursReceived = false;
    uclock.setBlankHoursZero(MqttCommandBlankZeroHours);
  }

  if(MqttCommandPulseBpmReceived) {
    MqttCommandPulseBpmReceived = false;
    backlights.setPulseRate(MqttCommandPulseBpm);
  }

  if(MqttCommandBreathBpmReceived) {
    MqttCommandBreathBpmReceived = false;
    backlights.setBreathRate(MqttCommandBreathBpm);
  }

  if(MqttCommandRainbowSecReceived) {
    MqttCommandRainbowSecReceived = false;
    backlights.setRainbowDuration(MqttCommandRainbowSec);
  }

  MqttStatusPower = tfts.isEnabled();
  MqttStatusMainPower = tfts.isEnabled();
  MqttStatusBackPower = backlights.getPower();
  MqttStatusState = (uclock.getActiveGraphicIdx()+1) * 5;   // 10 
  MqttStatusBrightness = backlights.getIntensity();
  MqttStatusMainBrightness = tfts.dimming;
  MqttStatusBackBrightness = backlights.getIntensity();
  strcpy(MqttStatusPattern, backlights.getPatternStr().c_str());
  strcpy(MqttStatusBackPattern, backlights.getPatternStr().c_str());
  backlights.getPatternStr().toCharArray(MqttStatusBackPattern, backlights.getPatternStr().length() + 1);
  MqttStatusBackColorPhase = backlights.getColorPhase();
  MqttStatusGraphic = uclock.getActiveGraphicIdx();
  MqttStatusMainGraphic = uclock.getActiveGraphicIdx();
  MqttStatusUseTwelveHours = uclock.getTwelveHour();
  MqttStatusBlankZeroHours = uclock.getBlankHoursZero();
  MqttStatusPulseBpm = backlights.getPulseRate();
  MqttStatusBreathBpm = backlights.getBreathRate();
  MqttStatusRainbowSec = backlights.getRainbowDuration();

  if(MqttCommandReceived) {
    lastMqttCommandExecuted = millis();

    MqttReportBackEverything(true);
  }

  if(lastMqttCommandExecuted != -1) {
    if (((millis() - lastMqttCommandExecuted) > (MQTT_SAVE_PREFERENCES_AFTER_SEC * 1000)) && menu.getState() == Menu::idle) {
      lastMqttCommandExecuted = -1;

      Serial.print("Saving config...");
      stored_config.save();
      Serial.println(" Done.");
    }
  }

  buttons.loop();

#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  HandleGestureInterupt();
#endif // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

  // Power button: If in menu, exit menu. Else turn off displays and backlight.
#ifndef ONE_BUTTON_ONLY_MENU
  if (buttons.power.isUpEdge() && (menu.getState() == Menu::idle)) {
    #ifdef DEBUG_OUTPUT
      Serial.println("Power button pressed.");
    #endif
    tfts.chip_select.setAll();
    tfts.fillScreen(TFT_BLACK);
    tfts.toggleAllDisplays();
    if (tfts.isEnabled()) {
    #ifndef HARDWARE_SI_HAI_CLOCK
      tfts.reinit();  // reinit (original EleksTube HW: after a few hours in OFF state the displays do not wake up properly)
    #endif
      tfts.chip_select.setAll();
      tfts.fillScreen(TFT_BLACK);
      updateClockDisplay(TFTs::force);
    }
    backlights.togglePower();
  }
#endif
 
  menu.loop(buttons);  // Must be called after buttons.loop()
  backlights.loop();
  uclock.loop();

  EveryFullHour(true); // night or daytime

  // Update the clock.
  updateClockDisplay();
  
  UpdateDstEveryNight();

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
        tfts.println("Hour format");
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

          EveryFullHour();

          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::yes);
          tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::yes);
        }

        setupMenu();
        tfts.println("UTC Offset");
        tfts.println(" +/- Hour");
        time_t offset = uclock.getTimeZoneOffset();
        int8_t offset_hour = offset/3600;
        int8_t offset_min = (offset%3600)/60;
        if(offset_min < 0) {
          offset_min = -offset_min;
        }
        tfts.printf("%d:%02d\n", offset_hour, offset_min);
      }
      // UTC Offset, 15 minutes
      else if (menu_state == Menu::utc_offset_15m) {
        if (menu_change != 0) {
          uclock.adjustTimeZoneOffset(menu_change * 900);

          EveryFullHour();

          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::yes);
          tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::yes);
          tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), TFTs::yes);
          tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), TFTs::yes);
        }

        setupMenu();
        tfts.println("UTC Offset");
        tfts.println(" +/- 15m");
        time_t offset = uclock.getTimeZoneOffset();
        int8_t offset_hour = offset/3600;
        int8_t offset_min = (offset%3600)/60;
        if(offset_min < 0) {
          offset_min = -offset_min;
        }
        tfts.printf("%d:%02d\n", offset_hour, offset_min);
      }
      // select clock "font"
      else if (menu_state == Menu::selected_graphic) {
        if (menu_change != 0) {
          uclock.adjustClockGraphicsIdx(menu_change);

          if(tfts.current_graphic != uclock.getActiveGraphicIdx()) {
            tfts.current_graphic = uclock.getActiveGraphicIdx();
            updateClockDisplay(TFTs::force);   // redraw everything
          }
        }

        setupMenu();
        tfts.println("Selected");
        tfts.println(" graphic:");
        tfts.printf("    %d\n", uclock.getActiveGraphicIdx());
      }
     

#ifdef WIFI_USE_WPS   ////  WPS code
      // connect to WiFi using wps pushbutton mode
      else if (menu_state == Menu::start_wps) {
        if (menu_change != 0) { // button was pressed
          if (menu_change < 0) { // left button
            Serial.println("WiFi WPS start request");
            tfts.clear();
            tfts.fillScreen(TFT_BLACK);
            tfts.setTextColor(TFT_WHITE, TFT_BLACK);
            tfts.setCursor(0, 0, 4);  // Font 4. 26 pixel high
            WiFiStartWps();
          }
        }
        
        setupMenu();
        tfts.println("Connect to WiFi?");
        tfts.println("Left=WPS");
      }
#endif   
    }
  }

  uint32_t time_in_loop = millis() - millis_at_top;
  if (time_in_loop < 20) {
    // we have free time, spend it for loading next image into buffer
    tfts.LoadNextImage();

    // we still have extra time
    time_in_loop = millis() - millis_at_top;
    if (time_in_loop < 20) {
      MqttLoopInFreeTime();
      PeriodicReadTemperature();
      if (bTemperatureUpdated) {
        tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::force);  // show latest clock digit and temperature readout together
        bTemperatureUpdated = false;
      }
      
      // run once a day (= 744 times per month which is below the limit of 5k for free account)
      if (DstNeedsUpdate) { // Daylight savings time changes at 3 in the morning
        if (GetGeoLocationTimeZoneOffset()) {
          uclock.setTimeZoneOffset(GeoLocTZoffset * 3600);
          DstNeedsUpdate = false;  // done for this night; retry if not sucessfull
        }
      }  
      // Sleep for up to 20ms, less if we've spent time doing stuff above.
      time_in_loop = millis() - millis_at_top;
      if (time_in_loop < 20) {
        delay(20 - time_in_loop);
      }
    }
  }
#ifdef DEBUG_OUTPUT
  if (time_in_loop <= 1) Serial.print(".");
  else {
    Serial.print("time spent in loop (ms): ");
    Serial.println(time_in_loop);
  }
#endif
}
#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void GestureStart()
{
    // for gesture sensor APDS9660 - Set interrupt pin on ESP32 as input
  pinMode(GESTURE_SENSOR_INPUT_PIN, INPUT);

  // Initialize interrupt service routine for interupt from APDS-9960 sensor
  attachInterrupt(digitalPinToInterrupt(GESTURE_SENSOR_INPUT_PIN), GestureInterruptRoutine, FALLING);

  // Initialize gesture sensor APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));

    //Set Gain to 1x, bacause the cheap chinese fake APDS sensor can't handle more (also remember to extend ID check in Sparkfun libary to 0x3B!)
    apds.setGestureGain(GGAIN_1X);
          
    // Start running the APDS-9960 gesture sensor engine
    if ( apds.enableGestureSensor(true) ) {
      Serial.println(F("Gesture sensor is now running"));
    } else {
      Serial.println(F("Something went wrong during gesture sensor enablimg in the APDS-9960 library!"));
    }
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
}

//Handle Interrupt from gesture sensor and simulate a short button press of the corresponding button, if a gesture is detected 
void HandleGestureInterupt()
{
  if( isr_flag == 1 ) {
    detachInterrupt(digitalPinToInterrupt(GESTURE_SENSOR_INPUT_PIN));
    HandleGesture();
    isr_flag = 0;
    attachInterrupt(digitalPinToInterrupt(GESTURE_SENSOR_INPUT_PIN), GestureInterruptRoutine, FALLING);
  }
  return;
}

//mark, that the Interrupt of the gesture sensor was signaled
void GestureInterruptRoutine() {
  isr_flag = 1;
  return;
}

//check which gesture was detected
void HandleGesture() { 
    //Serial.println("->main::HandleGesture()");
    if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_UP:
        buttons.left.setUpEdgeState();
        Serial.println("Gesture detected! LEFT");
        break;
      case DIR_DOWN:
        buttons.right.setUpEdgeState();
        Serial.println("Gesture detected! RIGHT");
        break;
      case DIR_LEFT:
        buttons.power.setUpEdgeState();
        Serial.println("Gesture detected! DOWN");
        break;
      case DIR_RIGHT:
        buttons.mode.setUpEdgeState();
        Serial.println("Gesture detected! UP");
        break;
      case DIR_NEAR:
        buttons.mode.setUpEdgeState();
        Serial.println("Gesture detected! NEAR");
        break;
      case DIR_FAR:
        buttons.power.setUpEdgeState();
        Serial.println("Gesture detected! FAR");
        break;
      default:
        Serial.println("Movement detected but NO gesture detected!");
    }
  }
  return;
}
#endif // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

void setupMenu() {
  tfts.chip_select.setHoursTens();
  tfts.setTextColor(TFT_WHITE, TFT_BLACK);
  tfts.fillRect(0, 120, 135, 120, TFT_BLACK);
  tfts.setCursor(0, 124, 4);  // Font 4. 26 pixel high
}

bool isNightTime(uint8_t current_hour) {
    if (DAY_TIME < NIGHT_TIME) {
      // "Night" spans across midnight
      return (current_hour < DAY_TIME) || (current_hour >= NIGHT_TIME);
    }
    else {
      // "Night" starts after midnight, entirely contained within the day
      return (current_hour >= NIGHT_TIME) && (current_hour < DAY_TIME);
    }
}

void EveryFullHour(bool loopUpdate) {
  // dim the clock in the night
#ifdef NIGHTTIME_DIMMING
  uint8_t current_hour = uclock.getHour24();
  FullHour = current_hour != hour_old;
  if (FullHour) {
    Serial.print("current hour = ");
    Serial.println(current_hour);
    if (isNightTime(current_hour)) {
      Serial.println("Setting night mode (dimmed)");
      tfts.dimming = TFT_DIMMED_INTENSITY;
      tfts.ProcessUpdatedDimming();
      backlights.setDimming(true);
      if (menu.getState() == Menu::idle || !loopUpdate) { // otherwise erases the menu
        updateClockDisplay(TFTs::force); // update all
      }
    } else {
      Serial.println("Setting daytime mode (max brightness)");
      tfts.dimming = 255; // 0..255
      tfts.ProcessUpdatedDimming();
      backlights.setDimming(false);
      if (menu.getState() == Menu::idle || !loopUpdate) { // otherwise erases the menu
        updateClockDisplay(TFTs::force); // update all
      }
    }
    hour_old = current_hour;
  }
#endif   
}

void UpdateDstEveryNight() {
  uint8_t currentDay = uclock.getDay();
  // This `DstNeedsUpdate` is True between 3:00:05 and 3:00:59. Has almost one minute of time slot to fetch updates, incl. eventual retries.
  DstNeedsUpdate = (currentDay != yesterday) && (uclock.getHour24() == 3) && (uclock.getMinute() == 0) && (uclock.getSecond() > 5);
  if (DstNeedsUpdate) {
  Serial.print("DST needs update...");

  // Update day after geoloc was sucesfully updated. Otherwise this will immediatelly disable the failed update retry.
  yesterday = currentDay;
  }
}

void updateClockDisplay(TFTs::show_t show) {
  // refresh starting on seconds
  tfts.setDigit(SECONDS_ONES, uclock.getSecondsOnes(), show);
  tfts.setDigit(SECONDS_TENS, uclock.getSecondsTens(), show);
  tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), show);
  tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), show);
  tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), show);
  tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), show);
}
