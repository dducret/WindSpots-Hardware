#include <cstdio>      // printf()
#include <cstdlib>     // exit()
#include <unistd.h>    // sleep()
#include <stdint.h>
#include <math.h>      // pow()
#include <pigpio.h>

#include "ads1015.h"

#define NEAREST(number, multiple) (((number) + ((multiple) / 2)) / (multiple) * (multiple))

// Globals updated by ISR callback
static volatile int anemometerCounter = 0;
static volatile uint32_t firstTick = 0;        // us
static volatile uint32_t lastTick  = 0;        // us
static volatile uint32_t fastestDeltaUs = 0;   // us

// pigpio ISR callback signature
// level: 0=falling, 1=rising, 2=watchdog timeout
static void count_cb(int gpio, int level, uint32_t tick)
{
  (void)gpio;

  if (level != 1) return; // only rising edges

  if (firstTick == 0) {
    firstTick = tick;
    lastTick = tick;
    fastestDeltaUs = 9999999u;
    ++anemometerCounter;
    return;
  }

  // pigpio tick is uint32_t microseconds; subtraction handles wrap-around correctly in unsigned arithmetic
  uint32_t deltaUs = tick - lastTick;

  // Jitter filter: 8 ms = 8000 us
  if (deltaUs < 8000u)
    return;

  ++anemometerCounter;

  if (deltaUs < fastestDeltaUs)
    fastestDeltaUs = deltaUs;

  lastTick = tick;
}

int main()
{
  ads1015 *ads = new ads1015(0x48);

  // Get direction
  ads->setGain(GAIN_ONE);
  int adc_value = ads->readADC_SingleEnded(0); // port 0 ADS1015
  double direction = adc_value * 360 / (double)1648;
  direction = NEAREST(direction, 5);
  if (direction > 360) direction -= 360;
  if (direction == 360) direction = 0;

  // Get speed
  double speed = 0.0;
  anemometerCounter = 0;
  firstTick = 0;
  lastTick = 0;
  fastestDeltaUs = 0;

  // pigpio init
  if (gpioInitialise() < 0) {
    printf("Error during pigpio Initialization\n");
    delete ads;
    return 1;
  }

  const int GPIO_ANEMO = 17;

  gpioSetMode(GPIO_ANEMO, PI_INPUT);

  // Optional (often useful for reed/hall sensors): enable pull-up
  // Use PI_PUD_UP or PI_PUD_DOWN depending on your wiring.
  // gpioSetPullUpDown(GPIO_ANEMO, PI_PUD_UP);

  // Register ISR for rising edge; timeout 0 means no watchdog
  if (gpioSetISRFunc(GPIO_ANEMO, RISING_EDGE, 0, count_cb) != 0) {
    printf("Failed to set ISR on GPIO %d\n", GPIO_ANEMO);
    gpioTerminate();
    delete ads;
    return 1;
  }

  // Measure over 2 seconds using pigpio ticks (microseconds)
  uint32_t startTick = gpioTick();
  sleep(2);
  uint32_t endTick = gpioTick();

  double elapsedSeconds = (double)(endTick - startTick) / 1e6;
  speed = (elapsedSeconds > 0.0) ? ((double)anemometerCounter / elapsedSeconds) : 0.0;

  // Disable ISR
  gpioSetISRFunc(GPIO_ANEMO, RISING_EDGE, 0, nullptr);

  gpioTerminate();
  delete ads;

  printf("SPEED=%.2f\n", speed);
  printf("DIRECTION=%.0f\n", direction);
  return 0;
}
