/*
 * Project: Alternative firmware for EleksTube IPS clock
 * Hardware: ESP32
 * File description: User preferences for the complete project
 * User defines are located in "USER_DEFINES.h"
 */

#ifndef USER_DEFINES_H_
#define USER_DEFINES_H_

//#define DEBUG_OUTPUT
//#define DEBUG_OUTPUT_EXTENDED

// ************* Type of clock  *************
#define HARDWARE_Elekstube_CLOCK  // uncomment for the original Elekstube clock
//#define HARDWARE_SI_HAI_CLOCK  // uncomment for the SI HAI copy of the clock
//#define HARDWARE_NovelLife_SE_CLOCK  // uncomment for the NovelLife SE version; non-SE not tested


// ************* Version Infomation  *************
#define DEVICE_NAME       "IPS-clock"
#define FIRMWARE_VERSION  "IPS clock v0.4"
#define USE_CLK_FILES   // select between .CLK and .BMP images
#define ESP_MANUFACTURER  "ESPRESSIF"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "IPS clock"

// ************* Display Dimming  *************
#define NIGHT_TIME  22                  // dim displays at 10 pm 
#define DAY_TIME     7                  // full brightness after 7 am
#define BACKLIGHT_DIMMED_INTENSITY  0   // 0..7
#define TFT_DIMMED_INTENSITY  48        // 0..255

// ************* WiFi config *************
#define WIFI_CONNECT_TIMEOUT_SEC  20
#define WIFI_RETRY_CONNECTION_SEC  15
// WPS Mode (use EITHER this or SSID and Password)
#define ESP_WPS_MODE      WPS_TYPE_PBC
#define WIFI_USE_WPS
// SSID and Password Mode
// CAUTION - Hard coded password stored clear text in BIN file
//#define SECRET_WIFI_SSID    "YOUR-SSID"
//#define SECRET_WIFI_PASSWD  "yourPassword"


//  *************  Geolocation  *************
// Get your API Key on https://www.abstractapi.com/ (login) 
// --> https://app.abstractapi.com/api/ip-geolocation/tester (key)
#define GEOLOCATION_API_KEY "keykeykey"


// ************* MQTT config *************
// Easy mode: Setup an account at smartnest.cz and make a new thermostat device
//#define MQTT_ENABLED
//#define MQTT_BROKER   "smartnest.cz"
//#define MQTT_CLIENT   "clientid"
//#define MQTT_USERNAME "username"
//#define MQTT_PASSWORD "apikey"
#define MQTT_PORT 1883                          // Broker port
#define MQTT_RECONNECT_WAIT_SEC  33             // how long to wait between retries to connect to broker
#define MQTT_REPORT_STATUS_EVERY_SEC  317       // How often to report status to MQTT Broker

      
#endif
