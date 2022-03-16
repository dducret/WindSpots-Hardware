#include "eventManager.h"
#include "singleton.h"
using namespace std;
#define NEAREST(number, multiple) (((number) + ((multiple) / 2)) / (multiple) * (multiple))
EventManager::EventManager(char * _piId) {
  eventListMutex = PTHREAD_MUTEX_INITIALIZER;
  piId = _piId;
  pthread_create(&myThread,NULL, eventLoop, this);
}
EventManager::~EventManager() {
  delete [] message;
  delete [] sql;
  delete [] sql2;
  delete [] currentTime;
  pthread_cancel(myThread);
  pthread_join(myThread, NULL);
  myThread = pthread_self();
}
bool EventManager::init(std::string log, std::string tmp, std::string address, int direction, bool b_debug) {
  message = new char[128];
  sql = new char[256];
  sql2 = new char[128];
  int rc;
  currentTime = new char[20];
  logFileName.assign(log);
  logFileName.append("/windspots.log");
  tmpFileName.assign(tmp);
  tmpFileName.append("/ws.db");
  addressMAC.assign(address);
  direction_correction = direction;
  bDebug = b_debug;
  logIt("");
  sprintf(message,"Program Starting. log:%s, tmp:%s, address:%s, direction-correction:%d, Debug: %u", log.c_str(), tmp.c_str(), address.c_str(), direction, b_debug);
  logIt(message);
  if(bDebug)
    printf("w3ble EventManager::init - %s\n",message);
  // create databse if no exist
  if(sqlite3_open(this->tmpFileName.c_str(), &db)) {
    sprintf(message,"Can't open database: %s, error: %s", this->tmpFileName.c_str(), sqlite3_errmsg(db));
    logIt(message);
    if(bDebug)
      printf("w3ble EventManager::init %s\n",message);
  return false;
  }
  const char *sql;
  sql = "CREATE TABLE IF NOT EXISTS data (id INTEGER PRIMARY KEY AUTOINCREMENT, last_update DATE, name TEXT, sensor_id TEXT, channel INTEGER, rollingcode INTEGER, battery TEXT, temperature TEXT, temperature_sign TEXT, relative_humidity TEXT, comfort TEXT, uv_index TEXT, rain_rate TEXT, total_rain TEXT, barometer TEXT, prediction TEXT, wind_direction TEXT, wind_speed TEXT, wind_speed_average TEXT)";
  rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
  if( rc != SQLITE_OK) {
    printf("w3ble EventManager::init SQL error: %s\n", sqlite3_errmsg(db));
  }
  sql = "CREATE INDEX IF NOT EXISTS i1 ON data(last_update)";
  rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
  if( rc != SQLITE_OK) {
    printf("w3ble EventManager::init SQL error: %s\n", sqlite3_errmsg(db));
  }
  sql = "CREATE TABLE IF NOT EXISTS log (id INTEGER PRIMARY KEY AUTOINCREMENT, last_update DATE)";
  rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
  if( rc != SQLITE_OK) {
    printf("w3ble EventManager::init SQL error: %s\n", sqlite3_errmsg(db));
  }
  rc = sqlite3_close(db);
  if( rc != SQLITE_OK) {
    printf("w3ble EventManager::init SQL error: %s\n", sqlite3_errmsg(db));
  }
  return true;  
}
void EventManager::store(char *name, int channel, double battery, double temperature, double humidity, int barometer, 
                           double windDirection, double windSpeed, double windSpeedAverage) {
  if(bDebug)
    printf("w3ble EventManager::store\n");
  char *err_msg = NULL;
  int rc;
  timeval tp;
  gettimeofday(&tp, 0);
  time_t curtime = tp.tv_sec;
  tm *t = localtime(&curtime);
  sprintf(currentTime,"%04d-%02d-%02d %02d:%02d:%02d", (1900 + t->tm_year), (t->tm_mon + 1), t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
  if(sqlite3_open(this->tmpFileName.c_str(), &db)) {
    sprintf(message,"Can't open database: %s, error: %s", this->tmpFileName.c_str(), sqlite3_errmsg(db));
    logIt(message);
    if(bDebug)
      printf("w3ble EventManager::store %s\n", message);
    return;
  }
  sprintf(sql,"INSERT INTO data (last_update, name, channel, battery, temperature, temperature_sign, relative_humidity, barometer, wind_direction, wind_speed, wind_speed_average) VALUES ( ");
  sprintf(sql2, "'%s', '%s', %d, '%0.1f', '%0.1f', '0', '%0.f', '%d', '%0.1f', '%0.2f', '%0.2f');", currentTime, name, channel, battery, temperature, humidity, barometer, windDirection, windSpeed, windSpeedAverage);
  strcat(sql, sql2);
  if(bDebug)
    printf("w3ble EventManager::store SQL:%s\n",sql);
  rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
  if( rc != SQLITE_OK) {
    printf("w3ble EventManager::store SQL Exec error: %s\n", err_msg);
  }
  sqlite3_close(db);
  if( rc != SQLITE_OK) {
    printf("w3ble EventManager::store SQL Close error: %s\n", sqlite3_errmsg(db));
  }
}
bool EventManager::isDebug() {
  return bDebug;
}
std::string EventManager::getAddressMAC() {
  if(bDebug)
    printf("w3ble EventManager::getAddressMAC - %s\n",addressMAC.c_str());
  return addressMAC.c_str();
}

void EventManager::logIt(char const *message) {
  std::ofstream log(this->logFileName.c_str(), std::ofstream::out | std::ofstream::app);
  timeval tp;
  gettimeofday(&tp, 0);
  time_t curtime = tp.tv_sec;
  tm *t = localtime(&curtime);
  char *time = new char[16];
  sprintf(time,"%02d:%02d:%02d:%03d", t->tm_hour, t->tm_min, t->tm_sec, tp.tv_usec/1000);
  log << time << " w3ble " << message << std::endl;
  log.close();
  delete [] time;
}
// Event enqueuing
void EventManager::enqueue(int newEvent, char * _strValue) {
  Event ev;
  ev.eventType = newEvent;
  if ( _strValue != NULL ) {
    if ( strlen(_strValue) > W3BLE_EVENT_MAX_SIZE-1 ) {
      if(bDebug)
        printf("w3ble EventManager::enqueue - (%s) TOO LONG \n",_strValue);
    }
    strncpy( ev.strValue, _strValue, W3BLE_EVENT_MAX_SIZE-1);
  } else ev.strValue[0] = '\0';
  pthread_mutex_lock( &this->eventListMutex );
  if(bDebug)
    printf("w3ble EventManager::enqueue (%s)\n",_strValue);
  this->eventList.push_back(ev);
  pthread_mutex_unlock( &this->eventListMutex );
}
// Thread to manage events ...
void * EventManager::eventLoop( void * _param ) {
  EventManager * myEventManager = (EventManager *) _param;
  struct timeval tp;
  char *token;
  const char *search = ";";
  int ble_count = 0;
  double ble_windSpeed = 0;
  double ble_windGust = 0;
  double ble_windGustComputed = 0;
  double ble_direction = 0;
  double ble_speed = 0;
  double ble_gust = 0;  
  double ble_gustSpeed = 0;
  long int ble_previousMs = 0;
  long int ble_currentMs = 0;
  
  // get first timestamp  
  gettimeofday(&tp, NULL);
  ble_previousMs = tp.tv_sec * 1000 + tp.tv_usec / 1000;
  // loop            
  while(1) {
    while ( ! myEventManager->eventList.empty() ) {
      pthread_mutex_lock( &myEventManager->eventListMutex );
      Event ev = myEventManager->eventList.front();
      myEventManager->eventList.pop_front();
      pthread_mutex_unlock( &myEventManager->eventListMutex );
      switch (ev.eventType) {
        // INIT OF MODULE - check leds & RX / TX
        case W3BLE_EVENT_INIT:
          if(myEventManager->bDebug)
            printf("w3ble EventManager::eventLoop INIT\n");
          if ( Singleton::get() ) {
            printf("[INFO] W3BLE Started ! \n");
          }
          break;
        // Just received an event from the RF module
        case W3BLE_EVENT_GETSENSORDATA:
          if(myEventManager->bDebug)
              printf("w3ble EventManager::eventLoop GETSENSORDATA %s", ev.strValue);
          token = strtok(ev.strValue, search);          
          ble_direction = atoi(token);
          token = strtok(NULL, search);
          ble_speed = atoi(token);
          // sscanf(ev.strValue,"%d;%d",&ble_direction,&ble_speed);
          ble_speed = ble_speed/10;
          if(myEventManager->bDebug)
              printf(" Direction: %0.f Speed: %0.2f\n", ev.strValue,ble_direction,ble_speed); // added to previous message
          // collect data
          ble_windSpeed += ble_speed;
          ++ble_count;
          // gust on 
          if(ble_speed > ble_gust)
            ble_gust = ble_speed;
          // publish every 15 seconds
          gettimeofday(&tp, NULL);
          ble_currentMs = tp.tv_sec * 1000 + tp.tv_usec / 1000;
          // printf("microsec(%d): %d\n",ble_count, (ble_currentMs - ble_previousMs));
          if((ble_currentMs - ble_previousMs) >= 15000) {
            ble_previousMs = ble_currentMs;
            if(ble_windSpeed > (double)1 )
              ble_windGustComputed = ble_windSpeed / (double)ble_count;
            sprintf(myEventManager->message,"WS300-0 - Direction:%0.f Wind Speed(%0.2f):%0.2f Average Speed:%0.2f",
                ble_direction, ble_speed, ble_gust, ble_windGustComputed);
            myEventManager->logIt(myEventManager->message);
            if(myEventManager->bDebug)
              printf("w3ble EventManager::eventLoop %s\n",myEventManager->message);
            myEventManager->store((char *)"WS300", 0, 0, 0, 0.0, 0, ble_direction, ble_gust, ble_windGustComputed);
            ble_windSpeed = ble_windGustComputed = ble_count = ble_gust = 0;
          }
          break;
        default:
          if(myEventManager->bDebug)
            printf("w3ble EventManager::eventLoop UNKNOWN (%s) \n",ev.strValue);
          break;
      }
    }
    usleep(300000); // 300 ms de repos c'est pas un mal
  }
  return NULL;
}