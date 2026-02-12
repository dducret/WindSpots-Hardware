#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ctime>
#include <cmath>
#include <cctype>
#include "oregon.h"
#include "w3rpi.h"

using namespace std;
using namespace w3rpi;

// Constructor: initialize sensor values and flags.
oregon::oregon(char * _strval) {
  time(&lastUpdate);
  Name[0] = '\0';
  ID[0] = '\0';
  Channel = -1;
  RollingCode = -1;
  Battery = -1;
  Temperature = 0.0;
  Humidity = 0.0;
  WaterTemperature = 0.0;
  Comfort[0] = '\0';
  UVindex = -1;
  RainRate = 0.0;
  TotalRain = 0.0;
  Barometer = 0;
  Prediction = -1;
  WindDirection = 0.0;
  Speed = 0.0;
  SpeedAverage = 0.0;
  haveBattery = false;
  haveTemperature = false;
  haveHumidity = false;
  haveWaterTemperature = false;
  haveComfort = false;
  haveUVindex = false;
  haveRain = false;
  haveBarometer = false;
  havePrediction = false;
  haveWind = false;
  isValid = false;
  packet[0] = '\0';
}

bool oregon::isBattery() { return haveBattery; }
bool oregon::isTemperature() { return haveTemperature; }
bool oregon::isHumidity() { return haveHumidity; }
bool oregon::isWaterTemperature() { return haveWaterTemperature; }
bool oregon::isComfort() { return haveComfort; }
bool oregon::isUVindex() { return haveUVindex; }
bool oregon::isRain() { return haveRain; }
bool oregon::isBarometer() { return haveBarometer; }
bool oregon::isPrediction() { return havePrediction; }
bool oregon::isWind() { return haveWind; }
bool oregon::isDecoded() { return isValid; }

time_t oregon::getLastUpdate() { return lastUpdate; }
char * oregon::getName() { return Name; }
char * oregon::getID() { return ID; }
int oregon::getChannel() { return Channel; }
int oregon::getRollingCode() { return RollingCode; }
int oregon::getBattery() { return Battery; }
double oregon::getTemperature() { return Temperature; }
double oregon::getHumidity() { return Humidity; }
double oregon::getWaterTemperature() { return WaterTemperature; }
char * oregon::getComfort() { return Comfort; }
int oregon::getUVindex() { return UVindex; }
double oregon::getRainRate() { return RainRate; }
double oregon::getTotalRain() { return TotalRain; }
int oregon::getBarometer() { return Barometer; }
int oregon::getPrediction() { return Prediction; }
double oregon::getWindDirection() { return WindDirection; }
double oregon::getSpeed() { return Speed; }
double oregon::getSpeedAverage() { return SpeedAverage; }
char * oregon::getPacket() { return packet; }

int oregon::getIntFromChar(char c) {
  const char hexDecod[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
  for (int i = 0; i < 16; i++) {
    if (hexDecod[i] == c) return i;
  }
  return -1;
}

int oregon::getIntFromString(char * s) {
  int r = 0;
  while (*s != '\0') {
    r *= 16;
    int t = getIntFromChar(*s);
    if (t == -1) return -1;
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
  return (battery == '0') ? 1 : 0;
}

int oregon::_wrBattery(char battery) {
  switch(battery) {
    case '0': return 100;
    case '1': return 90;
    case '2': return 80;
    case '3': return 70;
    case '4': return 60;
    case '5': return 50;
    case '6': return 40;
    case '7': return 30;
    case '8': return 20;
    case '9': return 10;
    default:  return 0;
  }
}

void oregon::_decodeRain(char * message) {
    int len = strlen(message);
    if(len < 16) {
      std::cerr << "_decodeRain: insufficient length (" << len << "): " << message << std::endl;
      return;
    }
    char reverse[6];
    double dResult = 0;
    reverse[0] = message[10];
    reverse[1] = message[9];
    reverse[2] = message[8];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->RainRate = dResult / 10;
    reverse[0] = message[15];
    reverse[1] = message[14];
    reverse[2] = message[13];
    reverse[3] = message[12];
    reverse[4] = message[11];
    reverse[5] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->TotalRain = dResult / 10;
    this->haveRain = true;
}

void oregon::_decodeRainInch(char * message) {
    int len = strlen(message);
    if(len < 16) {
      std::cerr << "_decodeRainInch: insufficient length (" << len << "): " << message << std::endl;
      return;
    }
    char reverse[6];
    double dResult = 0;
    reverse[0] = message[10];
    reverse[1] = message[9];
    reverse[2] = message[8];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->RainRate = dResult / 10;
    reverse[0] = message[15];
    reverse[1] = message[14];
    reverse[2] = message[13];
    reverse[3] = message[12];
    reverse[4] = message[11];
    reverse[5] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->TotalRain = dResult / 10;
    this->haveRain = true;
}

void oregon::_decodeTemperature(char * message) {
    int len = strlen(message);
    if(len < 11) {
      std::cerr << "_decodeTemperature: insufficient length (" << len << "): " << message << std::endl;
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
      dResult = -dResult;
    this->Temperature = dResult / 10;
    if(w3rpi_debug)
        std::cerr << "    _decodeTemperature: Temperature (" << reverse << "): " << this->Temperature << std::endl;
    this->haveTemperature = true;
}

void oregon::_decodeWaterTemperature(char * message) {
    int len = strlen(message);
    if(len < 11) {
      std::cerr << "_decodeWaterTemperature: insufficient length (" << len << "): " << message << std::endl;
      return;
    }
    char reverse[4];
    double dResult = 0;
    reverse[0] = message[10];
    reverse[1] = message[9];
    reverse[2] = message[8];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->WaterTemperature = dResult / 10;
    if(w3rpi_debug)
        std::cerr << "    _decodeWaterTemperature: Temperature (" << reverse << "): " << this->WaterTemperature << std::endl;
    this->haveWaterTemperature = true;
}

void oregon::_decodeTemperatureHumidity(char * message) {
    int len = strlen(message);
    if(len < 13) {
      std::cerr << "_decodeTemperatureHumidity: insufficient length (" << len << "): " << message << std::endl;
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
      dResult = -dResult;
    this->Temperature = dResult / 10;
    if(w3rpi_debug)
        std::cerr << "    _decodeTemperatureHumidity: Temperature (" << reverse << "): " << this->Temperature << std::endl;
    reverse[0] = message[13];
    reverse[1] = message[12];
    reverse[2] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->Humidity = dResult;
    if(w3rpi_debug)
        std::cerr << "    _decodeTemperatureHumidity: Humidity (" << reverse << "): " << this->Humidity << std::endl;
    this->haveTemperature = true;
    this->haveHumidity = true;
}

void oregon::_decodeTemperatureHumidityBarometer(char * message) {
    int len = strlen(message);
    if(len < 17) {
      std::cerr << "_decodeTemperatureHumidityBarometer: insufficient length (" << len << "): " << message << std::endl;
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
      dResult = -dResult;
    this->Temperature = dResult / 10;
    if(w3rpi_debug)
        std::cerr << "    _decodeTemperatureHumidityBarometer: Temperature (" << reverse << "): " << this->Temperature << std::endl;
    reverse[0] = message[13];
    reverse[1] = message[12];
    reverse[2] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->Humidity = dResult;
    if(w3rpi_debug)
        std::cerr << "    _decodeTemperatureHumidityBarometer: Humidity (" << reverse << "): " << this->Humidity << std::endl;
    reverse[0] = message[16];
    reverse[1] = message[15];
    reverse[2] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->Barometer = static_cast<int>(dResult) + 856;
    if(w3rpi_debug)
        std::cerr << "    _decodeTemperatureHumidityBarometer: Barometer (" << reverse << "): " << this->Barometer << std::endl;
    this->Prediction = getIntFromChar(message[17]);
    if(w3rpi_debug)
        std::cerr << "    _decodeTemperatureHumidityBarometer: Prediction: " << this->Prediction << std::endl;
    this->haveTemperature = true;
    this->haveHumidity = true;
    this->haveBarometer = true;
}

void oregon::_decodeWind(char * message) {
    int len = strlen(message);
    if(len < 17) {
        std::cerr << "_decodeWind: insufficient message length (" << len << "): " << message << std::endl;
        return;
    }
    this->haveWind = false;

    // Decode wind direction from message[8].
    int dirVal = getIntFromChar(message[8]);
    this->WindDirection = static_cast<double>(dirVal) * 22.5;
    if(w3rpi_debug)
        std::cerr << "WGR800 _decodeWind: Direction (0x" << hex << dirVal << dec << ") = " << this->WindDirection << std::endl;

    // Decode wind speed from message positions 11-13.
    char speedStr[4] = {0};
    if(len >= 14) {
        // Expected digits in reverse order.
        speedStr[0] = message[13];
        speedStr[1] = message[12];
        speedStr[2] = message[11];
        speedStr[3] = '\0';
        double dResult = atof(speedStr);
        this->Speed = dResult / 10.0;
        if(w3rpi_debug)
            std::cerr << "WGR800 _decodeWind: Speed from \"" << speedStr << "\" = " << this->Speed << std::endl;
    } else {
        std::cerr << "_decodeWind: missing bytes for wind speed" << std::endl;
    }

    // Decode wind speed average from message positions 14-16.
    char avgSpeedStr[4] = {0};
    if(len >= 17) {
        avgSpeedStr[0] = message[16];
        avgSpeedStr[1] = message[15];
        avgSpeedStr[2] = message[14];
        avgSpeedStr[3] = '\0';
        double dResult = atof(avgSpeedStr);
        this->SpeedAverage = dResult / 10.0;
        if(w3rpi_debug)
            std::cerr << "WGR800 _decodeWind: SpeedAverage from \"" << avgSpeedStr << "\" = " << this->SpeedAverage << std::endl;
    } else {
        std::cerr << "_decodeWind: missing bytes for average wind speed" << std::endl;
    }
    this->haveWind = true;
}

void oregon::_decodeWind968(char * message) {
    int len = strlen(message);
    this->haveWind = false;
    if(len < 17) {
        std::cerr << "_decodeWind968: insufficient length (" << len << "): " << message << std::endl;
        return;
    }
    char reverse[5] = {0};
    double dResult = 0;
    reverse[0] = message[10];
    reverse[1] = message[9];
    reverse[2] = message[8];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->WindDirection = dResult;
    if(w3rpi_debug)
        std::cerr << "    _decodeWind968: Direction = " << this->WindDirection << std::endl;
    reverse[0] = message[13];
    reverse[1] = message[12];
    reverse[2] = message[11];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->Speed = dResult / 10;
    if(w3rpi_debug)
        std::cerr << "    _decodeWind968: Speed = " << this->Speed << std::endl;
    reverse[0] = message[16];
    reverse[1] = message[15];
    reverse[2] = message[14];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->SpeedAverage = dResult / 10;
    if(w3rpi_debug)
        std::cerr << "    _decodeWind968: SpeedAverage = " << this->SpeedAverage << std::endl;
    this->haveWind = true;
}

void oregon::_decodeWind928(char * message) {
    int len = strlen(message);
    this->haveWind = false;
    if(len < 18) {
        std::cerr << "_decodeWind928: insufficient length (" << len << "): " << message << std::endl;
        return;
    }
    char reverse[5] = {0};
    double dResult = 0;
    reverse[0] = message[10];
    reverse[1] = message[11];
    reverse[2] = message[8];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->WindDirection = dResult;
    if(w3rpi_debug)
        std::cerr << "    _decodeWind928: Direction (" << reverse << ") = " << this->WindDirection << std::endl;
    reverse[0] = message[15];
    reverse[1] = message[12];
    reverse[2] = message[13];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->Speed = dResult / 10;
    if(w3rpi_debug)
        std::cerr << "    _decodeWind928: Speed (" << reverse << ") = " << this->Speed << std::endl;
    reverse[0] = message[16];
    reverse[1] = message[17];
    reverse[2] = message[14];
    reverse[3] = '\0';
    sscanf(reverse, "%lf", &dResult);
    this->SpeedAverage = dResult / 10;
    if(w3rpi_debug)
        std::cerr << "    _decodeWind928: SpeedAverage (" << reverse << ") = " << this->SpeedAverage << std::endl;
    this->haveWind = true;
}

oregon * oregon::getRightOregon(char * s) {
    int len = strlen(s);
    if(len > 4) {
        char * message = &s[4];
        if(w3rpi_debug)
            std::cerr << "SENSOR message: " << s << std::endl;
        if(s[0] == 'O' && s[1] == 'S' && s[2] == '2') {
            OregonV2V3 * r = new OregonV2V3(message);
            return static_cast<oregon*>(r);
        } else if(s[0] == 'O' && s[1] == 'S' && s[2] == '3') {
            OregonV2V3 * r = new OregonV2V3(message);
            return static_cast<oregon*>(r);
        }
    }
    return nullptr;
}

OregonV2V3::OregonV2V3(char * _strval) : oregon(_strval) {
    this->isValid = this->decode(_strval);
}

bool OregonV2V3::decode(char * message) {
    int len = strlen(message);
    int isensorId;
    if(len > 10) {
        strncpy(this->ID, message, 4);
        this->ID[4] = '\0';
        isensorId = getIntFromString(this->ID);
        this->Channel = char2int(message[4]);
        this->RollingCode = char2int(message[5]);
        if(w3rpi_debug)
            std::cerr << "OS - decode - id(0x" << hex << isensorId << dec << ") length:" << len << std::endl;
        switch(isensorId) {
            case 0xF824:
                strcpy(this->Name, "THGR810");
                this->Battery = _wrBattery(message[7]);
                this->_decodeTemperatureHumidity(message);
                this->isValid = true;
                break;
            case 0xF8B4:
                strcpy(this->Name, "THGR810-1");
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
                strcpy(this->Name, "THWR288A");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeWaterTemperature(message);
                this->isValid = true;
                break;
            case 0xEC40:
                strcpy(this->Name, "THR238N");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeTemperature(message);
                this->isValid = true;
                break;
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
                strcpy(this->Name, "RTGR328N");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeTemperatureHumidity(message);
                this->isValid = true;
                break;
            case 0x1D30:
            case 0x3A0D:
                strcpy(this->Name, "STR918");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeTemperature(message);
                this->isValid = true;
                break;
            case 0x380D:
            case 0x390D:
            case 0x3B0D:
                strcpy(this->Name, "STR928N");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeWind928(message);
                this->isValid = this->haveWind;
                break;
            case 0xC844:
                strcpy(this->Name, "THWR800");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeWaterTemperature(message);
                this->isValid = true;
                break;
            case 0x5D50:
            case 0x5D60:
                strcpy(this->Name, "BTHR968");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeTemperatureHumidityBarometer(message);
                this->isValid = true;
                break;
            case 0x5A5D:
                strcpy(this->Name, "BTHR918");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeTemperatureHumidityBarometer(message);
                this->isValid = true;
                break;
            case 0x2D10:
                strcpy(this->Name, "RGR968");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeRain(message);
                this->isValid = true;
                break;
            case 0x2A1D:
                strcpy(this->Name, "RGR918");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeRain(message);
                this->isValid = true;
                break;
            case 0x2914:
                strcpy(this->Name, "PCR800");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeRainInch(message);
                this->isValid = true;
                break;
            case 0x1D20:
                strcpy(this->Name, "THGR122NX");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeTemperatureHumidity(message);
                this->isValid = true;
                break;
            case 0x1984:
            case 0x1994:
                this->haveWind = false;
                strcpy(this->Name, "WGR800");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeWind(message);
                this->isValid = this->haveWind;
                break;
            case 0x3D00:
                this->haveWind = false;
                strcpy(this->Name, "WGR968");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeWind968(message);
                this->isValid = this->haveWind;
                break;
            case 0x9ACC:
            case 0x9BCC:
            case 0xBACC:
            case 0xCBCC:
            case 0xDACC:
            case 0xDBCC:
                strcpy(this->Name, "RTGR328N");
                this->Battery = _batteryIndication(message[7]);
                this->_decodeTemperatureHumidity(message);
                this->isValid = true;
                break;
            default:
                this->isValid = false;
                std::cerr << "oregon.cpp - ID: " << this->ID << " - channel " << this->Channel
                          << " - not decoded (" << len << "): " << message << std::endl;
                break;
        }
        if(w3rpi_debug)
            std::cerr << "oregon.cpp - Name: " << this->Name << " - Valid: " << this->isValid
                      << " - Length(" << len << "): " << message << std::endl;
        return this->isValid;
    }
    std::cerr << "oregon.cpp - OS not decoded (" << len << ") - " << message << std::endl;
    return false;
}