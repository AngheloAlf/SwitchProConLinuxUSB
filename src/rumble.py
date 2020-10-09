from math import log2

def clamp(value, lower, upper):
  if value < lower: return lower
  if value > upper: return upper
  return value


def highAmplitude(amp):
  if amp < 0.007666:
    return 0

  if amp < 0.011823:
    return 2

  if amp <= 0.112491:
    return round((log2(amp * 119.6128) * 8) / 2)*2

  if amp <= 0.224982:
    return round((log2(amp * 17.0256) * 32) / 2)*2

  return round((log2(amp * 8.699) * 64) / 2)*2


def lowAmplitude(amp):
  high = highAmplitude(amp)
  return 0x40 + (high >> 2) + ((high&0x02) << 14)


def highFrequency(freq):
  return round((log2(freq / 80) * 128) / 4)*4

def  lowFrequency(freq):
  return round((log2(freq / 40) * 32))



amplitude_min = 0.0
amplitude_max = 1.0

highFreq_min = 81.75177
highFreq_max = 1252.572266

lowFreq_min  =  40.875885
lowFreq_max = 626.286133


def rumble(_high_freq, _low_freq, _high_ampl, _low_ampl):
  _high_ampl = clamp(_high_ampl, amplitude_min, amplitude_max)
  _low_ampl  = clamp(_low_ampl,  amplitude_min, amplitude_max)

  _high_freq = clamp(_high_freq, highFreq_min, highFreq_max)
  _low_freq  = clamp(_low_freq,  lowFreq_min,  lowFreq_max)


  amp_high = highAmplitude(_high_ampl)
  amp_low =  lowAmplitude(_low_ampl)

  freq_high = highFrequency(_high_freq)
  freq_low  =  lowFrequency(_low_freq)

  data = [0, 0, 0, 0]

  data[0] =  0xFF &  freq_high
  data[1] = (0xFF & (freq_high >> 8)) + amp_high

  data[2] = (0xFF & (amp_low >> 8)) + freq_low
  data[3] =  0xFF &  amp_low

  return data


#print(rumble(320, 160, 0, 0))





def decodeHighAmplitude(_amp):
  amp = _amp
  if amp == 0x00:
    return 0

  if amp <= 0x02:
    return 0.007843

  if amp <= 0x1e:
    return pow(2, amp/8) / 119.6128

  if amp <= 0x3e:
    return pow(2, amp/32) / 17.0256

  return pow(2, amp/64) / 8.699



def decodeHighFrequency(freq):
  return pow(2, freq / 128) * 80



i = 0
while i < 0xc9:
  amp = decodeHighAmplitude(i)
  data = rumble(320, 160, amp, amp)

  a = data[1]
  b = bool(data[2] & 0x80)
  c = data[3]

  #print(hex(i), "\t", hex(a), "\t", hex(c))
  #print(bin(i), "\t", bin(a))
  #print(bin(i))
  #print(bin(a))
  #print()
  i += 2

