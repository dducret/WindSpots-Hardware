#include <iostream>
#include <fstream> 
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h> 
#include <sys/time.h>
#include <sqlite3.h>
#include "w3rpi.h"
#include "eventManager.h"
#include "singleton.h"
#include "oregon.h"
using namespace std;
using namespace w3rpi;
#define NEAREST(number, multiple) (((number) + ((multiple) / 2)) / (multiple) * (multiple))
EventManager::EventManager(char * _piId) {
  eventListMutex = PTHREAD_MUTEX_INITIALIZER;
  piId = _piId;
  pthread_create(&myThread,NULL, eventLoop, this);
}
EventManager::~EventManager() {
  delete myBmp280;
  delete inaBattery0;
  delete inaSolar0;
  delete ina5v;
  delete ads;
  pthread_cancel(myThread);
  pthread_join(myThread, NULL);
  myThread = pthread_self();
}
bool EventManager::init(std::string log, std::string tmp, int anemometer_altitude, int direction, bool b_radio, bool b_temperature, bool b_anemometer, bool b_solar) {
  anemometerCounter = 0;
  fastestCount = 0;
  firstCount = 0;
  lastCount = 0;
  int rc = 0;
  logFileName.assign(log);
  logFileName.append("/windspots.log");
  tmpFileName.assign(tmp);
  tmpFileName.append("/ws.db");
  altitude = anemometer_altitude;
  direction_correction = direction;
  bRadio = b_radio;
  bTemperature = b_temperature;
  bAnemometer = b_anemometer;
  bSolar = b_solar;
  sprintf(message,"Program Starting. log:%s, tmp:%s, altitude:%d, dir-correction:%d, 433:%u, Temp:%u, Anemo:%u, Solar: %u", log.c_str(), tmp.c_str(), altitude, direction, b_radio, b_temperature, b_anemometer, b_solar);
  logIt();
  if(w3rpi_debug) {
    sprintf(message,"Debug Mode");
    logIt();
  }
  // i2c sensors
  myBmp280 = new bmp280();
  inaBattery0 = new ina219(0x41);
  inaSolar0 = new ina219(0x40);
  ina5v = new ina219(0x43);
  ads = new ads1015(0x48);
  // create databse if no exist
  if(sqlite3_open(this->tmpFileName.c_str(), &db)) {
    sprintf(message,"Can't open database: %s, error: %s", this->tmpFileName.c_str(), sqlite3_errmsg(db));
    logIt();
    return false;
  }
  const char *sql;
  sql = "CREATE TABLE IF NOT EXISTS data (id INTEGER PRIMARY KEY AUTOINCREMENT, last_update DATE, name TEXT, sensor_id TEXT, channel INTEGER, rollingcode INTEGER, battery TEXT, temperature TEXT, temperature_sign TEXT, relative_humidity TEXT, comfort TEXT, uv_index TEXT, rain_rate TEXT, total_rain TEXT, barometer TEXT, prediction TEXT, wind_direction TEXT, wind_speed TEXT, wind_speed_average TEXT)";
  rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
  if( rc != SQLITE_OK) {
    sprintf(message,"w3rpi EventManager::init SQL error: %s\n", sqlite3_errmsg(db));
    logIt();
    return false;
  }
  sql = "CREATE INDEX IF NOT EXISTS i1 ON data(last_update)";
  rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
  if( rc != SQLITE_OK) {
    sprintf(message,"w3rpi EventManager::init SQL error: %s\n", sqlite3_errmsg(db));
    logIt();
    return false;
  }
  sql = "CREATE TABLE IF NOT EXISTS log (id INTEGER PRIMARY KEY AUTOINCREMENT, last_update DATE)";
  rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
  if( rc != SQLITE_OK) {
    sprintf(message,"w3rpi EventManager::init SQL error: %s\n", sqlite3_errmsg(db));
    logIt();
    return false;
  }
  rc = sqlite3_close(db);
  if( rc != SQLITE_OK) {
    sprintf(message,"w3rpi EventManager::init SQL error: %s\n", sqlite3_errmsg(db));
    logIt();
    return false;
  }
  return true;  
}
void EventManager::store(const char * _name, int channel, double battery, double temperature, double humidity, int barometer, 
                           double windDirection, double windSpeed, double windSpeedAverage) {
  if( _name == NULL) {
    if(w3rpi_debug)
      printf("w3rpi EventManager::store _name == NULL\n");
    return;
  }
  // if no data => no record
  if(battery == 0 && temperature == 0 && humidity == 0 && barometer == 0 && windDirection == 0 && windSpeed == 0 && windSpeedAverage == 0) {
    sprintf(message,"%s-%d - Heartbeat", _name, channel); 
    return;
  }
  char *err_msg = NULL;
  timeval tp;
  gettimeofday(&tp, 0);
  time_t curtime = tp.tv_sec;
  tm *t = localtime(&curtime);
  sprintf(currentTime,"%04d-%02d-%02d %02d:%02d:%02d", (1900 + t->tm_year), (t->tm_mon + 1), t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
  if(sqlite3_open(this->tmpFileName.c_str(), &db)) {
    sprintf(message,"Can't open database: %s, error: %s", this->tmpFileName.c_str(), sqlite3_errmsg(db));
    logIt();
    return;
  }
  sprintf(sql,"INSERT INTO data (last_update, name, channel, battery, temperature, temperature_sign, relative_humidity, barometer, wind_direction, wind_speed, wind_speed_average) VALUES ( ");
  sprintf(sql2, "'%s', '%s', %d, '%0.1f', '%0.1f', '0', '%0.f', '%u', '%0.1f', '%0.2f', '%0.2f');", 
                  currentTime, _name, channel, battery, temperature, humidity, barometer, windDirection, windSpeed, windSpeedAverage);
  strcat(sql, sql2);
  if(w3rpi_debug)
    printf("w3rpi EventManager::store sql:%s\n\n",sql);
  if( sqlite3_exec(db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
    sprintf(message,"w3rpi EventManager::store SQL error: %s\n", err_msg);
    logIt();
    return;
  }
  sqlite3_close(db);
  if(battery != 0) {
    sprintf(message, "%s-%d - Accus:%0.0f%%",  _name, channel, battery); 
  } else {
    sprintf(message, "%s-%d - Accus:none", _name, channel); 
  }
  if(temperature != 0) {
    snprintf(message + strlen(message), MESSAGE_SIZE - strlen(message), ", Temp:%0.1f", temperature);  
  }
  if(humidity != 0) {
    snprintf(message + strlen(message), MESSAGE_SIZE - strlen(message), ", Hum:%0.2f%%", humidity); 
  }
  if(barometer != 0) {
    snprintf(message + strlen(message), MESSAGE_SIZE - strlen(message), ", Baro:%u", barometer); 
  }
  if(windDirection != 0 || windSpeed != 0 || windSpeedAverage != 0) {
    snprintf(message + strlen(message), MESSAGE_SIZE - strlen(message), ", Dir:%0.f, Speed:%0.2f km/h, Average:%0.2f km/h", windDirection, (windSpeed * 3.6), (windSpeedAverage * 3.6)); 
  }
  logIt();
}
void EventManager::anemometerAdd() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
  // printf("[tick]\n");
  if(firstCount == 0) {
    firstCount = ms;
    lastCount = ms;
    fastestCount = 9999;
    ++anemometerCounter;
    return;
  }
  long int currentDelta = ms - lastCount;
  if(currentDelta < 8) // probably jittering 8 = 125,7 m/s
    return;
  ++anemometerCounter;
  if(currentDelta < fastestCount) {
    fastestCount = currentDelta;
  }
  lastCount = ms;
}
double EventManager::getWindDirection(){
  ads->setGain(GAIN_ONE);
  int adc_value =  ads->readADC_SingleEnded(0); // port 0 ADS1015
  double direction = adc_value * 360 / (double) 1648;
  if( direction_correction > 0)
      direction =  direction_correction + direction; 
  direction = NEAREST(direction,5);
  if(direction > 360)
      direction = direction - 360;
  if( direction == 360)
    direction = 0;
  return direction;    
}
double EventManager::getGust(){
  // 1600 tour = 1609.344 m/heure = 1.00584 - vitesse en  m/s = distance en m / temps en s
  // float time =(float) currentDelta / (float) 1000;
  // float speed = 1.00584 / time;
  // printf("fastest: %d\n", fastestCount);
  double gust = 1.00584 / ((double)fastestCount / (double) 1000);
  return gust;
}
double EventManager::getThermistor(){
  double temperature = 0;
  ads->setGain(GAIN_ONE);
  int adc_value = ads->readADC_SingleEnded(1); // port 1 ADS1015
  double volts = (adc_value * 3.3) / 1648; // calculate the voltage base on 3.3v
  if(w3rpi_debug)
    printf("w3rpi EventManager::getThermistor volts = %0.2f\n",volts);
  if(volts > 0) {
    double ohm = round((3.3 - volts) / volts * 10000);
    // http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm
    // 3D
    double a =  0.0009333357965;
    double b =  0.0002340542448;
    double c =  0.00000008044859863;
    // Steinhart Hart Equation
    double log_r  = log(ohm);
    double log_r3 = log_r * log_r * log_r;
    double temp = 1.0 / (a + b * log_r + c * log_r3);
    // double temp= 1 / (a + b * log(ohm) + c * pow(log(ohm), 3));
    temperature=round(temp-273.15); 
  }
  return temperature;
}
double EventManager::getBatteryLevel(){
  double level = 0;
  level = inaBattery0->getBusVoltage_V();
  return level;
}
float EventManager::getBatteryPower(){
  float current_mA = inaBattery0->getBusVoltage_V() * (inaBattery0->getCurrent_mA() * 8.2);
  return current_mA;
}
float EventManager::getSolarPower(){
  float current_mA = inaSolar0->getBusVoltage_V() * inaSolar0->getCurrent_mA();
  return current_mA;
}
float EventManager::get5vPower(){
  float current_mA = ina5v->getBusVoltage_V() * ina5v->getCurrent_mA();
  return current_mA;
}
int EventManager::getBarometerSealevel(){
  double barometer = 0;
  int sealevel  = 0;
  // bmp280
  if( !myBmp280->update() ) {
    if(w3rpi_debug)
   		printf("w3rpi EventManager::eventLoop - Bmp280 error\n");
    sprintf(message,"w3rpi EventManager::eventLoop - Bmp280 error\n");
    logIt();
    return 0;
  }
  barometer = myBmp280->getPressure();
  if(w3rpi_debug)
      printf("w3rpi EventManager::getBarometerSealevel barometer = %0.f\n", barometer);
  if(barometer<700)
    return 0;
  // altitude correction
  sealevel = (int)((double)barometer / (double)pow(1.0 - (double)altitude / 44330.0, 5.255));
  return sealevel;
}
void EventManager::logIt() {
  timeval tp;
  gettimeofday(&tp, 0);
  time_t curtime = tp.tv_sec;
  tm *t = localtime(&curtime);
  sprintf(currentTime, "%02d:%02d:%02d:%03d", t->tm_hour, t->tm_min, t->tm_sec, (int)tp.tv_usec/1000);
  std::ofstream log(this->logFileName.c_str(), std::ofstream::out | std::ofstream::app);
  log << currentTime << " w3rpi " << message << std::endl;
  log.close();
}
// Event enqueuing
void EventManager::enqueue(int newEvent, char * _strValue) {
  Event ev;
  ev.eventType = newEvent;
  if ( _strValue != NULL ) {
    if ( strlen(_strValue) > w3rpi_EVENT_MAX_SIZE-1 ) {
      if(w3rpi_debug)
        printf("w3rpi EventManager::enqueue (%s) TOO LONG \n",_strValue);
    }
    strncpy( ev.strValue, _strValue, w3rpi_EVENT_MAX_SIZE-1);
  } else ev.strValue[0] = '\0';
  pthread_mutex_lock( &this->eventListMutex );
  if(w3rpi_debug)
    printf("w3rpi EventManager::enqueue (%s)\n",_strValue);
  this->eventList.push_back(ev);
  pthread_mutex_unlock( &this->eventListMutex );
}
// Thread to manage events ...
void * EventManager::eventLoop(void * _param ) {
  EventManager * myEventManager = (EventManager *) _param;
  int count = 0;
  double sealevel = 0;
  double temperature = 0;
  double windSpeed = 0;
  double windGust = 0;
  double windGustComputed = 0;
  double windDirection = 0;
  double speed = 0;
  double gustSpeed = 0;
  double batteryLevel = 0;
  long int previousMs = 0;
  long int currentMs = 0;
  struct timeval tp;
  oregon * s = NULL;
  Event ev;
  // get first timestamp  
  gettimeofday(&tp, NULL);
  previousMs = tp.tv_sec * 1000 + tp.tv_usec / 1000;
  // loop            
  while(1) {
    while ( ! myEventManager->eventList.empty() ) {
      pthread_mutex_lock( &myEventManager->eventListMutex );
      ev = myEventManager->eventList.front();
      myEventManager->eventList.pop_front();
      pthread_mutex_unlock( &myEventManager->eventListMutex );
      switch (ev.eventType) {
        // INIT OF MODULE - check leds & RX / TX
        case w3rpi_EVENT_INIT:
          if(w3rpi_debug) {
            printf("w3rpi EventManager::eventLoop INIT \n");
            if ( Singleton::get() ) {
              printf("w3rpi EventManager::eventLoop w3rpi Started ! \n");
            }
          }
          break;
        // Just received an event from the RF module
        case w3rpi_EVENT_GETSENSORDATA:
          if(w3rpi_debug)
            printf("w3rpi EventManager::eventLoop GETSENSORDATA \n");
          s = oregon::getRightOregon(ev.strValue);
          if ( s != NULL && s->isDecoded() ) {
            switch(s->getBattery()) {
              case 0:
                batteryLevel = 19;
                break;
              case 1:
                batteryLevel = 89;
                break;
              default:
                batteryLevel = s->getBattery();
                break;
            }
            windDirection = s->getWindDirection(); 
            if( windDirection != 0) {
              if( myEventManager->direction_correction > 0)
                windDirection =  myEventManager->direction_correction + windDirection; 
              windDirection = NEAREST(windDirection,5);
              if(windDirection > 360)
                windDirection = windDirection - 360;
              if( windDirection == 360)
                windDirection = 0;
            }
            myEventManager->store(s->getName(), s->getChannel(), batteryLevel, s->getTemperature(), s->getHumidity(), s->getBarometer(), windDirection, s->getSpeed(), s->getSpeedAverage());
            }
          break;
        default:
          if(w3rpi_debug)
            printf("w3rpi EventManager::eventLoop proceed UNKNOWN (%s) \n",ev.strValue);
          break;
      }
    }
    // every 3 seconds 
    gettimeofday(&tp, NULL);
    currentMs = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    if((currentMs - previousMs) >= 3000) {
      // collect speed if needed
      if(myEventManager->bAnemometer) {
        // printf("microsec(%d): %d\n", count, (currentMs - previousMs));
        speed = ((double)myEventManager->anemometerCounter) / ((double)(currentMs - previousMs) / (double) 1000);
        windSpeed += speed;
        // gust on 
        if(speed > windGustComputed)
          windGustComputed = speed;
        // gust on the fastest rotation
        gustSpeed = myEventManager->getGust();
        if(gustSpeed > windGust)
          windGust = gustSpeed;
        previousMs = currentMs;
        myEventManager->anemometerCounter = 0;
        myEventManager->firstCount = 0;
      }
      previousMs = currentMs;
      // every 15 (5*3) seconds store data
      if(++count > 4) {
        // printf("count: %d\n", count);
        // Barometer
        sealevel = 0;
        sealevel = myEventManager->getBarometerSealevel();
        // Thermistor
        temperature = 0;
        if(myEventManager->bTemperature) {
          temperature = myEventManager->getThermistor();
        }
        speed = gustSpeed = 0;  
        if(myEventManager->bAnemometer) {
          // Wind Speed and Direction
          if(windSpeed > (double)1 )
            speed = windSpeed / (double)count;
          gustSpeed = windGustComputed;
          // Wind Direction
          // Get last Direction - compute directions to difficult
          windDirection = myEventManager->getWindDirection();
        } 
        batteryLevel = 0;
        if(myEventManager->bSolar) {
          // Battery level
          if(myEventManager->getBatteryLevel() > (double) 6.0) {
            batteryLevel = (myEventManager->getBatteryLevel() - 11.0) * 34;
          }
        } 
        myEventManager->store((char *)"WS200", 0, batteryLevel, temperature, 0.0, sealevel, windDirection, gustSpeed, speed);
        windDirection = windSpeed = windGust = windGustComputed = 0;
        count = 0;
      }
    } else {
      if(previousMs > currentMs) {
        previousMs = currentMs;
      }
      usleep(5000); // 5 ms de repos c'est pas un mal
    }
  }
  return NULL;
}