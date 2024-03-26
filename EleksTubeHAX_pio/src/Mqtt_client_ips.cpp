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
void MqttReportGraphic();
void MqttReportBackOnChange();
void MqttReportBackEverything();
void MqttPeriodicReportBack();

char topic[100];
char msg[5];
uint32_t lastTimeSent = (uint32_t)(MQTT_REPORT_STATUS_EVERY_SEC * -1000);
uint8_t LastNotificationChecksum = 0;
uint32_t LastTimeTriedToConnect = 0;

bool MqttConnected = true; // skip error meggase if disabled
// commands from server    // = "directive/status"
bool MqttCommandPower = true;
bool MqttCommandPowerReceived = false;

int  MqttCommandState = 1;  
bool MqttCommandStateReceived = false;

uint8_t MqttCommandBrightness = -1;  
bool MqttCommandBrightnessReceived = false;

char MqttCommandPattern[24] = "";
bool MqttCommandPatternReceived = false;

uint8_t MqttCommandGraphic = -1;  
bool MqttCommandGraphicReceived = false;

// status to server
bool MqttStatusPower = true;
int MqttStatusState = 0;
int MqttStatusBattery = 7;
uint8_t MqttStatusBrightness = 0;
char MqttStatusPattern[24] = "";
uint8_t MqttStatusGraphic = 0;

int LastSentSignalLevel = 999;
int LastSentPowerState = -1;
int LastSentStatus = -1;
int LastSentBrightness = -1;
int LastSentGraphic = -1;

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

void MqttReportState() {
#ifdef MQTT_HOME_ASSISTANT
  if(!MQTTclient.connected()) {
    return;
  }
  JsonDocument state;
  state["state"] =  MqttStatusPower == 0 ? MQTT_STATE_OFF : MQTT_STATE_ON;
  state["brightness"] = map(MqttStatusBrightness, MQTT_ITENSITY_MIN, MQTT_ITENSITY_MAX, MQTT_BRIGHTNESS_MIN, MQTT_BRIGHTNESS_MAX);
  state["effect"] = MqttStatusPattern;

  char buffer[256];
  size_t n = serializeJson(state, buffer);
  MQTTclient.publish(MQTT_CLIENT, buffer, true);

  Serial.print("TX MQTT: ");
  Serial.print(MQTT_CLIENT);
  Serial.print(" ");
  Serial.println(buffer);
#endif
}

void MqttStart() {
#ifdef MQTT_ENABLED
  MqttConnected = false;
  if (((millis() - LastTimeTriedToConnect) > (MQTT_RECONNECT_WAIT_SEC * 1000)) || (LastTimeTriedToConnect == 0)) {
    LastTimeTriedToConnect = millis();
    MQTTclient.setServer(MQTT_BROKER, MQTT_PORT);
    MQTTclient.setCallback(callback);

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
    sprintf(subscibeTopic, "%s/set", MQTT_CLIENT);
    MQTTclient.subscribe(subscibeTopic);

    sprintf(subscibeTopic, "%s/graphic/set", MQTT_CLIENT);
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

  if (commandNumber < 3) {
    // otherwise code below crashes on the strmp on non-initialized pointers in command[] array
    Serial.println("Number of command in MQTT message < 3!");
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
  if (strcmp(command[0], "set") == 0){
    JsonDocument doc;
    deserializeJson(doc, payload, length);
    
    if(doc.containsKey("state")) {
      MqttCommandPower = doc["state"] == MQTT_STATE_ON;
      MqttCommandPowerReceived = true;
    }
    if(doc.containsKey("brightness")) {
      MqttCommandBrightness = map(doc["brightness"], MQTT_BRIGHTNESS_MIN, MQTT_BRIGHTNESS_MAX, MQTT_ITENSITY_MIN, MQTT_ITENSITY_MAX);
      MqttCommandBrightnessReceived = true;
    }
    if(doc.containsKey("effect")) {
      strcpy(MqttCommandPattern, doc["effect"]);
      MqttCommandPatternReceived = true;
    }

    doc.clear();
  } else if (strcmp(command[0], "graphic") == 0 && strcmp(command[1], "set") == 0){
    char graphic = (char) payload[0];
    MqttCommandGraphic = graphic - '0';
    MqttCommandGraphicReceived = true;
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

void MqttReportGraphic() {
  if (MqttStatusGraphic != LastSentGraphic) {
    char graphic[2] = "";
    sprintf(graphic, "%i", MqttStatusGraphic);
    sendToBroker("graphic", graphic);  // Reports the signal strength

    LastSentGraphic = MqttStatusGraphic;
  }
}


void MqttReportBackEverything() {
  if(!MQTTclient.connected()) {
    return;
  }

  #ifndef MQTT_HOME_ASSISTANT
  MqttReportPowerState();
  MqttReportStatus();
//    MqttReportBattery();
  MqttReportWiFiSignal();
  MqttReportTemperature();
  #endif

  #ifdef MQTT_HOME_ASSISTANT
  MqttReportState();
  MqttReportGraphic();
  #endif
  
  lastTimeSent = millis();
}

void MqttReportBackOnChange() {
  if(!MQTTclient.connected()) {
    return;
  }

  #ifndef MQTT_HOME_ASSISTANT
  MqttReportPowerState();
  MqttReportStatus();
  #endif
}
  
void MqttPeriodicReportBack() {
  if (((millis() - lastTimeSent) > (MQTT_REPORT_STATUS_EVERY_SEC * 1000)) && MQTTclient.connected()) {
    MqttReportBackEverything();
    }
}
