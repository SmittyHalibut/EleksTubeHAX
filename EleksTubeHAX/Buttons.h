#ifndef BUTTONS_H
#define BUTTONS_H
/*
 * A simple helper class to call common functions on all buttons at once.
 */

#include "Button.h"
#include "Hardware.h"

class Buttons {
public:
  Buttons() : left(BUTTON_LEFT_PIN), mode(BUTTON_MODE_PIN), right(BUTTON_RIGHT_PIN), power(BUTTON_POWER_PIN) {}

  void begin()
    { left.begin(); mode.begin(); right.begin(); power.begin(); }
  void loop()
    { left.loop(); mode.loop(); right.loop(); power.loop(); }
  bool stateChanged() 
    { return left.stateChanged() || mode.stateChanged() || right.stateChanged() || power.stateChanged(); }
    
  // Just making them public, so we don't have to proxy everything.
  Button left, mode, right, power;
private: 
};

#endif // BUTTONS_H
