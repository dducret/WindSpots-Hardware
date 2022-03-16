// oregon.h
#ifndef OREGON_H_
#define OREGON_H_
namespace w3rpi {
class oregon {
  protected:
    time_t  lastUpdate;   // objectCreation time
    char    Name[16];     // name of the sensor
    char    ID[5];        // Sensor ID
    int     Channel;
    int     RollingCode;
    int     Battery;   // 0= empty 1=ok >=10 percentage full
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
    // true if flag set
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
    bool    isValid;          // true when chaecksum is valid and other value valid
    char    packet[128];     // packet string
    virtual bool decode ( char * _str) = 0 ;    // decode the string and set the variable
  protected:
    int getIntFromChar(char c);         // transform a Hex value in char into a number
    int getIntFromString(char *);       // transform a Hex value in String into a number
    int char2int(char );
    int  _batteryIndication(char battery);
    int  _wrBattery(char battery);
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
    oregon( char * _strval );   // construct and decode value
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
    int    getChannel();
    int    getRollingCode();
    int    getBattery();
    double getTemperature();
    double getHumidity();
    double getWaterTemperature();
    char * getComfort(); 
    int    getUVindex(); 
    double getRainRate();
    double getTotalRain();
    int    getBarometer();
    int    getPrediction(); 
    double getWindDirection();
    double getSpeed();        
    double getSpeedAverage();    
    char * getPacket(); 
    static oregon * getRightOregon(char * s); // wrapper for child class
};
class OregonV2V3 : public oregon {
  public :
    OregonV2V3( char * _strval );
  private:
    bool decode( char * _str );        // wrapper to right decode method
    bool validate(char * _str, int _len, int _CRC, int _SUM); // Verify CRC & CKSUM
};
} /* namespace w3rpi */
#endif /* OREGON_H_ */