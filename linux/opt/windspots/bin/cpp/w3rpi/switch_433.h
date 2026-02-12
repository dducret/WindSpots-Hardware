#ifndef _switch433_h
#define _switch433_h

#include <stdint.h>
#include <atomic>
#include <thread>
#include <mutex>

// Use C linkage for basic types.
extern "C" {
    typedef uint8_t boolean;
    typedef uint8_t byte;
}

// Maximum size for the decoded message.
#define RCSWITCH_MAX_MESS_SIZE 128
// Special pulse value used to signal a reset (timeout).
#define RESET_PULSE 0xFFFF
// Fixed ring buffer size for pulse storage.
#define PULSE_BUFFER_SIZE 256

class switch_433 {
public:
    switch_433();
    ~switch_433();
    static bool OokAvailable();
    static bool getOokCode(char * _dest);
    static void OokResetAvailable();

private:
    // Ring buffer for storing pulse durations (in µs)
    static uint16_t ringBuffer[PULSE_BUFFER_SIZE];
    static std::atomic<size_t> head;
    static std::atomic<size_t> tail;

    // Shared static members for the decoded message.
    static char OokReceivedCode[RCSWITCH_MAX_MESS_SIZE];
    static bool OokAvailableCode;
    static std::mutex decodeMutex;

    // Decoding thread and control flag.
    std::thread decoderThread;
    std::atomic<bool> running;

    // ISR handler.
    // (Note: pigpio alert functions require a callback with signature:
    //  void callback(int gpio, int level, uint32_t tick))
    static void ISRHandler(int gpio, int level, uint32_t tick);

    // Decoding loop that runs in its own thread.
    void decodingLoop();
};

#endif
