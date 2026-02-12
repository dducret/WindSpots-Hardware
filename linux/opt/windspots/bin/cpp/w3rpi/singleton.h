#ifndef SINGLETON_H_
#define SINGLETON_H_

#include "core_433.h"
#include "eventManager.h"

class Singleton {
protected:
  core_433    * core433;
  EventManager* eventManager;
  static Singleton* m_instance;
public:
  Singleton();
  ~Singleton();
  core_433* getCore433();
  EventManager* getEventManager();
  static void init();
  static Singleton* get();
};

#endif
