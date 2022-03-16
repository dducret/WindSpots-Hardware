// oregon.cpp
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "oregon.h"
using namespace std;
using namespace w3rpi;
//#define SENSORDEBUG
// Construction - init variable then call decode function
oregon::oregon( char * _strval ) {
  time(&this->lastUpdate);
  this->Name[0] = '\0';
  this->ID[0] = '\0';
  this->Channel = -1;
  this->RollingCode = -1;
  this->Battery = -1;
  this->Temperature = 0.0;
  this->Humidity = 0.0;
  this->WaterTemperature = 0.0;
  this->Comfort[0] = '\0';
  this->UVindex = -1;
  this->RainRate = 0.0;
  this->TotalRain = 0.0;
  this->Barometer = 0;
  this->Prediction = -1;
  this->WindDirection = 0.0;
  this->Speed = 0.0;
  this->SpeedAverage = 0.0;
    // true if flag set = false;
  this->haveBattery = false;
  this->haveTemperature = false;
  this->haveHumidity = false;
  this->haveWaterTemperature = false;
  this->haveComfort = false;
  this->haveUVindex = false;
  this->haveRain = false;
  this->haveBarometer = false;
  this->havePrediction = false;
  this->haveWind = false;
  this->isValid = false;  
  this->packet[0] = '\0'; 
}
bool oregon::isBattery() { return this->haveBattery; }
bool oregon::isTemperature() { return this->haveTemperature; }
bool oregon::isHumidity() { return this->haveHumidity; }
bool oregon::isWaterTemperature() { return this->haveWaterTemperature; }
bool oregon::isComfort() { return this->haveComfort; }
bool oregon::isUVindex() { return this->haveUVindex; }
bool oregon::isRain() { return this->haveRain; }
bool oregon::isBarometer() { return this->haveBarometer; }
bool oregon::isPrediction() { return this->havePrediction; }
bool oregon::isWind() { return this->haveWind; }
bool oregon::isDecoded() { return this->isValid; }
//
time_t oregon::getLastUpdate() { return this->lastUpdate; }
char * oregon::getName() { return this->Name; } 
char * oregon::getID() { return this->ID; } 
int    oregon::getChannel() { return this->Channel; }
int    oregon::getRollingCode() { return this->RollingCode; }
int    oregon::getBattery() { return this->Battery; }
double oregon::getTemperature() {  return this->Temperature; }
double oregon::getHumidity() {  return this->Humidity; }
double oregon::getWaterTemperature() {  return this->WaterTemperature; }
char * oregon::getComfort() { return this->Comfort; }
int    oregon::getUVindex() { return this->UVindex; }
double oregon::getRainRate() {  return this->RainRate; }
double oregon::getTotalRain() {  return this->TotalRain; }
int    oregon::getBarometer() { return this->Barometer; }
int    oregon::getPrediction() { return this->Prediction; }
double oregon::getWindDirection() {  return this->WindDirection; }
double oregon::getSpeed() {  return this->Speed; }
double oregon::getSpeedAverage() {  return this->SpeedAverage; }
//
int oregon::getIntFromChar(char c) {
  char _hexDecod[16] = { '0', '1', '2', '3', '4', '5', '6', '7','8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
  for ( int i=0 ; i < 16 ; i++ )
    if ( _hexDecod[i] == c ) return i;
  return -1;
}
int oregon::getIntFromString(char * s) {
  int r = 0;
  while ( *s != '\0' ) {
    r *= 16;
    int t = getIntFromChar(*s);
    if ( t == -1 ) return -1;
    r += t;
    s++;
  }
  return r;
}
int oregon::char2int(char input) {
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  return 0;
}
int oregon::_batteryIndication(char battery) {
  if(battery == '0')
    return 1; // ok
  else
    return 0; // empty
}
int oregon::_wrBattery(char battery) {
  switch(battery) {
    case '0':
      return 100;
      break;
    case '1':
      return 90;
      break;
    case '2':
      return 80;
      break;
    case '3':
      return 70;
      break;
    case '4':
      return 60;
      break;
    case '5':
      return 50;
      break;
    case '6':
      return 40;
      break;
    case '7':
      return 30;
      break;
    case '8':
      return 20;
      break;
    case '9':
      return 10;
      break;
    default:
      return 0;
      break;
  }
}
void oregon::_decodeRain(char * message) {
    int  len = strlen(message);
    if( len < 16) {
      printf("_decodeRain < 16");
      return;
    }
    char reverse[6];
    double dResult = 0;
    reverse[0] = message[10];
    reverse[1] = message[9];
    reverse[2] = message[8];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->RainRate=dResult/10;
    reverse[0] = message[15];
    reverse[1] = message[14];
    reverse[2] = message[13];
    reverse[3] = message[12];
    reverse[4] = message[11];
    reverse[5] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->TotalRain=dResult/10;
    this->haveRain = true;
}    
void oregon::_decodeRainInch(char * message) {
    int  len = strlen(message);
    if( len < 16) {
      printf("_decodeRainInch < 16");
      return;
    }
    char reverse[6];
    double dResult = 0;
    reverse[0] = message[10];
    reverse[1] = message[9];
    reverse[2] = message[8];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->RainRate=dResult/10;
    reverse[0] = message[15];
    reverse[1] = message[14];
    reverse[2] = message[13];
    reverse[3] = message[12];
    reverse[4] = message[11];
    reverse[5] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->TotalRain=dResult/10;
    this->haveRain = true;
}    
void oregon::_decodeTemperature(char * message) {
    int  len = strlen(message);
    if( len < 11) {
      printf("_decodeTemperature < 11");
      return;
    }
    char reverse[4];
    double dResult = 0;
    reverse[0] = message[10];
    reverse[1] = message[9];
    reverse[2] = message[8];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    if(getIntFromChar(message[11]) != 0)
      dResult = dResult * -1;
    this->Temperature=dResult/10;
    #ifdef SENSORDEBUG
      printf("    _decodeTemperature - Temperature (%s): %0.1f\n", reverse, this->Temperature);
    #endif
    this->haveTemperature = true;
}
void oregon::_decodeWaterTemperature(char * message) {
    int  len = strlen(message);
    if( len < 11) {
      printf("_decodeWaterTemperature < 11");
      return;
    }
    char reverse[4];
    double dResult = 0;
    reverse[0] = message[10];
    reverse[1] = message[9];
    reverse[2] = message[8];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->WaterTemperature=dResult/10;
    #ifdef SENSORDEBUG
      printf("    _decodeWaterTemperature - Temperature (%s): %0.1f\n", reverse, this->WaterTemperature);
    #endif
    this->haveWaterTemperature = true;
}
void oregon::_decodeTemperatureHumidity(char * message) {
    int  len = strlen(message);
    if( len < 13) {
      printf("_decodeTemperatureHumidity < 13");
      return;
    }
    char reverse[4];
    double dResult = 0;
    reverse[0] = message[10];
    reverse[1] = message[9];
    reverse[2] = message[8];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    if(getIntFromChar(message[11]) != 0)
      dResult = dResult * -1;
    this->Temperature=dResult/10;
    #ifdef SENSORDEBUG
      printf("    _decodeTemperatureHumidity - Temperature (%s): %0.1f\n", reverse, this->Temperature);
    #endif
    reverse[0] = message[13];
    reverse[1] = message[12];
    reverse[2] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->Humidity=dResult;
    #ifdef SENSORDEBUG
      printf("    _decodeTemperatureHumidity - Humidity (%s): %0.f\n", reverse, this->Humidity);
    #endif
    this->haveTemperature = true;
    this->haveHumidity = true;
}
void oregon::_decodeTemperatureHumidityBarometer(char * message) {
    int  len = strlen(message);
    if( len < 17) {
      printf("_decodeTemperatureHumidityBarometer < 17");
      return;
    }
    char reverse[4];
    double dResult = 0;
    reverse[0] = message[10];
    reverse[1] = message[9];
    reverse[2] = message[8];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    if(getIntFromChar(message[11]) != 0)
      dResult = dResult * -1;
    this->Temperature=dResult/10;
    #ifdef SENSORDEBUG
      printf("    _decodeTemperatureHumidityBarometer - Temperature (%s): %0.1f\n", reverse, this->Temperature);
    #endif
    reverse[0] = message[13];
    reverse[1] = message[12];
    reverse[2] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->Humidity=dResult;
    #ifdef SENSORDEBUG
      printf("    _decodeTemperatureHumidityBarometer - Humidity (%s): %0.f\n", reverse, this->Humidity);
    #endif
    reverse[0] = message[16];
    reverse[1] = message[15];
    reverse[2] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->Barometer=dResult+856;
    #ifdef SENSORDEBUG
      printf("    _decodeTemperatureHumidityBarometer - Barometer (%s): %0.f\n", reverse, this->Barometer);
    #endif
    this->Prediction = getIntFromChar(message[17]);
    #ifdef SENSORDEBUG
      printf("    _decodeTemperatureHumidityBarometer - Prediction (%s): %d\n", reverse, this->Prediction);
    #endif
    this->haveTemperature = true;
    this->haveHumidity = true;
    this->haveBarometer = true;
}
void oregon::_decodeWind(char * message) {
  int  len = strlen(message);
  if( len < 17) {
    printf("_decodeWind < 17");
    return;
  }
  char reverse[5];
  double dResult;
  this->WindDirection = (double) getIntFromChar(message[8]) * 22.5;
  #ifdef SENSORDEBUG
      printf("    _decodeWind - Direction (%x): %f\n", getIntFromChar(message[8]), this->WindDirection);
  #endif
  reverse[0] = message[13];
  reverse[1] = message[12];
  reverse[2] = message[11];
  reverse[3] = '\0';
  sscanf(reverse, "%lf", &dResult);
  this->Speed=dResult/10;
  #ifdef SENSORDEBUG
      printf("    _decodeWind - Speed (%s): %0.1f\n", reverse, this->Speed);
  #endif
  reverse[0] = message[16];
  reverse[1] = message[15];
  reverse[2] = message[14];
  reverse[3] = '\0';
  sscanf(reverse, "%lf", &dResult);
  this->SpeedAverage=dResult/10;
  #ifdef SENSORDEBUG
      printf("    _decodeWind - SpeedAverage (%s): %0.1f\n", reverse, this->SpeedAverage);
  #endif
  this->haveWind = true;
}
void oregon::_decodeWind968(char * message) {
  int  len = strlen(message);
  if( len < 17) {
    printf("_decodeWind968 < 17");
    return;
  }
  char reverse[5];
  double dResult;
  reverse[0] = message[10];
  reverse[1] = message[9];
  reverse[2] = message[8];
  reverse[3] = '\0';
  sscanf(reverse, "%lf", &dResult);
  this->WindDirection = (double) dResult;
  #ifdef SENSORDEBUG
      printf("    _decodeWind968 - Direction (%x): %f\n", getIntFromChar(message[8]), this->WindDirection);
  #endif
  reverse[0] = message[13];
  reverse[1] = message[12];
  reverse[2] = message[11];
  reverse[3] = '\0';
  sscanf(reverse, "%lf", &dResult);
  this->Speed=dResult/10;
  #ifdef SENSORDEBUG
      printf("    _decodeWind968 - Speed (%s): %0.1f\n", reverse, this->Speed);
  #endif
  reverse[0] = message[16];
  reverse[1] = message[15];
  reverse[2] = message[14];
  reverse[3] = '\0';
  sscanf(reverse, "%lf", &dResult);
  this->SpeedAverage=dResult/10;
  #ifdef SENSORDEBUG
      printf("    _decodeWind968 - SpeedAverage (%s): %0.1f\n", reverse, this->SpeedAverage);
  #endif
  this->haveWind = true;
}
void oregon::_decodeWind928(char * message) {
  int  len = strlen(message);
  if( len < 18) {
    printf("_decodeWind928 < 18");
    return;
  }
  char reverse[5];
  double dResult;
  reverse[0] = message[10];
  reverse[1] = message[11];
  reverse[2] = message[8];
  reverse[3] = '\0';
  sscanf(reverse, "%lf", &dResult);
  this->WindDirection = (double) dResult;
  #ifdef SENSORDEBUG
    printf("    _decodeWind928 - Direction (%s): %f\n", reverse, this->WindDirection);
  #endif
  reverse[0] = message[15];
  reverse[1] = message[12];
  reverse[2] = message[13];
  reverse[3] = '\0';
  sscanf(reverse, "%lf", &dResult);
  this->Speed=dResult/10;
  #ifdef SENSORDEBUG
    printf("    _decodeWind928 - Speed (%s): %0.1f\n", reverse, this->Speed);
  #endif
  reverse[0] = message[16];
  reverse[1] = message[17];
  reverse[2] = message[14];
  reverse[3] = '\0';
  sscanf(reverse, "%lf", &dResult);
  this->SpeedAverage=dResult/10;
  #ifdef SENSORDEBUG
    printf("    _decodeWind928 - SpeedAverage (%s): %0.1f\n", reverse, this->SpeedAverage);
  #endif
  this->haveWind = true;
}
oregon * oregon::getRightOregon(char * s) {
  int  len = strlen(s);
  if ( len > 4 ) {
    char * message = & s[4];
    #ifdef SENSORDEBUG
      printf("SENSOR message: %s\n",s);
    #endif
    if ( s[0] == 'O' && s[1] == 'S' && s[2] == '2') {
      OregonV2V3 * r = new OregonV2V3(message);
      return (oregon *) r;
    } else if ( s[0] == 'O' && s[1] == 'S' && s[2] == '3') {
      OregonV2V3 * r = new OregonV2V3(message);
      return (oregon *) r;
    }
  }
  return NULL;
}
// ==================================================================
OregonV2V3::OregonV2V3(char * _strval) : oregon(_strval) {
  this->isValid = this->decode(_strval);
}
bool OregonV2V3::decode( char * message ) {
  int  len = strlen(message);
  int isensorId;
  // Proceed the right sensor
  if ( len > 10 ) {
    strncpy(this->ID, message, 4);
    this->ID[4] = '\0';
    isensorId = getIntFromString(this->ID);
    this->Channel = char2int(message[4]);
    this->RollingCode = char2int(message[5]);
    #ifdef SENSORDEBUG
      printf("OS - decode - id(0x%4X) length:%i\n", isensorId, len);
    #endif
    switch (isensorId) {
      case 0xF824:
        strcpy(this->Name,"THGR810");
        this->Battery = _wrBattery(message[7]);
        this->_decodeTemperatureHumidity(message);
        this->isValid = true;
        break;
      case 0xF8B4:
        strcpy(this->Name,"THGR810-1");
        this->Battery = _wrBattery(message[7]);
        this->_decodeTemperatureHumidity(message);
        this->isValid = true;
        break;
      case 0xE04C:
      case 0xE14C:
      case 0xE24C:
      case 0xE34C:
      case 0xE44C:
      case 0xE54C:
      case 0xE64C:
      case 0xE74C:
      case 0xE84C:
      case 0xE94C:
      case 0xEA4C:
      case 0xEB4C:
      case 0xEC4C:
      case 0xED4C:
      case 0xEE4C:
      case 0xEF4C:
        strcpy(this->Name,"THWR288A");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeWaterTemperature(message);
        this->isValid = true;
        break;
      case 0xEC40:
        strcpy(this->Name,"THR238N");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeTemperature(message);
        this->isValid = true;
        break;
        /*
      case 0xEC40: // same as THR228N but different packet size
        strcpy(this->Name,"THN132N");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeTemperature(message);
        this->isValid = true;
        break;
        */
      case 0x0CC3:
      case 0x1CC3:
      case 0x2CC3:
      case 0x3CC3:
      case 0x4CC3:
      case 0x5CC3:
      case 0x6CC3:
      case 0x7CC3:
      case 0x8CC3:
      case 0x9CC3:
      case 0xACC3:
      case 0xBCC3:
      case 0xCCC3:
      case 0xDCC3:
      case 0xECC3:
      case 0xFCC3:
        strcpy(this->Name,"RTGR328N");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeTemperatureHumidity(message);
        this->isValid = true;
        break;
      case 0x1D30:
      case 0x3A0D:
        strcpy(this->Name,"STR918");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeTemperature(message);
        this->isValid = true;
        break;
      case 0x380D: // old
      case 0x390D: // old
      case 0x3B0D: // old
        strcpy(this->Name,"STR928N");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeWind928(message);
        this->isValid = true;
        break;
      case 0xC844: // Flotting
        strcpy(this->Name,"THWR800");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeWaterTemperature(message);
        this->isValid = true;
        break;
      case 0x5D50:
      case 0x5D60:
        strcpy(this->Name,"BTHR968");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeTemperatureHumidityBarometer(message);
        this->isValid = true;
        break;
      case 0x5A5D:
        strcpy(this->Name,"BTHR918");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeTemperatureHumidityBarometer(message);
        this->isValid = true;
        break;
      case 0x2D10:
        strcpy(this->Name,"RGR968");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeRain(message);
        this->isValid = true;
        break;
      case 0x2A1D:
        strcpy(this->Name,"RGR918");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeRain(message);
        this->isValid = true;
        break;
      case 0x2914:
        strcpy(this->Name,"PCR800");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeRainInch(message);
        this->isValid = true;
        break;
      case 0x1D20:
        strcpy(this->Name,"THGR122NX");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeTemperatureHumidity(message);
        this->isValid = true;
        break;
      case 0x1984:
      case 0x1994:
        strcpy(this->Name,"WGR800");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeWind(message);
        this->isValid = true;
        break;
      case 0x3D00:
        strcpy(this->Name,"WGR968");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeWind968(message);
        this->isValid = true;
        break;
      case 0x9ACC:
      case 0x9BCC:
      case 0xBACC:
      case 0xCBCC:
      case 0xDACC:
      case 0xDBCC:
        strcpy(this->Name,"RTGR328N");
        this->Battery = _batteryIndication(message[7]);
        this->_decodeTemperatureHumidity(message);
        this->isValid = true;
        break;
      default:
        // sprintf(this->Name,"OS ID: %s - channel: %d - not decoded", this->ID, this->Channel);
        this->isValid = false;
        printf("OS ID: %s - channel - %d - not decoded(%d): %s\n", this->ID, this->Channel, len,  message);
        break;
    }
    return true;
  }
  printf("OS - not decoded(%d) - %s\n", len,  message);
  return false;
}