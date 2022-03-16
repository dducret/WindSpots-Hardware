// initwsdb.cpp
#include <iostream>
#include <fstream> 
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h> 
#include <sys/time.h>
#include <sqlite3.h>
#include "version.h"
using namespace std;
//  
std::string logFileName;
std::string dbFileName;
//    
static void show_usage(std::string name) {
    std::cerr << "Usage: dataupload <option(s)>\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-s,--station\tStation Name\n"
              << "\t-l,--log\t\tLog file default:/var/log\n"
              << "\t-t,--tmp\t\tLog file default:/var/tmp\n"
              << std::endl;
}
void logIt(char const *message) {
  std::ofstream log(logFileName.c_str(), std::ofstream::out | std::ofstream::app);
  timeval tp;
  gettimeofday(&tp, 0);
  time_t curtime = tp.tv_sec;
  tm *t = localtime(&curtime);
  char time[16];
  sprintf(time,"%02d:%02d:%02d:%03d", t->tm_hour, t->tm_min, t->tm_sec, tp.tv_usec/1000);
  log << time << " initwsdb " << message << std::endl;
  log.close();
}
bool initDB() {
  char *message = new char[128];
  sqlite3 * db;
  // create databse if no exist
  if(sqlite3_open(dbFileName.c_str(), &db)) {
    sprintf(message,"Can't open database: %s", sqlite3_errmsg(db));
    logIt(message);
    delete [] message;
    return false;
  }
  const char *sql;
  sql = "CREATE TABLE IF NOT EXISTS data (id INTEGER PRIMARY KEY AUTOINCREMENT, last_update DATE, name TEXT, sensor_id TEXT, channel INTEGER, rollingcode INTEGER, battery TEXT, temperature TEXT, temperature_sign TEXT, relative_humidity TEXT, comfort TEXT, uv_index TEXT, rain_rate TEXT, total_rain TEXT, barometer TEXT, prediction TEXT, wind_direction TEXT, wind_speed TEXT, wind_speed_average TEXT)";
  sqlite3_exec(db, sql, NULL, NULL, NULL);
  sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
  sql = "CREATE INDEX IF NOT EXISTS i1 ON data(last_update)";
  sqlite3_exec(db, sql, NULL, NULL, NULL);
  sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
  sql = "CREATE TABLE IF NOT EXISTS log (id INTEGER PRIMARY KEY AUTOINCREMENT, last_update DATE)";
  sqlite3_exec(db, sql, NULL, NULL, NULL);
  sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
  sqlite3_close(db);
  delete [] message;
  return true;
}
int main(int argc, char *argv[]) {
  std::vector <std::string> sources;
  std::string station = "CHGE99";
  std::string tmp = "/var/tmp";
  std::string log = "/var/log";
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-h") || (arg == "--help")) {
        show_usage(argv[0]);
        return 0;
    } 
    if ((arg == "-s") || (arg == "--station")) {
      if (i + 1 < argc) { 
          station = argv[i+1]; 
      } else { 
          std::cerr << "--station option requires one argument." << std::endl;
          return 1;
      }  
    } 
    if ((arg == "-l") || (arg == "--log")) {
      if (i + 1 < argc) { 
          log = argv[i+1];
      } else { 
          std::cerr << "--log option requires one argument." << std::endl;
          return 1;
      }  
    } 
    if ((arg == "-t") || (arg == "--tmp")) {
      if (i + 1 < argc) { 
          tmp = argv[i+1];
      } else { 
          std::cerr << "--tmp option requires one argument." << std::endl;
          return 1;
      }  
    } 
  }
  printf("Version (%d.%d) Branch (%s) Build date(%u)\n",INITWSDB_VERSION_MAJOR,INITWSDB_VERSION_MINOR,INITWSDB_VERSION_BRANCH,&INITWSDB_BUILD_DATE);
  printf("Initialize station:%s, log:%s, tmp:%s\n",station.c_str(), log.c_str(), tmp.c_str());
  //
  logFileName.assign(log);
  logFileName.append("/windspots.log");
  dbFileName.assign(tmp);
  dbFileName.append("/ws.db");
  //
  initDB();
  exit(0);
}