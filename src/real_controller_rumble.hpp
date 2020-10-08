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

    /**
     * @brief Encodes amplitude and frequency to controller byte format. The parameters will be clamped to theirs own limits.
     * 
     * @param high_amplitude Between 0 and 1.
     * @param low_amplitude Between 0 and 1.
     * @param high_frequency Between 81.75177 and 1252.572266 [Hz].
     * @param low_frequency Between 40.875885 and 626.286133 [Hz].
     * 
     * @return The encoded 4 byte array.
     */
    std::array<uint8_t, 4> rumble(double high_amplitude, double low_amplitude, double high_frequency, double low_frequency);
    /// Equal to the other rumble, but uses the same amplitude for both frequencies.
    std::array<uint8_t, 4> rumble(double amplitude, double high_frequency, double low_frequency);

    uint8_t highAmplitude(double amplitude);
    uint16_t lowAmplitude(double amplitude);

    uint16_t highFrequency(double freq);
    uint8_t   lowFrequency(double freq);
  };
};

#endif
