// bmp280.cpp
#include "bmp280.h"
//
bmp280::bmp280() {
  update();
}
bmp280::~bmp280() {
}
bool bmp280::update(void) {
  // Create I2C bus
  int file;
  const char *bus = "/dev/i2c-1";
  if((file = open(bus, O_RDWR)) < 0) {
    fprintf(stdout, "bmp280.cpp: Failed to open the bus. \n");
    return false;
  }
  // Get I2C device, BMP280 I2C address is 0x77
  if(ioctl(file, I2C_SLAVE, 0x77) < 0) {
    close(file);
    fprintf(stdout, "bmp280.cpp: Error while configuring the slave address 0x77. Error: %s\n", strerror(errno));
    return false;
  }
  // Read 24 bytes of data from address(0x88)
  char reg[1] = {0x88};
  if(write(file, reg, 1) <0) {
    close(file);
    fprintf(stdout, "bmp280.cpp: Error(2) : write Error \n");
    return false;
  }
  char data[24] = {0};
  if(read(file, data, 24) != 24) {
    close(file);
    fprintf(stdout, "bmp280.cpp: Error(3) : read Error \n");
    return false;
  }
  // Convert the data
  // temp coefficents
  int dig_T1 = data[1] * 256 + data[0];
  int dig_T2 = data[3] * 256 + data[2];
  if(dig_T2 > 32767) {
    dig_T2 -= 65536;
  }
  int dig_T3 = data[5] * 256 + data[4];
  if(dig_T3 > 32767) {
    dig_T3 -= 65536;
  }
  // pressure coefficents
  int dig_P1 = data[7] * 256 + data[6];
  int dig_P2  = data[9] * 256 + data[8];
  if(dig_P2 > 32767) {
    dig_P2 -= 65536;
  }
  int dig_P3 = data[11]* 256 + data[10];
  if(dig_P3 > 32767) {
    dig_P3 -= 65536;
  }
  int dig_P4 = data[13]* 256 + data[12];
  if(dig_P4 > 32767) {
    dig_P4 -= 65536;
  }
  int dig_P5 = data[15]* 256 + data[14];
  if(dig_P5 > 32767) {
    dig_P5 -= 65536;
  }
  int dig_P6 = data[17]* 256 + data[16];
  if(dig_P6 > 32767) {
    dig_P6 -= 65536;
  }
  int dig_P7 = data[19]* 256 + data[18];
  if(dig_P7 > 32767) {
    dig_P7 -= 65536;
  }
  int dig_P8 = data[21]* 256 + data[20];
  if(dig_P8 > 32767) {
    dig_P8 -= 65536;
  }
  int dig_P9 = data[23]* 256 + data[22];
  if(dig_P9 > 32767) {
    dig_P9 -= 65536;
  }
  // Select control measurement register(0xF4)
  // Normal mode, temp and pressure over sampling rate = 1(0x27)
  char config[2] = {0};
  config[0] = 0xF4;
  config[1] = 0x27;
  if(write(file, config, 2) < 0) {
    printf("bmp280.cpp: Error(3) : write Error 1 \n");
    close(file);
    return false;
  }
  // Select config register(0xF5)
  // Stand_by time = 1000 ms(0xA0)
  config[0] = 0xF5;
  config[1] = 0xA0;
  if(write(file, config, 2) < 0) {
    printf("bmp280.cpp: Error(3) : write Error 2 \n");
    close(file);
    return false;
  }
  usleep(100);
  // Read 8 bytes of data from register(0xF7)
  // pressure msb1, pressure msb, pressure lsb, temp msb1, temp msb, temp lsb, humidity lsb, humidity msb
  reg[0] = 0xF7;
  if(write(file, reg, 1) < 0) {
    close(file);
    printf("bmp280.cpp: Error(4) : Input/output Error \n");
    return false;
  }
  if(read(file, data, 8) != 8) {
    close(file);
    printf("bmp280.cpp: Error(5) : Input/output Error \n");
    return false;
  }
  // Convert pressure and temperature data to 19-bits
  long adc_p = (((long)data[0] * 65536) + ((long)data[1] * 256) + (long)(data[2] & 0xF0)) / 16;
  long adc_t = (((long)data[3] * 65536) + ((long)data[4] * 256) + (long)(data[5] & 0xF0)) / 16;
  // Temperature offset calculations
  double var1 = (((double)adc_t) / 16384.0 - ((double)dig_T1) / 1024.0) * ((double)dig_T2);
  double var2 = ((((double)adc_t) / 131072.0 - ((double)dig_T1) / 8192.0) *(((double)adc_t)/131072.0 - ((double)dig_T1)/8192.0)) * ((double)dig_T3);
  double t_fine = (long)(var1 + var2);
  double cTemp = (var1 + var2) / 5120.0;
  this->temperature = cTemp;
  double fTemp = cTemp * 1.8 + 32;
  this->temperatureF = fTemp;
  // Pressure offset calculations
  var1 = ((double)t_fine / 2.0) - 64000.0;
  var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
  var2 = var2 + var1 * ((double)dig_P5) * 2.0;
  var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
  var1 = (((double) dig_P3) * var1 * var1 / 524288.0 + ((double) dig_P2) * var1) / 524288.0;
  var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);
  double p = 1048576.0 - (double)adc_p;
  p = (p - (var2 / 4096.0)) * 6250.0 / var1;
  var1 = ((double) dig_P9) * p * p / 2147483648.0;
  var2 = p * ((double) dig_P8) / 32768.0;
  this->pressure = (p + (var1 + var2 + ((double)dig_P7)) / 16.0) / 100;
  close(file);
  return true;
}
float bmp280::getTemperature() {
    return temperature;
}
float bmp280::getTemperatureF() {
    return temperatureF;
}
float bmp280::getPressure() {
  return pressure;
}
float bmp280::getAltitude(float seaLevelhPa) {
  altitude = 44330 * (1.0 - pow(this->pressure / seaLevelhPa, 0.1903));
  return altitude;
}