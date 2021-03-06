#include "real_controller_rumble.hpp"
using namespace RealController;

#include <cmath>
#include "utils.hpp"

Rumble::RumbleArray Rumble::rumble(double _high_freq, double _low_freq, double _high_ampl, double _low_ampl) {
  _high_ampl = Utils::Number::clamp<double>(_high_ampl, Rumble::Constants::amplitude_min, Rumble::Constants::amplitude_max);
  _low_ampl  = Utils::Number::clamp<double>(_low_ampl,  Rumble::Constants::amplitude_min, Rumble::Constants::amplitude_max);

  _high_freq = Utils::Number::clamp<double>(_high_freq, Rumble::Constants::highFreq_min, Rumble::Constants::highFreq_max);
  _low_freq  = Utils::Number::clamp<double>(_low_freq,  Rumble::Constants::lowFreq_min,  Rumble::Constants::lowFreq_max);


  uint8_t amp_high = amplitude(_high_ampl);
  uint8_t amp_low  = amplitude(_low_ampl);

  uint8_t freq_high = highFrequency(_high_freq);
  uint8_t freq_low  =  lowFrequency(_low_freq);

  Rumble::RumbleArray data {0};

  data[0] = freq_high << 2;
  data[1] = ((freq_high >> 6) & 0x01) + (amp_high << 1);

  data[2] = ((amp_low & 0x01) << 7) + freq_low;
  data[3] = 0x40 + (amp_low >> 1);

  return data;
}

Rumble::RumbleArray Rumble::rumble(double high_freq, double low_freq, double ampl) {
  return rumble(high_freq, low_freq, ampl, ampl);
}


// https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/rumble_data_table.md
// https://docs.google.com/spreadsheets/d/1Sg12Sv8iFP4C8pbEmW-e58YEuU3hD_Yo6_yhG3xk5LM/edit?usp=sharing
uint8_t Rumble::amplitude(double amp) {
  if (amp < 0.007666L) {
    return 0;
  }

  if (amp < 0.011823L) {
    return 2;
  }

  if (amp <= 0.112491L) {
    return round((log2(amp * 119.6128L) * 4));
  }

  if (amp <= 0.224982L) {
    return round((log2(amp * 17.0256L) * 16));
  }

  return round((log2(amp * 8.699L) * 32));
}

uint8_t Rumble::highFrequency(double freq) {
  return round((log2(freq / 80) * 32));
}
uint8_t   Rumble::lowFrequency(double freq) {
  return round((log2(freq / 40) * 32));
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
  return pow(2, freq / 32) * 80;
}
double  Rumble::decodeLowFrequency(uint8_t _freq) {
  double freq = _freq;
  return pow(2, freq / 32) * 40;
}
