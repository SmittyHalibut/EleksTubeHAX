#ifndef TEMPSENSOR_H_
#define TEMPSENSOR_H_

extern float fTemperature;
extern char sTemperatureTxt[10];
extern bool bTemperatureUpdated;

void PeriodicReadTemperature(void);

#endif // TEMPSENSOR_H_
