#include "Menu.h"

// Big ol' state machine: menu and buttons as the state, buttons as the transition triggers.
void Menu::loop(Buttons &buttons)
{
  Button::state left_state = buttons.left.getState();   // decrement value
  Button::state mode_state = buttons.mode.getState();   // next menu
  Button::state right_state = buttons.right.getState(); // increment value
  Button::state power_state = buttons.power.getState(); // exit menu

  // Reset the change value in every case.  We don't always change the state though.
  change = 0;
  state_changed = false;

  // Early out for idle state, which will be most of the time.
  if (state == idle && left_state == Button::idle && right_state == Button::idle && mode_state == Button::idle)
  {
    // Everything is idle.
    return;
  }

  // Go idle if the user hasn't pressed a button in a long time.
  if (state != idle && millis() - millis_last_button_press > idle_timeout_ms)
  {
    // Go idle.
    state = idle;
    state_changed = true;
    return;
  }

  // Menu is idle. A button is pressed, go into the menu, but don't act on the button press. It just wakes up the menu.
  if (state == idle && (left_state == Button::down_edge || right_state == Button::down_edge || mode_state == Button::down_edge))
  {
    state = states(1); // Start at the beginning of the menu.

    millis_last_button_press = millis();
    state_changed = true;
    return;
  }

  // Go to the next menu option
  if (state != idle && mode_state == Button::down_edge)
  {
    uint8_t new_state = (uint8_t(state) + 1) % num_states;
    if (new_state == 0)
    {
      new_state = 1; // Skip over idle when incrementing through the menu.
    }
    state = states(new_state);

    millis_last_button_press = millis();
    state_changed = true;
    return;
  }

  // Exit with a power button.
  if (state != idle && (power_state == Button::down_edge))
  {
    state = idle;
    state_changed = true;
    return;
  }

  // In a menu, and a left or right button has been pressed
  if (state != idle && (left_state == Button::down_edge || right_state == Button::down_edge))
  {
    // Pressing both left and right at the same time cancels out?  Sure, why not...
    if (left_state == Button::down_edge)
    {
      change--;
    }
    if (right_state == Button::down_edge)
    {
      change++;
    }

    millis_last_button_press = millis();
    state_changed = true;
    return;
  }
  // Some other button state, but it doesn't trigger any change in state.  There are LOTS of states that will
  // get here, but I think they're all "just do nothing."  If there's an explicit state we want to handle,
  // add an if() block above.
}
