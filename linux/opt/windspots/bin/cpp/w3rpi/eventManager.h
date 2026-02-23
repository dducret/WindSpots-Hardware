#ifndef EVENTMANAGER_H_
#define EVENTMANAGER_H_
#include <pthread.h>
#include <list>
#include <iostream>
#include <string>
#include <sqlite3.h>
#include "../lib/bmp280/bmp280.h"
#include "../lib/ina219/ina219.h"
#include "../lib/ads1015/ads1015.h"

#define w3rpi_EVENT_NONE        0
#define w3rpi_EVENT_INIT        1
#define w3rpi_EVENT_GETSENSORDATA   2
#define w3rpi_EVENT_MAX_SIZE      1024  // Maximum event message size
#define MESSAGE_SIZE      256

class EventManager {
  private:
    class Event {
      public:
        int eventType;
        char strValue[w3rpi_EVENT_MAX_SIZE];
    };
    std::list<Event> eventList;           // Event queue
    pthread_t      myThread;              // Thread handle
    pthread_mutex_t  eventListMutex;       // Mutex for queue
    pthread_cond_t   eventCond;            // Condition variable for new events
    char * piId;
    volatile bool running;                // Flag to control thread loop

    static void * eventLoop(void *);
    std::string logFileName;
    std::string tmpFileName;
    char message[MESSAGE_SIZE];
    int altitude;
    int direction_correction; 
    bool bTemperature;
    bool bAnemometer;
    bool bSolar;
    bool bDebug;
    bmp280 *myBmp280; 
    ina219 *inaBattery0;
    ina219 *inaSolar0;
    ina219 *ina5v;
    ads1015 *ads;

    int anemometerCounter;
    long int fastestCount;
    long int firstCount;
    long int lastCount;
    sqlite3 *db;
    char sql[256];
    char sql2[256];
    char currentTime[32];
  public:
    EventManager(char * _piId);
    ~EventManager();
    bool init(std::string log, std::string tmp, int altitude, int direction, bool temperature, bool anemometer, bool solar);
    void logIt();
    void enqueue(int newEvent, char * _strValue);
    void anemometerAdd();
    int readadc(int port);
    double getWindDirection();
    double getGust();
    double getThermistor();
    double getBatteryLevel();
    float getBatteryPower();
    float getSolarPower();
    float get5vPower();
    int getBarometerSealevel();
    bool isRunning() const;
    void store(const char * _name, int channel, double battery, double temperature, double humidity, int barometer, double wind_direction, double wind_speed, double wind_speed_average);
};
#endif /* EVENTMANAGER_H_ */
