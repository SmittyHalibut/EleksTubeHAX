#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>
#include <TimeLib.h>

// For NTP
#include <WiFi.h>
#include <NTPClient.h>

// For the DS3231 RTC
#include <DS1307RTC.h>

class Clock {
public:
  // twelve_hour and time_zone_offset are overwritten immediately in begin() from values in wifi_creds.h
  Clock() : loop_time(0), local_time(0), time_valid(false), twelve_hour(true), time_zone_offset(0) {}
  
  // The global WiFi from WiFi.h must already be .begin()'d before calling Clock::begin()
  void begin(); 
  void loop();

  // Calls NTPClient::getEpochTime() or RTC::get() as appropriate
  // This has to be static to pass to TimeLib::setSyncProvider.
  static time_t syncProvider();

  // Set preferred hour format. true = 12hr, false = 24hr
  void setTwelveHour(bool twelve_hour_) { twelve_hour = twelve_hour_; }
  bool getTwelveHour()                  { return twelve_hour; }
  void toggleTwelveHour()               { twelve_hour = !twelve_hour; }

  // Internal time is kept in UTC. This affects the displayed time.
  void setTimeZoneOffset(time_t offset) { time_zone_offset = offset; }
  time_t getTimeZoneOffset()            { return time_zone_offset; }
  void adjustTimeZoneOffset(time_t adj) { time_zone_offset += adj; }

  // Proxy C functions from TimeLib.h
  // I really wish it were a class we could just subclass, but here we are.
  uint16_t getYear()       { return year(local_time); }
  uint8_t getMonth()       { return month(local_time); }
  uint8_t getDay()         { return day(local_time); }
  uint8_t getHour()        { return twelve_hour ? hourFormat12(local_time) : hour(local_time); }
  uint8_t getHour12()      { return hourFormat12(local_time); }
  uint8_t getHour24()      { return hour(local_time); }
  uint8_t getMinute()      { return minute(local_time); }
  uint8_t getSecond()      { return second(local_time); }
  bool isAm()              { return isAM(local_time); }
  bool isPm()              { return isPM(local_time); }

  // Helper functions for making a clock.
  uint8_t getHoursTens()    { return getHour()/10; }
  uint8_t getHoursOnes()    { return getHour()%10; }
  uint8_t getHours12Tens()  { return getHour12()/10; }
  uint8_t getHours12Ones()  { return getHour12()%10; }
  uint8_t getHours24Tens()  { return getHour24()/10; }
  uint8_t getHours24Ones()  { return getHour24()%10; }
  uint8_t getMinutesTens()  { return getMinute()/10; }
  uint8_t getMinutesOnes()  { return getMinute()%10; }
  uint8_t getSecondsTens()  { return getSecond()/10; }
  uint8_t getSecondsOnes()  { return getSecond()%10; }
  
private:
  time_t loop_time, local_time;
  bool time_valid;
  bool twelve_hour;
  time_t time_zone_offset;  // In seconds.  This is just added to the UTC time when returning values.

  // Static variables needed for syncProvider()
  static WiFiUDP ntpUDP;
  static NTPClient ntpTimeClient;
  static uint32_t millis_last_ntp;
  const static uint32_t refresh_ntp_every_ms = 3600000; // Get new NTP every hour, use RTC in between.
};



#endif // CLOCK_H
