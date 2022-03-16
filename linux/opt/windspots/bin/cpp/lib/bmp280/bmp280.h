// bmp280.h
#ifndef BMP280_H_
#define BMP280_H_
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../linux/i2c-dev.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
//
class bmp280 {
  private:
    double temperature;
    double temperatureF;
    double pressure;
    double altitude;
  public:
    bmp280();
    ~bmp280();
    bool update(void);
    float getTemperature(void);
    float getTemperatureF(void);
    float getPressure(void);
    float getAltitude(float seaLevelhPa);
};
#endif /* BMP280_H_ */