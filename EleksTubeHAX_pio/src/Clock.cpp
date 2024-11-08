#include "Clock.h"
#include "WiFi_WPS.h"

#ifdef HARDWARE_SI_HAI_CLOCK // SI HAI IPS Clock XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// If it is a SI HAI Clock, use differnt RTC chip drivers
#include <ThreeWire.h>
#include <RtcDS1302.h>
ThreeWire myWire(DS1302_IO, DS1302_SCLK, DS1302_CE); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
void RtcBegin()
{
  Rtc.Begin();
  // check if chip is connected and alive
  /*  TCS default value is 0x00 instead of 0x5C, but RTC seems to be working. Let's skip this check.
      Serial.print("Checking DS1302 RTC... ");
      uint8_t TCS = Rtc.GetTrickleChargeSettings();  // 01011100  Initial power-on state
      Serial.print("TCS = ");
      Serial.println(TCS);
      if (TCS != 0x5C) {
        Serial.println("Error communicating with DS1302 !");
      }
  */
  if (!Rtc.IsDateTimeValid())
  {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing
    Serial.println("RTC lost confidence in the DateTime!");
  }
  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
}

uint32_t RtcGet()
{
  RtcDateTime temptime;
  temptime = Rtc.GetDateTime();
  uint32_t returnvalue = temptime.Unix32Time();
  Serial.println(returnvalue);
  return returnvalue;
}

void RtcSet(uint32_t tt)
{
  RtcDateTime temptime;
  temptime.InitWithUnix32Time(tt);
  Rtc.SetDateTime(temptime);
}
#else
// For the DS1307 RTC
#include <DS1307RTC.h>
void RtcBegin() {}
uint32_t RtcGet()
{
  return RTC.get();
}
void RtcSet(uint32_t tt)
{
  RTC.set(tt);
}
#endif

void Clock::begin(StoredConfig::Config::Clock *config_)
{
  config = config_;

  if (config->is_valid != StoredConfig::valid)
  {
    // Config is invalid, probably a new device never had its config written.
    // Load some reasonable defaults.
    Serial.println("Loaded Clock config is invalid, using default.  This is normal on first boot.");
    setTwelveHour(false);
    setBlankHoursZero(false);
    setTimeZoneOffset(1 * 3600); // CET
    setActiveGraphicIdx(1);
    config->is_valid = StoredConfig::valid;
  }

  RtcBegin();
  ntpTimeClient.begin();
  ntpTimeClient.update();
  Serial.print("NTP time = ");
  Serial.println(ntpTimeClient.getFormattedTime());
  setSyncProvider(&Clock::syncProvider);
}

void Clock::loop()
{
  if (timeStatus() == timeNotSet)
  {
    time_valid = false;
  }
  else
  {
    loop_time = now();
    local_time = loop_time + config->time_zone_offset;
    time_valid = true;
  }
}

// Static methods used for sync provider to TimeLib library.
time_t Clock::syncProvider()
{
  Serial.println("syncProvider()");
  time_t rtc_now;
  rtc_now = RtcGet();

  if (millis() - millis_last_ntp > refresh_ntp_every_ms || millis_last_ntp == 0)
  {
    if (WifiState == connected)
    {
      // It's time to get a new NTP sync
      Serial.print("Getting NTP.");
      //      ntpTimeClient.forceUpdate();  // maybe this breaks the NTP requests as this should not be done more than every minute.
      if (ntpTimeClient.update())
      {
        Serial.print(".");
        time_t ntp_now = ntpTimeClient.getEpochTime();
        Serial.println("NTP query done.");
        Serial.print("NTP time = ");
        Serial.println(ntpTimeClient.getFormattedTime());
        //      if (ntp_now > 1644601505) { //is it valid - reasonable number?
        // Sync the RTC to NTP if needed.
        Serial.println("NTP, RTC, Diff: ");
        Serial.println(ntp_now);
        Serial.println(rtc_now);
        Serial.println(ntp_now - rtc_now);
        if (ntp_now != rtc_now)
        {
          RtcSet(ntp_now);
          Serial.println("Updating RTC");
        }
        millis_last_ntp = millis();
        Serial.println("Using NTP time.");
        return ntp_now;
      }
      else
      { // NTP valid
        Serial.println("Invalid NTP response, using RTC time.");
        return rtc_now;
      }
    } // no wifi
    Serial.println("No WiFi, using RTC time.");
    return rtc_now;
  }
  Serial.println("Using RTC time.");
  return rtc_now;
}

uint8_t Clock::getHoursTens()
{
  uint8_t hour_tens = getHour() / 10;

  if (config->blank_hours_zero && hour_tens == 0)
  {
    return TFTs::blanked;
  }
  else
  {
    return hour_tens;
  }
}

uint32_t Clock::millis_last_ntp = 0;
WiFiUDP Clock::ntpUDP;
NTPClient Clock::ntpTimeClient(ntpUDP);
