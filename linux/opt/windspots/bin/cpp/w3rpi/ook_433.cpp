#include <iostream>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "w3rpi.h"
#include "switch_433.h"
#include "ook_433.h"

using namespace std;

#define GET_LO_CHAR(a)  ((a)&0x0F)
#define GET_HI_CHAR(a)  (((a)>>4)&0x0F)

// ---------------------------------------------------------------------
// DecodeOOK Class Implementation
// ---------------------------------------------------------------------

// Constructor: initialize state and clear data buffer.
DecodeOOK::DecodeOOK(switch_433 * _rcs)
    : total_bits(0), bits(0), flip(0), state(UNKNOWN), pos(0), rcs(_rcs)
{
    memset(data, 0, sizeof(data));
}

// Virtual destructor.
DecodeOOK::~DecodeOOK() { }

// Process the next pulse. If a complete message is received, return true.
bool DecodeOOK::nextPulse(word width) {
    if (state != DONE) {
        int ret = decode(width);
        if(ret == -1)
            resetDecoder();
        else if(ret == 1)
            done();
    }
    return isDone();
}

// Returns true if message is completely decoded.
bool DecodeOOK::isDone() const {
    return state == DONE;
}

// Return pointer to the data buffer along with the count of bytes.
const byte* DecodeOOK::getData(byte& count) const {
    count = pos;
    return data;
}

// Reset the decoder state.
void DecodeOOK::resetDecoder() {
    total_bits = bits = pos = flip = 0;
    state = UNKNOWN;
    memset(data, 0, sizeof(data));
}

// Append one bit to the data buffer.
void DecodeOOK::gotBit(char value) {
    total_bits++;
    byte *ptr = data + pos;
    *ptr = (*ptr >> 1) | (value << 7);
    if (++bits >= 8) {
        bits = 0;
        if (++pos >= sizeof(data)) {
            resetDecoder();
            return;
        }
    }
    state = OK;
}

// Manchester decoding: flip the bit accordingly.
void DecodeOOK::manchester(char value) {
    flip ^= value; // Flip the bit for Manchester encoding.
    gotBit(flip);
}

// When decoding is complete, pad with zeros if necessary.
void DecodeOOK::done() {
    while (bits)
        gotBit(0);
    state = DONE;
}

// Revised sprint: Convert decoded data into a hex string with a header.
// Optimized to use memcpy and direct assignments instead of sprintf.
void DecodeOOK::sprint(const char * s, char * d) {
    static const char hexChars[] = "0123456789ABCDEF";
    // Copy the header and a space.
    size_t headerLen = strlen(s);
    memcpy(d, s, headerLen);
    d[headerLen] = ' ';
    d[headerLen+1] = '\0';
    // Move pointer to end of header.
    char *ptr = d + headerLen;

    byte count;
    const byte *dataPtr = getData(count);
    for (byte i = 0; i < count; ++i) {
        *ptr++ = hexChars[dataPtr[i] >> 4];
        *ptr++ = hexChars[dataPtr[i] & 0x0F];
    }
    *ptr = '\0';
    if (w3rpi_debug) {
        // For debugging, print the entire formatted string.
        std::cout << "\nw3rpi DecodeOOK::sprint received [" << d << "]" << std::endl;
    }
}

/*
// Simple print function using sprint.
void DecodeOOK::print(const char* s) {
    char t[128];
    sprint(s, t);
    printf("%s\n", t);
}
*/
// ---------------------------------------------------------------------
// OregonDecoderV2 Implementation
// ---------------------------------------------------------------------

OregonDecoderV2::OregonDecoderV2(switch_433 * _rcs) : DecodeOOK(_rcs) { }
OregonDecoderV2::~OregonDecoderV2() { }

void OregonDecoderV2::gotBit(char value) {
    if (total_bits < 4) {
        state = OK;
        total_bits++;
        return;
    }
    if (!(total_bits & 0x01))
        data[pos] = (data[pos] >> 1) | (value ? 0x80 : 0x00);
    total_bits++;
    int x = total_bits - 4;
    if ((x & 0x07) == 0) {
        char newByte = ((GET_LO_CHAR(data[pos-1]) << 4) | (GET_HI_CHAR(data[pos-1]) & 0x0F));
        data[pos-1] = newByte;
    }
    pos = total_bits >> 4;
    if (pos >= sizeof(data)) {
        resetDecoder();
        return;
    }
    state = OK;
}

int OregonDecoderV2::decode(word width) {
    if (width >= 200 && width < 1200) {
        byte w = (width >= 700);
        switch(state) {
            case UNKNOWN:
                if (w != 0)
                    ++flip;
                else if (flip >= 24) {
                    flip = 0;
                    state = T0;
                } else {
                    return -1;
                }
                break;
            case OK:
                if (w == 0)
                    state = T0;
                else
                    manchester(1);
                break;
            case T0:
                if (w == 0)
                    manchester(0);
                else
                    return -1;
                break;
        }
    } else {
        return (total_bits == 136) ? 1 : -1;
    }
    return (total_bits == 160) ? 1 : 0;
}

// ---------------------------------------------------------------------
// OregonDecoderV3 Implementation
// ---------------------------------------------------------------------

OregonDecoderV3::OregonDecoderV3(switch_433 * _rcs) : DecodeOOK(_rcs) { }
OregonDecoderV3::~OregonDecoderV3() { }

void OregonDecoderV3::gotBit(char value) {
    if (total_bits < 4) {
        state = OK;
        total_bits++;
        return;
    }
    data[pos] = (data[pos] >> 1) | (value ? 0x80 : 0x00);
    total_bits++;
    int x = total_bits - 4;
    pos = x >> 3;
    if ((x & 0x07) == 0) {
        char newByte = ((GET_LO_CHAR(data[pos-1]) << 4) | (GET_HI_CHAR(data[pos-1]) & 0x0F));
        data[pos-1] = newByte;
    }
    if (pos >= sizeof(data)) {
        resetDecoder();
        return;
    }
    state = OK;
}

int OregonDecoderV3::decode(word width) {
    if (width >= 200 && width < 1200) { // OFF: 400 - 850  850 - 1400 ON: 200 - 615 615 - 1100
        byte w = (width >= 700);
        switch(state) {
            case UNKNOWN:
                if (!w) {
                    ++flip;
                } else if (flip >= 32) {
                    flip = 1;
                    manchester(1);
                } else {
                    return -1;
                }
                break;
            case OK:
                if (!w)
                    state = T0;
                else
                    manchester(1);
                break;
            case T0:
                if (!w)
                    manchester(0);
                else
                    return -1;
                break;
        }
    } else {
        return (total_bits < 104 && total_bits >= 40) ? 1 : -1;
    }
    return (total_bits == 80) ? 1 : 0;
}