#include <iostream>
#include <string>
#include <cstdio> // printf()
#include <cstdlib> // atoi()
#include <cmath>  // pow()
#include "bmp280.h"
#include "bmp388.h"

namespace {
enum class SensorType {
  BMP388,
  BMP280
};

bool parseSensor(const std::string &value, SensorType &sensor) {
  if (value == "bmp388") {
    sensor = SensorType::BMP388;
    return true;
  }

  if (value == "bmp280") {
    sensor = SensorType::BMP280;
    return true;
  }

  return false;
}

const char *sensorName(const SensorType sensor) {
  return sensor == SensorType::BMP388 ? "bmp388" : "bmp280";
}
}

int main(int argc, char *argv[]) {
	int altitude = 374;
	SensorType sensor = SensorType::BMP388; // Default sensor
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-h") || (arg == "--help")) {
    	std::cerr << "Usage: getbaro <option(s)>\n"
                << "Options:\n"
                << "\t-h,--help\t\tShow this help message\n"
                << "\t-a,--altitude\t\tAltitude in meter\n"
                << "\t-s,--sensor\t\tSensor type: bmp388 (default) or bmp280\n"
                << std::endl;
      return 0;
    } 
    if ((arg == "-a") || (arg == "--altitude")) {
      if (i + 1 < argc) {
        altitude = std::atoi(argv[++i]);
        if (altitude < 0 || altitude > 3500) {
          std::cerr << "altitude should be from 0 to 3500." << std::endl;
          return 1;
        }
      } else {
        std::cerr << "--altitude option requires one argument." << std::endl;
        return 2;
      }
      continue;
    }

    if ((arg == "-s") || (arg == "--sensor")) {
      if (i + 1 < argc) {
        if (!parseSensor(argv[++i], sensor)) {
          std::cerr << "Unknown sensor type. Use bmp388 or bmp280." << std::endl;
          return 3;
        }
      } else {
        std::cerr << "--sensor option requires one argument." << std::endl;
        return 4;
      }
      continue;
    }
  }

  float pressure = 0;
  float temperature = 0;
  float altitudeRef = 0;
  bool ok = false;

  if (sensor == SensorType::BMP388) {
    bmp388 myBmp;
    ok = myBmp.update();
    pressure = myBmp.getPressure();
    temperature = myBmp.getTemperature();
    altitudeRef = myBmp.getAltitude(1013.25);
  } else {
    bmp280 myBmp;
    ok = myBmp.update();
    pressure = myBmp.getPressure();
    temperature = myBmp.getTemperature();
    altitudeRef = myBmp.getAltitude(1013.25);
  }
  if (!ok) {
    printf("getbaro - %s read error", sensorName(sensor));
    return 1;
  }

  float sealevel = static_cast<float>(
      static_cast<double>(pressure) /
      std::pow(1.0 - static_cast<double>(altitude) / 44330.0, 5.255));
  
  // Output data to screen
  printf("SENSOR=%s\n", sensorName(sensor));
  printf("PRESSURE=%.0f\n", pressure);
  printf("SEALEVEL=%.0f\n", sealevel);
  printf("TEMPERATURE=%.1f\n", temperature);
  printf("ALTITUDE1013=%.0f\n", altitudeRef);
  printf("ALTITUDE=%u\n", altitude);
  return 0;
}
