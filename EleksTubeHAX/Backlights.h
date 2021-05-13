#ifndef BACKLIGHTS_H
#define BACKLIGHTS_H

/*
 * A simple sub-class of the Adafruit_NeoPixel class, to configure it for
 * the EleksTube-IPS clock, and to add a little functionality.
 * 
 * The good news is, the pixel numbers in the string line up perfectly 
 * with the #defines in Hardware.h, so you can pass SECONDS_ONES directly
 * as the pixel index, no mapping required.
 * 
 * Otherwise, class Backlights behaves exactly as Adafruit_NeoPixel does.
 */
#include <stdint.h>
#include "Hardware.h"
#include <Adafruit_NeoPixel.h>

class Backlights: public Adafruit_NeoPixel {
public:
  Backlights() : Adafruit_NeoPixel(NUM_DIGITS, BACKLIGHTS_PIN, NEO_GRB + NEO_KHZ800), 
    pattern(dark), pattern_needs_init(true),
    // Sane default for the patterns. These can be over-written with setters.
    test_ms_delay(200), pattern_color(0x913500), pulse_bpm(72), breath_per_min(10)
    {};

  enum patterns { dark, test, constant, rainbow, pulse, breath, num_patterns };
  const static String patterns_str[num_patterns];

  void setPattern(patterns p) { pattern = p; pattern_needs_init = true; }
  patterns getPattern()       { return pattern; }
  String getPatternStr()      { return patterns_str[pattern]; }
  void setNextPattern(int8_t i=1);
  void setPrevPattern()       { setNextPattern(-1); }

  void loop();

  // Configure the patterns
  void setTestPatternDelay(uint32_t ms_delay) { test_ms_delay = ms_delay; }
  void setPulseRate(uint8_t bpm)              { pulse_bpm = bpm; }
  void setBreathRate(uint8_t per_min)         { breath_per_min = per_min; }
  // Used by all constant color patterns.
  void setPatternColor(uint32_t color)        { pattern_color = color; }
  uint32_t getPatternColor()                  { return pattern_color; }
  
private:
  patterns pattern;
  bool pattern_needs_init;

  // Pattern configs
  uint32_t pattern_color;
  uint32_t test_ms_delay;
  uint8_t pulse_bpm;
  uint8_t breath_per_min;

  // Pattern methods
  void testPattern();
  void rainbowPattern();
  void pulsePattern();
  void breathPattern();

  // Helper methods
  uint8_t phaseToColor(uint16_t phase);
};

#endif // BACKLIGHTS_H
