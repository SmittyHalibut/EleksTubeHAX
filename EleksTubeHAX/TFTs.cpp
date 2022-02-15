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
  chip_select.setSecondsOnes();
  setTextColor(TFT_RED, TFT_BLACK);
  fillRect(0, TFT_HEIGHT - 27, 135, 27, TFT_BLACK);
  setCursor(10, TFT_HEIGHT-29, 4);
  println("NO WIFI !");
  }

void TFTs::showNoMqttStatus() {
  chip_select.setSecondsTens();
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

    if (digit == SECONDS_ONES) 
      if (WifiState != connected) { 
        showNoWifiStatus();
      }    

    if (digit == SECONDS_TENS) 
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
    DrawImage(file_index);
    
    uint8_t NextNumber = digits[SECONDS_ONES] + 1;
    if (NextNumber > 9) NextNumber = 0; // pre-load only seconds, because they are drawn first
    NextFileRequired = current_graphic * 10 + NextNumber;
  }
}

void TFTs::LoadNextImage() {
  if (NextFileRequired != FileInBuffer) {
#ifdef DEBUG_OUTPUT
    Serial.println("Preload img");
#endif
    LoadImageIntoBuffer(NextFileRequired);
  }
}

// These BMP functions are stolen directly from the TFT_SPIFFS_BMP example in the TFT_eSPI library.
// Unfortunately, they aren't part of the library itself, so I had to copy them.
// I've modified DrawImage to buffer the whole image at once instead of doing it line-by-line.


// Too big to fit on the stack.
uint16_t TFTs::output_buffer[TFT_HEIGHT][TFT_WIDTH];

#ifndef USE_CLK_FILES
bool TFTs::LoadImageIntoBuffer(uint8_t file_index) {
  uint32_t StartTime = millis();

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
#ifdef DEBUG_OUTPUT
  Serial.print("image W, H: ");
  Serial.print(w); 
  Serial.print(", "); 
  Serial.println(h);
  Serial.print("dimming: ");
  Serial.println(dimming);
#endif
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
    for (col = 0; col < w; col++)
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
#ifdef DEBUG_OUTPUT
  Serial.print("img load : ");
  Serial.println(millis() - StartTime);  
#endif
  return (true);
}
#endif


#ifdef USE_CLK_FILES
bool TFTs::LoadImageIntoBuffer(uint8_t file_index) {
  uint32_t StartTime = millis();

  fs::File bmpFS;
  // Filenames are no bigger than "255.clk\0"
  char filename[10];
  sprintf(filename, "/%d.clk", file_index);
  
  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");
  if (!bmpFS)
  {
    Serial.print("File not found: ");
    Serial.println(filename);
    return(false);
  }

  int16_t w, h, row, col;
  uint16_t  r, g, b;

  // black background - clear whole buffer
  memset(output_buffer, '\0', sizeof(output_buffer));
  
  uint16_t magic = read16(bmpFS);
  if (magic == 0xFFFF) {
    Serial.print("Can't openfile. Make sure you upload the SPIFFs image with images. : ");
    Serial.println(filename);
    bmpFS.close();
    return(false);
  }
  
  if (magic != 0x4B43) { // look for "CK" header
    Serial.print("File not a CLK. Magic: ");
    Serial.println(magic);
    bmpFS.close();
    return(false);
  }

  w = read16(bmpFS);
  h = read16(bmpFS);
#ifdef DEBUG_OUTPUT
  Serial.print("image W, H: ");
  Serial.print(w); 
  Serial.print(", "); 
  Serial.println(h);
  Serial.print("dimming: ");
  Serial.println(dimming);
#endif  
  // center image on the display
  int16_t x = (TFT_WIDTH - w) / 2;
  int16_t y = (TFT_HEIGHT - h) / 2;
  
  uint8_t lineBuffer[w * 2];
  
  // 0,0 coordinates are top left
  for (row = 0; row < h; row++) {

    bmpFS.read(lineBuffer, sizeof(lineBuffer));
    uint8_t PixM, PixL;
    
    // Colors are already in 16-bit R5, G6, B5 format
    for (col = 0; col < w; col++)
    {
      if (dimming == 255) { // not needed, copy directly
        output_buffer[row+y][col+x] = (lineBuffer[col*2+1] << 8) | (lineBuffer[col*2]);
      } else {
        // 16 BPP pixel format: R5, G6, B5 ; bin: RRRR RGGG GGGB BBBB
        PixM = lineBuffer[col*2+1];
        PixL = lineBuffer[col*2];
        // align to 8-bit value (MSB left aligned)
        r = (PixM) & 0xF8;
        g = ((PixM << 5) | (PixL >> 3)) & 0xFC;
        b = (PixL << 3) & 0xF8;
        r *= dimming;
        g *= dimming;
        b *= dimming;
        r = r >> 8;
        g = g >> 8;
        b = b >> 8;
        output_buffer[row+y][col+x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
      }
    }
  }
  FileInBuffer = file_index;
  
  bmpFS.close();
#ifdef DEBUG_OUTPUT
  Serial.print("img load : ");
  Serial.println(millis() - StartTime);  
#endif
  return (true);
}
#endif 

void TFTs::DrawImage(uint8_t file_index) {

  uint32_t StartTime = millis();
  
  // check if file is already loaded into buffer; skip loading if it is. Saves 50 to 150 msec of time.
  if (file_index != FileInBuffer) {
    LoadImageIntoBuffer(file_index);
  }
  
  bool oldSwapBytes = getSwapBytes();
  setSwapBytes(true);
  pushImage(0,0, TFT_WIDTH, TFT_HEIGHT, (uint16_t *)output_buffer);
  setSwapBytes(oldSwapBytes);

#ifdef DEBUG_OUTPUT
  Serial.print("img transfer: ");  
  Serial.println(millis() - StartTime);  
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
