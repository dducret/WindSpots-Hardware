// core433.h
#ifndef CORE_433_H_
#define CORE_433_H_
#include <pthread.h>
#include <unistd.h>
#include "switch_433.h"
#include "ook_433.h"
class core_433 {
  private:
    switch_433   * mySwitch;  // Used for RF433 communication
    pthread_t    myThread;  // Thread structure
    static void * receptionLoop(void *);  // Thread function
  public:
    core_433();
    ~core_433();
    void loop (void);
};
#endif /* CORE_433_H_ */