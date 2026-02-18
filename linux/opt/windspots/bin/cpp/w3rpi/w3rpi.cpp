/* ==========================================================================
 * w3rpi_no433.cpp
 * --------------------------------------------------------------------------
 * WindSpots 2 - RaspBerry PI Cape (without 433MHz receiver loop)
 * --------------------------------------------------------------------------
 */
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <pigpio.h>
#include "w3rpi.h"
#include "eventManager.h"

using namespace std;

bool w3rpi_debug;
static EventManager * g_event_manager = NULL;

static void show_usage() {
    std::cerr << "Usage: w3rpi_no433 <option(s)>\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-d,--dir-adjustment\tDirection adjustement (0, 360)\n"
              << "\t-a,--altitude\t\tAltitude in meter\n"
              << "\t-l,--log\t\tLog file default:/var/log\n"
              << "\t-t,--tmp\t\tLog file default:/var/tmp\n"
              << "\t--debug\t\tDebug mode\n"
              << std::endl;
}

void count(int gpio, int level, uint32_t tick) { // Wind Speed
  (void)gpio;
  (void)tick;
  if (level != 1) {
    return;
  }
  if (g_event_manager != NULL) {
    g_event_manager->anemometerAdd();
  }
}

int main(int argc, char *argv[]) {
  std::string direction = "0";
  std::string altitude = "0";
  std::string tmp = "/var/tmp";
  std::string log = "/var/log";
  char first;
  bool anemometer = false;
  bool temperature = false;
  bool solar = false;
  w3rpi_debug = false;

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
    if (arg == "--debug") {
      if (i + 1 < argc) {
          first = argv[i+1][0];
          first = toupper(first);
          if(first == 'Y') {
            w3rpi_debug = true;
          }
      } else {
          std::cerr << "--debug." << std::endl;
          return 1;
      }
    }
  }

  if(gpioInitialise() < 0) {
    printf("Error during pigpio Initialization\n");
    exit(1);
  }

  EventManager eventManager((char *)"rfrpi0");
  g_event_manager = &eventManager;

  int iAltitude = atoi(altitude.c_str());
  int iDirection = atoi(direction.c_str());
  eventManager.init(log, tmp, iAltitude, iDirection, false, temperature, anemometer, solar);

  if(anemometer) {
    gpioSetMode(17, PI_INPUT);
    gpioSetPullUpDown(17, PI_PUD_UP);
    gpioSetAlertFunc(17, &count);
  }

  while (1) {
    sleep(1);
  }

  if(anemometer) {
    gpioSetAlertFunc(17, NULL);
  }
  gpioTerminate();
  return 0;
}