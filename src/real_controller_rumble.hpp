#pragma once
#ifndef PRO__REAL_RUMBLE_HPP
#define PRO__REAL_RUMBLE_HPP

#include <array>

namespace RealController {
  namespace Rumble {
    constexpr double middle_highFreq {320};
    constexpr double middle_lowFreq  {160};

    /**
     * @brief
     * 
     * @param amplitude Between 0 and 1.
     * @param high_frequency Between 81.75177 and 1252.572266 [Hz].
     * @param low_frequency Between 40.875885 and 626.286133 [Hz].
     */
    std::array<uint8_t, 4> rumble(double amplitude, double high_frequency, double low_frequency);

    uint8_t highAmplitude(double amplitude);
    uint16_t lowAmplitude(double amplitude);

    uint16_t highFrequency(double freq);
    uint8_t   lowFrequency(double freq);
  };
};

#endif
