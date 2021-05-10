#include <TFT_eSPI.h>
/*
 * If your screens are blank after loading this code, make sure you 
 * modify Arduino/libraries/TFT_eSPI/User_Setup_Select.h
 * so that it #include </path/to/your/code/hello_world/User_Setup.h>
 * instead of including <User_Setup.h> or any of the other setup files.
 * 
 * Yes, this means we have to modify the distributed library. I'm not
 * happy about it either, but that's how the library is written. It's
 * documentaiton tells you how to do this.
 */

TFT_eSPI tft = TFT_eSPI();

// Buttons
const uint8_t buttonLeft = 33;
const uint8_t buttonMode = 32;
const uint8_t buttonRight = 35;
const uint8_t buttonPower = 34;

// Chip Select shift register
const uint8_t latchPin = 17;
const uint8_t clockPin = 16;
const uint8_t dataPin = 14;

// TFT not handled by TFT_eSPI
const uint8_t tftGndEnable = 27;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(200);
  Serial.println("In setup()");
#ifdef ST7789_DRIVER
  Serial.println("Using ST7789 driver, good.");
#else
  Serial.println("NOT USING ST7789 DRIVER! User_Setup.h didn't work!");
#endif


  // Setup buttons
  pinMode(buttonLeft, INPUT);
  pinMode(buttonMode, INPUT);
  pinMode(buttonRight, INPUT);
  pinMode(buttonPower, INPUT);
  
  // Setup 74HC595 chip select. For now, just enable all of them.
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  digitalWrite(latchPin, LOW);
  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, 0xFF ^ B0010101);  // 0 = On, Right most bit is left most display.. here we light every other display 
  digitalWrite(latchPin, HIGH);

  // Enabled ground on the TFTs
  pinMode(tftGndEnable, OUTPUT);
  digitalWrite(tftGndEnable, HIGH); // Drives a MOSFET to GND, so HIGH here means enable GND.

  tft.init();
  tft.fillScreen(TFT_BLUE);
  delay(1000);
  // Set "cursor" at top left corner of display (0,0) and select font 4
  tft.setCursor(0, 0, 4);
  // Set the font colour to be white with a black background
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // We can now plot text on screen using the "print" class
  tft.println("Intialised default\n");
  tft.println("White text");
  
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.println("Red text");
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("Green text");
  
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.println("Blue text");
  
  Serial.println("Done with setup()");
}

void loop() {
  Serial.println("Hello world!");
  if (digitalRead(buttonMode) == LOW) {
    Serial.println("Mode Button.");
  }
  if (digitalRead(buttonLeft) == LOW) {
    Serial.println("Left Button.");
  }
  if (digitalRead(buttonRight) == LOW) {
    Serial.println("Right Button.");
  }
  if (digitalRead(buttonPower) == LOW) {
    Serial.println("Power Button.");
  }

  delay(1000);
}
