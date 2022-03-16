// core433.cpp
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "core_433.h"
#include "eventManager.h"
#include "singleton.h"
using namespace std;
core_433::core_433() {
   mySwitch = NULL;
   mySwitch = new switch_433();
   pthread_create(&myThread,NULL, receptionLoop, this);
}
core_433::~core_433() {
  mySwitch = NULL;
}
// Thread to manage reception
void * core_433::receptionLoop( void * _param ) {
  core_433 * myCore = (core_433 *) _param;
  switch_433 * mySwitch = myCore->mySwitch;
  char _tmpStr[RCSWITCH_MAX_MESS_SIZE];
  while(1) {
    // Scan for sensor code
    if ( mySwitch->getOokCode(_tmpStr) ) {
      #ifdef TRACECORE433
        std::cout << "* core_433::receptionLoop() - received message [" << _tmpStr << "]" << std::endl;
      #endif
      if ( _tmpStr && strlen(_tmpStr) > 4 ) {
        if ( Singleton::get() != NULL ) {
           Singleton::get()->getEventManager()->enqueue(w3rpi_EVENT_GETSENSORDATA,_tmpStr);
        }
      }
    }
    usleep(5000); // 5 ms sleep before next pooling
  }
}
void core_433::loop( void ) {
  pthread_join(myThread,NULL);
}