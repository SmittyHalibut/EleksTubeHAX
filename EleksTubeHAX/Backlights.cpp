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
void Backlights::rainbowPattern() {}
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

const String Backlights::patterns_str[Backlights::num_patterns] = 
  { "dark", "test", "constant", "rainbow", "pulse", "breath" };
