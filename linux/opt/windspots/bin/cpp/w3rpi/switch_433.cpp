#include "switch_433.h"
#include "ook_433.h"    // Assume Oregon decoder implementations exist here.
#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <thread>

// Define static variables.
uint16_t switch_433::ringBuffer[PULSE_BUFFER_SIZE] = {0};
std::atomic<size_t> switch_433::head{0};
std::atomic<size_t> switch_433::tail{0};

char switch_433::OokReceivedCode[RCSWITCH_MAX_MESS_SIZE] = {0};
bool switch_433::OokAvailableCode = false;
std::mutex switch_433::decodeMutex;

// Global decoder pointers.
OregonDecoderV2 *orscV2 = nullptr;
OregonDecoderV3 *orscV3 = nullptr;

// Global instance pointer for ISR access (assuming single instance).
static switch_433* g_instance = nullptr;

// Define our own tickDiff function if not provided by pigpio.
static inline uint32_t tickDiff(uint32_t tickStart, uint32_t tickEnd)
{
    return (tickEnd >= tickStart) ? (tickEnd - tickStart) : ((0xFFFFFFFF - tickStart) + tickEnd);
}

switch_433::switch_433() : running(true) {
    g_instance = this;
    OokAvailableCode = false;
    OokReceivedCode[0] = '\0';
    // Initialize the decoders.
    orscV2 = new OregonDecoderV2(this);
    orscV3 = new OregonDecoderV3(this);
    // Set GPIO pin 19 to input using pigpio.
    gpioSetMode(19, PI_INPUT);
    // Attach the alert callback (ISR) to pin 19.
    gpioSetAlertFunc(19, &ISRHandler);
    // Start the decoding thread.
    decoderThread = std::thread(&switch_433::decodingLoop, this);
}

switch_433::~switch_433() {
    running = false;
    if(decoderThread.joinable())
        decoderThread.join();
    delete orscV2;
    delete orscV3;
    // Remove the alert callback.
    gpioSetAlertFunc(19, NULL);
    g_instance = nullptr;
}

bool switch_433::OokAvailable() {
    std::lock_guard<std::mutex> lock(decodeMutex);
    return OokAvailableCode;
}

bool switch_433::getOokCode(char * _dest) {
    std::lock_guard<std::mutex> lock(decodeMutex);
    if(OokAvailableCode) {
        strncpy(_dest, OokReceivedCode, RCSWITCH_MAX_MESS_SIZE - 1);
        _dest[RCSWITCH_MAX_MESS_SIZE - 1] = '\0';
        OokAvailableCode = false;
        return true;
    }
    return false;
}

void switch_433::OokResetAvailable() {
    std::lock_guard<std::mutex> lock(decodeMutex);
    OokAvailableCode = false;
}

// ISR callback using pigpio's signature.
// This function is called on every level change of pin 19.
void switch_433::ISRHandler(int gpio, int level, uint32_t tick) {
    if(g_instance == nullptr)
        return;
    // Ignore level 2 (no valid level).
    if(level == 2)
        return;
    // Use a static variable to keep track of the last tick.
    static uint32_t lastTick = 0;
    // On the first call, initialize lastTick.
    if(lastTick == 0) {
        lastTick = tick;
        return;
    }
    // Calculate the pulse duration in microseconds.
    uint32_t duration = tickDiff(lastTick, tick);
    lastTick = tick;
    uint16_t pulse = (duration < 32000 ? duration : RESET_PULSE);

    // Write the pulse into the fixed ring buffer.
    size_t curHead = g_instance->head.load(std::memory_order_relaxed);
    size_t nextHead = (curHead + 1) % PULSE_BUFFER_SIZE;
    if(nextHead != g_instance->tail.load(std::memory_order_acquire)) {
        g_instance->ringBuffer[curHead] = pulse;
        g_instance->head.store(nextHead, std::memory_order_release);
    }
    // No further operations or locks in the ISR.
}

// Decoding loop: continuously poll the ring buffer and process pulses.
void switch_433::decodingLoop() {
    while(running) {
        size_t curTail = tail.load(std::memory_order_acquire);
        size_t curHead = head.load(std::memory_order_acquire);
        if(curTail == curHead) {
            // Buffer is empty; sleep briefly.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        uint16_t pulse = ringBuffer[curTail];
        tail.store((curTail + 1) % PULSE_BUFFER_SIZE, std::memory_order_release);

        if(pulse == RESET_PULSE) {
            orscV2->resetDecoder();
            orscV3->resetDecoder();
            continue;
        }

        // Feed the pulse to both decoders.
        bool doneV2 = orscV2->nextPulse(pulse);
        bool doneV3 = orscV3->nextPulse(pulse);
        if(doneV2 || doneV3) {
            std::lock_guard<std::mutex> lock(decodeMutex);
            if(doneV2)
                orscV2->sprint("OS2:", OokReceivedCode);
            else if(doneV3)
                orscV3->sprint("OS3:", OokReceivedCode);
            OokAvailableCode = true;
            orscV2->resetDecoder();
            orscV3->resetDecoder();
        }
    }
}