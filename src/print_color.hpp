#pragma once
#ifndef PRINT_COLORS_H
#define PRINT_COLORS_H

#include <cstdio>

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

namespace PrintColor{
  void red() {
    printf("%s", KRED);
    fflush(stdout);
  }
  void normal() {
    printf("%s", KNRM);
    fflush(stdout);
  }
  void blue() {
    printf("%s", KBLU);
    fflush(stdout);
  }
  void yellow() {
    printf("%s", KYEL);
    fflush(stdout);
  }
  void green() {
    printf("%s", KGRN);
    fflush(stdout);
  }
  void magenta() {
    printf("%s", KMAG);
    fflush(stdout);
  }
  void cyan() {
    printf("%s", KCYN);
    fflush(stdout);
  }
};

#endif
