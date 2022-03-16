/* ==========================================================================
 * w3rpi.cpp
 * --------------------------------------------------------------------------
 * WindSpots 2 - RaspBerry PI Cape
 * inspired by disk91 - Paul Pinault see : http://www.disk91.com/?p=1323
 * --------------------------------------------------------------------------
 * to check:  more /dev/kmw3rpi
 */
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>
#include "singleton.h"
#include "version.h"
using namespace std;
//  
static void show_usage() {
    std::cerr << "Usage: w3rpi <option(s)>\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-d,--dir-adjustment\tDirection adjustement (0, 360)\n"
              << "\t-a,--altitude\t\tAltitude in meter\n"
              << "\t-l,--log\t\tLog file default:/var/log\n"
              << "\t-t,--tmp\t\tLog file default:/var/tmp\n"
              << std::endl;
}
void count() { // Wind Speed
  Singleton::get()->getEventManager()->anemometerAdd();
  return;
}
void handleNoInterrupt() {
}
int main(int argc, char *argv[]) {
  std::vector <std::string> sources;
  std::string direction = "0";
  std::string altitude = "0";
  std::string tmp = "/var/tmp";
  std::string log = "/var/log";
  char first;
  bool radio = false; // radio option is just for information due to the fact that infinity loop of w3rpi is on 433
  bool anemometer = false;
  bool temperature = false;
  bool solar = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-h") || (arg == "--help")) {
        show_usage();
        return 0;
    } 
    if ((arg == "-d") || (arg == "--dir-adjustment")) {
      if (i + 1 < argc) { 
          direction = argv[i+1]; 
      } else { 
          std::cerr << "--dir-adjustment option requires one argument." << std::endl;
          return 1;
      }  
    } 
    if ((arg == "-a") || (arg == "--altitude")) {
      if (i + 1 < argc) { 
          altitude = argv[i+1]; 
      } else { 
          std::cerr << "--altitude option requires one argument." << std::endl;
          return 1;
      }  
    } 
    if ((arg == "-r") || (arg == "--radio")) {
      if (i + 1 < argc) { 
          first = argv[i+1][0];
          first = toupper(first);
          if(first == 'Y') {
            radio = true;
          } 
      } else { 
          std::cerr << "--radio option requires one argument." << std::endl;
          return 1;
      }  
    } 
    if ((arg == "-n") || (arg == "--anemometer")) {
      if (i + 1 < argc) { 
          first = argv[i+1][0];
          first = toupper(first);
          if(first == 'Y') {
            anemometer = true;
          } 
      } else { 
          std::cerr << "--anemometer option requires one argument." << std::endl;
          return 1;
      }  
    } 
    if ((arg == "-p") || (arg == "--temperature")) {
      if (i + 1 < argc) { 
          first = argv[i+1][0];
          first = toupper(first);
          if(first == 'Y') {
            temperature = true;
          } 
      } else { 
          std::cerr << "--temperature option requires one argument." << std::endl;
          return 1;
      }  
    } 
    if ((arg == "-o") || (arg == "--solar")) {
      if (i + 1 < argc) { 
          first = argv[i+1][0];
          first = toupper(first);
          if(first == 'Y') {
            solar = true;
          } 
      } else { 
          std::cerr << "--solar option requires one argument." << std::endl;
          return 1;
      }  
    } 
    if ((arg == "-l") || (arg == "--log")) {
      if (i + 1 < argc) { 
          log = argv[i+1];
      } else { 
          std::cerr << "--log option requires one argument." << std::endl;
          return 1;
      }  
    } 
    if ((arg == "-t") || (arg == "--tmp")) {
      if (i + 1 < argc) { 
          tmp = argv[i+1];
      } else { 
          std::cerr << "--tmp option requires one argument." << std::endl;
          return 1;
      }  
    } 
  }
  // wiring Pi startup
  // if(wiringPiSetupGpio() == -1) {
  if(wiringPiSetupSys() == -1) {
    printf("Error during wiringPi Initialization\n");
    exit(1);
  }
  Singleton::init();
  // Create Register event right after initialization
  int iAltitude = atoi(altitude.c_str());
  int iDirection = atoi(direction.c_str());
  Singleton::get()->getEventManager()->init(log, tmp, iAltitude, iDirection, radio, temperature, anemometer, solar);
  // write information to log
  // printf("w3rpi Version (%d.%d) Branch (%s) Build date(%u)",W3RPI_VERSION_MAJOR,W3RPI_VERSION_MINOR,W3RPI_VERSION_BRANCH,&W3RPI_BUILD_DATE);
  // printf("w3rpi Initialize log:%s, tmp:%s, altitude:%d, direction:%d, 433:%u, Temp:%u, Anemo:%u, Solar: %u",log.c_str(),tmp.c_str(),iAltitude,iDirection,radio,temperature,anemometer,solar);
  // Wind Speed
  if(anemometer) {
    pinMode(17,INPUT);
    wiringPiISR(17, INT_EDGE_RISING, &count);
  }
  //    
  // infinite loop
  Singleton::get()->getCore433()->loop();
  // loop ended, then
  printf("w3rpi END ****************");
  delete Singleton::get();
  if(anemometer) wiringPiISR(17, INT_EDGE_RISING, &handleNoInterrupt);
  exit(0);
}