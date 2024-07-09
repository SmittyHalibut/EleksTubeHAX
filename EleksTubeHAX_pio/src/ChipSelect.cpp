#include "ChipSelect.h"

#ifdef HARDWARE_IPSTUBE_H401_CLOCK
// Define the pins for each LCD's enable wire
const int lcdEnablePins[NUM_DIGITS] = {GPIO_NUM_15,GPIO_NUM_2,GPIO_NUM_27,GPIO_NUM_14,GPIO_NUM_12,GPIO_NUM_13};
const int numLCDs = NUM_DIGITS;
#endif

void ChipSelect::begin() {
  #ifdef DEBUG_OUTPUT_VERBOSE
    Serial.println("ChipSelect::begin!");
  #endif
  #ifndef HARDWARE_IPSTUBE_H401_CLOCK
    pinMode(CSSR_LATCH_PIN, OUTPUT);
    pinMode(CSSR_DATA_PIN, OUTPUT);
    pinMode(CSSR_CLOCK_PIN, OUTPUT);

    digitalWrite(CSSR_DATA_PIN, LOW);
    digitalWrite(CSSR_CLOCK_PIN, LOW);
    digitalWrite(CSSR_LATCH_PIN, LOW);
    update();
  #else
    // Initialize all six different pins for the CS of each LCD as OUTPUT and set it to HIGH (disabled)
    for (int i = 0; i < numLCDs; ++i) {
      pinMode(lcdEnablePins[i], OUTPUT);
      digitalWrite(lcdEnablePins[i], HIGH);
    }
  #endif
}

void ChipSelect::clear(bool update_) {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.println("ChipSelect::clear!");
#endif
#ifndef HARDWARE_IPSTUBE_H401_CLOCK
  setDigitMap(all_off, update_);
#else
  disableAllCSPinsH401();
#endif
}

void ChipSelect::setAll(bool update_) {
#ifdef DEBUG_OUTPUT_VERBOSE 
  Serial.println("ChipSelect::setAll!");
#endif
#ifndef HARDWARE_IPSTUBE_H401_CLOCK
  setDigitMap(all_on,  update_);
#else
  enableAllCSPinsH401();
#endif
}

void ChipSelect::setDigit(uint8_t digit, bool update_) {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.print("ChipSelect::setDigit! digit: ");Serial.println(digit);
#endif
  #ifndef HARDWARE_IPSTUBE_H401_CLOCK
    // Set the bit for the given digit in the digits_map
    setDigitMap(1 << digit, update_);
    if (update_) update();
  #else
    // Set the actual currentLCD value for the given digit and activate the corresponding LCD

    // first deactivate the current LCD
    disableDigitCSPinsH401(currentLCD);
    //store the current
    currentLCD = digit;
    //activate the new one
    enableDigitCSPinsH401(digit);
    // NO UPDATE, cause update enables and disables the pin, but needs to be "enabled", while eTFT_SPI is writing into it.
  #endif
}

void ChipSelect::update() {
  #ifdef DEBUG_OUTPUT_VERBOSE
    Serial.println("ChipSelect::update!");
  #endif

  #ifndef HARDWARE_IPSTUBE_H401_CLOCK
    // Documented in README.md.  Q7 and Q6 are unused. Q5 is Seconds Ones, Q0 is Hours Tens.
    // Q7 is the first bit written, Q0 is the last.  So we push two dummy bits, then start with
    // Seconds Ones and end with Hours Tens.
    // CS is Active Low, but digits_map is 1 for enable, 0 for disable.  So we bit-wise NOT first.

    uint8_t to_shift = (~digits_map) << 2;

    digitalWrite(CSSR_LATCH_PIN, LOW);
    shiftOut(CSSR_DATA_PIN, CSSR_CLOCK_PIN, LSBFIRST, to_shift);
    digitalWrite(CSSR_LATCH_PIN, HIGH);
  #else
    #ifdef DEBUG_OUTPUT_VERBOSE
      Serial.print("currentLCD/digit: ");Serial.println(currentLCD);
      Serial.print("lcdEnablePins[currentLCD]: ");Serial.println(lcdEnablePins[currentLCD]);
    #endif
    //this is just, to follow the "update" logic of the other hardware!
    //for H401, the CS pin is already pulled to LOW by the "setDigit" function and stays there, till another "setDigit" is called.
    //so all writing done by the eTFT_SPI lib functions in the time, the pin is low, will write out directly to the LCD.
    //"Update" never will work, because, if pin was HIGH, no writing was done.
    digitalWrite(lcdEnablePins[currentLCD], LOW);
  #endif
}

bool ChipSelect::isSecondsOnes() {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.println("ChipSelect::isSecondsOnes!");
#endif
#ifndef HARDWARE_IPSTUBE_H401_CLOCK
  return (digits_map&SECONDS_ONES_MAP > 0);
#else
  return true;
#endif
}

bool ChipSelect::isSecondsTens() {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.println("ChipSelect::isSecondsTens!");
#endif
#ifndef HARDWARE_IPSTUBE_H401_CLOCK
  return (digits_map&SECONDS_TENS_MAP > 0);
#else
  return true;
#endif
}

bool ChipSelect::isMinutesOnes() {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.println("ChipSelect::isMinutesOnes!");
#endif
#ifndef HARDWARE_IPSTUBE_H401_CLOCK
  return (digits_map&MINUTES_ONES_MAP > 0);
#else
  return true;
#endif
}

bool ChipSelect::isMinutesTens() {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.println("ChipSelect::isMinutesTens!");
#endif
#ifndef HARDWARE_IPSTUBE_H401_CLOCK
  return (digits_map&MINUTES_TENS_MAP > 0);
#else
  return true;
#endif
}

bool ChipSelect::isHoursOnes() {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.println("ChipSelect::isHoursOnes!");
#endif
#ifndef HARDWARE_IPSTUBE_H401_CLOCK
  return (digits_map&HOURS_ONES_MAP > 0);
#else
  return true;
#endif
}

bool ChipSelect::isHoursTens() {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.println("ChipSelect::isHoursTens!");
#endif
#ifndef HARDWARE_IPSTUBE_H401_CLOCK
  return (digits_map&HOURS_TENS_MAP > 0);
#else
  return true;
#endif
}


void ChipSelect::enableAllCSPinsH401() {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.println("ChipSelect::enableAllCSPinsH401!");
#endif
#ifdef HARDWARE_IPSTUBE_H401_CLOCK
  // enable each LCD
  for (int i = 0; i < numLCDs; ++i) {
    digitalWrite(lcdEnablePins[i], LOW);
  }
#endif
}

void ChipSelect::disableAllCSPinsH401() {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.println("ChipSelect::disableAllCSPinsH401!");
#endif
#ifdef HARDWARE_IPSTUBE_H401_CLOCK
  // disable each LCD
  for (int i = 0; i < numLCDs; ++i) {
    digitalWrite(lcdEnablePins[i], HIGH);
  }
#endif
}

void ChipSelect::enableDigitCSPinsH401(uint8_t digit) {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.print("ChipSelect::enableDigitCSPinsH401! digit: ");Serial.println(digit);
#endif
#ifdef HARDWARE_IPSTUBE_H401_CLOCK
  // enable the LCD for the given digit
  digitalWrite(lcdEnablePins[digit], LOW);
#endif
}

void ChipSelect::disableDigitCSPinsH401(uint8_t digit) {
#ifdef DEBUG_OUTPUT_VERBOSE
  Serial.print("ChipSelect::disableDigitCSPinsH401! digit: ");Serial.println(digit);
#endif
#ifdef HARDWARE_IPSTUBE_H401_CLOCK
  // disable the LCD for the given digit
  digitalWrite(lcdEnablePins[digit], HIGH);
#endif
}

