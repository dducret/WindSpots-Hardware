/* ==========================================================================
 * w3rpi
 * --------------------------------------------------------------------------
 * WindSpots 3 - RaspBerry PI Cape (without 433MHz receiver loop)
 * --------------------------------------------------------------------------
 */
#include <iostream>
#include <string>
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include "w3rpi.h"
#include "eventManager.h"

using namespace std;

bool w3rpi_debug;
static EventManager * g_event_manager = NULL;

static void show_usage() {
    std::cerr << "Usage: w3rpi <option(s)>\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-s,--station\t\tStation identifier\n"
              << "\t-d,--dir-adjustment\tDirection adjustement (0, 360)\n"
              << "\t-a,--altitude\t\tAltitude in meter\n"
              << "\t-n,--anemometer\t\tAnemometer enabled (Y/N)\n"
              << "\t-p,--temperature\tTemperature enabled (Y/N)\n"
              << "\t-o,--solar\t\tSolar monitoring enabled (Y/N)\n"
              << "\t-l,--log\t\tLog file default:/var/log\n"
              << "\t-t,--tmp\t\tLog file default:/var/tmp\n"
              << "\t--debug\t\t\tDebug mode (Y/N)\n"
              << std::endl;
}

void runAnemometerListener() {
  const unsigned int lineOffset = 17;
  int chipFd = -1;
  int eventFd = -1;
  char selectedChip[64] = {0};

  for (int chipIndex = 0; chipIndex < 8 && eventFd < 0; ++chipIndex) {
    char chipPath[64];
    snprintf(chipPath, sizeof(chipPath), "/dev/gpiochip%d", chipIndex);
    chipFd = open(chipPath, O_RDONLY);
    if (chipFd < 0) {
      continue;
    }

    struct gpioevent_request req;
    memset(&req, 0, sizeof(req));
    req.lineoffset = lineOffset;
    req.handleflags = GPIOHANDLE_REQUEST_INPUT;
#ifdef GPIOHANDLE_REQUEST_BIAS_PULL_UP
    req.handleflags |= GPIOHANDLE_REQUEST_BIAS_PULL_UP;
#endif
    req.eventflags = GPIOEVENT_REQUEST_RISING_EDGE;
    snprintf(req.consumer_label, sizeof(req.consumer_label), "w3rpi");

    if (ioctl(chipFd, GPIO_GET_LINEEVENT_IOCTL, &req) == 0) {
      eventFd = req.fd;
      snprintf(selectedChip, sizeof(selectedChip), "%s", chipPath);
      close(chipFd);
      chipFd = -1;
      break;
    }

    close(chipFd);
    chipFd = -1;
  }

  if (eventFd < 0) {
    std::cerr << "Error: unable to request GPIO line 17 events from /dev/gpiochip*." << std::endl;
    return;
  }
  if (w3rpi_debug) {
    std::cerr << "Anemometer listening on " << selectedChip << " line " << lineOffset << std::endl;
  }

  struct pollfd pfd;
  pfd.fd = eventFd;
  pfd.events = POLLIN;

  struct timespec lastPulseTs;
  clock_gettime(CLOCK_MONOTONIC, &lastPulseTs);

  while (true) {
    int status = poll(&pfd, 1, -1);
    if (status <= 0) {
      if (status < 0) {
        std::cerr << "Error: GPIO poll failed (" << strerror(errno) << ")." << std::endl;
      }
      continue;
    }

    if ((pfd.revents & POLLIN) == 0) {
      continue;
    }

    struct gpioevent_data eventData;
    ssize_t bytesRead = read(eventFd, &eventData, sizeof(eventData));
    if (bytesRead != sizeof(eventData)) {
      continue;
    }

    if (eventData.id == GPIOEVENT_EVENT_RISING_EDGE) {
      struct timespec now;
      clock_gettime(CLOCK_MONOTONIC, &now);
      long deltaUs = (now.tv_sec - lastPulseTs.tv_sec) * 1000000L +
                     (now.tv_nsec - lastPulseTs.tv_nsec) / 1000L;
      if (deltaUs >= 5000L) {
        if (g_event_manager != NULL) {
          g_event_manager->anemometerAdd();
        }
        lastPulseTs = now;
      }
    }
  }
  close(eventFd);
}
int main(int argc, char *argv[]) {
  std::string station = "CHGE99";
  std::string direction = "0";
  std::string altitude = "0";
  std::string tmp = "/var/tmp";
  std::string log = "/var/log";
  bool anemometer = false;
  bool temperature = false;
  bool solar = false;
  w3rpi_debug = false;
  char first;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    bool knownOption = false;
    if ((arg == "-h") || (arg == "--help")) {
        show_usage();
        return 0;
    }
    if ((arg == "-s") || (arg == "--station")) {
      knownOption = true;
      if (i + 1 < argc) {
        station = argv[i + 1];
      } else {
        std::cerr << "--station option requires one argument." << std::endl;
        return 1;
      }
    }
    if ((arg == "-d") || (arg == "--dir-adjustment")) {
      knownOption = true;
      if (i + 1 < argc) {
        direction = argv[i+1];
      } else {
        std::cerr << "--dir-adjustment option requires one argument." << std::endl;
        return 1;
      }
    }
    if ((arg == "-a") || (arg == "--altitude")) {
    	knownOption = true;
      if (i + 1 < argc) {
          altitude = argv[i+1];
      } else {
          std::cerr << "--altitude option requires one argument." << std::endl;
          return 1;
      }
    }
    if ((arg == "-n") || (arg == "--anemometer")) {
      knownOption = true;
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
      knownOption = true;
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
      knownOption = true;
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
      knownOption = true;
      if (i + 1 < argc) {
          log = argv[i+1];
      } else {
          std::cerr << "--log option requires one argument." << std::endl;
          return 1;
      }
    }
    if ((arg == "-t") || (arg == "--tmp")) {
      knownOption = true;
      if (i + 1 < argc) {
          tmp = argv[i+1];
      } else {
          std::cerr << "--tmp option requires one argument." << std::endl;
          return 1;
      }
    }
    if (arg == "--debug") {
      knownOption = true;
      if (i + 1 < argc) {
          first = argv[i+1][0];
          first = toupper(first);
          if(first == 'Y') {
            w3rpi_debug = true;
          }
      } else {
          std::cerr << "--debug option requires one argument." << std::endl;
          return 1;
      }
    }
    
    if (!knownOption && !arg.empty() && arg[0] == '-') {
      std::cerr << "Unknown option: " << arg << std::endl;
      show_usage();
      return 1;
    }
  }
  
  EventManager eventManager((char *)"rfrpi0");
  g_event_manager = &eventManager;

  int iAltitude = atoi(altitude.c_str());
  int iDirection = atoi(direction.c_str());
  eventManager.init(log, tmp, iAltitude, iDirection, temperature, anemometer, solar);

  std::thread anemometerThread;
  if(anemometer) {
    anemometerThread = std::thread(runAnemometerListener);
    anemometerThread.detach();
  }

  while (eventManager.isRunning()) {
    sleep(1);
  }
	if(!anemometer) {
    std::cerr << "No anemometer, the program will be restarted with health-check.sh in 2 minutes" << std::endl;
  }
  return 0;
}
