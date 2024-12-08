#ifndef mqtt_client_H_
#define mqtt_client_H_

#include "GLOBAL_DEFINES.h"
#include <FS.h>

#ifdef MQTT_ENABLED
extern bool MqttConnected;

// commands from server
extern bool MqttCommandPower;
extern bool MqttCommandMainPower;
extern bool MqttCommandBackPower;
extern bool MqttCommandPowerReceived;
extern bool MqttCommandMainPowerReceived;
extern bool MqttCommandBackPowerReceived;
extern int MqttCommandState;
extern bool MqttCommandStateReceived;
extern uint8_t MqttCommandBrightness;
extern uint8_t MqttCommandMainBrightness;
extern uint8_t MqttCommandBackBrightness;
extern bool MqttCommandBrightnessReceived;
extern bool MqttCommandMainBrightnessReceived;
extern bool MqttCommandBackBrightnessReceived;
extern char MqttCommandPattern[];
extern char MqttCommandBackPattern[];
extern bool MqttCommandPatternReceived;
extern bool MqttCommandBackPatternReceived;
extern uint16_t MqttCommandBackColorPhase;
extern bool MqttCommandBackColorPhaseReceived;
extern uint8_t MqttCommandGraphic;
extern uint8_t MqttCommandMainGraphic;
extern bool MqttCommandGraphicReceived;
extern bool MqttCommandMainGraphicReceived;
extern bool MqttCommandUseTwelveHours;
extern bool MqttCommandUseTwelveHoursReceived;
extern bool MqttCommandBlankZeroHours;
extern bool MqttCommandBlankZeroHoursReceived;
extern uint8_t MqttCommandPulseBpm;
extern bool MqttCommandPulseBpmReceived;
extern uint8_t MqttCommandBreathBpm;
extern bool MqttCommandBreathBpmReceived;
extern float MqttCommandRainbowSec;
extern bool MqttCommandRainbowSecReceived;

// status to server
extern bool MqttStatusPower;
extern bool MqttStatusMainPower;
extern bool MqttStatusBackPower;
extern int MqttStatusState;
extern int MqttStatusBattery;
extern uint8_t MqttStatusBrightness;
extern uint8_t MqttStatusMainBrightness;
extern uint8_t MqttStatusBackBrightness;
extern char MqttStatusPattern[];
extern char MqttStatusBackPattern[];
extern uint16_t MqttStatusBackColorPhase;
extern uint8_t MqttStatusGraphic;
extern uint8_t MqttStatusMainGraphic;
extern bool MqttStatusUseTwelveHours;
extern bool MqttStatusBlankZeroHours;
extern uint8_t MqttStatusPulseBpm;
extern uint8_t MqttStatusBreathBpm;
extern float MqttStatusRainbowSec;

// functions
void MqttStart();
void MqttLoopFrequently();
void MqttLoopInFreeTime();
void MqttReportBackEverything(bool force);

#endif // MQTT_ENABLED

#endif /* mqtt_client_H_ */
