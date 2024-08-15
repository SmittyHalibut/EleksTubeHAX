/*
 * Author: Aljaz Ogrin
 * Project: Alternative firmware for EleksTube IPS clock
 * Original location: https://github.com/aly-fly/EleksTubeHAX
 * Hardware: ESP32
 * File description: Connects to the MQTT Broker "smartnest.cz". Uses a device "Thermostat".
 * Sends status and receives commands from WebApp, Android app or connected devices (SmartThings, Google assistant, Alexa, etc.)
 * Configuration: open file "GLOBAL_DEFINES.h"
 * Reference: https://github.com/aososam/Smartnest/tree/master/Devices/thermostat
 * Documentation: https://www.docu.smartnest.cz/
 */

#include "Mqtt_client_ips.h"
#include "WiFi.h"       // for ESP32
#include <PubSubClient.h>  // Download and install this library first from: https://www.arduinolibraries.info/libraries/pub-sub-client
#include <ArduinoJson.h>
#include "TempSensor.h"
#include "TFTs.h"
#include "Backlights.h"
#include "Clock.h" 

#define concat2(first, second) first second
#define concat3(first, second, third) first second third
#define concat4(first, second, third, fourth) first second third fourth

WiFiClient espClient;
PubSubClient MQTTclient(espClient);

#define MQTT_STATE_ON "ON"
#define MQTT_STATE_OFF "OFF"

#define MQTT_BRIGHTNESS_MIN 0
#define MQTT_BRIGHTNESS_MAX 255

#define MQTT_ITENSITY_MIN 0
#define MQTT_ITENSITY_MAX 7

// private:
int splitCommand(char* topic, char* tokens[], int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);

void MqttProcessCommand();
void MqttReportBattery();
void MqttReportStatus();
void MqttReportPowerState();
void MqttReportWiFiSignal();
void MqttReportTemperature();
void MqttReportNotification(String message);
void MqttReportGraphic(bool force);
void MqttReportBackOnChange();
void MqttReportBackEverything(bool force);
void MqttPeriodicReportBack();

char topic[100];
char msg[5];
uint32_t lastTimeSent = (uint32_t)(MQTT_REPORT_STATUS_EVERY_SEC * -1000);
uint8_t LastNotificationChecksum = 0;
uint32_t LastTimeTriedToConnect = 0;

bool MqttConnected = true; // skip error meggase if disabled
// commands from server    // = "directive/status"
bool MqttCommandPower = true;
bool MqttCommandMainPower = true;
bool MqttCommandBackPower = true;
bool MqttCommandPowerReceived = false;
bool MqttCommandMainPowerReceived = false;
bool MqttCommandBackPowerReceived = false;

bool MqttCommandUseTwelveHours = false;
bool MqttCommandUseTwelveHoursReceived = false;
bool MqttCommandBlankZeroHours = false;
bool MqttCommandBlankZeroHoursReceived = false;

int  MqttCommandState = 1;  
bool MqttCommandStateReceived = false;

uint8_t MqttCommandBrightness = -1;
uint8_t MqttCommandMainBrightness = -1;
uint8_t MqttCommandBackBrightness = -1;    
bool MqttCommandBrightnessReceived = false;
bool MqttCommandMainBrightnessReceived = false;
bool MqttCommandBackBrightnessReceived = false;

char MqttCommandPattern[24] = "";
bool MqttCommandPatternReceived = false;
char MqttCommandBackPattern[24] = "";
bool MqttCommandBackPatternReceived = false;

uint16_t MqttCommandBackColorPhase = -1;
bool MqttCommandBackColorPhaseReceived = false;

uint8_t MqttCommandGraphic = -1;  
bool MqttCommandGraphicReceived = false;
uint8_t MqttCommandMainGraphic = -1;  
bool MqttCommandMainGraphicReceived = false;

uint8_t MqttCommandPulseBpm = -1;  
bool MqttCommandPulseBpmReceived = false;

uint8_t MqttCommandBreathBpm = -1;  
bool MqttCommandBreathBpmReceived = false;

float MqttCommandRainbowSec = -1;  
bool MqttCommandRainbowSecReceived = false;

// status to server
bool MqttStatusPower = true;
bool MqttStatusMainPower = true;
bool MqttStatusBackPower = true;
bool MqttStatusUseTwelveHours = true;
bool MqttStatusBlankZeroHours = true;
int MqttStatusState = 0;
int MqttStatusBattery = 7;
uint8_t MqttStatusBrightness = 0;
uint8_t MqttStatusMainBrightness = 0;
uint8_t MqttStatusBackBrightness = 0;
char MqttStatusPattern[24] = "";
char MqttStatusBackPattern[24] = "";
uint16_t MqttStatusBackColorPhase = 0;
uint8_t MqttStatusGraphic = 0;
uint8_t MqttStatusMainGraphic = 0;
uint8_t MqttStatusPulseBpm = 0;
uint8_t MqttStatusBreathBpm = 0;
float MqttStatusRainbowSec = 0;

int LastSentSignalLevel = 999;
int LastSentPowerState = -1;
int LastSentMainPowerState = -1;
int LastSentBackPowerState = -1;
int LastSentStatus = -1;
int LastSentBrightness = -1;
int LastSentMainBrightness = -1;
int LastSentBackBrightness = -1;
char LastSentPattern[24] = "";
char LastSentBackPattern[24] = "";
int LastSentBackColorPhase = -1;
int LastSentGraphic = -1;
int LastSentMainGraphic = -1;
bool LastSentUseTwelveHours = false;
bool LastSentBlankZeroHours = false;
uint8_t LastSentPulseBpm = -1;
uint8_t LastSentBreathBpm = -1;
float LastSentRainbowSec = -1;

double round1(double value) {
   return (int)(value * 10 + 0.5) / 10.0;
}

void sendToBroker(const char* topic, const char* message) {
  if (MQTTclient.connected()) {
    char topicArr[100];
    sprintf(topicArr, "%s/%s", MQTT_CLIENT, topic);
    MQTTclient.publish(topicArr, message, true);
#ifdef DEBUG_OUTPUT // long output
    Serial.print("Sending to MQTT: ");
    Serial.print(topicArr);
    Serial.print("/");
    Serial.println(message);
#else
    Serial.print("TX MQTT: ");
    Serial.print(topicArr);
    Serial.print(" ");
    Serial.println(message);
#endif    
    delay (120);  
  }
}

void MqttReportState(bool force) {
#ifdef MQTT_HOME_ASSISTANT
  if(MQTTclient.connected()) {
    // time_t rawtime;
    // struct tm * timeinfo;
    // char last_seen [80];
    // rawtime = uclock.local_time;
    // timeinfo = localtime (&rawtime);
    // strftime (last_seen, 80, "%Y-%m-%dT%H:%M:%SZ", timeinfo);

    if(force 
      || MqttStatusMainPower != LastSentMainPowerState 
      || MqttStatusMainBrightness != LastSentMainBrightness 
      || MqttStatusMainGraphic != LastSentMainGraphic) {
      
      JsonDocument state;
      state["state"] =  MqttStatusMainPower == 0 ? MQTT_STATE_OFF : MQTT_STATE_ON;
      state["brightness"] = MqttStatusMainBrightness;
      state["effect"] = MqttStatusMainGraphic;
      // state["Last seen"] = last_seen;

      char buffer[256];
      size_t n = serializeJson(state, buffer);
      const char* topic = concat2(MQTT_CLIENT, "/main");
      MQTTclient.publish(topic, buffer, true);
      LastSentMainPowerState = MqttStatusMainPower;
      LastSentMainBrightness = MqttStatusMainBrightness;
      LastSentMainGraphic = MqttStatusMainGraphic;

      Serial.print("TX MQTT: ");
      Serial.print(topic);
      Serial.print(" ");
      Serial.println(buffer);
    }

    if(force 
      || MqttStatusBackPower != LastSentBackPowerState 
      || MqttStatusBackBrightness != LastSentBackBrightness 
      || strcmp(MqttStatusBackPattern, LastSentBackPattern) != 0
      || MqttStatusBackColorPhase != LastSentBackColorPhase ) {
      
      JsonDocument state;
      state["state"] =  MqttStatusBackPower == 0 ? MQTT_STATE_OFF : MQTT_STATE_ON;
      state["brightness"] = MqttStatusBackBrightness; //map(MqttStatusBackBrightness, MQTT_ITENSITY_MIN, MQTT_ITENSITY_MAX, MQTT_BRIGHTNESS_MIN, MQTT_BRIGHTNESS_MAX);
      state["effect"] = MqttStatusBackPattern;
      state["color_mode"] = "hs";
      state["color"]["h"] =  backlights.phaseToHue(MqttStatusBackColorPhase);
      state["color"]["s"] = 100.f;
      // state["color"]["b"] =  backlights.phaseToIntensity((MqttStatusBackColorPhase + 512) % backlights.max_phase);
      // state["Last seen"] = last_seen;

      char buffer[256];
      size_t n = serializeJson(state, buffer);
      const char* topic = concat2(MQTT_CLIENT, "/back");
      MQTTclient.publish(topic, buffer, true);
      LastSentBackPowerState = MqttStatusBackPower;
      LastSentBackBrightness = MqttStatusBackBrightness;
      strcpy(LastSentBackPattern, MqttStatusBackPattern);
      LastSentBackColorPhase = MqttStatusBackColorPhase;

      Serial.print("TX MQTT: ");
      Serial.print(topic);
      Serial.print(" ");
      Serial.println(buffer);
    }

    if(force 
      || MqttStatusUseTwelveHours != LastSentUseTwelveHours) {
      
      JsonDocument state;
      state["state"] =  MqttStatusUseTwelveHours ? MQTT_STATE_ON : MQTT_STATE_OFF;
      // state["Last seen"] = last_seen;

      char buffer[256];
      size_t n = serializeJson(state, buffer);
      const char* topic = concat2(MQTT_CLIENT, "/use_twelve_hours");
      MQTTclient.publish(topic, buffer, true);
      LastSentUseTwelveHours = MqttStatusUseTwelveHours;

      Serial.print("TX MQTT: ");
      Serial.print(topic);
      Serial.print(" ");
      Serial.println(buffer);
    }

    if(force 
      || MqttStatusBlankZeroHours != LastSentBlankZeroHours) {
      
      JsonDocument state;
      state["state"] =  MqttStatusBlankZeroHours ? MQTT_STATE_ON : MQTT_STATE_OFF;
      // state["Last seen"] = last_seen;

      char buffer[256];
      size_t n = serializeJson(state, buffer);
      const char* topic = concat2(MQTT_CLIENT, "/blank_zero_hours");
      MQTTclient.publish(topic, buffer, true);
      LastSentBlankZeroHours = MqttStatusBlankZeroHours;

      Serial.print("TX MQTT: ");
      Serial.print(topic);
      Serial.print(" ");
      Serial.println(buffer);
    }

    if(force 
      || MqttStatusPulseBpm != LastSentPulseBpm) {
      
      JsonDocument state;
      state["state"] =  MqttStatusPulseBpm;
      // state["Last seen"] = last_seen;

      char buffer[256];
      size_t n = serializeJson(state, buffer);
      const char* topic = concat2(MQTT_CLIENT, "/pulse_bpm");
      MQTTclient.publish(topic, buffer, true);
      LastSentPulseBpm = MqttStatusPulseBpm;

      Serial.print("TX MQTT: ");
      Serial.print(topic);
      Serial.print(" ");
      Serial.println(buffer);
    }

    if(force 
      || MqttStatusBreathBpm != LastSentBreathBpm) {
      
      JsonDocument state;
      state["state"] =  MqttStatusBreathBpm;
      // state["Last seen"] = last_seen;

      char buffer[256];
      size_t n = serializeJson(state, buffer);
      const char* topic = concat2(MQTT_CLIENT, "/breath_bpm");
      MQTTclient.publish(topic, buffer, true);
      LastSentBreathBpm = MqttStatusBreathBpm;

      Serial.print("TX MQTT: ");
      Serial.print(topic);
      Serial.print(" ");
      Serial.println(buffer);
    }

    if(force 
      || MqttStatusRainbowSec != LastSentRainbowSec) {
      
      JsonDocument state;
      state["state"] =  round1(MqttStatusRainbowSec);
      // state["Last seen"] = last_seen;

      char buffer[256];
      serializeJson(state, buffer);
      const char* topic = concat2(MQTT_CLIENT, "/rainbow_duration");
      MQTTclient.publish(topic, buffer, true);
      LastSentRainbowSec = MqttStatusRainbowSec;

      Serial.print("TX MQTT: ");
      Serial.print(topic);
      Serial.print(" ");
      Serial.println(buffer);
    }
  }
#endif
}

void MqttStart() {
#ifdef MQTT_ENABLED
  MqttConnected = false;
  if (((millis() - LastTimeTriedToConnect) > (MQTT_RECONNECT_WAIT_SEC * 100)) || (LastTimeTriedToConnect == 0)) {
    LastTimeTriedToConnect = millis();
    MQTTclient.setServer(MQTT_BROKER, MQTT_PORT);
    MQTTclient.setCallback(callback);
    MQTTclient.setBufferSize(2048);

    Serial.println("Connecting to MQTT...");
    if (MQTTclient.connect(MQTT_CLIENT, MQTT_USERNAME, MQTT_PASSWORD)) {
        Serial.println("MQTT connected");
        MqttConnected = true;
    } else {
      if (MQTTclient.state() == 5) {
          Serial.println("Connection not allowed by broker, possible reasons:");
          Serial.println("- Device is already online. Wait some seconds until it appears offline");
          Serial.println("- Wrong Username or password. Check credentials");
          Serial.println("- Client Id does not belong to this username, verify ClientId");
      } else {
          Serial.print("Not possible to connect to Broker Error code:");
          Serial.println(MQTTclient.state());
      }
      return;  // do not continue if not connected
    }
      
  #ifndef MQTT_HOME_ASSISTANT
    char subscibeTopic[100];
    sprintf(subscibeTopic, "%s/#", MQTT_CLIENT);
    MQTTclient.subscribe(subscibeTopic);  //Subscribes to all messages send to the device

    sendToBroker("report/online", "true");  // Reports that the device is online
    sendToBroker("report/firmware", FIRMWARE_VERSION);  // Reports the firmware version
    sendToBroker("report/ip", (char*)WiFi.localIP().toString().c_str());  // Reports the ip
    sendToBroker("report/network", (char*)WiFi.SSID().c_str());  // Reports the network name
    MqttReportWiFiSignal();
  #endif

  #ifdef MQTT_HOME_ASSISTANT
    char subscibeTopic[100];
    sprintf(subscibeTopic, "%s/main/set", MQTT_CLIENT);
    MQTTclient.subscribe(subscibeTopic);

    sprintf(subscibeTopic, "%s/back/set", MQTT_CLIENT);
    MQTTclient.subscribe(subscibeTopic);

    sprintf(subscibeTopic, "%s/use_twelve_hours/set", MQTT_CLIENT);
    MQTTclient.subscribe(subscibeTopic);

    sprintf(subscibeTopic, "%s/blank_zero_hours/set", MQTT_CLIENT);
    MQTTclient.subscribe(subscibeTopic);

    sprintf(subscibeTopic, "%s/pulse_bpm/set", MQTT_CLIENT);
    MQTTclient.subscribe(subscibeTopic);

    sprintf(subscibeTopic, "%s/breath_bpm/set", MQTT_CLIENT);
    MQTTclient.subscribe(subscibeTopic);

    sprintf(subscibeTopic, "%s/rainbow_duration/set", MQTT_CLIENT);
    MQTTclient.subscribe(subscibeTopic);
  #endif
  }
#endif
}

int splitCommand(char* topic, char* tokens[], int tokensNumber) {
  int mqttClientLength = strlen(MQTT_CLIENT);
  int topicLength = strlen(topic);
  int finalLength = topicLength - mqttClientLength + 2;
  char* command = (char*) malloc(finalLength);

  strncpy(command, topic + (mqttClientLength + 1), finalLength - 2);
  
  const char s[2] = "/";
  int pos = 0;
  tokens[0] = strtok(command, s);
  while (pos < tokensNumber - 1 && tokens[pos] != NULL) {
      pos++;
      tokens[pos] = strtok(NULL, s);
  }

  free(command);

  return pos;
}

void checkMqtt() {
  MqttConnected = MQTTclient.connected();
  if (!MqttConnected) {
    MqttStart();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {  //A new message has been received
  #ifdef DEBUG_OUTPUT
  Serial.print("Received MQTT topic: ");
  Serial.print(topic);                       // long output
  #endif    
  int commandNumber = 10;
  char* command[commandNumber];
  commandNumber = splitCommand(topic, command, commandNumber);

  #ifndef MQTT_HOME_ASSISTANT
  char message[length + 1];
  sprintf(message, "%c", (char)payload[0]);
  for (int i = 1; i < length; i++) {
      sprintf(message, "%s%c", message, (char)payload[i]);
  }
  #ifdef DEBUG_OUTPUT
  Serial.print("\t     Message: ");
  Serial.println(message);
  #else    
  Serial.print("MQTT RX: ");
  Serial.print(command[0]);
  Serial.print("/");
  Serial.print(command[1]);
  Serial.print("/");
  Serial.println(message);
  #endif    

  if (commandNumber < 2) {
    // otherwise code below crashes on the strmp on non-initialized pointers in command[] array
    Serial.println("Number of command in MQTT message < 2!");
    return; 
  }
    
    //------------------Decide what to do depending on the topic and message---------------------------------
  if (strcmp(command[0], "directive") == 0 && strcmp(command[1], "powerState") == 0) {  // Turn On or OFF
    if (strcmp(message, "ON") == 0) {
      MqttCommandPower = true;
      MqttCommandPowerReceived = true;
    } else if (strcmp(message, "OFF") == 0) {
      MqttCommandPower = false;
      MqttCommandPowerReceived = true;
    }                                                       //      SmartNest:                         // SmartThings
  } else if (strcmp(command[0], "directive") == 0 && (strcmp(command[1], "setpoint") == 0) || (strcmp(command[1], "percentage") == 0)) {
      double valueD = atof(message);
      if (!isnan(valueD)) {
        MqttCommandState = (int) valueD;
        MqttCommandStateReceived = true;
      }
  }
  #endif

  #ifdef MQTT_HOME_ASSISTANT
  char message[length + 1];
  sprintf(message, "%c", (char)payload[0]);
  for (int i = 1; i < length; i++) {
      sprintf(message, "%s%c", message, (char)payload[i]);
  }
  Serial.print("RX MQTT: ");
  Serial.print(topic);
  Serial.print(" ");
  Serial.println(message);
  if (strcmp(command[0], "main") == 0 && strcmp(command[1], "set") == 0) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);
    
    if(doc.containsKey("state")) {
      MqttCommandMainPower = doc["state"] == MQTT_STATE_ON;
      MqttCommandMainPowerReceived = true;
    }
    if(doc.containsKey("brightness")) {
       MqttCommandMainBrightness = doc["brightness"];
       MqttCommandMainBrightnessReceived = true;
     }
    if(doc.containsKey("effect")) {
      MqttCommandMainGraphic = doc["effect"];   
      MqttCommandMainGraphicReceived = true;
      }

    doc.clear();
  } 
  if (strcmp(command[0], "back") == 0 && strcmp(command[1], "set") == 0) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);
    
    if(doc.containsKey("state")) {
      MqttCommandBackPower = doc["state"] == MQTT_STATE_ON;
      MqttCommandBackPowerReceived = true;
    }
    if(doc.containsKey("brightness")) {
      MqttCommandBackBrightness = doc["brightness"]; //map(doc["brightness"], MQTT_BRIGHTNESS_MIN, MQTT_BRIGHTNESS_MAX, MQTT_ITENSITY_MIN, MQTT_ITENSITY_MAX);
      MqttCommandBackBrightnessReceived = true;
    }
    if(doc.containsKey("effect")) {
      strcpy(MqttCommandBackPattern, doc["effect"]);
      MqttCommandBackPatternReceived = true;
      }
    if(doc.containsKey("color")) {
      MqttCommandBackColorPhase = backlights.hueToPhase(doc["color"]["h"]);   
      MqttCommandBackColorPhaseReceived = true;
      }
    doc.clear();
  }
  if (strcmp(command[0], "use_twelve_hours") == 0 && strcmp(command[1], "set") == 0) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);
    
    if(doc.containsKey("state")) {
      MqttCommandUseTwelveHours = doc["state"] == MQTT_STATE_ON;
      MqttCommandUseTwelveHoursReceived = true;
    }
    doc.clear();
  }
  if (strcmp(command[0], "blank_zero_hours") == 0 && strcmp(command[1], "set") == 0) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);
    
    if(doc.containsKey("state")) {
      MqttCommandBlankZeroHours = doc["state"] == MQTT_STATE_ON;
      MqttCommandBlankZeroHoursReceived = true;
    }
    doc.clear();
  }
  if (strcmp(command[0], "pulse_bpm") == 0 && strcmp(command[1], "set") == 0) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);
    
    if(doc.containsKey("state")) {
      MqttCommandPulseBpm = uint8_t(doc["state"]);
      MqttCommandPulseBpmReceived = true;
    }
    doc.clear();
  }
  if (strcmp(command[0], "breath_bpm") == 0 && strcmp(command[1], "set") == 0) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);
    
    if(doc.containsKey("state")) {
      MqttCommandBreathBpm = uint8_t(doc["state"]);
      MqttCommandBreathBpmReceived = true;
    }
    doc.clear();
  }
  if (strcmp(command[0], "rainbow_duration") == 0 && strcmp(command[1], "set") == 0) {
    JsonDocument doc;
    deserializeJson(doc, payload, length);
    
    if(doc.containsKey("state")) {
      MqttCommandRainbowSec = float(doc["state"]);
      MqttCommandRainbowSecReceived = true;
    }
    doc.clear();
  }
  #endif
 }

void MqttLoopFrequently(){
#ifdef MQTT_ENABLED
  MQTTclient.loop(); 
  checkMqtt();
#endif  
}

void MqttLoopInFreeTime(){
#ifdef MQTT_ENABLED
  MqttReportBackOnChange();
  MqttPeriodicReportBack();
#endif  
}

void MqttReportBattery() {
  char message2[5];
  sprintf(message2, "%d", MqttStatusBattery);
  sendToBroker("report/battery", message2); 
} 

void MqttReportStatus() {
  if (LastSentStatus != MqttStatusState) {
    char message2[5];
    sprintf(message2, "%d", MqttStatusState);
    sendToBroker("report/setpoint", message2);
    LastSentStatus = MqttStatusState;
  }
}    

void MqttReportTemperature() {
  #ifdef ONE_WIRE_BUS_PIN
  if (fTemperature > -30) { // transmit data to MQTT only if data is valid
    sendToBroker("report/temperature", sTemperatureTxt);
  }
  #endif  
}    

void MqttReportPowerState() {
  if (MqttStatusPower != LastSentPowerState) {  
    sendToBroker("report/powerState", MqttStatusPower == 0 ? MQTT_STATE_OFF : MQTT_STATE_ON);

    LastSentPowerState = MqttStatusPower;
  }
}

void MqttReportWiFiSignal() {
  char signal[5];
  int SignalLevel = WiFi.RSSI();
  if (SignalLevel != LastSentSignalLevel) {
    sprintf(signal, "%d", SignalLevel);
    sendToBroker("report/signal", signal);  // Reports the signal strength
    LastSentSignalLevel = SignalLevel;
  }
}

void MqttReportNotification(String message) {
  int i;
  byte NotificationChecksum = 0;
  for (i=0; i<message.length(); i++) {    
    NotificationChecksum += byte(message[i]);
  }
  // send only different notification, do not re-send same notifications!
  if (NotificationChecksum != LastNotificationChecksum) {
    // string to char array
    char msg2[message.length() + 1]; 
    strcpy(msg2, message.c_str());    
    sendToBroker("report/notification", msg2);
    LastNotificationChecksum = NotificationChecksum;
  }
}

void MqttReportGraphic(bool force) {
  if (force || MqttStatusGraphic != LastSentGraphic) {
    char graphic[2] = "";
    sprintf(graphic, "%i", MqttStatusGraphic);
    sendToBroker("graphic", graphic);  // Reports the signal strength

    LastSentGraphic = MqttStatusGraphic;
  }
}

void MqttReportBackEverything(bool force) {
  if(MQTTclient.connected()) {
    #ifndef MQTT_HOME_ASSISTANT
    MqttReportPowerState();
    MqttReportStatus();
    // MqttReportBattery();
    MqttReportWiFiSignal();
    MqttReportTemperature();
    #endif

    #ifdef MQTT_HOME_ASSISTANT
    MqttReportState(force);
    #endif
    
    lastTimeSent = millis();
  }
}

bool discoveryReported = false;

void MqttReportDiscovery() {
  #ifdef MQTT_HOME_ASSISTANT_DISCOVERY
  char json_buffer[1024];
  JsonDocument discovery;

  // Main Light
  discovery["device"]["identifiers"][0] = MQTT_CLIENT;
  discovery["device"]["manufacturer"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MANUFACTURER;
  discovery["device"]["model"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["name"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["sw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_SW_VERSION;
  discovery["device"]["hw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_HW_VERSION;
  discovery["device"]["connections"][0][0] = "mac";
  discovery["device"]["connections"][0][1] = WiFi.macAddress();
  discovery["unique_id"] = concat2(MQTT_CLIENT, "_main");
  discovery["object_id"] = concat2(MQTT_CLIENT, "_main");
  discovery["name"] = "Main";
  discovery["schema"] = "json";
  discovery["state_topic"] = concat2(MQTT_CLIENT, "/main");
  discovery["json_attributes_topic"] = concat2(MQTT_CLIENT, "/main");
  discovery["command_topic"] = concat2(MQTT_CLIENT, "/main/set");
  discovery["brightness"] = true;
  discovery["brightness_scale"] = 255;
  discovery["effect"] = true;
  for (size_t i = 1; i <= tfts.NumberOfClockFaces; i++)
  {
    discovery["effect_list"][i-1] = i;
  }
  size_t main_n = serializeJson(discovery, json_buffer);
  const char* main_topic = concat3("homeassistant/light/", MQTT_CLIENT, "_main/light/config");
  MQTTclient.publish(main_topic, json_buffer, true);
  delay(120);
  Serial.print("TX MQTT: ");
  Serial.print(main_topic);
  Serial.print(" ");
  Serial.println(json_buffer);
  discovery.clear();

  // Back Light
  discovery["device"]["identifiers"][0] = MQTT_CLIENT;
  discovery["device"]["manufacturer"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MANUFACTURER;
  discovery["device"]["model"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["name"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["sw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_SW_VERSION;
  discovery["device"]["hw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_HW_VERSION;
  discovery["device"]["connections"][0][0] = "mac";
  discovery["device"]["connections"][0][1] = WiFi.macAddress();
  discovery["unique_id"] = concat2(MQTT_CLIENT, "_back");
  discovery["object_id"] = concat2(MQTT_CLIENT, "_back");
  discovery["name"] = "Back";
  discovery["schema"] = "json";
  discovery["state_topic"] = concat2(MQTT_CLIENT, "/back");
  discovery["json_attributes_topic"] = concat2(MQTT_CLIENT, "/back");
  discovery["command_topic"] = concat2(MQTT_CLIENT, "/back/set");
  discovery["brightness"] = true;
  discovery["brightness_scale"] = 7;
  discovery["effect"] = true;
  for (size_t i = 0; i < backlights.num_patterns; i++)
  {
    discovery["effect_list"][i] = backlights.patterns_str[i];
  }
  discovery["supported_color_modes"][0] = "hs";
  size_t back_n = serializeJson(discovery, json_buffer);
  const char* back_topic = concat3("homeassistant/light/", MQTT_CLIENT, "_back/light/config");
  MQTTclient.publish(back_topic, json_buffer, true);
  delay(120);
  Serial.print("TX MQTT: ");
  Serial.print(back_topic);
  Serial.print(" ");
  Serial.println(json_buffer);
  discovery.clear();

  // Use Twelwe Hours
  discovery["device"]["identifiers"][0] = MQTT_CLIENT;
  discovery["device"]["manufacturer"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MANUFACTURER;
  discovery["device"]["model"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["name"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["sw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_SW_VERSION;
  discovery["device"]["hw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_HW_VERSION;
  discovery["device"]["connections"][0][0] = "mac";
  discovery["device"]["connections"][0][1] = WiFi.macAddress();
  discovery["unique_id"] = concat2(MQTT_CLIENT, "_use_twelve_hours");
  discovery["object_id"] = concat2(MQTT_CLIENT, "_use_twelve_hours");
  discovery["entity_category"] = "config";
  discovery["name"] = "Use Twelve Hours";
  discovery["state_topic"] = concat2(MQTT_CLIENT, "/use_twelve_hours");
  discovery["json_attributes_topic"] = concat2(MQTT_CLIENT, "/use_twelve_hours");
  discovery["command_topic"] = concat2(MQTT_CLIENT, "/use_twelve_hours/set");
  discovery["value_template"] = "{{ value_json.state }}";
  discovery["state_on"] = "ON";
  discovery["state_off"] = "OFF";
  discovery["payload_on"] = "{\"state\":\"ON\"}";
  discovery["payload_off"] = "{\"state\":\"OFF\"}";
  size_t useTwelveHours_n = serializeJson(discovery, json_buffer);
  const char* useTwelveHours_topic = concat3("homeassistant/switch/", MQTT_CLIENT, "_use_twelve_hours/switch/config");
  MQTTclient.publish(useTwelveHours_topic, json_buffer, true);
  delay(120);
  Serial.print("TX MQTT: ");
  Serial.print(useTwelveHours_topic);
  Serial.print(" ");
  Serial.println(json_buffer);
  discovery.clear();

  // Blank Zero Hours
  discovery["device"]["identifiers"][0] = MQTT_CLIENT;
  discovery["device"]["manufacturer"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MANUFACTURER;
  discovery["device"]["model"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["name"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["sw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_SW_VERSION;
  discovery["device"]["hw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_HW_VERSION;
  discovery["device"]["connections"][0][0] = "mac";
  discovery["device"]["connections"][0][1] = WiFi.macAddress();
  discovery["unique_id"] = concat2(MQTT_CLIENT, "_blank_zero_hours");
  discovery["object_id"] = concat2(MQTT_CLIENT, "_blank_zero_hours");
  discovery["entity_category"] = "config";
  discovery["name"] = "Blank Zero Hours";
  discovery["state_topic"] = concat2(MQTT_CLIENT, "/blank_zero_hours");
  discovery["json_attributes_topic"] = concat2(MQTT_CLIENT, "/blank_zero_hours");
  discovery["command_topic"] = concat2(MQTT_CLIENT, "/blank_zero_hours/set");
  discovery["value_template"] = "{{ value_json.state }}";
  discovery["state_on"] = "ON";
  discovery["state_off"] = "OFF";
  discovery["payload_on"] = "{\"state\":\"ON\"}";
  discovery["payload_off"] = "{\"state\":\"OFF\"}";
  size_t blankZeroHours_n = serializeJson(discovery, json_buffer);
  const char* blankZeroHours_topic = concat3("homeassistant/switch/", MQTT_CLIENT, "_blank_zero_hours/switch/config");
  MQTTclient.publish(blankZeroHours_topic, json_buffer, true);
  delay(120);
  Serial.print("TX MQTT: ");
  Serial.print(blankZeroHours_topic);
  Serial.print(" ");
  Serial.println(json_buffer);
  discovery.clear();

  // Pulses per minute
  discovery["device"]["identifiers"][0] = MQTT_CLIENT;
  discovery["device"]["manufacturer"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MANUFACTURER;
  discovery["device"]["model"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["name"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["sw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_SW_VERSION;
  discovery["device"]["hw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_HW_VERSION;
  discovery["device"]["connections"][0][0] = "mac";
  discovery["device"]["connections"][0][1] = WiFi.macAddress();
  discovery["unique_id"] = concat2(MQTT_CLIENT, "_pulse_bpm");
  discovery["object_id"] = concat2(MQTT_CLIENT, "_pulse_bpm");
  discovery["entity_category"] = "config";
  discovery["name"] = "Pulse, bpm";
  discovery["state_topic"] = concat2(MQTT_CLIENT, "/pulse_bpm");
  discovery["json_attributes_topic"] = concat2(MQTT_CLIENT, "/pulse_bpm");
  discovery["command_topic"] = concat2(MQTT_CLIENT, "/pulse_bpm/set");
  discovery["command_template"] = "{\"state\":{{value}}}";
  discovery["step"] = 1;
  discovery["min"] = 20;
  discovery["max"] = 120;
  discovery["mode"] = "slider";
  discovery["value_template"] = "{{ value_json.state }}";
  size_t pulseBpm_n = serializeJson(discovery, json_buffer);
  const char* pulseBpm_topic = concat3("homeassistant/number/", MQTT_CLIENT, "_pulse_bpm/number/config");
  MQTTclient.publish(pulseBpm_topic, json_buffer, true);
  delay(120);
  Serial.print("TX MQTT: ");
  Serial.print(pulseBpm_topic);
  Serial.print(" ");
  Serial.println(json_buffer);
  discovery.clear();

  // Breathes per minute
  discovery["device"]["identifiers"][0] = MQTT_CLIENT;
  discovery["device"]["manufacturer"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MANUFACTURER;
  discovery["device"]["model"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["name"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["sw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_SW_VERSION;
  discovery["device"]["hw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_HW_VERSION;
  discovery["device"]["connections"][0][0] = "mac";
  discovery["device"]["connections"][0][1] = WiFi.macAddress();
  discovery["unique_id"] = concat2(MQTT_CLIENT, "_breath_bpm");
  discovery["object_id"] = concat2(MQTT_CLIENT, "_breath_bpm");
  discovery["entity_category"] = "config";
  discovery["name"] = "Breath, bpm";
  discovery["state_topic"] = concat2(MQTT_CLIENT, "/breath_bpm");
  discovery["json_attributes_topic"] = concat2(MQTT_CLIENT, "/breath_bpm");
  discovery["command_topic"] = concat2(MQTT_CLIENT, "/breath_bpm/set");
  discovery["command_template"] = "{\"state\":{{value}}}";
  discovery["step"] = 1;
  discovery["min"] = 5;
  discovery["max"] = 60;
  discovery["mode"] = "slider";
  discovery["value_template"] = "{{ value_json.state }}";
  size_t breathBpm_n = serializeJson(discovery, json_buffer);
  const char* breathBpm_topic = concat3("homeassistant/number/", MQTT_CLIENT, "_breath_bpm/number/config");
  MQTTclient.publish(breathBpm_topic, json_buffer, true);
  delay(120);
  Serial.print("TX MQTT: ");
  Serial.print(breathBpm_topic);
  Serial.print(" ");
  Serial.println(json_buffer);
  discovery.clear();

  // Rainbow duration
  discovery["device"]["identifiers"][0] = MQTT_CLIENT;
  discovery["device"]["manufacturer"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MANUFACTURER;
  discovery["device"]["model"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["name"] = MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL;
  discovery["device"]["sw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_SW_VERSION;
  discovery["device"]["hw_version"] = MQTT_HOME_ASSISTANT_DISCOVERY_HW_VERSION;
  discovery["device"]["connections"][0][0] = "mac";
  discovery["device"]["connections"][0][1] = WiFi.macAddress();
  discovery["unique_id"] = concat2(MQTT_CLIENT, "_rainbow_duration");
  discovery["object_id"] = concat2(MQTT_CLIENT, "_rainbow_duration");
  discovery["entity_category"] = "config";
  discovery["name"] = "Rainbow, sec";
  discovery["state_topic"] = concat2(MQTT_CLIENT, "/rainbow_duration");
  discovery["json_attributes_topic"] = concat2(MQTT_CLIENT, "/rainbow_duration");
  discovery["command_topic"] = concat2(MQTT_CLIENT, "/rainbow_duration/set");
  discovery["command_template"] = "{\"state\":{{value}}}";
  discovery["step"] = 0.1;
  discovery["min"] = 0.2;
  discovery["max"] = 10;
  discovery["mode"] = "slider";
  discovery["value_template"] = "{{ value_json.state }}";
  size_t rainbowSec_n = serializeJson(discovery, json_buffer);
  const char* rainbowSec_topic = concat3("homeassistant/number/", MQTT_CLIENT, "_rainbow_duration/number/config");
  MQTTclient.publish(rainbowSec_topic, json_buffer, true);
  delay(120);
  Serial.print("TX MQTT: ");
  Serial.print(rainbowSec_topic);
  Serial.print(" ");
  Serial.println(json_buffer);
  discovery.clear();

  #endif
}

void MqttReportBackOnChange() {
  if(MQTTclient.connected()) {
    #ifndef MQTT_HOME_ASSISTANT
    MqttReportPowerState();
    MqttReportStatus();
    #endif
    #ifdef MQTT_HOME_ASSISTANT_DISCOVERY
    if(!discoveryReported) {
      MqttReportDiscovery();
      discoveryReported = true;
    }
    #endif
    #ifdef MQTT_HOME_ASSISTANT
    MqttReportState(false);
    #endif
  }
}
  
void MqttPeriodicReportBack() {
  if (((millis() - lastTimeSent) > (MQTT_REPORT_STATUS_EVERY_SEC * 1000)) && MQTTclient.connected()) {
    #ifdef MQTT_HOME_ASSISTANT_DISCOVERY
    if(!discoveryReported) {
      MqttReportDiscovery();
      discoveryReported = true;
    }
    #endif
    MqttReportBackEverything(true);
  }
}
