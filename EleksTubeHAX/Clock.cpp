#include "Clock.h"

void Clock::begin() {
  ntpTimeClient.begin();
  ntpTimeClient.update();
  Serial.println(ntpTimeClient.getFormattedTime());
  setSyncProvider(&Clock::syncProvider);
}

void Clock::loop() {
  if (timeStatus() == timeNotSet) {
    time_valid = false;
  }
  else {
    loop_time = now();
    local_time = loop_time + time_zone_offset;
    time_valid = true;
  }
}


// Static methods used for sync provider to TimeLib library.
time_t Clock::syncProvider() {
  Serial.println("syncProvider()");
  time_t ntp_now, rtc_now;
  rtc_now = RTC.get();

  if (millis() - millis_last_ntp > refresh_ntp_every_ms || millis_last_ntp == 0) {
    // It's time to get a new NTP sync
    Serial.print("Getting NTP.");
    ntpTimeClient.forceUpdate();
    Serial.print(".");
    ntp_now = ntpTimeClient.getEpochTime();
    Serial.println(" Done.");

    // Sync the RTC to NTP if needed.
    if (ntp_now - rtc_now != 0) {
      RTC.set(ntp_now);
      Serial.println("NTP, RTC, Diff: ");
      Serial.println(ntp_now);
      Serial.println(rtc_now);
      Serial.println(ntp_now-rtc_now);
    }
    millis_last_ntp = millis();
    return ntp_now;    
  }
  Serial.println("Using RTC time.");
  return rtc_now;
}

uint32_t Clock::millis_last_ntp = 0;
WiFiUDP Clock::ntpUDP;
NTPClient Clock::ntpTimeClient(ntpUDP);
