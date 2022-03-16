#include "ads1015.h"
int i2cHandle;
//
static void beginTransmission(uint8_t i2cAddress) {
  // Create the file descriptor for the i2c bus
  i2cHandle = open("/dev/i2c-1", O_RDWR);
  if(i2cHandle < 0) {
    fprintf(stdout, "Error while opening the i2c-2 device! Error: %s\n", strerror(errno));
    return;
  }
  // Set the slave address
  if(ioctl(i2cHandle, I2C_SLAVE, i2cAddress) < 0) {
    fprintf(stdout, "Error while configuring the slave address %d. Error: %s\n", i2cAddress, strerror(errno));
    return;
  }
}
static void endTransmission(void) {
  close(i2cHandle);
}
static void writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value) {
  uint8_t lsb = (uint8_t)(value >> 8);
  uint8_t msb = (uint8_t)value;
  uint16_t payload = (msb << 8) | lsb; 
  beginTransmission(i2cAddress);
  i2c_smbus_write_word_data(i2cHandle, reg, payload);
  endTransmission();
}
static uint16_t readRegister(uint8_t i2cAddress, uint8_t reg) {
  beginTransmission(i2cAddress);
  uint16_t res = i2c_smbus_read_word_data(i2cHandle, reg);
  endTransmission();
  uint8_t msb = (uint8_t)res;
  uint8_t lsb = (uint8_t)(res >> 8);
  return (msb << 8) | lsb;
}
ads1015::ads1015(uint8_t i2cAddress) {
  m_i2cAddress = i2cAddress;
  m_bitShift = 4;
  m_gain = GAIN_TWOTHIRDS; /* +/- 6.144V range (limited to VDD +0.3V max!) */
  m_sps  = SPS_1600;
  setConversionDelay();
}
void ads1015::setGain(adsGain_t gain){
  m_gain = gain;
}
adsGain_t ads1015::getGain(){
  return m_gain;
}
void ads1015::setSps(adsSps_t sps) {
  m_sps = sps;
  setConversionDelay();
}
adsSps_t ads1015::getSps(){
  return m_sps;
}
uint16_t ads1015::readADC_SingleEnded(uint8_t channel) {
  if (channel > 3) {
    return 0;
  }
  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)
  // Set PGA/voltage range
  config |= m_gain;
  // Set the sample rate
  config |= m_sps;
  // Set single-ended input channel
  switch (channel) {
    case (0):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
      break;
    case (1):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
      break;
    case (2):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
      break;
    case (3):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
      break;
  }
  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;
  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);
  // Wait for the conversion to complete
  usleep(m_conversionDelay);
  // Read the conversion results
  // Shift 12-bit results right 4 bits for the ADS1015
  return readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;  
}
int16_t ads1015::readADC_Differential_0_1() {
  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)
  // Set PGA/voltage range
  config |= m_gain;
  // Set the sample rate
  config |= m_sps;
  // Set channels
  config |= ADS1015_REG_CONFIG_MUX_DIFF_0_1;          // AIN0 = P, AIN1 = N
  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;
  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);
  // Wait for the conversion to complete
  usleep(m_conversionDelay);
  // Read the conversion results
  uint16_t res = readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;
  if (m_bitShift == 0) {
    return (int16_t)res;
  } else {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if (res > 0x07FF) {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}
int16_t ads1015::readADC_Differential_2_3() {
  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)
  // Set PGA/voltage range
  config |= m_gain;
  // Set the sample rate
  config |= m_sps;
  // Set channels
  config |= ADS1015_REG_CONFIG_MUX_DIFF_2_3;          // AIN2 = P, AIN3 = N
  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;
  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);
  // Wait for the conversion to complete
  usleep(m_conversionDelay);
  // Read the conversion results
  uint16_t res = readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;
  if (m_bitShift == 0) {
    return (int16_t)res;
  } else {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if (res > 0x07FF) {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}
void ads1015::startComparator_SingleEnded(uint8_t channel, int16_t threshold) {
  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_1CONV   | // Comparator enabled and asserts on 1 match
                    ADS1015_REG_CONFIG_CLAT_LATCH   | // Latching mode
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_MODE_CONTIN  | // Continuous conversion mode
                    ADS1015_REG_CONFIG_MODE_CONTIN;   // Continuous conversion mode
  // Set PGA/voltage range
  config |= m_gain;
  // Set the sample rate
  config |= m_sps;
  // Set single-ended input channel
  switch (channel) {
    case (0):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
      break;
    case (1):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
      break;
    case (2):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
      break;
    case (3):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
      break;
  }
  // Set the high threshold register
  // Shift 12-bit results left 4 bits for the ADS1015
  writeRegister(m_i2cAddress, ADS1015_REG_POINTER_HITHRESH, threshold << m_bitShift);
  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);
}
int16_t ads1015::getLastConversionResults() {
  // Wait for the conversion to complete
  usleep(m_conversionDelay);
  // Read the conversion results
  uint16_t res = readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;
  if (m_bitShift == 0) {
    return (int16_t)res;
  } else {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if (res > 0x07FF){
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}
void ads1015::setConversionDelay() {
  switch(m_sps) {
    case SPS_128:
      m_conversionDelay = 1000000 / 128;
      break;
    case SPS_250:
      m_conversionDelay = 1000000 / 250;
      break;
    case SPS_490:
      m_conversionDelay = 1000000 / 490;
      break;
    case SPS_920:
      m_conversionDelay = 1000000 / 920;
      break;
    case SPS_1600:
      m_conversionDelay = 1000000 / 1600;
      break;
    case SPS_2400:
      m_conversionDelay = 1000000 / 2400;
      break;
    case SPS_3300:
      m_conversionDelay = 1000000 / 3300;
      break;
    case SPS_860:
      m_conversionDelay = 1000000 / 3300;
      break;
    default:
      m_conversionDelay = 8000;
      break;
  }
  m_conversionDelay += 100; // Add 100 us to be safe
}