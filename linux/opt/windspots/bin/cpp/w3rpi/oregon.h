#ifndef OREGON_H_
#define OREGON_H_

namespace w3rpi {
class oregon {
  protected:
    time_t  lastUpdate;
    char    Name[16];
    char    ID[5];
    int     Channel;
    int     RollingCode;
    int     Battery;
    double  Temperature;
    double  Humidity;
    double  WaterTemperature;
    char    Comfort[16];
    int     UVindex;
    double  RainRate;
    double  TotalRain;
    int     Barometer;
    int     Prediction;
    double  WindDirection;
    double  Speed;
    double  SpeedAverage;
    bool    haveBattery;
    bool    haveTemperature;
    bool    haveHumidity;   
    bool    haveWaterTemperature;
    bool    haveComfort;
    bool    haveUVindex;
    bool    haveRain;
    bool    haveBarometer;
    bool    havePrediction;
    bool    haveWind;
    bool    isValid;
    char    packet[128];
    virtual bool decode(char * _str) = 0;
  protected:
    int getIntFromChar(char c);
    int getIntFromString(char *);
    int char2int(char);
    int _batteryIndication(char battery);
    int _wrBattery(char battery);
    void _decodeRain(char * message);
    void _decodeRainInch(char * message);
    void _decodeTemperature(char * message);
    void _decodeWaterTemperature(char * message);
    void _decodeTemperatureHumidity(char * message);
    void _decodeTemperatureHumidityBarometer(char * message);
    void _decodeWind(char * message);
    void _decodeWind968(char * message);
    void _decodeWind928(char * message);
  public:
    oregon(char * _strval);
    bool isBattery();
    bool isTemperature();
    bool isHumidity();   
    bool isWaterTemperature();
    bool isComfort();
    bool isUVindex();
    bool isRain();
    bool isBarometer();
    bool isPrediction();
    bool isWind();
    bool isDecoded();           
    time_t getLastUpdate();
    char * getName();
    char * getID();
    int getChannel();
    int getRollingCode();
    int getBattery();
    double getTemperature();
    double getHumidity();
    double getWaterTemperature();
    char * getComfort(); 
    int getUVindex(); 
    double getRainRate();
    double getTotalRain();
    int getBarometer();
    int getPrediction(); 
    double getWindDirection();
    double getSpeed();        
    double getSpeedAverage();    
    char * getPacket(); 
    static oregon * getRightOregon(char * s);
};

class OregonV2V3 : public oregon {
  public:
    OregonV2V3(char * _strval);
  private:
    bool decode(char * _str);
    bool validate(char * _str, int _len, int _CRC, int _SUM);
};
} /* namespace w3rpi */

#endif /* OREGON_H_ */
