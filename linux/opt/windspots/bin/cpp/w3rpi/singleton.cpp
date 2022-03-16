// Singleton.cpp
#include <cstdio>
#include <cstring>
#include <iostream>
#include "singleton.h"
Singleton * Singleton::m_instance = NULL;
Singleton::Singleton() {
  this->core433 = NULL;
  this->eventManager = NULL;
}
Singleton::~Singleton() {
  delete this->core433;
  delete this->eventManager;
}
core_433      * Singleton::getCore433() { return this->core433; }
EventManager  * Singleton::getEventManager() { return this->eventManager; }
void Singleton::init() {
  Singleton * s = new Singleton();
  // Start 433MHz reception
  s->core433 = new core_433();
  // Event Manager ..
  s->eventManager = new EventManager((char *)"rfrpi0");                   // rpi Id
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