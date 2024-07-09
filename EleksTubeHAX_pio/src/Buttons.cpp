#include "Buttons.h"

void Button::begin() {
  millis_at_last_transition = millis();
  millis_at_last_loop = millis_at_last_transition;

#ifdef DEBUG_OUTPUT
    Serial.print("init button: ");Serial.println(bpin);
#endif
  pinMode(bpin, INPUT);  
  down_last_time = isButtonDown();
  if (down_last_time) {
    button_state = down_edge;
  } else {
    button_state = idle;
  }
}

void Button::loop() {
  millis_at_last_loop = millis();
  bool down_now = isButtonDown();
  #ifdef DEBUG_OUTPUT
  if (down_now) {
    Serial.print("[B ");Serial.print(bpin);Serial.println("]");
  }
  #endif

  state previous_state = button_state;
  
  if (down_last_time == false && down_now == false) {
    // Idle
    button_state = idle;
  } 
  else if (down_last_time == false && down_now == true) {
    // Just pressed
    #ifdef DEBUG_OUTPUT_BUTTONS
      Serial.println("BUTTON: Just Pressed!");
    #endif
    button_state = down_edge;
    millis_at_last_transition = millis_at_last_loop;
  } 
  else if (down_last_time == true && down_now == true) {
    // Been pressed. For how long?
    #ifdef DEBUG_OUTPUT_BUTTONS 
      Serial.println("BUTTON: Been pressed. For how long?");
    #endif
    if (millis_at_last_loop - millis_at_last_transition >= long_press_ms) {
      // Long pressed. Did we just transition?
      #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: Long pressed. Did we just transition?");
      #endif
      if (previous_state == down_long_edge || previous_state == down_long) {
        // No, we already detected the edge.
        #ifdef DEBUG_OUTPUT_BUTTONS
          Serial.println("BUTTON: No, we already detected the edge.");
        #endif
        button_state = down_long;
      }
      else {
        // Previous state was something else, so this is the transition.
        // down -> down_long_edge does NOT update millis_at_last_transition.
        // We'd rather know how long it's been down than been down_long.
        #ifdef DEBUG_OUTPUT_BUTTONS
          Serial.println("BUTTON: else something! set button_stage to down_long_edge.");
        #endif
        button_state = down_long_edge;
      }
    }
    else {
      // Not yet long pressed
      #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: Not yet long pressed! set button_stage to down.");
      #endif
      button_state = down;
    }
  }
  else if (down_last_time == true && down_now == false) {
    // Just released.  From how long?
    #ifdef DEBUG_OUTPUT_BUTTONS
      Serial.println("BUTTON: Just released.  From how long?");
    #endif
    if (previous_state == down_long_edge || previous_state == down_long) {
      #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: Just released from a long press.");
      #endif
      // Just released from a long press.
      button_state = up_long_edge;
    }
    else {
      // Just released from a short press.
      #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: Just released from a short press.");
      #endif
      button_state = up_edge;
    }
    millis_at_last_transition = millis_at_last_loop;
  }

  state_changed = previous_state != button_state;
  down_last_time = down_now;
}

const String Button::state_str[Button::num_states] = 
  { "idle", 
    "down_edge", 
    "down", 
    "down_long_edge", 
    "down_long", 
    "up_edge", 
    "up_long_edge"
  };

//--------------------------------------------

#ifdef ONE_BUTTON_ONLY_MENU
//One Button in Buttons only
void Buttons::begin() {
  mode.begin();
}

void Buttons::loop() {
  mode.loop();
}

bool Buttons::stateChanged() {
  return mode.stateChanged();
}

#endif

#ifndef ONE_BUTTON_ONLY_MENU
//Buttons
void Buttons::begin() { 
  left.begin();
  mode.begin();
  right.begin();
  power.begin();
}

void Buttons::loop() {
  left.loop();
  mode.loop();
  right.loop();
  power.loop();
}

bool Buttons::stateChanged() {
  return 
    left.stateChanged() ||
    mode.stateChanged() ||
    right.stateChanged() ||
    power.stateChanged();
}
#endif