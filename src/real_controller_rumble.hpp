#pragma once
#ifndef PRO__REAL_RUMBLE_HPP
#define PRO__REAL_RUMBLE_HPP

#include <array>

namespace RealController {
  namespace Rumble {
    namespace Constants {
      constexpr double amplitude_min   { 0.0 };
      constexpr double amplitude_max   { 1.0 };

      constexpr double highFreq_min   { 81.75177 };
      constexpr double highFreq_max { 1252.572266 };

      constexpr double lowFreq_min    { 40.875885 };
      constexpr double lowFreq_max   { 626.286133 };

      constexpr double highFreq_neutral  { 320 };
      constexpr double lowFreq_neutral   { 160 };
    };

    using RumbleArray = std::array<uint8_t, 4>;

    /**
     * @brief Encodes frequency and amplitude to controller byte format. The parameters will be clamped to theirs own limits.
     * 
     * @param high_frequency Between 81.75177 and 1252.572266 [Hz].
     * @param low_frequency Between 40.875885 and 626.286133 [Hz].
     * @param high_amplitude Between 0 and 1.
     * @param low_amplitude Between 0 and 1.
     * 
     * @return The encoded 4 byte array.
     */
    RumbleArray rumble(double high_frequency, double low_frequency, double high_amplitude, double low_amplitude);
    /// Equal to the other rumble, but uses the same amplitude for both frequencies.
    RumbleArray rumble(double high_frequency, double low_frequency, double amplitude);

    uint8_t highAmplitude(double amp);
    uint16_t lowAmplitude(double amp);

    uint16_t highFrequency(double freq);
    uint8_t   lowFrequency(double freq);


    double decodeHighAmplitude(uint8_t amp);
    double  decodeLowAmplitude(uint16_t amp);

    double decodeHighFrequency(uint16_t freq);
    double  decodeLowFrequency(uint8_t freq);
  };
};

#endif
