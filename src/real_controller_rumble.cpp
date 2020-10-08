#include "real_controller_rumble.hpp"
using namespace RealController;

#include <cmath>
#include "utils.hpp"

std::array<uint8_t, 4> Rumble::rumble(double _high_freq, double _low_freq, double _high_ampl, double _low_ampl) {
  _high_ampl = Utils::Number::clamp<double>(_high_ampl, Rumble::Constants::amplitude_min, Rumble::Constants::amplitude_max);
  _low_ampl  = Utils::Number::clamp<double>(_low_ampl,  Rumble::Constants::amplitude_min, Rumble::Constants::amplitude_max);

  _high_freq = Utils::Number::clamp<double>(_high_freq, Rumble::Constants::highFreq_min, Rumble::Constants::highFreq_max);
  _low_freq  = Utils::Number::clamp<double>(_low_freq,  Rumble::Constants::lowFreq_min,  Rumble::Constants::lowFreq_max);


  uint8_t amp_high = highAmplitude(_high_ampl);
  uint16_t amp_low =  lowAmplitude(_low_ampl);

  uint16_t freq_high = highFrequency(_high_freq);
  uint8_t  freq_low  =  lowFrequency(_low_freq);

  std::array<uint8_t, 4> data {0};

  data[0] =  0xFF &  freq_high;
  data[1] = (0xFF & (freq_high >> 8)) + amp_high;

  data[2] = (0xFF & (amp_low >> 8)) + freq_low;
  data[3] =  0xFF &  amp_low;

  return data;
}

std::array<uint8_t, 4> Rumble::rumble(double high_freq, double low_freq, double ampl) {
  return rumble(ampl, ampl, high_freq, low_freq);
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
  return 0x40 + (high >> 2) + ((high&0x02) << 14);
}

uint16_t Rumble::highFrequency(double freq) {
  return round((log2(freq * 0.0125L) * 128) / 4)*4;
}
uint8_t   Rumble::lowFrequency(double freq) {
  return round((log2(freq * 0.025L) * 32));
}
