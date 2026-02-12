#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "w3rpi.h"
#include "core_433.h"
#include "eventManager.h"
#include "singleton.h"

using namespace std;

core_433::core_433() : running(true) {
  mySwitch = new switch_433();
  if(pthread_create(&myThread, NULL, receptionLoop, this) != 0) {
    std::cerr << "Error: Unable to create reception thread." << std::endl;
    running = false;
  }
}

core_433::~core_433() {
  stop();
  delete mySwitch;
  mySwitch = nullptr;
}

void core_433::stop() {
  running = false;
  // Join thread if it is running
  pthread_join(myThread, NULL);
}

// Thread function to manage reception
void * core_433::receptionLoop(void * _param) {
  core_433 * myCore = static_cast<core_433*>(_param);
  switch_433 * mySwitch = myCore->mySwitch;
  char _tmpStr[RCSWITCH_MAX_MESS_SIZE];
  while(myCore->running) {
    // Scan for sensor code
    if (mySwitch->getOokCode(_tmpStr)) {
      if (w3rpi_debug)
        std::cout << "w3rpi core_433::receptionLoop() - received message [" << _tmpStr << "]" << std::endl;
      if (strlen(_tmpStr) > 4) {
        if (Singleton::get() != NULL) {
          Singleton::get()->getEventManager()->enqueue(w3rpi_EVENT_GETSENSORDATA, _tmpStr);
        }
      }
    }
    usleep(5000); // 5ms sleep before next poll
  }
  return nullptr;
}
void core_433::loop() {
  pthread_join(myThread, NULL);
}