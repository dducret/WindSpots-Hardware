/* ==========================================================================
 * w3rpi.cpp
 * --------------------------------------------------------------------------
 * WindSpots 2 - Raspberry Pi Cape using pigpio
 * --------------------------------------------------------------------------
 */

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <pigpio.h>
#include "w3rpi.h"
#include "singleton.h"
#include <cctype>  // For toupper

using namespace std;

bool w3rpi_debug = false;

static void show_usage() {
    cerr << "Usage: w3rpi <option(s)>\n"
         << "Options:\n"
         << "\t-h,--help\t\tShow this help message\n"
         << "\t-d,--dir-adjustment\tDirection adjustment (0, 360)\n"
         << "\t-a,--altitude\t\tAltitude in meters\n"
         << "\t-l,--log\t\tLog file directory (default: /var/log)\n"
         << "\t-t,--tmp\t\tTemporary file directory (default: /var/tmp)\n"
         << "\t--debug\t\t\tDebug mode (Y/N)\n"
         << "\t-r,--radio\t\tRadio option (Y/N)\n"
         << "\t-n,--anemometer\t\tAnemometer option (Y/N)\n"
         << "\t-p,--temperature\tTemperature option (Y/N)\n"
         << "\t-o,--solar\t\tSolar option (Y/N)\n"
         << endl;
}

void count() { // Wind Speed callback
  Singleton::get()->getEventManager()->anemometerAdd();
}

void handleNoInterrupt() {
  // Dummy no-op.
}

int main(int argc, char *argv[]) {
  vector<string> sources;
  string direction = "0";
  string altitude = "0";
  string tmp = "/var/tmp";
  string log = "/var/log";
  char first;
  bool radio = false;
  bool anemometer = false;
  bool temperature = false;
  bool solar = false;

  // Initialize pigpio.
  if(gpioInitialise() < 0) {
    cerr << "pigpio initialization failed" << endl;
    exit(1);
  }

  // Process command-line arguments.
  for (int i = 1; i < argc; ++i) {
    string arg = argv[i];
    if(arg == "-h" || arg == "--help") {
      show_usage();
      return 0;
    }
    if(arg == "-d" || arg == "--dir-adjustment") {
      if(i + 1 < argc) {
          direction = argv[++i];
      } else {
          cerr << "--dir-adjustment option requires one argument." << endl;
          return 1;
      }
    }
    if(arg == "-a" || arg == "--altitude") {
      if(i + 1 < argc) {
          altitude = argv[++i];
      } else {
          cerr << "--altitude option requires one argument." << endl;
          return 1;
      }
    }
    if(arg == "-r" || arg == "--radio") {
      if(i + 1 < argc) {
          first = argv[++i][0];
          radio = (toupper(first) == 'Y');
      } else {
          cerr << "--radio option requires one argument." << endl;
          return 1;
      }
    }
    if(arg == "-n" || arg == "--anemometer") {
      if(i + 1 < argc) {
          first = argv[++i][0];
          anemometer = (toupper(first) == 'Y');
      } else {
          cerr << "--anemometer option requires one argument." << endl;
          return 1;
      }
    }
    if(arg == "-p" || arg == "--temperature") {
      if(i + 1 < argc) {
          first = argv[++i][0];
          temperature = (toupper(first) == 'Y');
      } else {
          cerr << "--temperature option requires one argument." << endl;
          return 1;
      }
    }
    if(arg == "-o" || arg == "--solar") {
      if(i + 1 < argc) {
          first = argv[++i][0];
          solar = (toupper(first) == 'Y');
      } else {
          cerr << "--solar option requires one argument." << endl;
          return 1;
      }
    }
    if(arg == "-l" || arg == "--log") {
      if(i + 1 < argc) {
          log = argv[++i];
      } else {
          cerr << "--log option requires one argument." << endl;
          return 1;
      }
    }
    if(arg == "-t" || arg == "--tmp") {
      if(i + 1 < argc) {
          tmp = argv[++i];
      } else {
          cerr << "--tmp option requires one argument." << endl;
          return 1;
      }
    }
    if(arg == "--debug") {
      if(i + 1 < argc) {
          first = argv[++i][0];
          w3rpi_debug = (toupper(first) == 'Y');
      } else {
          cerr << "--debug option requires one argument." << endl;
          return 1;
      }
    }
  }

  // Initialize singleton and event manager.
  Singleton::init();
  int iAltitude = atoi(altitude.c_str());
  int iDirection = atoi(direction.c_str());
  Singleton::get()->getEventManager()->init(log, tmp, iAltitude, iDirection, radio, temperature, anemometer, solar);

  // Setup anemometer interrupt if enabled.
  if(anemometer) {
    gpioSetMode(17, PI_INPUT);
    // Use pigpio alert function for pin 17.
    gpioSetAlertFunc(17, reinterpret_cast<gpioAlertFunc_t>(&count));
  }

  // Enter main loop for 433MHz reception (this call blocks until termination).
  Singleton::get()->getCore433()->loop();

  cerr << "w3rpi END ****************" << endl;
  delete Singleton::get();

  if(anemometer)
    gpioSetAlertFunc(17, NULL);

  gpioTerminate();
  exit(0);
}
