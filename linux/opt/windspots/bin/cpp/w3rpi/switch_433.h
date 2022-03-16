// switch433.h
#ifndef _switch433_h
#define _switch433_h
#include <pthread.h>
#include <stdint.h>
#define CHANGE 1
extern "C"{
  typedef uint8_t boolean;
  typedef uint8_t byte;
  }
// Number of maximum High/Low changes per packet.
// We can handle up to (unsigned long) => 32 bit * 2 H/L changes per bit + 2 for sync
//#define RCSWITCH_MAX_CHANGES 67
// Taille max d'un message de type Oregon Scientific
#define RCSWITCH_MAX_MESS_SIZE 128
class switch_433 {
  public:
    switch_433();
    ~switch_433();
    void enableReceive(int interrupt);
    void enableReceive();
    void disableReceive();
    static bool OokAvailable();
    static bool getOokCode(char * _dest);
    static void OokResetAvailable();
  private:
    static void handleInterrupt();
    static void handleNoInterrupt();
    int nReceiverInterrupt;
    int nReceiverEnablePin;
    static char OokReceivedCode[RCSWITCH_MAX_MESS_SIZE];
    static bool OokAvailableCode;
    bool          rxEnable;
};
#endif