#include "real_controller_rumble.hpp"
using namespace RealController;

#include <cmath>
#include "utils.hpp"

std::array<uint8_t, 4> Rumble::rumble(double ampl, double high_freq, double low_freq) {
  ampl = Utils::Number::clamp(ampl, 0.L, 1.L);
  high_freq = Utils::Number::clamp(high_freq, 81.75177L, 1252.572266L);
  low_freq = Utils::Number::clamp(low_freq, 40.875885L, 626.286133L);

  uint8_t amp_high = highAmplitude(ampl);
  uint16_t amp_low =  lowAmplitude(ampl);

  uint16_t freq_high = highFrequency(high_freq);
  uint8_t  freq_low  =  lowFrequency(low_freq);

  std::array<uint8_t, 4> data {0};

  data[0] =  0xFF &  freq_high;
  data[1] = (0xFF & (freq_high >> 8)) + amp_high;

  data[2] = (0xFF & (amp_low >> 8)) + freq_low;
  data[3] =  0xFF &  amp_low;
  
  return data;
}

// https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/rumble_data_table.md
// https://docs.google.com/spreadsheets/d/1Sg12Sv8iFP4C8pbEmW-e58YEuU3hD_Yo6_yhG3xk5LM/edit?usp=sharing
uint8_t Rumble::highAmplitude(double amplitude) {
  if (amplitude < 0.007666L) {
    return 0;
  }

  if (amplitude < 0.011823L) {
    return 2;
  }

  if (amplitude <= 0.112491L) {
    return round((log2(amplitude * 119.6128L) * 8) / 2)*2;
  }

  if (amplitude <= 0.224982L) {
    return round((log2(amplitude * 17.0256L) * 32) / 2)*2;
  }

  return round((log2(amplitude * 8.699L) * 64) / 2)*2;
}

uint16_t Rumble::lowAmplitude(double amplitude) {
  uint16_t high = highAmplitude(amplitude);
  return (high << 6) + 0x4000;
}

uint16_t Rumble::highFrequency(double freq) {
  return round((log2(freq * 0.0125L) * 128) / 4)*4;
}
uint8_t   Rumble::lowFrequency(double freq) {
  return round((log2(freq * 0.025L) * 32));
}
