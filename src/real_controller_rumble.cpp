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
uint8_t Rumble::highAmplitude(double amp) {
  if (amp < 0.007666L) {
    return 0;
  }

  if (amp < 0.011823L) {
    return 2;
  }

  if (amp <= 0.112491L) {
    return round((log2(amp * 119.6128L) * 8) / 2)*2;
  }

  if (amp <= 0.224982L) {
    return round((log2(amp * 17.0256L) * 32) / 2)*2;
  }

  return round((log2(amp * 8.699L) * 64) / 2)*2;
}

uint16_t Rumble::lowAmplitude(double amp) {
  uint16_t high = highAmplitude(amp);
  return 0x40 + (high >> 2) + ((high&0x02) << 14);
}

uint16_t Rumble::highFrequency(double freq) {
  return round((log2(freq * 0.0125L) * 128) / 4)*4;
}
uint8_t   Rumble::lowFrequency(double freq) {
  return round((log2(freq * 0.025L) * 32));
}


double Rumble::decodeHighAmplitude(uint8_t _amp) {
  double amp = _amp;
  if (amp == 0x00) {
    return 0;
  }

  if (amp <= 0x02) {
    return 0.007843;
  }

  if (amp <= 0x1e) {
    return pow(2, amp/8) / 119.6128L;
  }

  if (amp <= 0x3e) {
    return pow(2, amp/32) / 17.0256L;
  }

  return pow(2, amp/64) / 8.699L;
}

double Rumble::decodeLowAmplitude(uint16_t _amp) {
  uint16_t amp = _amp - 0x40;
  amp = ((amp & 0x8000) >> 14) + ((amp & 0x00FF) << 2);
  return decodeHighAmplitude(amp);
}

double Rumble::decodeHighFrequency(uint16_t _freq) {
  double freq = _freq;
  return pow(2, freq/128) / 0.0125L;
}
double  Rumble::decodeLowFrequency(uint8_t _freq) {
  double freq = _freq;
  return pow(2, freq/32) / 0.025L;
}
