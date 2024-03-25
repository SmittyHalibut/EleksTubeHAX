#ifndef mqtt_client_H_
#define mqtt_client_H_

#include "GLOBAL_DEFINES.h"

extern bool MqttConnected;

// commands from server
extern bool MqttCommandPower;
extern bool MqttCommandPowerReceived;
extern int  MqttCommandState;
extern bool MqttCommandStateReceived;
extern uint8_t MqttCommandBrightness;
extern bool MqttCommandBrightnessReceived;
extern char MqttCommandPattern[];
extern bool MqttCommandPatternReceived;

// status to server
extern bool MqttStatusPower;
extern int MqttStatusState;
extern int MqttStatusBattery;
extern int MqttStatusBrightness;
extern char MqttStatusPattern[];

// functions
void MqttStart();
void MqttLoopFrequently();
void MqttLoopInFreeTime();
void MqttReportBackEverything();


#endif /* mqtt_client_H_ */
