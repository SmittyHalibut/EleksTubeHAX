/*
 * Author: Aljaz Ogrin
 * Project: Alternative firmware for EleksTube IPS clock
 * Hardware: ESP32
 * File description: Global configuration for the complete project
 */
 
#ifndef GLOBAL_DEFINES_H_
#define GLOBAL_DEFINES_H_

#include <stdint.h>
#include <Arduino.h>

// ************ General config *********************
//#define DEBUG_OUTPUT
#define DEVICE_NAME       "IPS-clock"
#define FIRMWARE_VERSION  "IPS clock v0.2"
#define NUMBER_OF_CLOCK_FONTS  6  // Number of .CLK or .BMP file sets 1X, 2X, 4X,...
#define SAVED_CONFIG_NAMESPACE  "configs"
#define USE_CLK_FILES   // select between .CLK and .BMP images

// ************ WiFi config *********************
#define ESP_WPS_MODE      WPS_TYPE_PBC  // push-button
#define ESP_MANUFACTURER  "ESPRESSIF"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "IPS clock"

#define WIFI_CONNECT_TIMEOUT_SEC  20
#define WIFI_RETRY_CONNECTION_SEC  15

#define GEOLOCATION_API_KEY "e11dc0f9bab446bfa9957aad2c4ad064"  // free for 5k loopkups per month. Get yours on https://www.abstractapi.com/ (login) --> https://app.abstractapi.com/api/ip-geolocation/tester (key)

// ************ MQTT config *********************

#define MQTT_ENABLED  // enable after creating an account, setting up the device on smartnest.cz and pilling in all the data below:

#define MQTT_BROKER "smartnest.cz"             // Broker host
#define MQTT_PORT 1883                         // Broker port
#define MQTT_USERNAME " username "             // Username from Smartnest
#define MQTT_PASSWORD " api key "              // Password from Smartnest or API key (under MY Account)
#define MQTT_CLIENT "deviceid "                // Device Id from smartnest
#define MQTT_RECONNECT_WAIT_SEC  30  // how long to wait between retries to connect to broker
#define MQTT_REPORT_STATUS_EVERY_SEC  71 // How often report status to MQTT Broker

// ************ Night time operation *********************
#define BACKLIGHT_DIMMED_INTENSITY  1  // 0..7
#define TFT_DIMMED_INTENSITY  25    // 0..255

// ************ Hardware definitions *********************

// Common indexing scheme, used to identify the digit
#define SECONDS_ONES (0)
#define SECONDS_TENS (1)
#define MINUTES_ONES (2)
#define MINUTES_TENS (3)
#define HOURS_ONES   (4)
#define HOURS_TENS   (5)
#define NUM_DIGITS   (6)

#define SECONDS_ONES_MAP (0x01 << SECONDS_ONES)
#define SECONDS_TENS_MAP (0x01 << SECONDS_TENS)
#define MINUTES_ONES_MAP (0x01 << MINUTES_ONES)
#define MINUTES_TENS_MAP (0x01 << MINUTES_TENS)
#define HOURS_ONES_MAP   (0x01 << HOURS_ONES)
#define HOURS_TENS_MAP   (0x01 << HOURS_TENS)

// WS2812 (or compatible) LEDs on the back of the display modules.
#define BACKLIGHTS_PIN (12)

// Buttons, active low, externally pulled up (with actual resistors!)
#define BUTTON_LEFT_PIN (33)
#define BUTTON_MODE_PIN (32)
#define BUTTON_RIGHT_PIN (35)
#define BUTTON_POWER_PIN (34)

// I2C to DS3231 RTC.
#define RTC_SCL_PIN (22)
#define RTC_SDA_PIN (21)

// Chip Select shift register, to select the display
#define CSSR_DATA_PIN (14)
#define CSSR_CLOCK_PIN (16)
#define CSSR_LATCH_PIN (17)

// SPI to displays
// DEFINED IN User_Setup.h
// Look for: TFT_MOSI, TFT_SCLK, TFT_CS, TFT_DC, and TFT_RST

// Power for all TFT displays are grounded through a MOSFET so they can all be turned off.
// Active HIGH.
#define TFT_ENABLE_PIN (27)

#endif /* GLOBAL_DEFINES_H_ */
