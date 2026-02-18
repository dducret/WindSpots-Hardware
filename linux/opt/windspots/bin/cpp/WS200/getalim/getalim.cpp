#include <cstdio> // printf()
#include <unistd.h> // sleep()
#include "ina219.h"
//
int main() {
  ina219 ina0(0x40); // Solar
  ina219 ina1(0x41); // Battery
  ina219 ina3(0x43); // 5v
  ina219 ina4(0x44); // ??
  float busvoltage = 0;
  float current_mA = 0;
      // ina1(0x41); // Battery
  current_mA = ina1.getBusVoltage_V() * (ina1.getCurrent_mA() * 8.2);
  printf("BATTERYPOWER=%0.2f\n", current_mA);
  busvoltage = ina1.getBusVoltage_V();
  printf("BATTERYVOLTAGE=%0.2f\n", busvoltage);
              // ina0(0x40); // Solar
  current_mA = ina0.getBusVoltage_V() * (ina0.getCurrent_mA() * 8.2);
  printf("SOLARPOWER=%0.2f\n", current_mA);
  busvoltage = ina0.getBusVoltage_V();
  printf("SOLARVOLTAGE=%0.2f\n", busvoltage);
  // ina3(0x43); // 5v
  current_mA = ina3.getBusVoltage_V() * (ina3.getCurrent_mA() * 8.2);
  printf("LOADPOWER=%0.2f\n", current_mA);
  busvoltage = ina3.getBusVoltage_V();
  printf("LOADVOLTAGE=%0.2f\n", busvoltage);
  // ina4(0x44); // 5v
  // current_mA = ina4.getBusVoltage_V() * (ina4.getCurrent_mA() * 8.2);
  // printf("LOADPOWER=%0.2f\n", current_mA);
  // busvoltage = ina4.getBusVoltage_V();
  // printf("LOADVOLTAGE=%0.2f\n", busvoltage);
}
