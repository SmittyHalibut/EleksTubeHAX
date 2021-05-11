#include "Backlights.h"

void Backlights::setNextPattern(int8_t i) {
  int8_t next_pattern = (pattern + i) % num_patterns;
  // https://stackoverflow.com/questions/11720656/modulo-operation-with-negative-numbers
  while (next_pattern < 0) {
    next_pattern += num_patterns;
  }
  setPattern(patterns(next_pattern));
}

void Backlights::loop() {
  //   enum patterns { dark, test, constant, rainbow, pulse, breath, num_patterns };
  if (pattern == dark) {
    if (pattern_needs_init) {
      clear();
      show();
    }
  }
  else if (pattern == test) {
    testPattern();
  }
  else if (pattern == constant) {
    if (pattern_needs_init) {
      fill(pattern_color);
      show();
    }
  }
  else if (pattern == rainbow) {
    rainbowPattern();
  }
  else if (pattern == pulse) {
    pulsePattern();
  }
  else if (pattern == breath) {
    breathPattern();
  }

  pattern_needs_init = false;
}

// TODO Implement these
void Backlights::pulsePattern() {}
void Backlights::breathPattern() {}

void Backlights::testPattern() {
  const uint8_t num_colors = 4;  // or 3 if you don't want black
  uint8_t num_states = NUM_DIGITS * num_colors;
  uint8_t state = (millis()/test_ms_delay) % num_states;

  uint8_t digit = state/num_colors;
  uint32_t color = 0xFF0000 >> (state%num_colors)*8;
  
  clear();
  setPixelColor(digit, color);
  show();

}

uint8_t Backlights::phaseToColor(uint16_t phase) {
  uint16_t color = 0;
  if (phase <= 255) {
    // Ramping up
    color = phase;
  }
  else if (phase <= 511) {
    // Ramping down
    color = 511-phase;
  }
  else {
    // Off
    color = 0;
  }
  if (color > 255) {
    // TODO: Trigger ERROR STATE, bug in code.
  }
  return uint8_t(color % 256);
}

void Backlights::rainbowPattern() {
  const uint16_t max_phase = 768;   // 256 up, 256 down, 256 off
  // TODO Make this /3 a parameter
  const uint16_t phase_per_digit = (max_phase/NUM_DIGITS)/3;

  // TODO Make this /10 a parameter
  uint16_t phase = millis()/10 % max_phase;  
  
  for (uint8_t digit=0; digit < NUM_DIGITS; digit++) {
    // Shift the phase for this LED.
    uint16_t my_phase = (phase + digit*phase_per_digit) % max_phase;
    uint8_t red = phaseToColor(my_phase);
    uint8_t green = phaseToColor((my_phase + 256)%max_phase);
    uint8_t blue = phaseToColor((my_phase + 512)%max_phase);
    setPixelColor(digit, red, green, blue);
  }
  show();
}

const String Backlights::patterns_str[Backlights::num_patterns] = 
  { "dark", "test", "constant", "rainbow", "pulse", "breath" };
