// bmp388.cpp
#include "bmp388.h"

bmp388::bmp388() {
  update();
}

bmp388::~bmp388() {
}

bool bmp388::readCalibration(int file) {
  char reg[1] = {0x31};
  if(write(file, reg, 1) < 0) {
    fprintf(stdout, "bmp388.cpp: Error: write calibration register address failed.\n");
    return false;
  }

  unsigned char calib[21] = {0};
  if(read(file, calib, 21) != 21) {
    fprintf(stdout, "bmp388.cpp: Error: reading calibration data failed.\n");
    return false;
  }

  unsigned short raw_t1 = (unsigned short)((calib[1] << 8) | calib[0]);
  unsigned short raw_t2 = (unsigned short)((calib[3] << 8) | calib[2]);
  signed char raw_t3 = (signed char)calib[4];

  short raw_p1 = (short)((calib[6] << 8) | calib[5]);
  short raw_p2 = (short)((calib[8] << 8) | calib[7]);
  signed char raw_p3 = (signed char)calib[9];
  signed char raw_p4 = (signed char)calib[10];
  unsigned short raw_p5 = (unsigned short)((calib[12] << 8) | calib[11]);
  unsigned short raw_p6 = (unsigned short)((calib[14] << 8) | calib[13]);
  signed char raw_p7 = (signed char)calib[15];
  signed char raw_p8 = (signed char)calib[16];
  short raw_p9 = (short)((calib[18] << 8) | calib[17]);
  signed char raw_p10 = (signed char)calib[19];
  signed char raw_p11 = (signed char)calib[20];

  par_t1 = ((double)raw_t1) / 0.00390625;
  par_t2 = ((double)raw_t2) / 1073741824.0;
  par_t3 = ((double)raw_t3) / 281474976710656.0;

  par_p1 = (((double)raw_p1) - 16384.0) / 1048576.0;
  par_p2 = (((double)raw_p2) - 16384.0) / 536870912.0;
  par_p3 = ((double)raw_p3) / 4294967296.0;
  par_p4 = ((double)raw_p4) / 137438953472.0;
  par_p5 = ((double)raw_p5) / 0.125;
  par_p6 = ((double)raw_p6) / 64.0;
  par_p7 = ((double)raw_p7) / 256.0;
  par_p8 = ((double)raw_p8) / 32768.0;
  par_p9 = ((double)raw_p9) / 281474976710656.0;
  par_p10 = ((double)raw_p10) / 281474976710656.0;
  par_p11 = ((double)raw_p11) / 36893488147419103232.0;

  return true;
}

bool bmp388::update(void) {
  int file;
  const char *bus = "/dev/i2c-1";

  if((file = open(bus, O_RDWR)) < 0) {
    fprintf(stdout, "bmp388.cpp: Failed to open the bus.\n");
    return false;
  }

  if(ioctl(file, I2C_SLAVE, 0x77) < 0) {
    close(file);
    fprintf(stdout, "bmp388.cpp: Error while configuring the slave address 0x77. Error: %s\n", strerror(errno));
    return false;
  }

  // Verify chip id (BMP388 should return 0x50)
  char reg[1] = {0x00};
  if(write(file, reg, 1) < 0) {
    close(file);
    fprintf(stdout, "bmp388.cpp: Error: write chip-id register address failed.\n");
    return false;
  }

  unsigned char chip_id = 0;
  if(read(file, &chip_id, 1) != 1) {
    close(file);
    fprintf(stdout, "bmp388.cpp: Error: reading chip-id failed.\n");
    return false;
  }

  if(chip_id != 0x50) {
    close(file);
    fprintf(stdout, "bmp388.cpp: Unexpected chip-id 0x%02X (expected 0x50).\n", chip_id);
    return false;
  }

  if(!readCalibration(file)) {
    close(file);
    return false;
  }

  // Enable pressure + temperature and set normal mode
  char config[2] = {0};
  config[0] = 0x1B; // PWR_CTRL
  config[1] = 0x33; // press_en + temp_en + normal mode
  if(write(file, config, 2) < 0) {
    close(file);
    fprintf(stdout, "bmp388.cpp: Error: write PWR_CTRL failed.\n");
    return false;
  }

  // Oversampling: pressure x4, temperature x2
  config[0] = 0x1C; // OSR
  config[1] = 0x0A;
  if(write(file, config, 2) < 0) {
    close(file);
    fprintf(stdout, "bmp388.cpp: Error: write OSR failed.\n");
    return false;
  }

  // IIR filter coefficient 3
  config[0] = 0x1F; // CONFIG
  config[1] = 0x02;
  if(write(file, config, 2) < 0) {
    close(file);
    fprintf(stdout, "bmp388.cpp: Error: write CONFIG failed.\n");
    return false;
  }

  usleep(50000);

  // Read pressure + temperature raw data (6 bytes starting at 0x04)
  reg[0] = 0x04;
  if(write(file, reg, 1) < 0) {
    close(file);
    fprintf(stdout, "bmp388.cpp: Error: selecting data register failed.\n");
    return false;
  }

  unsigned char data[6] = {0};
  if(read(file, data, 6) != 6) {
    close(file);
    fprintf(stdout, "bmp388.cpp: Error: reading raw data failed.\n");
    return false;
  }

  unsigned long uncomp_press = ((unsigned long)data[2] << 16) | ((unsigned long)data[1] << 8) | (unsigned long)data[0];
  unsigned long uncomp_temp = ((unsigned long)data[5] << 16) | ((unsigned long)data[4] << 8) | (unsigned long)data[3];

  // Temperature compensation
  double partial_data1 = ((double)uncomp_temp) - par_t1;
  double partial_data2 = partial_data1 * par_t2;
  this->temperature = partial_data2 + (partial_data1 * partial_data1) * par_t3;
  this->temperatureF = this->temperature * 1.8 + 32.0;

  // Pressure compensation
  partial_data1 = par_p6 * this->temperature;
  partial_data2 = par_p7 * this->temperature * this->temperature;
  double partial_data3 = par_p8 * this->temperature * this->temperature * this->temperature;
  double out1 = par_p5 + partial_data1 + partial_data2 + partial_data3;

  partial_data1 = par_p2 * this->temperature;
  partial_data2 = par_p3 * this->temperature * this->temperature;
  partial_data3 = par_p4 * this->temperature * this->temperature * this->temperature;
  double out2 = ((double)uncomp_press) * (par_p1 + partial_data1 + partial_data2 + partial_data3);

  partial_data1 = ((double)uncomp_press) * ((double)uncomp_press);
  partial_data2 = par_p9 + par_p10 * this->temperature;
  partial_data3 = partial_data1 * partial_data2;
  double partial_data4 = partial_data3 + partial_data1 * ((double)uncomp_press) * par_p11;

  // Convert Pa -> hPa
  this->pressure = (out1 + out2 + partial_data4) / 100.0;

  close(file);
  return true;
}

float bmp388::getTemperature() {
  return temperature;
}

float bmp388::getTemperatureF() {
  return temperatureF;
}

float bmp388::getPressure() {
  return pressure;
}

float bmp388::getAltitude(float seaLevelhPa) {
  altitude = 44330 * (1.0 - pow(this->pressure / seaLevelhPa, 0.1903));
  return altitude;
}