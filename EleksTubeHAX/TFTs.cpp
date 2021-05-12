#include "TFTs.h"

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

void TFTs::setDigit(uint8_t digit, uint8_t value, show_t show) {
  uint8_t old_value = digits[digit];
  digits[digit] = value;
  
  if (show != no && (old_value != value || show == force)) {
    showDigit(digit);
  }
}

/* 
 * Where the rubber meets the road.  Displays the bitmap for the value to the given digit. 
 */
void TFTs::showDigit(uint8_t digit, bool debug) {
  chip_select.setDigit(digit);

  // Filenames are no bigger than "255.bmp\0"
  char file_name[10];
  sprintf(file_name, "/%d.bmp", digits[digit]);
  drawBmp(file_name, 0, 0);
  
  /*
  // Dummy code, to just write text to the given display, for testing.
  fillScreen(TFT_BLUE);
  setCursor(40, 80, 8);
  setTextColor(TFT_WHITE, TFT_BLUE);
  //print("Digit: ");
  //println(digit);
  //print("Value: ");
  println(digits[digit]);
  */

  // Debugging code, just overlays the BMP
  if (debug) {
    setCursor(0, 0, 2);
    setTextColor(TFT_WHITE, TFT_BLACK);
    println(file_name);
  }
}


// These BMP functions are stolen directly from the TFT_SPIFFS_BMP example in the TFT_eSPI library.
// Unfortunately, they aren't part of the library itself, so I had to copy them.

//// BEGIN STOLEN CODE
bool TFTs::drawBmp(const char *filename, int16_t x, int16_t y) {

  // Nothing to do.
  if ((x >= width()) || (y >= height())) return(true);

  fs::File bmpFS;

  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");

  if (!bmpFS)
  {
    Serial.println("File not found");
    return(false);
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;

  uint32_t startTime = millis();

  uint16_t magic = read16(bmpFS);
  if (magic == 0xFFFF) {
    Serial.println("File not found. Make sure you upload the SPIFFs image with BMPs.");
    return(false);
  }
  
  if (magic == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      bool oldSwapBytes = getSwapBytes();
      setSwapBytes(true);
      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++) {

        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }

        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      setSwapBytes(oldSwapBytes);
      //Serial.print("Loaded in ");
      //Serial.print(millis() - startTime);
      //Serial.println(" ms");
    }
    else {
      Serial.println("BMP format not recognized.");
      bmpFS.close();
      return(false);
    }
  }
  else {
    Serial.print("File not a BMP. Magic: ");
    Serial.println(magic);
    bmpFS.close();
    return(false);
  }
  bmpFS.close();
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
