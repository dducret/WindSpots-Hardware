/* ==========================================================================
 * w3ble.cpp
 * --------------------------------------------------------------------------
 * WindSpots 3 - RaspBerry Bluetooth Anemometer
 * --------------------------------------------------------------------------
 * source https://github.com/edrosten/libblepp
 */
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include "singleton.h"
#include "version.h"
using namespace std;
//  
static void show_usage(std::string name) {
    std::cerr << "Usage: w3ble <option(s)>\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-a,--address\t\tMAC address of Scarlet ANemometer (ie: 88:6b:0f:58:b5:88)\n"
              << "\t-d,--dir-adjustment\tDirection adjustement (0, 360)\n"
              << "\t-l,--log\t\tLog file default:/var/log\n"
              << "\t-t,--tmp\t\tLog file default:/var/tmp\n"
              << "\t-d,--debug\t\tDebug default:N\n"
              << std::endl;
}
int main(int argc, char *argv[]) {
  std::vector <std::string> sources;
  std::string address = "88:6b:0f:58:b5:88";
  std::string direction = "0";
  std::string tmp = "/var/tmp";
  std::string log = "/var/log";
  char first;
  bool debug = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-h") || (arg == "--help")) {
        show_usage(argv[0]);
        return 0;
    } 
    if ((arg == "-a") || (arg == "--address")) {
      if (i + 1 < argc) { 
          address = argv[i+1];
      } else { 
          std::cerr << "--address option requires one argument, format 88:6b:0f:58:b5:88." << std::endl;
          return 1;
      }  
    } 
    if ((arg == "-d") || (arg == "--dir-adjustment")) {
      if (i + 1 < argc) { 
          direction = argv[i+1]; 
      } else { 
          std::cerr << "--dir-adjustment option requires one argument." << std::endl;
          return 1;
      }  
    } 
    if (arg == "--debug") {
      if (i + 1 < argc) { 
          first = argv[i+1][0];
          first = toupper(first);
          if(first == 'Y') {
            debug = true;
          } 
      } else { 
          std::cerr << "--debug option requires one argument." << std::endl;
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
  Singleton::init();
  // Create Register event right after initialization
  int iDirection = atoi(direction.c_str());
  printf("w3ble Version (%d.%d) Branch (%s) Build date(%u)\n",W3BLE_VERSION_MAJOR,W3BLE_VERSION_MINOR,W3BLE_VERSION_BRANCH,&W3BLE_BUILD_DATE);
  if(debug)
  	printf("w3ble Initialize log:%s, tmp:%s, address:%s, direction:%d, Debug: %u\n",log.c_str(),tmp.c_str(),address.c_str(),iDirection,debug);
  Singleton::get()->getEventManager()->init(log, tmp, address, iDirection, debug);
  // infinite loop
  Singleton::get()->getCoreble()->loop();
  delete Singleton::get();
  exit(0);
}