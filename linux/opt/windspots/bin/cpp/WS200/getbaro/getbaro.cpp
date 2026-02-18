#include <iostream>
#include <string>
#include <cstdio> // printf()
#include <math.h> // pow()
#include "bmp280.h"
//
int main(int argc, char *argv[]) {
	int altitude = 374;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-h") || (arg == "--help")) {
    	std::cerr << "Usage: getbaro <option(s)>\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-a,--altitude\t\tAltitude in meter\n"
              << std::endl;
    	return 0;
    } 
    if ((arg == "-a") || (arg == "--altitude")) {
      if (i + 1 < argc) { 
          altitude = atoi(argv[i+1]); 
          if(altitude < 0 || altitude > 3500) {
          	std::cerr << "altitude should be from 0 to 3500." << std::endl;
          	return 1;
          }
      } else { 
          std::cerr << "--altitude option requires one argument." << std::endl;
          return 2;
      }  
    } 
  }
	bmp280 *myBmp280;
	myBmp280 = new bmp280();
	// Get data
	if( !myBmp280->update() ) {
    printf("getbaro - Bmp280 error");
  }
  float pressure = myBmp280->getPressure();
  float temperature = myBmp280->getTemperature();
  float sealevel = (double)pressure / (double)pow(1.0 - (double)altitude / 44330.0, 5.255);
  float altitudeRef = myBmp280->getAltitude(1013.25);
  // Output data to screen
  printf("PRESSURE=%.0f\n", pressure);
  printf("SEALEVEL=%.0f\n", sealevel);
  printf("TEMPERATURE=%.1f\n", temperature);
  printf("ALTITUDE1013=%.0f\n", altitudeRef);
  printf("ALTITUDE=%u\n", altitude);
  delete myBmp280;
}