#include "TFTs.h"
#include "WiFi_WPS.h"
#include "Mqtt_client_ips.h"

void TFTs::begin() {
  // Start with all displays selected.
  chip_select.begin();
  chip_select.setAll();

  // Turn power on to displays.
  pinMode(TFT_ENABLE_PIN, OUTPUT);
  enableAllDisplays();
  InvalidateImageInBuffer();

  // Initialize the super class.
  init();

  // Set SPIFFS ready
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialization failed!");
  }

  NumberOfClockFaces = CountNumberOfClockFaces();
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

void TFTs::InvalidateImageInBuffer() { // force reload from Flash with new dimming settings
  FileInBuffer=255; // invalid, always load first image
}

bool TFTs::FileExists(const char* path) {
    fs::File f = SPIFFS.open(path, "r");
    bool Exists = ((f == true) && !f.isDirectory());
    f.close();
    return Exists;
}

// These BMP functions are stolen directly from the TFT_SPIFFS_BMP example in the TFT_eSPI library.
// Unfortunately, they aren't part of the library itself, so I had to copy them.
// I've modified DrawImage to buffer the whole image at once instead of doing it line-by-line.


// Too big to fit on the stack.
uint16_t TFTs::output_buffer[TFT_HEIGHT][TFT_WIDTH];

#ifndef USE_CLK_FILES

int8_t TFTs::CountNumberOfClockFaces() {
  int8_t i, found;
  char filename[10];

  Serial.print("Searching for BMP clock files... ");
  found = 0;
  for (i=1; i < 10; i++) {
    sprintf(filename, "/%d.bmp", i*10); // search for files 10.bmp, 20.bmp,...
    if (!FileExists(filename)) {
      found = i-1;
      break;
    }
  }
  Serial.print(found);
  Serial.println(" fonts found.");
  return found;
}

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

  uint32_t seekOffset, headerSize, paletteSize = 0;
  int16_t w, h, row, col;
  uint16_t  r, g, b, bitDepth;

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

  read32(bmpFS); // filesize in bytes
  read32(bmpFS); // reserved
  seekOffset = read32(bmpFS); // start of bitmap
  headerSize = read32(bmpFS); // header size
  w = read32(bmpFS); // width
  h = read32(bmpFS); // height
  read16(bmpFS); // color planes (must be 1)
  bitDepth = read16(bmpFS);
#ifdef DEBUG_OUTPUT
  Serial.print("image W, H, BPP: ");
  Serial.print(w); 
  Serial.print(", "); 
  Serial.print(h);
  Serial.print(", "); 
  Serial.println(bitDepth);
  Serial.print("dimming: ");
  Serial.println(dimming);
#endif
  // center image on the display
  int16_t x = (TFT_WIDTH - w) / 2;
  int16_t y = (TFT_HEIGHT - h) / 2;
  
  if (read32(bmpFS) != 0 || (bitDepth != 24 && bitDepth != 1 && bitDepth != 4 && bitDepth != 8)) {
    Serial.println("BMP format not recognized.");
    bmpFS.close();
    return(false);
  }

  uint32_t palette[256];
  if (bitDepth <= 8) // 1,4,8 bit bitmap: read color palette
  {
    read32(bmpFS); read32(bmpFS); read32(bmpFS); // size, w resolution, h resolution
    paletteSize = read32(bmpFS);
    if (paletteSize == 0) paletteSize = bitDepth * bitDepth; // if 0, size is 2^bitDepth
    bmpFS.seek(14 + headerSize); // start of color palette
    for (uint16_t i = 0; i < paletteSize; i++) {
      palette[i] = read32(bmpFS);
    }
  }

  bmpFS.seek(seekOffset);

  uint32_t lineSize = ((bitDepth * w +31) >> 5) * 4;
  uint8_t lineBuffer[lineSize];
  
  // row is decremented as the BMP image is drawn bottom up
  for (row = h-1; row >= 0; row--) {

    bmpFS.read(lineBuffer, sizeof(lineBuffer));
    uint8_t*  bptr = lineBuffer;
    
    // Convert 24 to 16 bit colours while copying to output buffer.
    for (col = 0; col < w; col++)
    {
      if (bitDepth == 24) {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
        } else {
          uint32_t c = 0;
          if (bitDepth == 8) {
            c = palette[*bptr++];
          }
          else if (bitDepth == 4) {
            c = palette[(*bptr >> ((col & 0x01)?0:4)) & 0x0F];
            if (col & 0x01) bptr++;
          }
          else { // bitDepth == 1
            c = palette[(*bptr >> (7 - (col & 0x07))) & 0x01];
            if ((col & 0x07) == 0x07) bptr++;
          }
          b = c; g = c >> 8; r = c >> 16;
        }
        if (dimming < 255) { // only dim when needed
          b *= dimming;
          g *= dimming;
          r *= dimming;
          b = b >> 8;
          g = g >> 8;
          r = r >> 8;
        }
        output_buffer[row][col] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xFF) >> 3);
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


#ifdef USE_CLK_FILES

int8_t TFTs::CountNumberOfClockFaces() {
  int8_t i, found;
  char filename[10];

  Serial.print("Searching for CLK clock files... ");
  found = 0;
  for (i=1; i < 10; i++) {
    sprintf(filename, "/%d.clk", i*10); // search for files 10.clk, 20.clk,...
    if (!FileExists(filename)) {
      found = i-1;
      break;
    }
  }
  Serial.print(found);
  Serial.println(" fonts found.");
  return found;
}

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
