#include <cstdio>
#include <cstring>
#include <iostream>
#include "singleton.h"

Singleton * Singleton::m_instance = NULL;

Singleton::Singleton() {
	core433 = nullptr;
	eventManager = nullptr;
}

Singleton::~Singleton() {
	delete core433;
	delete eventManager;
}

core_433 * Singleton::getCore433() { return core433; }
EventManager * Singleton::getEventManager() { return eventManager; }

void Singleton::init() {
	Singleton * s = new Singleton();
	s->core433 = new core_433();
	s->eventManager = new EventManager((char *)"rfrpi0");
	m_instance = s;
}

Singleton * Singleton::get() {
	if(m_instance != NULL)
		return m_instance;
	else {
		std::cout << "Singleton::get() - singleton not initialized" << std::endl;
		return NULL;
	}
}