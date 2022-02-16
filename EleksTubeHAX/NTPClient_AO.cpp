/**
 * Copyright (c) 2015 by Fabrice Weinberg
 *
 * Library copied from: https://github.com/arduino-libraries/NTPClient
 * additions from: https://github.com/MHotchin/NTPClient/blob/master/NTPClient.cpp
 * and from: https://github.com/arduino-libraries/NTPClient/pull/28/commits/bbcc429f68c7624ada4a24f1a103fc34be8d72f8
 * - more debug outputs
 * - clearing data buffer before contacting server
 * - checking NTP protocol version
 */

#include "NTPClient_AO.h"

#ifdef DEBUG_NTPClient
  #define DBG(X) Serial.println(F(X))
#else
  #define DBG(X) (void)0
#endif

NTPClient::NTPClient(UDP& udp) {
  this->_udp            = &udp;
}

NTPClient::NTPClient(UDP& udp, long timeOffset) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName) {
  this->_udp            = &udp;
  this->_poolServerName = poolServerName;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName, long timeOffset) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerName = poolServerName;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName, long timeOffset, unsigned long updateInterval) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerName = poolServerName;
  this->_updateInterval = updateInterval;
}

void NTPClient::begin() {
  this->begin(NTP_DEFAULT_LOCAL_PORT);
}

void NTPClient::begin(int port) {
  this->_port = port;

  this->_udp->begin(this->_port);

  this->_udpSetup = true;
}

bool NTPClient::forceUpdate() {
  DBG("Update from NTP Server...");

  // flush any existing packets
  while(this->_udp->parsePacket() != 0)
    this->_udp->flush();

  if (!this->sendNTPPacket()) {
    DBG("NTP err: Could not send packet");
    return false;
  }

  // Wait till data is there or timeout...
  byte timeout = 0;
  int cb = 0;
  do {
    delay ( 10 );
    cb = this->_udp->parsePacket();
    if (timeout > 100) {
      DBG("NTP Timeout!");
      return false; // timeout after 1000 ms
      }
    timeout++;
  } while (cb == 0);

  this->_lastUpdate = millis() - (10 * (timeout + 1)); // Account for delay in reading the time

  byte _packetBuffer[NTP_PACKET_SIZE];
  // clear  buffer before receiving data from server
  memset(_packetBuffer, 0, sizeof(_packetBuffer));

  if (this->_udp->read(_packetBuffer, NTP_PACKET_SIZE) != NTP_PACKET_SIZE) {
    DBG("NTP err: Incorrect data size");
    return false;
  }
  
  #ifdef DEBUG_NTPClient
    Serial.print("NTP Data:");
    char s1[4];
    for (int i = 0; i < NTP_PACKET_SIZE; i++) {
      sprintf(s1, " %02X", _packetBuffer[i]);
      Serial.print(s1);
      }
    Serial.println(".");
  #endif

/*
  unsigned char version = _packetBuffer[0];
  version = (version >> 3) & 0x07;
  if (version != 4) {
    #ifdef DEBUG_NTPClient
      Serial.println("Incorrect NTP version!");
    #endif
    return false;
    }
*/

  // code from: https://github.com/arduino-libraries/NTPClient/pull/28/commits/bbcc429f68c7624ada4a24f1a103fc34be8d72f8
	//Perform a few validity checks on the packet
	if((_packetBuffer[0] & 0b11000000) == 0b11000000)		//Check for LI=UNSYNC
    {
    #ifdef DEBUG_NTPClient
      Serial.println("err: NTP UnSync");
    #endif
    return false;
    }
  
	if((_packetBuffer[0] & 0b00111000) >> 3 < 0b100)		//Check for Version >= 4
    {
    #ifdef DEBUG_NTPClient
      Serial.println("err: Incorrect NTP Version");
    #endif
    return false;
    }

	if((_packetBuffer[0] & 0b00000111) != 0b100)			//Check for Mode == Server
    {
    #ifdef DEBUG_NTPClient
      Serial.println("err: NTP mode is not Server");
    #endif
    return false;
    }

	if((_packetBuffer[1] < 1) || (_packetBuffer[1] > 15))		//Check for valid Stratum
    {
    #ifdef DEBUG_NTPClient
      Serial.println("err: Incorrect NTP Stratum");
    #endif
    return false;
    }

	if(	_packetBuffer[16] == 0 && _packetBuffer[17] == 0 && 
		_packetBuffer[18] == 0 && _packetBuffer[19] == 0 &&
		_packetBuffer[20] == 0 && _packetBuffer[21] == 0 &&
		_packetBuffer[22] == 0 && _packetBuffer[22] == 0)		//Check for ReferenceTimestamp != 0
    {
    #ifdef DEBUG_NTPClient
      Serial.println("err: Incorrect NTP Ref Timestamp");
    #endif
    return false;
    }

  
  unsigned long highWord = word(_packetBuffer[40], _packetBuffer[41]);
  unsigned long lowWord = word(_packetBuffer[42], _packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;

  this->_currentEpoc = secsSince1900 - SEVENZYYEARS;

  return true;
}

bool NTPClient::update() {
  if ((millis() - this->_lastUpdate >= this->_updateInterval)     // Update after _updateInterval
    || this->_lastUpdate == 0) {                                // Update if there was no update yet.
    if (!this->_udpSetup) this->begin();                         // setup the UDP client if needed
    return this->forceUpdate();
  }
  return true;
}

unsigned long NTPClient::getEpochTime() const {
  return this->_timeOffset + // User offset
         this->_currentEpoc + // Epoc returned by the NTP server
         ((millis() - this->_lastUpdate) / 1000); // Time since last update
}

int NTPClient::getDay() const {
  return (((this->getEpochTime()  / 86400L) + 4 ) % 7); //0 is Sunday
}
int NTPClient::getHours() const {
  return ((this->getEpochTime()  % 86400L) / 3600);
}
int NTPClient::getMinutes() const {
  return ((this->getEpochTime() % 3600) / 60);
}
int NTPClient::getSeconds() const {
  return (this->getEpochTime() % 60);
}

String NTPClient::getFormattedTime() const {
  unsigned long rawTime = this->getEpochTime();
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  unsigned long seconds = rawTime % 60;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}

void NTPClient::end() {
  this->_udp->stop();

  this->_udpSetup = false;
}

void NTPClient::setTimeOffset(int timeOffset) {
  this->_timeOffset     = timeOffset;
}

void NTPClient::setUpdateInterval(unsigned long updateInterval) {
  this->_updateInterval = updateInterval;
}

void NTPClient::setPoolServerName(const char* poolServerName) {
    this->_poolServerName = poolServerName;
}

bool NTPClient::sendNTPPacket() {
  byte _packetBuffer[NTP_PACKET_SIZE];
  // set all bytes in the buffer to 0
  memset(_packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  _packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  _packetBuffer[1] = 0;     // Stratum, or type of clock
  _packetBuffer[2] = 6;     // Polling Interval
  _packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  _packetBuffer[12]  = 49;
  _packetBuffer[13]  = 0x4E;
  _packetBuffer[14]  = 49;
  _packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  bool returnValue;
  
  returnValue = this->_udp->beginPacket(this->_poolServerName, 123); //NTP requests are to port 123
  
  if (returnValue) {
    //  This will always execute both lines, but will return 'false' if *either* fails
    returnValue = (this->_udp->write(_packetBuffer, NTP_PACKET_SIZE) == NTP_PACKET_SIZE);
    returnValue = this->_udp->endPacket() && returnValue;
  }
  return returnValue;
}
