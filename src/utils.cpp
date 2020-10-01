#include "utils.hpp"
using namespace Utils;

#include <cstdio>

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"


void PrintColor::normal() {
  printf("%s", KNRM);
}
void PrintColor::red() {
  printf("%s", KRED);
}
void PrintColor::green() {
  printf("%s", KGRN);
}
void PrintColor::yellow() {
  printf("%s", KYEL);
}
void PrintColor::blue() {
  printf("%s", KBLU);
}
void PrintColor::magenta() {
  printf("%s", KMAG);
}
void PrintColor::cyan() {
  printf("%s", KCYN);
}
void PrintColor::white() {
  printf("%s", KWHT);
}
