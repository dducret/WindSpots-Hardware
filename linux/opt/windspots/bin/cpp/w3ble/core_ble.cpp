// core_ble.cpp
#include "core_ble.h"
#include "eventManager.h"
#include "singleton.h"
#include "utils.h"
using namespace std;
using namespace BLEPP;
//#define TRACECOREBLE

core_ble::core_ble() {
   pthread_create(&myThread,NULL, receptionLoop, this);
}
core_ble::~core_ble() {
}
// Thread to manage reception
void * core_ble::receptionLoop( void * _param ) {
  core_ble * myCore = (core_ble *) _param;
  char _tmpStr[512];
  HCIScanner::ScanType type = HCIScanner::ScanType::Passive;
  HCIScanner::FilterDuplicates filter = HCIScanner::FilterDuplicates::Off;
  HCIScanner scanner(true, filter, type);
  // init
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 300000;
  unsigned int high = 0;
  unsigned int low = 0;
  unsigned int speed = 0;
  unsigned int direction = 0;
  bool bDebug = Singleton::get()->getEventManager()->isDebug();
  string address = Singleton::get()->getEventManager()->getAddressMAC();
	if(bDebug)
    printf("w3ble core_ble::receptionLoop - address: %s\n",address.c_str());
  while(1) {
    //Check to see if there's anything to read from the HCI
    //and wait if there's not.
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(scanner.get_fd(), &fds);
    int err = select(scanner.get_fd()+1, &fds, NULL, NULL,  &timeout);
    //Interrupted, so quit and clean up properly.
    if(err < 0 && errno == EINTR)
      break;
    if(FD_ISSET(scanner.get_fd(), &fds)) {
      //Only read id there's something to read
      vector<AdvertisingResponse> ads = scanner.get_advertisements();
      for(const auto& ad: ads) {
        if(ad.address.length()<5)
          continue;
        if(ad.manufacturer_specific_data.empty())
          continue;
        if(ad.address != address)
           continue;
        high = (ad.manufacturer_specific_data[0][21]<<8)+ad.manufacturer_specific_data[0][22];
        low = (ad.manufacturer_specific_data[0][22]<<8)+ad.manufacturer_specific_data[0][23];
        speed = bitExtracted(high,9,4);
        direction = bitExtracted(low,9,1);
        sprintf(_tmpStr, "%u;%u",direction,speed);
        if(direction < 361)
        	Singleton::get()->getEventManager()->enqueue(W3BLE_EVENT_GETSENSORDATA,_tmpStr);
        usleep(990000); // packet received, wait a little 990 ms 
      }
    }
  }
}
void core_ble::loop( void ) {
  pthread_join(myThread,NULL);
}