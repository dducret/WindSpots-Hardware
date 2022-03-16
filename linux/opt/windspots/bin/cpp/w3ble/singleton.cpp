// Singleton.cpp
#include <cstdio>
#include <cstring>
#include <iostream>
#include "singleton.h"
Singleton * Singleton::m_instance = NULL;
Singleton::Singleton() {
  this->coreble = NULL;
  this->eventManager = NULL;
}
Singleton::~Singleton() {
  delete this->coreble;
  delete this->eventManager;
}
core_ble      * Singleton::getCoreble() { return this->coreble; }
EventManager  * Singleton::getEventManager() { return this->eventManager; }
void Singleton::init() {
  Singleton * s = new Singleton();
  // Start BLE reception
  s->coreble = new core_ble();
  // Event Manager ..
  s->eventManager = new EventManager((char *)"w3ble");                   // w3ble Id
  Singleton::m_instance = s;
}
// Get instance
Singleton * Singleton::get() {
  if( Singleton::m_instance != NULL ) {
   return Singleton::m_instance;
  } else {
   std::cout << "Singleton::get() - singleton not initialized" << std::endl;
   return NULL;
  }
}