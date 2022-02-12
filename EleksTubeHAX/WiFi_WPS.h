#ifndef WIFI_WPS_H
#define WIFI_WPS_H

#include "GLOBAL_DEFINES.h"

#include <Arduino.h>
#include <WiFi.h> // ESP32
#include "StoredConfig.h"
#include "TFTs.h"
#include "esp_wps.h"

  enum WifiState_t {disconnected, connected, wps_active, wps_success, wps_failed, num_states};
  void WifiBegin();
  bool WiFiStartWps();
  void WifiReconnect();

  extern WifiState_t WifiState;

#endif // WIFI_WPS_H
