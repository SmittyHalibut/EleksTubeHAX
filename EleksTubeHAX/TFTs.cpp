#include "TFTs.h"
#include "WiFi_WPS.h"

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
  setCursor(5, TFT_HEIGHT-29, 4);
  println("NO WIFI !");
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
  }
}

/* 
 * Where the rubber meets the road.  Displays the bitmap for the value to the given digit. 
 */
void TFTs::showDigit(uint8_t digit) {
  chip_select.setDigit(digit);

  if (digits[digit] == blanked) {
    fillScreen(TFT_BLACK);
  }
  else {
    // Filenames are no bigger than "255.bmp\0"
    char file_name[10];
    sprintf(file_name, "/%d%d.bmp", current_graphic, digits[digit]);
    drawBmp(file_name, 0, 0);
  }
}


// These BMP functions are stolen directly from the TFT_SPIFFS_BMP example in the TFT_eSPI library.
// Unfortunately, they aren't part of the library itself, so I had to copy them.
// I've modified drawBmp to buffer the whole image at once instead of doing it line-by-line.

//// BEGIN STOLEN CODE

// Too big to fit on the stack.
uint16_t TFTs::output_buffer[TFT_HEIGHT][TFT_WIDTH];

//bool TFTs::drawBmp(const char *filename) {
  bool TFTs::drawBmp(const char *filename, int16_t xx, int16_t yy) { // ignore x and y; center on the screen

  uint32_t StartTime = millis();
  fs::File bmpFS;

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

  bool oldSwapBytes = getSwapBytes();
  setSwapBytes(true);
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

  uint32_t LoadConvertTime = millis();


  pushImage(0,0, TFT_WIDTH, TFT_HEIGHT, (uint16_t *)output_buffer);
  setSwapBytes(oldSwapBytes);

  uint32_t TransferTime = millis();

  bmpFS.close();
/*  
  Serial.print("image load and transfer (ms): ");
  Serial.print(LoadConvertTime - StartTime);  
  Serial.print(", ");  
  Serial.println(TransferTime - LoadConvertTime);  
*/  
  return(true);
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
