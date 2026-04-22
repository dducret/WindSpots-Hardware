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

#include <iomanip>

using namespace std;

#define NEAREST(number, multiple) (((number) + ((multiple) / 2)) / (multiple) * (multiple))

static void writeWindValues(double direction, double speed)
{
  std::ofstream windValues("/var/tmp/windvalues", std::ofstream::out | std::ofstream::trunc);
  if (!windValues.is_open()) {
    return;
  }
  windValues << std::fixed << std::setprecision(2) << speed << " "
             << std::setprecision(0) << direction << std::endl;
}

static bool execSql(sqlite3 *db, const char *sql, char *messageBuffer, size_t messageBufferSize)
{
  char *errMsg = NULL;
  const int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
  if (rc == SQLITE_OK) {
    return true;
  }

  snprintf(messageBuffer, messageBufferSize, "SQLite error: %s", errMsg != NULL ? errMsg : sqlite3_errmsg(db));
  if (errMsg != NULL) {
    sqlite3_free(errMsg);
  }
  return false;
}

EventManager::EventManager(const char * _piId)
    : piId(_piId),
      running(true),
      db(NULL),
      storeStmt(NULL) {
  eventListMutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_init(&eventCond, NULL);
  pthread_create(&myThread, NULL, eventLoop, this);
}

EventManager::~EventManager() {
  running = false;
  // Signal the condition variable to unblock eventLoop if waiting.
  pthread_cond_signal(&eventCond);
  pthread_join(myThread, NULL);
  if (storeStmt != NULL) {
    sqlite3_finalize(storeStmt);
    storeStmt = NULL;
  }
  if (db != NULL) {
    sqlite3_close(db);
    db = NULL;
  }

  pthread_cond_destroy(&eventCond);
  pthread_mutex_destroy(&eventListMutex);
}

bool EventManager::init(std::string log, std::string tmp, int anemometer_altitude, int direction, bool b_temperature, bool b_anemometer, bool b_solar) {
  anemometerCounter = 0;
  fastestCount = 0;
  firstCount = 0;
  lastCount = 0;
  logFileName = log + "/windspots.log";
  tmpFileName = tmp + "/ws.db";
  altitude = anemometer_altitude;
  direction_correction = direction;
  bTemperature = b_temperature;
  bAnemometer = b_anemometer;
  bSolar = b_solar;

  snprintf(message, MESSAGE_SIZE, "Program Starting. log:%s, tmp:%s, altitude:%d, dir-correction:%d, Temp:%u, Anemo:%u, Solar: %u",
           log.c_str(), tmp.c_str(), altitude, direction, b_temperature, b_anemometer, b_solar);
  logIt();
  if(w3rpi_debug) {
    snprintf(message, MESSAGE_SIZE, "Debug Mode");
    logIt();
  }
  // Initialize sensors
  myBmp280.reset(new bmp280());
  inaBattery0.reset(new ina219(0x41));
  inaSolar0.reset(new ina219(0x40));
  ina5v.reset(new ina219(0x43));
  ads.reset(new ads1015(0x48));

  // Create database if not exist
  if(sqlite3_open(this->tmpFileName.c_str(), &db)) {
    snprintf(message, MESSAGE_SIZE, "Can't open database: %s, error: %s", this->tmpFileName.c_str(), sqlite3_errmsg(db));
    logIt();
    sqlite3_close(db);
    db = NULL;
    return false;
  }
  sqlite3_busy_timeout(db, 1000);

  if (!execSql(db, "CREATE TABLE IF NOT EXISTS data (id INTEGER PRIMARY KEY AUTOINCREMENT, last_update DATE, name TEXT, sensor_id TEXT, channel INTEGER, rollingcode INTEGER, battery TEXT, temperature TEXT, temperature_sign TEXT, relative_humidity TEXT, comfort TEXT, uv_index TEXT, rain_rate TEXT, total_rain TEXT, barometer TEXT, prediction TEXT, wind_direction TEXT, wind_speed TEXT, wind_speed_average TEXT)", message, sizeof(message)) ||
      !execSql(db, "CREATE INDEX IF NOT EXISTS i1 ON data(last_update)", message, sizeof(message)) ||
      !execSql(db, "CREATE TABLE IF NOT EXISTS log (id INTEGER PRIMARY KEY AUTOINCREMENT, last_update DATE)", message, sizeof(message))) {
    logIt();
    sqlite3_close(db);
    db = NULL;
    return false;
  }

  const char *insertSql =
      "INSERT INTO data (last_update, name, channel, battery, temperature, temperature_sign, "
      "relative_humidity, barometer, wind_direction, wind_speed, wind_speed_average) "
      "VALUES (?, ?, ?, ?, ?, '0', ?, ?, ?, ?, ?)";
  const int rc = sqlite3_prepare_v2(db, insertSql, -1, &storeStmt, NULL);
  if (rc != SQLITE_OK) {
    snprintf(message, MESSAGE_SIZE, "EventManager::init prepare SQL error: %s", sqlite3_errmsg(db));
    logIt();
    sqlite3_close(db);
    db = NULL;
    storeStmt = NULL;
    return false;
  }

  return true;
}

void EventManager::logIt() {
  timeval tp;
  gettimeofday(&tp, 0);
  time_t curtime = tp.tv_sec;
  tm tm_buf;
  tm *t = localtime_r(&curtime, &tm_buf);
  char timePart[16];
  if (t == NULL || strftime(timePart, sizeof(timePart), "%H:%M:%S", t) == 0) {
    strncpy(timePart, "00:00:00", sizeof(timePart) - 1);
    timePart[sizeof(timePart) - 1] = '\0';
  }
  snprintf(currentTime, sizeof(currentTime), "%s:%03d", timePart, (int)tp.tv_usec/1000);
  std::ofstream log(this->logFileName.c_str(), std::ofstream::out | std::ofstream::app);
  if (log.is_open()) {
    log << currentTime << " w3rpi " << message << std::endl;
    log.close();
  }
  std::cerr << currentTime << " w3rpi " << message << std::endl;
}

void EventManager::enqueue(int newEvent, char * _strValue) {
  Event ev;
  ev.eventType = newEvent;
  if (_strValue != NULL) {
    if (strlen(_strValue) > w3rpi_EVENT_MAX_SIZE-1) {
      if(w3rpi_debug)
        std::cerr << "EventManager::enqueue TOO LONG for:" << _strValue << std::endl;
    }
    strncpy(ev.strValue, _strValue, w3rpi_EVENT_MAX_SIZE-1);
    ev.strValue[w3rpi_EVENT_MAX_SIZE-1] = '\0';
  } else {
    ev.strValue[0] = '\0';
  }
  pthread_mutex_lock(&eventListMutex);
  if(w3rpi_debug)
    std::cerr << "EventManager::enqueue:" << _strValue << std::endl;
  eventList.push_back(ev);
  pthread_mutex_unlock(&eventListMutex);
  pthread_cond_signal(&eventCond);  // Signal waiting thread
}

void EventManager::anemometerAdd() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
  if(firstCount == 0) {
    firstCount = ms;
    lastCount = ms;
    fastestCount = 9999;
    ++anemometerCounter;
    return;
  }
  long int currentDelta = ms - lastCount;
  if(currentDelta < 8)
    return;
  ++anemometerCounter;
  if(currentDelta < fastestCount) {
    fastestCount = currentDelta;
  }
  lastCount = ms;
}

double EventManager::getWindDirection(){
  ads->setGain(GAIN_ONE);
  int adc_value = ads->readADC_SingleEnded(0);
  double direction = adc_value * 360 / static_cast<double>(1648);
  if(direction_correction > 0)
      direction += direction_correction;
  direction = NEAREST(direction,5);
  if(direction > 360)
      direction -= 360;
  if(direction == 360)
    direction = 0;
  return direction;
}

double EventManager::getGust(){
  double gust = 1.00584 / (static_cast<double>(fastestCount) / 1000.0);
  return gust;
}

double EventManager::getThermistor(){
  double temperature = 0;
  ads->setGain(GAIN_ONE);
  int adc_value = ads->readADC_SingleEnded(1);
  double volts = (adc_value * 3.3) / 1648.0;
  if(w3rpi_debug)
    std::cerr << "EventManager::getThermistor volts = " << std::fixed << std::setprecision(2) << volts << std::endl;
  if(volts > 0) {
    double ohm = round((3.3 - volts) / volts * 10000);
    double a =  0.0009333357965;
    double b =  0.0002340542448;
    double c =  0.00000008044859863;
    double log_r  = log(ohm);
    double log_r3 = log_r * log_r * log_r;
    double temp = 1.0 / (a + b * log_r + c * log_r3);
    temperature = round(temp - 273.15);
  }
  return temperature;
}

double EventManager::getBatteryLevel(){
  return inaBattery0->getBusVoltage_V();
}

float EventManager::getBatteryPower(){
  return static_cast<float>(inaBattery0->getBusVoltage_V() * (inaBattery0->getCurrent_mA() * 8.2));
}

float EventManager::getSolarPower(){
  return static_cast<float>(inaSolar0->getBusVoltage_V() * inaSolar0->getCurrent_mA());
}

float EventManager::get5vPower(){
  return static_cast<float>(ina5v->getBusVoltage_V() * ina5v->getCurrent_mA());
}

int EventManager::getBarometerSealevel(){
  double barometer = 0;
  int sealevel  = 0;
  if(!myBmp280->update()) {
    if(w3rpi_debug)
                std::cerr << "EventManager::eventLoop - Bmp280 error" << std::endl;
    snprintf(message, MESSAGE_SIZE, "EventManager::eventLoop - Bmp280 error\n");
    logIt();
    return 0;
  }
  barometer = myBmp280->getPressure();
  if(w3rpi_debug)
    std::cerr << "EventManager::getBarometerSealevel barometer = " << std::fixed << std::setprecision(0) << barometer << std::endl;
  if(barometer < 700)
    return 0;
  sealevel = static_cast<int>(barometer / pow(1.0 - static_cast<double>(altitude) / 44330.0, 5.255));
  return sealevel;
}

bool EventManager::isRunning() const {
  return running;
}

void EventManager::store(const char * _name, int channel, double battery, double temperature,
                         double humidity, int barometer, double windDirection, double windSpeed,
                         double windSpeedAverage) {
  if (_name == NULL) {
    if(w3rpi_debug)
      std::cerr << "EventManager::store _name == NULL" << std::endl;
    return;
  }
  // Avoid logging heartbeat with no data
  if(battery == 0 && temperature == 0 && humidity == 0 && barometer == 0 &&
     windDirection == 0 && windSpeed == 0 && windSpeedAverage == 0) {
    snprintf(message, sizeof(message), "%s-%d - Heartbeat", _name, channel);
    return;
  }
  timeval tp;
  gettimeofday(&tp, 0);
  time_t curtime = tp.tv_sec;
  tm tm_buf;
  tm *t = localtime_r(&curtime, &tm_buf);
  if (t == NULL || strftime(currentTime, sizeof(currentTime), "%Y-%m-%d %H:%M:%S", t) == 0) {
    strncpy(currentTime, "1970-01-01 00:00:00", sizeof(currentTime) - 1);
    currentTime[sizeof(currentTime) - 1] = '\0';
  }

  if (db == NULL || storeStmt == NULL) {
    snprintf(message, sizeof(message), "EventManager::store database is not initialized");
    logIt();
    return;
  }

  sqlite3_reset(storeStmt);
  sqlite3_clear_bindings(storeStmt);
  sqlite3_bind_text(storeStmt, 1, currentTime, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(storeStmt, 2, _name, -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(storeStmt, 3, channel);
  sqlite3_bind_double(storeStmt, 4, battery);
  sqlite3_bind_double(storeStmt, 5, temperature);
  sqlite3_bind_double(storeStmt, 6, humidity);
  sqlite3_bind_int(storeStmt, 7, barometer);
  sqlite3_bind_double(storeStmt, 8, windDirection);
  sqlite3_bind_double(storeStmt, 9, windSpeed);
  sqlite3_bind_double(storeStmt, 10, windSpeedAverage);

  int execRc = SQLITE_ERROR;
  for (int attempt = 0; attempt < 5; ++attempt) {
    execRc = sqlite3_step(storeStmt);
    if (execRc == SQLITE_DONE) {
      break;
    }
    if (execRc == SQLITE_BUSY || execRc == SQLITE_LOCKED) {
      sqlite3_reset(storeStmt);
      sqlite3_clear_bindings(storeStmt);
      sqlite3_bind_text(storeStmt, 1, currentTime, -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(storeStmt, 2, _name, -1, SQLITE_TRANSIENT);
      sqlite3_bind_int(storeStmt, 3, channel);
      sqlite3_bind_double(storeStmt, 4, battery);
      sqlite3_bind_double(storeStmt, 5, temperature);
      sqlite3_bind_double(storeStmt, 6, humidity);
      sqlite3_bind_int(storeStmt, 7, barometer);
      sqlite3_bind_double(storeStmt, 8, windDirection);
      sqlite3_bind_double(storeStmt, 9, windSpeed);
      sqlite3_bind_double(storeStmt, 10, windSpeedAverage);
      usleep(100000);
      continue;
    }
    break;
  }

  if(execRc != SQLITE_DONE) {
    snprintf(message, sizeof(message), "EventManager::store SQL error: %s", sqlite3_errmsg(db));
    logIt();
    sqlite3_reset(storeStmt);
    sqlite3_clear_bindings(storeStmt);
    return;
  }
  sqlite3_reset(storeStmt);
  sqlite3_clear_bindings(storeStmt);

  // Build a log message with safe concatenation.
  if(battery != 0)
    snprintf(message, sizeof(message), "%s-%d - Accus:%0.0f%%", _name, channel, battery);
  else
    snprintf(message, sizeof(message), "%s-%d - Accus:none", _name, channel);
  if(temperature != 0)
    snprintf(message + strlen(message), sizeof(message) - strlen(message), ", Temp:%0.1f", temperature);
  if(humidity != 0)
    snprintf(message + strlen(message), sizeof(message) - strlen(message), ", Hum:%0.2f%%", humidity);
  if(barometer != 0)
    snprintf(message + strlen(message), sizeof(message) - strlen(message), ", Baro:%u", barometer);
  if(windDirection != 0 || windSpeed != 0 || windSpeedAverage != 0)
    snprintf(message + strlen(message), sizeof(message) - strlen(message), ", Dir:%0.f, Speed:%0.2f km/h, Average:%0.2f km/h",
             windDirection, (windSpeed * 3.6), (windSpeedAverage * 3.6));
  logIt();
}

// Event loop with condition variable and periodic wakeups
void * EventManager::eventLoop(void * _param) {
  EventManager * myEventManager = static_cast<EventManager*>(_param);
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
  Event ev;

  gettimeofday(&tp, NULL);
  previousMs = tp.tv_sec * 1000 + tp.tv_usec / 1000;

  while(myEventManager->running) {
    // Wait for new events or timeout after 5ms
    pthread_mutex_lock(&myEventManager->eventListMutex);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 5 * 1000000; // 5ms
    if (ts.tv_nsec >= 1000000000L) {
      ts.tv_sec += ts.tv_nsec / 1000000000L;
      ts.tv_nsec %= 1000000000L;
    }
    pthread_cond_timedwait(&myEventManager->eventCond, &myEventManager->eventListMutex, &ts);

    while (!myEventManager->eventList.empty()) {
      ev = myEventManager->eventList.front();
      myEventManager->eventList.pop_front();
      pthread_mutex_unlock(&myEventManager->eventListMutex);

      switch(ev.eventType) {
        case w3rpi_EVENT_INIT:
          if(w3rpi_debug) {
            std::cerr <<  "EventManager::eventLoop INIT" << std::endl;
          }
          break;
        case w3rpi_EVENT_GETSENSORDATA:
          if(w3rpi_debug)
            std::cerr <<  "EventManager::eventLoop GETSENSORDATA" << std::endl;
          break;
        default:
          if(w3rpi_debug)
            std::cerr <<  "EventManager::eventLoop proceed UNKNOWN: " << ev.strValue << std::endl;
          break;
      }
      pthread_mutex_lock(&myEventManager->eventListMutex);
    }
    pthread_mutex_unlock(&myEventManager->eventListMutex);

    // Periodic sensor processing
    gettimeofday(&tp, NULL);
    currentMs = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    if((currentMs - previousMs) >= 3000) {
      if(myEventManager->bAnemometer) {
        speed = (static_cast<double>(myEventManager->anemometerCounter)) / ((currentMs - previousMs) / 1000.0);
        windSpeed += speed;
        if(speed > windGustComputed)
          windGustComputed = speed;
        gustSpeed = myEventManager->getGust();
        if(gustSpeed > windGust)
          windGust = gustSpeed;
        previousMs = currentMs;
        myEventManager->anemometerCounter = 0;
        myEventManager->firstCount = 0;
      }
      previousMs = currentMs;
      if(++count > 4) {
        sealevel = myEventManager->getBarometerSealevel();
        temperature = 0;
        if(myEventManager->bTemperature)
          temperature = myEventManager->getThermistor();
        speed = gustSpeed = windDirection = 0;
        if(myEventManager->bAnemometer) {
          if(windSpeed > 1.0)
            speed = windSpeed / count;
          gustSpeed = windGustComputed;
          windDirection = myEventManager->getWindDirection();
          writeWindValues(windDirection, speed);
        }
        batteryLevel = 0;
        if(myEventManager->bSolar) {
          if(myEventManager->getBatteryLevel() > 6.0)
            batteryLevel = (myEventManager->getBatteryLevel() - 11.0) * 34;
          else
            batteryLevel = 0.1;
        }
        myEventManager->store("WS200", 0, batteryLevel, temperature, 0.0, sealevel, windDirection, gustSpeed, speed);
        if(!myEventManager->bAnemometer) {
          myEventManager->running = false;
        }
        windDirection = windSpeed = windGust = windGustComputed = 0;
        count = 0;
      }
    } else {
      if(previousMs > currentMs)
        previousMs = currentMs;
    }
  }
  return nullptr;
}
