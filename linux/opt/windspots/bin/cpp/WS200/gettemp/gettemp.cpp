#include <cstdio>      // std::printf, std::fprintf
#include <cmath>       // std::log, std::round, std::isnan
#include <cstdint>     // std::int16_t
#include <limits>      // std::numeric_limits
#include "ads1015.h"

// Anonymous namespace for internal constants and helper functions
namespace {

// ADC / hardware constants
constexpr double ADC_FS_V      = 4.096;   // Full-scale for GAIN_ONE
constexpr double ADC_COUNTS    = 2048.0;  // 2^11 steps for positive range
constexpr double SUPPLY_V      = 3.3;     // ADS1015 powered from 3.3 V
constexpr double R_FIXED  = 10'000.0;   // Fixed resistor in divider (10k)

// Steinhart–Hart coefficients for your NTC (3D curve)
constexpr double A = 0.0009333357965;
constexpr double B = 0.0002340542448;
constexpr double C = 0.00000008044859863;

// Compute temperature in °C from resistance in ohms using Steinhart–Hart
double steinhartHartToCelsius(double resistance_ohm)
{
    if (resistance_ohm <= 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    const double log_r  = std::log(resistance_ohm);
    const double log_r3 = log_r * log_r * log_r;

    const double temp_k = 1.0 / (A + B * log_r + C * log_r3);
    return temp_k - 273.15;  // Kelvin → Celsius
}

} // namespace

int main()
{
    // Prefer automatic storage over new/delete
    ads1015 adc(0x48);

    // Configure ADC
    adc.setGain(GAIN_ONE);

    // Read raw ADC value (channel 1)
    const std::int16_t adc_value = adc.readADC_SingleEnded(1);

    // NOTE: The divisor (1648.0) comes from your original code.
    // Make sure this matches the effective full-scale count of your ADS1015 library.
    // const double volts = (static_cast<double>(raw_value) * V_REF) / 1648.0;
    double volts = static_cast<double>(adc_value) * ADC_FS_V / ADC_COUNTS;

    double resistance_ohm = 0.0;
    double temperature_c  = std::numeric_limits<double>::quiet_NaN();

    // Basic sanity checks to avoid division by zero and bogus values
    if (volts > 0.0 && volts < SUPPLY_V) {
        // Voltage divider:
        // Vout = V_REF * (R_ntc / (R_fixed + R_ntc))
        // → R_ntc = (V_REF - Vout) * R_fixed / Vout
        resistance_ohm = (SUPPLY_V - volts) * R_FIXED / volts;
        resistance_ohm = std::round(resistance_ohm);

        temperature_c = steinhartHartToCelsius(resistance_ohm);
    }

    if (std::isnan(temperature_c)) {
        std::fprintf(stderr,
                     "Error: failed to compute temperature "
                     "(raw=%d, volts=%.4f, R=%.1f Ω)\n",
                     adc_value, volts, resistance_ohm);
        return 1;
    }

    std::printf("TEMPERATURE=%.1f C\n"
                "OHM=%.1f Ω\n"
                "VOLTAGE=%.4f V\n"
                "RAW=%d\n",
                temperature_c, resistance_ohm, volts, adc_value);

    return 0;
}