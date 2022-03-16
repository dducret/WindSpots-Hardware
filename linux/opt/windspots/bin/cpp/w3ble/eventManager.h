// eventManager.h
#ifndef EVENTMANAGER_H_
#define EVENTMANAGER_H_

#include <vector> 
#include <math.h> 

#include <pthread.h>
#include <list>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <sys/time.h>
#include <sqlite3.h>
#define W3BLE_EVENT_NONE        0
// Receiving a sensor Data
#define W3BLE_EVENT_INIT        1
#define W3BLE_EVENT_GETSENSORDATA   2
// Maximum size for an event message
#define W3BLE_EVENT_MAX_SIZE      512
class EventManager {
  private:
    class Event {
      public:
        int eventType;
        char strValue[W3BLE_EVENT_MAX_SIZE];
    };
    std::list<Event> eventList;     // event queue
    pthread_t      myThread;      // Thread structure
    pthread_mutex_t  eventListMutex;  // ensure queuing / dequeuing well managed
    char * piId;
    static void * eventLoop(void *);  // Thread function
    std::string logFileName;
    std::string tmpFileName;
    std::string addressMAC;
    char * message;
    int direction_correction; 
    bool bDebug;
    sqlite3 *db;
    char * sql;
    char * sql2;
    char * currentTime;
  public:
    EventManager(char * _piId);
    ~EventManager();
    bool init(std::string log, std::string tmp, std::string address, int direction, bool debug);
    void logIt(char const * message);
    std::string getAddressMAC();
    bool isDebug();
    void enqueue(int newEvent, char * _strValue);     // Add an event with eventQueue type
    void store(char *name, int channel, double battery, double temperature, double humidity, int barometer, double wind_direction, double wind_speed, double wind_speed_average);
};
#endif /* EVENTMANAGER_H_ */