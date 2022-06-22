// switch433.cpp
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include "switch_433.h"
#include "ook_433.h"
using namespace std;
//
char switch_433::OokReceivedCode[RCSWITCH_MAX_MESS_SIZE];
bool switch_433::OokAvailableCode;
OregonDecoderV2 * orscV2;
OregonDecoderV3 * orscV3;
switch_433::switch_433() {
  switch_433::OokAvailableCode = false;
  switch_433::OokReceivedCode[0] = '\0';
  orscV2 = new OregonDecoderV2(this);
  orscV3 = new OregonDecoderV3(this);
  pinMode(19,INPUT);
  wiringPiISR(19, INT_EDGE_BOTH, &handleInterrupt);
}
switch_433::~switch_433() {
  delete orscV2;
  delete orscV3;
  wiringPiISR(19, INT_EDGE_BOTH, &handleNoInterrupt);
}
// Set to true when a code has been decode by the OoK module
bool switch_433::OokAvailable() {
  return switch_433::OokAvailableCode;
}
// ==============================================
// Return the received code decoded by Ook engine
// if available and true, otherwith return false
// can be used w/o OokAvailable
//
// The decoded value is stored in the v this string
// must have a size equal to RCSWITCH_MAX_MESS_SIZE
//
// Reset available flag (this allow new capture from
// interrupt (locked otherwize to avoid reentrance
bool switch_433::getOokCode(char * _dest) {
  if ( switch_433::OokAvailableCode ) {
    strcpy(_dest,switch_433::OokReceivedCode);
    switch_433::OokAvailableCode = false;
    return true;
  } else return false;
}
// reset available (autorize new capture)
void switch_433::OokResetAvailable() {
  switch_433::OokAvailableCode = false;
}
// ==============================================
// Interrupt Handler to manage the different protocols
void switch_433::handleInterrupt() {
  static unsigned int duration;
  static unsigned long lastTime;
  long time = micros();
  duration = time - lastTime;
  lastTime = time;
  word p = (unsigned short int) duration;
  //if ( p != duration ) printf("*** OUPS *** %d // %d \n",duration,p);
  // Avoid re-entry
  if ( !OokAvailableCode && duration < 32000 ) {                // avoid reentrance -- wait until data is read
    if (orscV2->nextPulse(p))     { orscV2->sprint("OS2:",switch_433::OokReceivedCode); orscV2->resetDecoder(); switch_433::OokAvailableCode = true; }
    if (orscV3->nextPulse(p))     { orscV3->sprint("OS3:",switch_433::OokReceivedCode); orscV3->resetDecoder(); switch_433::OokAvailableCode = true; }
  } else if (duration >=32000 ) { // too long wait ... timeout, previously we had noise ...
    orscV2->resetDecoder();
    orscV3->resetDecoder();
  }
}
// ============================================
// Not clear how to disable interrupt so, the more
// simple way is to change the functin for doing nothing.
void switch_433::handleNoInterrupt() {
   // do nothing on interrupt
}