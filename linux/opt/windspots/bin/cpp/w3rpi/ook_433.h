#ifndef RCOOK_H_
#define RCOOK_H_
#include <stdint.h>

typedef uint16_t word;
typedef uint8_t byte;
#define OOK_MAX_DATA_LEN 25
#define OOK_MAX_STR_LEN  100

class switch_433; // Forward declaration

class DecodeOOK {
protected:
    byte total_bits, bits, flip, state, pos, data[OOK_MAX_DATA_LEN];
    switch_433 * rcs;
    virtual int decode(word width) = 0;
public:
    enum { UNKNOWN, T0, T1, T2, T3, OK, DONE };
    DecodeOOK(switch_433 * _rcs);
    virtual ~DecodeOOK();
    virtual void gotBit(char value);
    bool nextPulse(word width);
    bool isDone() const;
    const byte* getData(byte& count) const;
    void resetDecoder();
    void manchester(char value);
    void done();
    // void print(const char* s);
    void sprint(const char * s, char * d);
};

class OregonDecoderV2 : public DecodeOOK {
public:
    OregonDecoderV2(switch_433 * _rcs);
    ~OregonDecoderV2();
    inline void gotBit(char value);
    int decode(word width);
};

class OregonDecoderV3 : public DecodeOOK {
public:
    OregonDecoderV3(switch_433 * _rcs);
    ~OregonDecoderV3();
    inline void gotBit(char value);
    int decode(word width);
};

#endif /* RCOOK_H_ */
