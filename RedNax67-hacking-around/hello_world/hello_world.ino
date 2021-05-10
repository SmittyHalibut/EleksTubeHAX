#include <TFT_eSPI.h>
/*
   If your screens are blank after loading this code, make sure you
   modify Arduino/libraries/TFT_eSPI/User_Setup_Select.h
   so that it #include </path/to/your/code/hello_world/User_Setup.h>
   instead of including <User_Setup.h> or any of the other setup files.

   Yes, this means we have to modify the distributed library. I'm not
   happy about it either, but that's how the library is written. It's
   documentaiton tells you how to do this.
*/

TFT_eSPI tft = TFT_eSPI();

//RTC
#include <Wire.h>
#include "RTClib.h"
RTC_DS3231 rtc;


//Wifi for NTP
#include <WiFi.h>
#include "time.h"

const char* ssid       = "xxxx";
const char* password   = "xxxx";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

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


/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true

String fileName;
int Digit;
unsigned long  epochTime = 0;
unsigned long  oldEpochTime = -1;
char oldDigit[6];

void printLocalTime()
{
  struct tm timeinfo;
  time_t now;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    //return;
  }
  epochTime = time(&now);
  Serial.println(epochTime);
  Serial.println(oldEpochTime);
  
  if (( epochTime - oldEpochTime )  > 60 ) {
    Serial.println("Getting time from NTP");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    oldEpochTime = epochTime;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  char timeStringBuff[7]; 
  strftime(timeStringBuff, sizeof(timeStringBuff), "%H%M%S", &timeinfo);
  Serial.println(timeStringBuff);

  byte Digit = 0x01;

  for (int i = 0; i < 6; i++) {
    if ( timeStringBuff[i] != oldDigit[i]) {
      fileName = String("/") + timeStringBuff[i] + String(".bmp");
      Serial.println(fileName);
      Serial.println(Digit, HEX);
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, MSBFIRST, Digit ^ 0xff);
      digitalWrite(latchPin, HIGH);
      oldDigit[i] = timeStringBuff[i];
      drawBmp(fileName.c_str(), 0, 0);
    }   
    Digit = Digit << 1;
  }


  //Serial.println(fileName);


}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(200);
  Serial.println("In setup()");

  for (int i=0; i<6; i++) { oldDigit[i] = 0x00 ; }

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date &amp; time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date &amp; time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

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
  shiftOut(dataPin, clockPin, MSBFIRST, 0xFF ^ B00111111);  // 0 = On, Right most bit is left most display.. here we light every other display
  digitalWrite(latchPin, HIGH);

  // Enabled ground on the TFTs
  pinMode(tftGndEnable, OUTPUT);
  digitalWrite(tftGndEnable, HIGH); // Drives a MOSFET to GND, so HIGH here means enable GND.

  tft.init();
  tft.fillScreen(TFT_BLUE);
  // Set "cursor" at top left corner of display (0,0) and select font 4
  tft.setCursor(0, 0, 4);
  // Set the font colour to be white with a black background
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  if (!SPIFFS.begin()) {
    tft.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  tft.println("\r\nSPIFFS initialised.");

  //connect to WiFi
  tft.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
  }
  tft.println(" CONNECTED");

  //init and get the time
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // We can now plot text on screen using the "print" class
  tft.println("Intialised default\n");
  //tft.println("White text");

  tft.setTextColor(TFT_RED, TFT_BLACK);
  //tft.println("Red text");

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  //tft.println("Green text");

  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  //tft.println("Blue text");

  delay(5000);
  printLocalTime();

  tft.println("Done with setup()");

  Digit = 0;
  fileName = String();
}

void loop() {
  //Serial.println("Hello world!");
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

  printLocalTime();


}
