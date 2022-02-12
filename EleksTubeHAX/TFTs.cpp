#include "TFTs.h"
#include "WiFi_WPS.h"
#include "mqtt_client_ips.h"

void TFTs::begin() {
  // Start with all displays selected.
  chip_select.begin();
  chip_select.setAll();

  // Turn power on to displays.
  pinMode(TFT_ENABLE_PIN, OUTPUT);
  enableAllDisplays();

  // Initialize the super class.
  init();

  // Set SPIFFS ready
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialization failed!");
  }
}

void TFTs::clear() {
  // Start with all displays selected.
  chip_select.setAll();
  enableAllDisplays();
}

void TFTs::showNoWifiStatus() {
  chip_select.setSecondsTens();
  setTextColor(TFT_RED, TFT_BLACK);
  fillRect(0, TFT_HEIGHT - 27, 135, 27, TFT_BLACK);
  setCursor(10, TFT_HEIGHT-29, 4);
  println("NO WIFI !");
  }

void TFTs::showNoMqttStatus() {
  chip_select.setSecondsOnes();
  setTextColor(TFT_RED, TFT_BLACK);
  fillRect(0, TFT_HEIGHT - 27, 135, 27, TFT_BLACK);
  setCursor(10, TFT_HEIGHT-29, 4);
  println("NO MQTT !");
  }


void TFTs::setDigit(uint8_t digit, uint8_t value, show_t show) {
  uint8_t old_value = digits[digit];
  digits[digit] = value;
  
  if (show != no && (old_value != value || show == force)) {
    showDigit(digit);

    if (digit == SECONDS_TENS) 
      if (WifiState != connected) { 
        showNoWifiStatus();
      }    

    if (digit == SECONDS_ONES) 
      if (!MqttConnected) { 
        showNoMqttStatus();
      }          
  }
}

/* 
 * Displays the bitmap for the value to the given digit. 
 */
 uint8_t FileInBuffer=255; // invalid, always load first image
 uint8_t NextFileRequired = 0; 
 
void TFTs::showDigit(uint8_t digit) {
  chip_select.setDigit(digit);

  if (digits[digit] == blanked) {
    fillScreen(TFT_BLACK);
  }
  else {
    uint8_t file_index = current_graphic * 10 + digits[digit];
    drawBmp(file_index, 0, 0);
    
    uint8_t NextNumber = digits[SECONDS_ONES] + 1;
    if (NextNumber > 9) NextNumber = 0; // pre-load only seconds, because they are drawn first
    NextFileRequired = current_graphic * 10 + NextNumber;
  }
}

void TFTs::LoadNextImage() {
  if (NextFileRequired != FileInBuffer) {
    LoadBmpIntoBuffer(NextFileRequired);
#ifdef DEBUG_OUTPUT
    Serial.println("Preoad");
#endif
  }
}

// These BMP functions are stolen directly from the TFT_SPIFFS_BMP example in the TFT_eSPI library.
// Unfortunately, they aren't part of the library itself, so I had to copy them.
// I've modified drawBmp to buffer the whole image at once instead of doing it line-by-line.

//// BEGIN STOLEN CODE

// Too big to fit on the stack.
uint16_t TFTs::output_buffer[TFT_HEIGHT][TFT_WIDTH];

bool TFTs::LoadBmpIntoBuffer(uint8_t file_index) {
  fs::File bmpFS;
  
  // Filenames are no bigger than "255.bmp\0"
  char filename[10];
  sprintf(filename, "/%d.bmp", file_index);
  
  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");
  if (!bmpFS)
  {
    Serial.print("File not found: ");
    Serial.println(filename);
    return(false);
  }

  uint32_t seekOffset;
  int16_t w, h, row, col;
  uint16_t  r, g, b;

  // black background - clear whole buffer
  memset(output_buffer, '\0', sizeof(output_buffer));
  
  uint16_t magic = read16(bmpFS);
  if (magic == 0xFFFF) {
    Serial.print("Can't openfile. Make sure you upload the SPIFFs image with BMPs. : ");
    Serial.println(filename);
    bmpFS.close();
    return(false);
  }
  
  if (magic != 0x4D42) {
    Serial.print("File not a BMP. Magic: ");
    Serial.println(magic);
    bmpFS.close();
    return(false);
  }

  read32(bmpFS);
  read32(bmpFS);
  seekOffset = read32(bmpFS);
  read32(bmpFS);
  w = read32(bmpFS);
  h = read32(bmpFS);
/*
  Serial.print("image W, H: ");
  Serial.print(w); 
  Serial.print(", "); 
  Serial.println(h);
*/  
  // center image on the display
  int16_t x = (TFT_WIDTH - w) / 2;
  int16_t y = (TFT_HEIGHT - h) / 2;
  
  if ((read16(bmpFS) != 1) || (read16(bmpFS) != 24) || (read32(bmpFS) != 0)) {
    Serial.println("BMP format not recognized.");
    bmpFS.close();
    return(false);
  }

  bmpFS.seek(seekOffset);

  uint16_t padding = (4 - ((w * 3) & 3)) & 3;
  uint8_t lineBuffer[w * 3 + padding];
  
  // row is decremented as the BMP image is drawn bottom up
  for (row = h-1; row >= 0; row--) {

    bmpFS.read(lineBuffer, sizeof(lineBuffer));
    uint8_t*  bptr = lineBuffer;
    
    // Convert 24 to 16 bit colours while copying to output buffer.
    for (uint16_t col = 0; col < w; col++)
    {
      b = *bptr++;
      g = *bptr++;
      r = *bptr++;
      b *= dimming;
      g *= dimming;
      r *= dimming;
      b = b >> 8;
      g = g >> 8;
      r = r >> 8;
      output_buffer[row+y][col+x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
  }
  FileInBuffer = file_index;
  
  bmpFS.close();
  return (true);
}

void TFTs::drawBmp(uint8_t file_index, int16_t xx, int16_t yy) { // ignore x and y; center on the screen

  uint32_t StartTime = millis();
  
  // check if file is already loaded into buffer; skip loading if it is. Saves 50 to 150 msec of time.
  if (file_index != FileInBuffer) {
    LoadBmpIntoBuffer(file_index);
  }
  
  uint32_t LoadConvertTime = millis();

  bool oldSwapBytes = getSwapBytes();
  setSwapBytes(true);
  pushImage(0,0, TFT_WIDTH, TFT_HEIGHT, (uint16_t *)output_buffer);
  setSwapBytes(oldSwapBytes);

  uint32_t TransferTime = millis();

#ifdef DEBUG_OUTPUT
  Serial.print("load : ");
  Serial.print(LoadConvertTime - StartTime);  
  Serial.print(", transfer: ");  
  Serial.println(TransferTime - LoadConvertTime);  
#endif
}


// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t TFTs::read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t TFTs::read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
//// END STOLEN CODE
