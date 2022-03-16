// Singleton.h
#ifndef SINGLETON_H_
#define SINGLETON_H_
#include "core_ble.h"
#include "eventManager.h"
class Singleton {
protected:
  core_ble      * coreble;
  EventManager    * eventManager;
  static  Singleton * m_instance;
public:
  Singleton();
  ~Singleton();
  core_ble    * getCoreble();
  EventManager  * getEventManager();
  static void init();
  static Singleton * get();
};
#endif 