#include "real_controller.hpp"
#include "config.hpp"

#include <chrono>
#include <thread>

#ifndef PROCON_ID
#define PROCON_ID 0x2009
#endif
#define NINTENDO_ID 0x057E



enum freq_nota {
  fa5 = 1396910,
  mi5 = 1318510,
//re♯5/mi♭5 	D♯6/E♭6 	1244,51 

  re5 = 1174660,
//do♯5/re♭5 	C♯6/D♭6 	1108,73
  do5 = 1046500,
  si4 = 987767,
//la♯4/si♭4 	A♯5/B♭5 	932,328
  la4   = 880000,
  la4_b = 830609,
//sol♯4/la♭4 	G♯5/A♭5 	830,609
  sol4 = 783991,
//fa♯4/sol♭4 	F♯5/G♭5 	739,989
  fa4 = 698456,
  mi4 = 659255,
//re♯4/mi♭4 	D♯5/E♭5 	622,254 
  re4 = 587330,

//do♯4/re♭4 	C♯5/D♭5 	554,365
  do4 = 523251,
};

enum simbolo_nota {
  redonda = 1,
  blanca = 2,
  negra = 4,
  corchea = 8,
  semicorchea = 16,
};


std::chrono::milliseconds tiempo(simbolo_nota nota) {
  return std::chrono::milliseconds((int64_t)(2*1000/nota));
}


void silencio(simbolo_nota nota) {
  std::this_thread::sleep_for(tiempo(nota));
}

void vibrar(RealController::Controller &c, freq_nota freq, simbolo_nota nota) {
  printf("%lf %lf\n", freq/1000.0, freq/1000.0/2.0);
  //c.rumble(freq/1000.0, freq/1000.0/2.0, 0.3, 0.6);
  c.rumble(freq/1000.0, freq/1000.0/2.0, 0, 0.9);
}

void tocar_nota(RealController::Controller &c, freq_nota freq, simbolo_nota nota) {
  vibrar(c, freq, nota);
  silencio(nota);
}

void mantener_nota(RealController::Controller &c, freq_nota freq, simbolo_nota nota) {
  auto inicio = std::chrono::steady_clock::now();
  auto t = tiempo(nota);
  while (std::chrono::steady_clock::now() < inicio + t) {
    vibrar(c, freq, nota);
  }
  //tiempo(nota)
  //silencio(nota);
}


int main(int argc, char *argv[]) {
  Config config(argc, argv);
  HidApi::Enumerate iter(NINTENDO_ID, PROCON_ID);
  
  RealController::Controller c(iter, 0);
  c.led(0x01);
  c.led(0x01);

  tocar_nota(c, fa4, negra);
  tocar_nota(c, la4, negra);
  tocar_nota(c, si4, blanca);

  c.led(0x03);
  c.led(0x03);

  tocar_nota(c, fa4, negra);
  tocar_nota(c, la4, negra);
  tocar_nota(c, si4, blanca);

  c.led(0x02);
  c.led(0x02);

  tocar_nota(c, fa4, negra);
  tocar_nota(c, la4, negra);
  tocar_nota(c, si4, negra);
  tocar_nota(c, mi5, negra);

  c.led(0x06);
  c.led(0x06);

  tocar_nota(c, re5, blanca);
  tocar_nota(c, si4, negra);
  tocar_nota(c, do5, negra);

  c.led(0x04);
  c.led(0x04);

  tocar_nota(c, si4, negra);
  tocar_nota(c, sol4, negra);
  mantener_nota(c, mi4, blanca);

  c.led(0x0c);
  c.led(0x0c);

  tocar_nota(c, mi4, blanca);
  silencio(negra);
  tocar_nota(c, re4, negra);


  c.led(0x08);
  c.led(0x08);

  tocar_nota(c, mi4, negra);
  tocar_nota(c, sol4, negra);
  mantener_nota(c, mi4, blanca);

  c.led(0x09);
  c.led(0x09);

  tocar_nota(c, mi4, redonda);

  return 0;
}
