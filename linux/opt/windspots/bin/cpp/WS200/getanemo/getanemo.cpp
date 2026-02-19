#include <cstdio>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <dirent.h>

#include "ads1015.h"

#define NEAREST(number, multiple) (((number) + ((multiple) / 2)) / (multiple) * (multiple))

static int64_t monotonicUs()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (int64_t)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL;
}

static int requestAnemometerEventFd(unsigned int lineOffset)
{
  int eventFd = -1;
  int lastErrno = ENOENT;

  DIR *dir = opendir("/dev");
  if (dir == NULL) {
    return -1;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strncmp(entry->d_name, "gpiochip", 8) != 0) {
      continue;
    }

    char chipPath[128];
    snprintf(chipPath, sizeof(chipPath), "/dev/%.118s", entry->d_name);

    int chipFd = open(chipPath, O_RDONLY);
    if (chipFd < 0) {
      lastErrno = errno;
      continue;
    }

    struct gpioevent_request req;
    memset(&req, 0, sizeof(req));
    req.lineoffset = lineOffset;
    req.handleflags = GPIOHANDLE_REQUEST_INPUT;
#ifdef GPIOHANDLE_REQUEST_BIAS_PULL_UP
    req.handleflags |= GPIOHANDLE_REQUEST_BIAS_PULL_UP;
#endif
    req.eventflags = GPIOEVENT_REQUEST_RISING_EDGE;
    snprintf(req.consumer_label, sizeof(req.consumer_label), "getanemo");

    if (ioctl(chipFd, GPIO_GET_LINEEVENT_IOCTL, &req) == 0) {
      eventFd = req.fd;
      close(chipFd);
      break;
    }

    lastErrno = errno;
    close(chipFd);
  }

  closedir(dir);

  if (eventFd < 0) {
    errno = lastErrno;
  }

  return eventFd;
}

static double measureSpeedFromGpioEvents(unsigned int gpioLine)
{
  int eventFd = requestAnemometerEventFd(gpioLine);
  if (eventFd < 0) {
    return -1.0;
  }

  struct pollfd pfd;
  pfd.fd = eventFd;
  pfd.events = POLLIN;

  const int64_t measurementUs = 2000000LL; // 2s
  const int64_t debounceUs = 5000LL;       // same idea as w3rpi.cpp

  int pulses = 0;
  int64_t startUs = monotonicUs();
  int64_t lastPulseUs = startUs;

  while (true) {
    int64_t elapsedUs = monotonicUs() - startUs;
    if (elapsedUs >= measurementUs) {
      break;
    }

    int remainingMs = (int)((measurementUs - elapsedUs) / 1000LL);
    if (remainingMs < 1) {
      remainingMs = 1;
    }

    int status = poll(&pfd, 1, remainingMs);
    if (status <= 0) {
      if (status < 0 && errno != EINTR) {
        close(eventFd);
        return -1.0;
      }
      continue;
    }

    if ((pfd.revents & POLLIN) == 0) {
      continue;
    }

    struct gpioevent_data eventData;
    ssize_t bytesRead = read(eventFd, &eventData, sizeof(eventData));
    if (bytesRead != sizeof(eventData)) {
      continue;
    }

    if (eventData.id == GPIOEVENT_EVENT_RISING_EDGE) {
      int64_t nowUs = monotonicUs();
      if ((nowUs - lastPulseUs) >= debounceUs) {
        ++pulses;
        lastPulseUs = nowUs;
      }
    }
  }

  close(eventFd);

  double elapsedSeconds = (double)(monotonicUs() - startUs) / 1e6;
  return (elapsedSeconds > 0.0) ? ((double)pulses / elapsedSeconds) : 0.0;
}

static bool writeTextFile(const char *path, const char *value)
{
  int fd = open(path, O_WRONLY);
  if (fd < 0) {
    return false;
  }

  ssize_t len = (ssize_t)strlen(value);
  ssize_t written = write(fd, value, len);
  close(fd);
  return written == len;
}

static double measureSpeedFromSysfsSampling(unsigned int gpioLine)
{
  char gpioPath[128];
  snprintf(gpioPath, sizeof(gpioPath), "/sys/class/gpio/gpio%u", gpioLine);

  if (access(gpioPath, F_OK) != 0) {
    char gpioNumber[16];
    snprintf(gpioNumber, sizeof(gpioNumber), "%u", gpioLine);
    if (!writeTextFile("/sys/class/gpio/export", gpioNumber) && errno != EBUSY) {
      return -1.0;
    }
  }

  char valuePath[160];
  snprintf(valuePath, sizeof(valuePath), "/sys/class/gpio/gpio%u/value", gpioLine);
  int valueFd = open(valuePath, O_RDONLY | O_NONBLOCK);
  if (valueFd < 0) {
    return -1.0;
  }

  const int64_t measurementUs = 2000000LL;
  const int64_t debounceUs = 5000LL;

  int pulses = 0;
  int64_t startUs = monotonicUs();
  int64_t lastPulseUs = 0;

  char prev = '0';
  lseek(valueFd, 0, SEEK_SET);
  if (read(valueFd, &prev, 1) != 1) {
    prev = '0';
  }

  while (true) {
    int64_t elapsedUs = monotonicUs() - startUs;
    if (elapsedUs >= measurementUs) {
      break;
    }

    usleep(1000); // 1 ms polling

    char cur;
    lseek(valueFd, 0, SEEK_SET);
    if (read(valueFd, &cur, 1) != 1) {
      continue;
    }

    if (prev == '0' && cur == '1') {
      int64_t nowUs = monotonicUs();
      if (lastPulseUs == 0 || (nowUs - lastPulseUs) >= debounceUs) {
        ++pulses;
        lastPulseUs = nowUs;
      }
    }

    prev = cur;
  }

  close(valueFd);

  double elapsedSeconds = (double)(monotonicUs() - startUs) / 1e6;
  return (elapsedSeconds > 0.0) ? ((double)pulses / elapsedSeconds) : 0.0;
}


static bool isW3rpiRunning()
{
  DIR *dir = opendir("/proc");
  if (dir == NULL) {
    return false;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_name[0] < '0' || entry->d_name[0] > '9') {
      continue;
    }

    char commPath[128];
    snprintf(commPath, sizeof(commPath), "/proc/%.116s/comm", entry->d_name);

    int fd = open(commPath, O_RDONLY);
    if (fd < 0) {
      continue;
    }

    char name[128] = {0};
    ssize_t n = read(fd, name, sizeof(name) - 1);
    close(fd);

    if (n <= 0) {
      continue;
    }

    if (name[n - 1] == '\n') {
      name[n - 1] = '\0';
    }

    if (strcmp(name, "w3rpi") == 0) {
      closedir(dir);
      return true;
    }
  }

  closedir(dir);
  return false;
}

static bool loadLatestWindFromWsDb(double *speed, double *direction)
{
  const char *dbPaths[] = {"/var/tmp/ws.db", "/tmp/ws.db"};
  const char *query = "SELECT wind_direction, wind_speed_average FROM data WHERE name='WS200' ORDER BY id DESC LIMIT 1
;";

  for (size_t i = 0; i < sizeof(dbPaths) / sizeof(dbPaths[0]); ++i) {
    if (access(dbPaths[i], R_OK) != 0) {
      continue;
    }

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "sqlite3 -separator ',' %s \"%s\" 2>/dev/null", dbPaths[i], query);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
      continue;
    }

    char line[256] = {0};
    bool ok = false;
    if (fgets(line, sizeof(line), fp) != NULL) {
      double dbDirection = 0.0;
      double dbSpeed = 0.0;
      if (sscanf(line, "%lf,%lf", &dbDirection, &dbSpeed) == 2) {
        *direction = dbDirection;
        *speed = dbSpeed;
        ok = true;
      }
    }

    pclose(fp);

    if (ok) {
      return true;
    }
  }

  return false;
}

int main()
{
  ads1015 ads(0x48);

  // Direction (same as previous behavior)
  ads.setGain(GAIN_ONE);
  int adc_value = ads.readADC_SingleEnded(0);
  double direction = adc_value * 360 / (double)1648;
  direction = NEAREST(direction, 5);
  if (direction > 360) direction -= 360;
  if (direction == 360) direction = 0;

  // Speed over 2 seconds (same GPIO event style as w3rpi.cpp)
  if (isW3rpiRunning()) {
    printf("w3rpi process is running and already uses the anemometer GPIO. Stop w3rpi before running getanemo.\n");
    return 1;
  }

  const unsigned int GPIO_ANEMO = 17;
  double speed = measureSpeedFromGpioEvents(GPIO_ANEMO);
  if (speed < 0.0 && errno == EBUSY) {
    // If another process (e.g. w3rpi) already owns the GPIO line event, fallback to passive sysfs sampling.
    speed = measureSpeedFromSysfsSampling(GPIO_ANEMO);

    // If sysfs is unavailable too, reuse last measurement produced by running w3rpi.
    if (speed < 0.0) {
      double dbDirection = direction;
      if (loadLatestWindFromWsDb(&speed, &dbDirection)) {
        direction = dbDirection;
      }
    }
  }

  if (speed < 0.0) {
    printf("Failed to measure GPIO line %u (gpiochip/sysfs/db): %s\n", GPIO_ANEMO, strerror(errno));
    return 1;
  }

  printf("SPEED=%.2f\n", speed);
  printf("DIRECTION=%.0f\n", direction);
  return 0;
}
