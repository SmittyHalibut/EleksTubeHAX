#ifndef STORED_CONFIG_H
#define STORED_CONFIG_H

#include <Preferences.h>

class StoredConfig {
public:
  StoredConfig() : prefs(), config_size(sizeof(config)), loaded(false) {}
  void begin()    { prefs.begin(prefs_namespace, false); Serial.print("Config size: "); Serial.println(config_size); }
  void load()     { prefs.getBytes(prefs_namespace, &config, config_size); loaded = true; }
  void save()     { prefs.putBytes(prefs_namespace, &config, config_size); }
  bool isLoaded() { return loaded; }
  bool isValid()  { return config.backlights.pattern < 255 && config.backlights.intensity < 8; } // Simple test for valid data
  // An un-written to Flash will return all 0xFF.  So if we haven't been written to, we should start up with defaults.

  const static uint8_t str_buffer_size = 32;

  struct Config {
    struct Backlights {
      uint8_t  pattern;
      uint16_t color_phase;
      uint8_t  intensity;
      uint8_t  pulse_bpm;
      uint8_t  breath_per_min;
      uint8_t  is_valid;       // Write StoredConfig::valid here when valid data is loaded.
    } backlights;
  
    struct Clock {
      bool     twelve_hour;
      time_t   time_zone_offset;
      uint8_t  is_valid;       // Write StoredConfig::valid here when valid data is loaded.
    } uclock;
  
    struct Wifi {
      char     ssid[str_buffer_size];
      char     password[str_buffer_size];
      uint8_t  is_valid;       // Write StoredConfig::valid here when valid data is loaded.
    } wifi;
  } config;

  const static uint8_t valid = 0x55;  // neither 0x00 nor 0xFF, signaling loaded config isn't just default data.
  
private:
  Preferences prefs;
  const static char prefs_namespace[];
  uint16_t config_size; 
  bool loaded;
};


#endif // STORED_CONFIG_H
