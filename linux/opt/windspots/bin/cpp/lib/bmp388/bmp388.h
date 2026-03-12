// bmp388.h
#ifndef BMP388_H_
#define BMP388_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../linux/i2c-dev.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>

class bmp388 {
  private:
    double temperature;
    double temperatureF;
    double pressure;
    double altitude;

    // Compensation parameters
    double par_t1;
    double par_t2;
    double par_t3;
    double par_p1;
    double par_p2;
    double par_p3;
    double par_p4;
    double par_p5;
    double par_p6;
    double par_p7;
    double par_p8;
    double par_p9;
    double par_p10;
    double par_p11;

    bool readCalibration(int file);

  public:
    bmp388();
    ~bmp388();
    bool update(void);
    float getTemperature(void);
    float getTemperatureF(void);
    float getPressure(void);
    float getAltitude(float seaLevelhPa);
};

#endif /* BMP388_H_ */