#include <Arduino.h>
#include <WiFi.h> // ESP32
#include "StoredConfig.h"
#include "TFTs.h"
#include "esp_wps.h"
#include "WiFi_WPS.h"


#include "IPGeolocation_AO.h"

extern StoredConfig stored_config;

WifiState_t WifiState = disconnected;


uint32_t TimeOfWifiReconnectAttempt = 0;
double GeoLocTZoffset = 0;


#ifdef WIFI_USE_WPS   ////  WPS code

static esp_wps_config_t wps_config;
void wpsInitConfig(){
  wps_config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
  wps_config.wps_type = ESP_WPS_MODE;
  strcpy(wps_config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(wps_config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(wps_config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(wps_config.factory_info.device_name, DEVICE_NAME);
}
#endif

void WiFiEvent(WiFiEvent_t event, system_event_info_t info){
  switch(event){
    case SYSTEM_EVENT_STA_START:
      WifiState = disconnected;
      Serial.println("Station Mode Started");
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
//      WifiState = connected;  // IP not yet assigned
      Serial.println("Connected to AP: " + String(WiFi.SSID()));
      
/* https://github.com/espressif/arduino-esp32/blob/04963009eedfbc1e0ea2e1378ae69e7cebda6fd6/tools/sdk/include/esp32/esp_event_legacy.h#L80
typedef struct {
    uint8_t ssid[32];         // < SSID of connected AP 
    uint8_t ssid_len;         // < SSID length of connected AP 
    uint8_t bssid[6];         // < BSSID of connected AP
    uint8_t channel;          // < channel of connected AP
    wifi_auth_mode_t authmode;
} system_event_sta_connected_t;
*/

  /*
//      Serial.println(String(info.connected.ssid));
  //    Serial.println(String(info.connected.bssid));
    //  Serial.println(String(info.connected.channel));

   memset(dest, '\0', sizeof(dest));
   strcpy(src, "This is tutorialspoint.com");
   strcpy(dest, src);      
*/
      break;     
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.print("Got IP: ");
//      IPAddress ip = IPAddress(WiFi.localIP());
  //    Serial.println(ip);
      Serial.println(WiFi.localIP());
  
      /*
      if (ip[0] == 0) {
        WifiState = disconnected; // invalid IP
      } else {  */
        WifiState = connected;
//      }
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      WifiState = disconnected;
      Serial.print("WiFi lost connection. Reason: ");
      Serial.println(info.disconnected.reason);
      WifiReconnect();
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      WifiState = wps_success;
/*
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html#_CPPv431wifi_event_sta_wps_er_success_t
  structwifi_event_sta_wps_er_success_t
  Argument structure for WIFI_EVENT_STA_WPS_ER_SUCCESS event
  
  Public Members
  
  uint8_t ap_cred_cnt
  Number of AP credentials received
  
  uint8_t ssid[MAX_SSID_LEN]
  SSID of AP
  
  uint8_t passphrase[MAX_PASSPHRASE_LEN]
  Passphrase for the AP
  
  structwifi_event_sta_wps_er_success_t::[anonymous]ap_cred[MAX_WPS_AP_CRED]
  All AP credentials received from WPS handshake


    sprintf(stored_config.config.wifi.ssid, info.ssid); 
    sprintf(stored_config.config.wifi.password, info.passphrase); 



memcpy(someBuffer, evt->ap_cred[0].ssid, sizeof(*evt->ap_cred[0].ssid));
That is, assuming cred[0].ssid is defined as an array... you may be better off using strncpy() if it's not.
*/

                  
      Serial.println("WPS Successful, stopping WPS and connecting to: " + String(WiFi.SSID()));
      esp_wifi_wps_disable();
      delay(10);
// https://stackoverflow.com/questions/48024780/esp32-wps-reconnect-on-power-on      
      WiFi.begin();
      break;

#ifdef WIFI_USE_WPS   ////  WPS code      
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      WifiState = wps_failed;
      Serial.println("WPS Failed, retrying");
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&wps_config);
      esp_wifi_wps_start(0);
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
//      WifiState = wps_failed;
      Serial.println("WPS Timeout, retrying");
      tfts.setTextColor(TFT_RED, TFT_BLACK);      
      tfts.print("/");  // retry
      tfts.setTextColor(TFT_BLUE, TFT_BLACK);
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&wps_config);
      esp_wifi_wps_start(0);
      WifiState = wps_active;
      break;
 #endif     
    default:
      break;
  }
}



void WifiBegin()  {
  WifiState = disconnected;

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiEvent);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);  
  WiFi.setHostname(DEVICE_NAME);  

#ifdef WIFI_USE_WPS   ////  WPS code
  // no data is saved, start WPS imediatelly
  if (stored_config.config.wifi.WPS_connected != StoredConfig::valid) {
    // Config is invalid, probably a new device never had its config written.
    Serial.println("Loaded Wifi config is invalid. Not connecting to WiFi.");
    WiFiStartWps();  // infinite loop until connected
  } else {
    // data is saved, connect now
    // WiFi credentials are known, connect
    tfts.println("Joining wifi");
    tfts.println(stored_config.config.wifi.ssid);
    Serial.print("Joining wifi ");
    Serial.println(stored_config.config.wifi.ssid);
  
    // https://stackoverflow.com/questions/48024780/esp32-wps-reconnect-on-power-on
    WiFi.begin();  // use internally saved data

    unsigned long StartTime = millis();

    while ((WiFi.status() != WL_CONNECTED)) {
      delay(500);
      tfts.print(".");
      Serial.print(".");
      if ((millis() - StartTime) > (WIFI_CONNECT_TIMEOUT_SEC * 1000)) {
        Serial.println("\r\nWiFi connection timeout!");
        tfts.println("\nTIMEOUT!");
        WifiState = disconnected;
        return; // exit loop, exit procedure, continue clock startup
//        WiFiStartWps(); // infinite loop until connected
      }
    }
  }
#else   ////NO WPS -- Hard coded credentials

  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSWD); 
  unsigned long StartTime = millis();
  while ((WiFi.status() != WL_CONNECTED)) {
    delay(500);
    tfts.print(".");
    Serial.print(".");
    if ((millis() - StartTime) > (WIFI_CONNECT_TIMEOUT_SEC * 1000)) {
      Serial.println("\r\nWiFi connection timeout!");
      tfts.println("\nTIMEOUT!");
      WifiState = disconnected;
      return; // exit loop, exit procedure, continue clock startup
      //        WiFiStartWps(); // infinite loop until connected
      //}
    }
  }
  
#endif

 
  WifiState = connected;

  tfts.println("\n Connected!");
  tfts.println(WiFi.localIP());
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
  delay(200);
}

void WifiReconnect() {
  if ((WifiState == disconnected) && ((millis() - TimeOfWifiReconnectAttempt) > WIFI_RETRY_CONNECTION_SEC * 1000)) {
    Serial.println("Attempting WiFi reconnection...");
    WiFi.reconnect();
    TimeOfWifiReconnectAttempt = millis();
  }    
}

#ifdef WIFI_USE_WPS   ////  WPS code
bool WiFiStartWps() {
  // erase settings
  sprintf(stored_config.config.wifi.ssid, ""); 
  sprintf(stored_config.config.wifi.password, ""); 
  stored_config.config.wifi.WPS_connected = 0x11; // invalid = different than 0x55
  Serial.print("Saving config.");
  stored_config.save();
  Serial.println(" Done.");
   
  tfts.setTextColor(TFT_GREEN, TFT_BLACK);
  tfts.println("WPS STARTED!");
  tfts.setTextColor(TFT_RED, TFT_BLACK);
  tfts.println("PRESS WPS BUTTON ON THE ROUTER");

  
  WifiState = wps_active;
  WiFi.onEvent(WiFiEvent);
  WiFi.mode(WIFI_MODE_STA);

  Serial.println("Starting WPS");

  wpsInitConfig();
  esp_wifi_wps_enable(&wps_config);
  esp_wifi_wps_start(0);  


  // loop until connected
  tfts.setTextColor(TFT_BLUE, TFT_BLACK);
  while (WifiState != connected) {
    delay(2000);
    tfts.print(".");
    Serial.print(".");
  }
  tfts.setTextColor(TFT_WHITE, TFT_BLACK);
  Serial.print("Saving config.");
  sprintf(stored_config.config.wifi.ssid, "%s", WiFi.SSID()); 
//   memset(stored_config.config.wifi.ssid, '\0', sizeof(stored_config.config.wifi.ssid));
//   strcpy(stored_config.config.wifi.ssid, WiFi.SSID()); 
     
  sprintf(stored_config.config.wifi.password, ""); // can't save a password from WPS
  stored_config.config.wifi.WPS_connected = StoredConfig::valid;
  stored_config.save();
  Serial.println(" WPS finished."); 
}
#endif

// Get an API Key by registering on
// https://ipgeolocation.io
// OR


bool GetGeoLocationTimeZoneOffset() {
  Serial.println("Starting Geolocation query...");
// https://app.abstractapi.com/api/ip-geolocation/    // free for 5k loopkups per month.
// Live test:  https://ipgeolocation.abstractapi.com/v1/?api_key=e11dc0f9bab446bfa9957aad2c4ad064
  IPGeolocation location(GEOLOCATION_API_KEY,"ABSTRACT");
  IPGeo IPG;
  if (location.updateStatus(&IPG)) {
   
    Serial.println(String("Geo Time Zone: ") + String(IPG.tz));
    Serial.println(String("Geo TZ Offset: ") + String(IPG.offset));  // we are interested in this one, type = double
    Serial.println(String("Geo Current Time: ") + String(IPG.current_time)); // currently not used
    GeoLocTZoffset = IPG.offset;
    return true;
  } else {
    Serial.println("Geolocation failed.");    
    return false;
  }
}
