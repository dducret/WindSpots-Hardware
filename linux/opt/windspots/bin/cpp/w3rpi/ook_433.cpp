// RcOoK.cpp
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "switch_433.h"
#include "ook_433.h"
using namespace std;
#define GET_LO_CHAR(a)  (a&0x0f)
#define GET_HI_CHAR(a)  ((a>>4)&0x0f)
// Master class OOK
DecodeOOK::DecodeOOK(switch_433 * _rcs) {
  this->rcs = _rcs;
  resetDecoder();
}
DecodeOOK::~DecodeOOK() {}
bool DecodeOOK::nextPulse (word width) {
  if (state != DONE) {
    switch (decode(width)) {
      case -1: resetDecoder(); break;
      case 1:  done(); break;
    }
  }
  return isDone();
}
bool DecodeOOK::isDone() const { 
  return state == DONE; 
}
const byte* DecodeOOK::getData(byte& count) const {
  count = pos;
  return data;
}
void DecodeOOK::resetDecoder() {
  total_bits = bits = pos = flip = 0;
  state = UNKNOWN;
}
// add one bit to the packet data buffer
void DecodeOOK::gotBit(char value) {
  total_bits++;
  byte *ptr = data + pos;
  *ptr = (*ptr >> 1) | (value << 7);
  if (++bits >= 8) {
    bits = 0;
    if (++pos >= sizeof data) {
      resetDecoder();
      return;
    }
  }
  state = OK;
}
// store a bit using Manchester encoding
void DecodeOOK::manchester(char value) {
  flip ^= value; // manchester code, long pulse flips the bit
  gotBit(flip);
}
void DecodeOOK::done() {
  while (bits) {
    gotBit(0); // padding
  }
  state = DONE;
}
/**
 * Print in hex the received value into d string adding s string header
 */
void DecodeOOK::sprint(const char * s, char * d) {
  char v[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
  // debug char * _d = d;
  byte pos;
  const byte* data = this->getData(pos);
  sprintf(d,"%s ",s);
  d+=strlen(s);
  for (byte i = 0; i < pos ; ++i) {
    sprintf(d,"%c",v[ data[i] >> 4 ]);d++;
    sprintf(d,"%c",v[ data[i] & 0x0F]);d++;
  }
  sprintf(d,"%c",'\0');
  // debug std::cout << " * DecodeOOK received [" << _d << "]" << std::endl;
}
void DecodeOOK::print(const char* s) {
  char t[128];
  this->sprint(s,t);
  printf("%s\n",t);
}
/* ======================================================
 * OregonDecoderV2
 * ------------------------------------------------------
 */
OregonDecoderV2::OregonDecoderV2(switch_433 * _rcs) : DecodeOOK(_rcs) {}
OregonDecoderV2::~OregonDecoderV2() {}
void OregonDecoderV2::gotBit(char value) {
  if(total_bits<4) {
    state = OK;
    total_bits++;
    return;
  }
  if(!(total_bits & 0x01)) {
    data[pos] = (data[pos] >> 1) | (value ? 0x80 : 00);
  }
  total_bits++;
  int x = total_bits-4;
  if (((x >> 3) << 3) == x) {
    // swap low and high
    char newByte = ((GET_LO_CHAR(data[pos-1]) << 4) | (GET_HI_CHAR(data[pos-1])&(0xf))); 
    data[pos-1] = newByte;
  }
  pos = total_bits >> 4;
  if (pos >= sizeof data) {
    resetDecoder();
    return;
  }
  state = OK;
}
int OregonDecoderV2::decode(word width) {
  if (200 <= width && width < 1200) {
    byte w = width >= 700;
    switch (state) {
      case UNKNOWN:
        if (w != 0) { // Long pulse
            ++flip;
        } else if ( 24 <= flip ) { // Short pulse, start bit
          //BugFix : initialement on test 32b mais il semble que
          // tous n'en aient pas autant, en tout cas on constate
          // plus de message reçus avec 24 que 32 ; obligatoire pour THN132N
          flip = 0;
          state = T0;
        } else {  // Reset decoder
          return -1;
        }
        break;
      case OK:
        if (w == 0) { // Short pulse
          state = T0;
        } else { // Long pulse
          manchester(1);
        }
        break;
      case T0:
        if (w == 0) { // Second short pulse
          manchester(0);
        } else { // Reset decoder
          return -1;
        }
        break;
      }
  } else {
    // Dans le cas du THN132N on a seulement 136b
    // donc si on depasse le timing et que l'on a 136b
    // c'est sans doute qu'il s'agit de celui-ci
    return ( total_bits == 136 )? 1 : -1;
  }
  return total_bits == 160 ? 1: 0 ;
}
/* ======================================================
 * OregonDecoderV3
 * ------------------------------------------------------
 */
OregonDecoderV3::OregonDecoderV3(switch_433 * _rcs) : DecodeOOK(_rcs) {}
OregonDecoderV3::~OregonDecoderV3() {}
void OregonDecoderV3::gotBit(char value) {
    if(total_bits<4) {
      state = OK;
      total_bits++;
      return;
    }
    data[pos] = (data[pos] >> 1) | (value ? 0x80 : 00);
    total_bits++;
    int x = total_bits-4;
    pos = x >> 3;
    if (((x >> 3) << 3) == x) {
      // swap low and high
      char newByte = ((GET_LO_CHAR(data[pos-1]) << 4) | (GET_HI_CHAR(data[pos-1])&(0xf))); 
      data[pos-1] = newByte;
    }
    if (pos >= sizeof data) {
        resetDecoder();
        return;
    }
    state = OK;
} 
int OregonDecoderV3::decode(word width) {
  if (200 <= width && width < 1200) {
    byte w = width >= 700;
    switch(state) {
      case UNKNOWN:
        if (w == false) { // Short pulse
            ++flip;
        } else if (32 <= flip) { // long pulse
            flip = 1;
            manchester(1);
        } else {
            return -1; // Reset decoder
        }
        break;
      case OK:
        if (w == false) { // Short pulse
            state = T0;
        } else { // Long pulse
          manchester(1);
        }
        break;
      case T0:
        if (w == false) { // Second short pulse
          manchester(0);
        } else { 
            return -1; // Reset decoder
        }
        break;
    }
  } else {
    return  (total_bits <104 && total_bits>=40  ) ? 1: -1;
    
  }
  return  total_bits == 80 ? 1: 0;
}