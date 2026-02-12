#ifndef CORE_433_H_
#define CORE_433_H_
#include <pthread.h>
#include <unistd.h>
#include "switch_433.h"
#include "ook_433.h"

class core_433 {
private:
  switch_433   * mySwitch;   // RF433 communication
  pthread_t      myThread;   // Thread handle
  volatile bool running;     // Flag to control the thread loop
  static void * receptionLoop(void *);
public:
  core_433();
  ~core_433();
  void stop();             // Signal thread to stop and join it
  void loop();
};
#endif /* CORE_433_H_ */
