#ifndef TEMPSENSOR_INC_H_
#define TEMPSENSOR_INC_H_

// #include "GLOBAL_DEFINES.h"

float fTemperature = -127;
char sTemperatureTxt[10];
bool bTemperatureUpdated = false;
uint32_t lastTimeRead = (uint32_t)(TEMPERATURE_READ_EVERY_SEC * -1000);

#ifdef ONE_WIRE_BUS_PIN
// For some reason the following two library includes fail on compilation if placed in a .c file outside of the main .ino file.
// Therefore we had to make .h file out of this one, that is included directly into the .ino file.
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

#ifdef ONE_WIRE_BUS_PIN

// Setup a oneWire instance to communicate with OneWire devices (DS18B20)
OneWire oneWire(ONE_WIRE_BUS_PIN);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

void GetTemperatureDS18B20()
{
  Serial.print("Reading temperature... ");
  sensors.requestTemperatures();             // all sensors on the bus
  fTemperature = sensors.getTempCByIndex(0); // only one sensor (search ROM and read temp)

  // sprintf(sTemperatureTxt, "%d", (int)fTemperature);

  // char* tempToAscii(float temp) // convert long to type char and store in variable array ascii
  // https://forum.arduino.cc/t/printing-a-double-variable/44327/17
  // http://www.nongnu.org/avr-libc/user-manual/group__avr__stdlib.html#gaa571de9e773dde59b0550a5ca4bd2f00
  int frac;
  int rnd;

  rnd = (int)(fTemperature * 1000) % 10;
  frac = (int)(fTemperature * 100) % 100; // get three numbers to the right of the deciaml point.
  if (rnd >= 5)
    frac = frac + 1;
  itoa((int)fTemperature, sTemperatureTxt, 10);
  strcat(sTemperatureTxt, ".");

  if (frac < 10)
  {
    itoa(0, &sTemperatureTxt[strlen(sTemperatureTxt)], 10);
  } // if fract < 10 should print .0 fract ie if fract=6 print .06

  itoa(frac, &sTemperatureTxt[strlen(sTemperatureTxt)], 10); // put the frac after the deciaml

  //    sprintf(sTemperatureTxt, "%d.%d", intPart, fracPart);

  bTemperatureUpdated = true;

  // Serial.print(fTemperature); // print float directly, for testing purposes only
  // Serial.print("  /  ");
  Serial.print(sTemperatureTxt);
  Serial.println(" C");
}
#endif // sensor defined

void PeriodicReadTemperature()
{
#ifdef ONE_WIRE_BUS_PIN
  if ((millis() - lastTimeRead) > (TEMPERATURE_READ_EVERY_SEC * 1000))
  {
    GetTemperatureDS18B20();
    lastTimeRead = millis();
  }
#endif
}

#endif // TEMPSENSOR_INC_H_
