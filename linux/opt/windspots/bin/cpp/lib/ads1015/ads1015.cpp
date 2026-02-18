#include "ads1015.h"

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {

// Path to I²C bus on Raspberry Pi
constexpr const char* I2C_DEVICE = "/dev/i2c-1";

// File descriptor used during a transaction
int i2cHandle = -1;

bool beginTransmission(uint8_t i2cAddress)
{
    i2cHandle = open(I2C_DEVICE, O_RDWR);
    if (i2cHandle < 0) {
        std::fprintf(stderr,
                     "ADS1015: failed to open %s: %s\n",
                     I2C_DEVICE, std::strerror(errno));
        return false;
    }

    if (ioctl(i2cHandle, I2C_SLAVE, i2cAddress) < 0) {
        std::fprintf(stderr,
                     "ADS1015: failed to select slave 0x%02X on %s: %s\n",
                     i2cAddress, I2C_DEVICE, std::strerror(errno));
        close(i2cHandle);
        i2cHandle = -1;
        return false;
    }

    return true;
}

void endTransmission()
{
    if (i2cHandle >= 0) {
        close(i2cHandle);
        i2cHandle = -1;
    }
}

// Convert between ADS1015 MSB-first and SMBus LSB-first word layout
inline uint16_t swapBytes(uint16_t value)
{
    uint8_t msb = static_cast<uint8_t>(value >> 8);
    uint8_t lsb = static_cast<uint8_t>(value & 0xFF);
    return static_cast<uint16_t>((lsb << 8) | msb);
}

// Common base config for single-shot conversions
inline uint16_t makeSingleShotConfig(adsGain_t gain, adsSps_t sps)
{
    return static_cast<uint16_t>(
        ADS1015_REG_CONFIG_CQUE_NONE    | // Disable comparator
        ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching
        ADS1015_REG_CONFIG_CPOL_ACTVLOW | // ALERT/RDY active low
        ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator
        ADS1015_REG_CONFIG_MODE_SINGLE  | // Single-shot mode
        gain                            | // PGA / voltage range
        sps                             | // Sample rate
        ADS1015_REG_CONFIG_OS_SINGLE      // Start single conversion
    );
}

} // namespace

// ---- Low-level register helpers ----------------------------------------

static void writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value)
{
    if (!beginTransmission(i2cAddress)) {
        return;
    }

    // SMBus sends low byte first, ADS1015 expects MSB first → swap
    const uint16_t payload = swapBytes(value);

    const int rc = i2c_smbus_write_word_data(i2cHandle, reg, payload);
    if (rc < 0) {
        std::fprintf(stderr,
                     "ADS1015: write failure to reg 0x%02X: %s\n",
                     reg, std::strerror(errno));
    }

    endTransmission();
}

static uint16_t readRegister(uint8_t i2cAddress, uint8_t reg)
{
    if (!beginTransmission(i2cAddress)) {
        return 0;
    }

    const int rc = i2c_smbus_read_word_data(i2cHandle, reg);
    if (rc < 0) {
        std::fprintf(stderr,
                     "ADS1015: read failure from reg 0x%02X: %s\n",
                     reg, std::strerror(errno));
        endTransmission();
        return 0;
    }

    endTransmission();

    // SMBus returns LSB-first; convert back to MSB-first
    const uint16_t raw = static_cast<uint16_t>(rc);
    return swapBytes(raw);
}

// ---- ads1015 class implementation --------------------------------------

ads1015::ads1015(uint8_t i2cAddress)
{
    m_i2cAddress = i2cAddress;
    m_bitShift   = 4;                    // ADS1015: 12-bit result (shift by 4)
    m_gain       = GAIN_TWOTHIRDS;       // +/- 6.144V (limited by VDD)
    m_sps        = SPS_1600;             // Reasonable default
    setConversionDelay();
}

void ads1015::setGain(adsGain_t gain)
{
    m_gain = gain;
}

adsGain_t ads1015::getGain()
{
    return m_gain;
}

void ads1015::setSps(adsSps_t sps)
{
    m_sps = sps;
    setConversionDelay();
}

adsSps_t ads1015::getSps()
{
    return m_sps;
}

uint16_t ads1015::readADC_SingleEnded(uint8_t channel)
{
    if (channel > 3) {
        std::fprintf(stderr, "ADS1015: invalid single-ended channel %u\n", channel);
        return 0;
    }

    uint16_t config = makeSingleShotConfig(m_gain, m_sps);

    // Select input channel
    switch (channel) {
        case 0:
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
            break;
        case 1:
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
            break;
        case 2:
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
            break;
        case 3:
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
            break;
        default:
            // Should not happen due to check above
            return 0;
    }

    // Write config register and wait for conversion
    writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);
    usleep(m_conversionDelay);

    // Read conversion result and shift to 12 bits for ADS1015
    return readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;
}

int16_t ads1015::readADC_Differential_0_1()
{
    uint16_t config = makeSingleShotConfig(m_gain, m_sps);

    // AIN0 = P, AIN1 = N
    config |= ADS1015_REG_CONFIG_MUX_DIFF_0_1;

    writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);
    usleep(m_conversionDelay);

    uint16_t res = readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;

    if (m_bitShift == 0) {
        return static_cast<int16_t>(res);
    }

    // For ADS1015: 12-bit data, sign extend if negative
    if (res > 0x07FF) {
        res |= 0xF000;
    }
    return static_cast<int16_t>(res);
}

int16_t ads1015::readADC_Differential_2_3()
{
    uint16_t config = makeSingleShotConfig(m_gain, m_sps);

    // AIN2 = P, AIN3 = N
    config |= ADS1015_REG_CONFIG_MUX_DIFF_2_3;

    writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);
    usleep(m_conversionDelay);

    uint16_t res = readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;

    if (m_bitShift == 0) {
        return static_cast<int16_t>(res);
    }

    // For ADS1015: 12-bit data, sign extend if negative
    if (res > 0x07FF) {
        res |= 0xF000;
    }
    return static_cast<int16_t>(res);
}

void ads1015::startComparator_SingleEnded(uint8_t channel, int16_t threshold)
{
    if (channel > 3) {
        std::fprintf(stderr, "ADS1015: invalid comparator channel %u\n", channel);
        return;
    }

    uint16_t config =
        ADS1015_REG_CONFIG_CQUE_1CONV   | // Comparator enabled, assert on 1 match
        ADS1015_REG_CONFIG_CLAT_LATCH   | // Latching comparator
        ADS1015_REG_CONFIG_CPOL_ACTVLOW | // ALERT/RDY active low
        ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator
        ADS1015_REG_CONFIG_MODE_CONTIN;   // Continuous conversion mode

    // PGA and data rate
    config |= m_gain;
    config |= m_sps;

    // Select channel
    switch (channel) {
        case 0:
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
            break;
        case 1:
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
            break;
        case 2:
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
            break;
        case 3:
            config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
            break;
    }

    // High threshold – shift 12-bit threshold left for ADS1015
    writeRegister(m_i2cAddress, ADS1015_REG_POINTER_HITHRESH,
                  static_cast<uint16_t>(threshold) << m_bitShift);

    // Start continuous conversions with comparator
    writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);
}

int16_t ads1015::getLastConversionResults()
{
    usleep(m_conversionDelay);

    uint16_t res = readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;

    if (m_bitShift == 0) {
        return static_cast<int16_t>(res);
    }

    // For ADS1015: 12-bit data, sign extend if negative
    if (res > 0x07FF) {
        res |= 0xF000;
    }
    return static_cast<int16_t>(res);
}

void ads1015::setConversionDelay()
{
    // Conversion delay in microseconds: 1e6 / SPS, plus small margin
    switch (m_sps) {
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
        case SPS_860:          // Typically ADS1115, but handle if present in your enum
            m_conversionDelay = 1000000 / 860;
            break;
        default:
            m_conversionDelay = 8000; // Fallback
            break;
    }

    m_conversionDelay += 100; // Add 100 µs safety margin
}
