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



WiFiClient espClient;
PubSubClient MQTTclient(espClient);

// private:
int splitTopic(char* topic, char* tokens[], int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);

void MqttProcessCommand();
void MqttReportBattery();
void MqttReportStatus();
void MqttReportPowerState();
void MqttReportWiFiSignal();
void MqttReportTemperature();
void MqttReportNotification(String message);
void MqttReportBackOnChange();
void MqttReportBackEverything();
void MqttPeriodicReportBack();



char topic[100];
char msg[5];
uint32_t lastTimeSent = -20000;
uint8_t LastNotificationChecksum = 0;
uint32_t LastTimeTriedToConnect = 0;

bool MqttConnected = true; // skip error meggase if disabled
// commands from server    // = "directive/status"
bool MqttCommandPower = true;
int  MqttCommandState = 1;  
bool MqttCommandPowerReceived = false;
bool MqttCommandStateReceived = false;

// status to server
bool MqttStatusPower = true;
int MqttStatusState = 0;
int MqttStatusBattery = 7;
int MqttStatusTemperature = 5;

int LastSentSignalLevel = 999;
int LastSentPowerState = -1;
int LastSentStatus = -1;


void sendToBroker(char* topic, char* message) {
  if (MQTTclient.connected()) {
    char topicArr[100];
    sprintf(topicArr, "%s/%s", MQTT_CLIENT, topic);
    MQTTclient.publish(topicArr, message);
#ifdef DEBUG_OUTPUT // long output
    Serial.print("Sending to MQTT: ");
    Serial.print(topicArr);
    Serial.print("/");
    Serial.println(message);
#else
    Serial.print("TX MQTT: ");
    Serial.print(topic);
    Serial.print("/");
    Serial.println(message);
#endif    
    delay (120);  
  }
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
        // delay(0x7530);
      }

    char subscibeTopic[100];
    sprintf(subscibeTopic, "%s/#", MQTT_CLIENT);
    MQTTclient.subscribe(subscibeTopic);  //Subscribes to all messages send to the device

    sendToBroker("report/online", "true");  // Reports that the device is online
    sendToBroker("report/firmware", FIRMWARE_VERSION);  // Reports the firmware version
    sendToBroker("report/ip", (char*)WiFi.localIP().toString().c_str());  // Reports the ip
    sendToBroker("report/network", (char*)WiFi.SSID().c_str());  // Reports the network name
    MqttReportWiFiSignal();
  }
#endif
}

int splitTopic(char* topic, char* tokens[], int tokensNumber) {
    const char s[2] = "/";
    int pos = 0;
    tokens[0] = strtok(topic, s);
    while (pos < tokensNumber - 1 && tokens[pos] != NULL) {
        pos++;
        tokens[pos] = strtok(NULL, s);
    }
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
    int tokensNumber = 10;
    char* tokens[tokensNumber];
    char message[length + 1];
    tokensNumber = splitTopic(topic, tokens, tokensNumber);
    sprintf(message, "%c", (char)payload[0]);
    for (int i = 1; i < length; i++) {
        sprintf(message, "%s%c", message, (char)payload[i]);
    }
#ifdef DEBUG_OUTPUT
    Serial.print("\t     Message: ");
    Serial.println(message);
#else    
    Serial.print("MQTT RX: ");
    Serial.print(tokens[1]);
    Serial.print("/");
    Serial.print(tokens[2]);
    Serial.print("/");
    Serial.println(message);
#endif    
  
    if (tokensNumber < 3) {
        // otherwise code below crashes on the strmp on non-initialized pointers in tokens[] array
        return; 
    }
  
    //------------------Decide what to do depending on the topic and message---------------------------------
    if (strcmp(tokens[1], "directive") == 0 && strcmp(tokens[2], "powerState") == 0) {  // Turn On or OFF
        if (strcmp(message, "ON") == 0) {
            MqttCommandPower = true;
            MqttCommandPowerReceived = true;
            MqttReportBackEverything();
        } else if (strcmp(message, "OFF") == 0) {
            MqttCommandPower = false;
            MqttCommandPowerReceived = true;
            MqttReportBackEverything();
        }                                                       //      SmartNest:                         // SmartThings
    } else if (strcmp(tokens[1], "directive") == 0 && (strcmp(tokens[2], "setpoint") == 0) || (strcmp(tokens[2], "percentage") == 0)) {
            double valueD = atof(message);
            if (!isnan(valueD)) {
              MqttCommandState = (int) valueD;
              MqttCommandStateReceived = true;
              MqttReportBackEverything();
            }
      }
 }

void MqttLoop(){
#ifdef MQTT_ENABLED
  MQTTclient.loop(); 
  checkMqtt();
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
  char message2[5];
  sprintf(message2, "%d", MqttStatusTemperature);
//  sendToBroker("report/value", message2);
  sendToBroker("report/temperature", message2);
}    

void MqttReportPowerState() {
  if (MqttStatusPower != LastSentPowerState) {
    if (MqttStatusPower != 0) {
      sendToBroker("report/powerState", "ON");
    } else {
      sendToBroker("report/powerState", "OFF");
    }
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

void MqttReportBackEverything() {
    MqttReportPowerState();
    MqttReportStatus();
//    MqttReportBattery();
    MqttReportWiFiSignal();
    MqttReportTemperature();
    lastTimeSent = millis();
}

void MqttReportBackOnChange() {
    MqttReportPowerState();
    MqttReportStatus();
}
  
void MqttPeriodicReportBack() {
  if (((millis() - lastTimeSent) > (MQTT_REPORT_STATUS_EVERY_SEC * 1000)) && MQTTclient.connected()) {
    MqttReportBackEverything();
    lastTimeSent = millis();
    }
}
