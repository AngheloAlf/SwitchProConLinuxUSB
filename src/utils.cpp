#include "utils.hpp"
using namespace Utils;

#include <cstring>
#include <vector>

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"


void PrintColor::normal(FILE *f) {
  fprintf(f, "%s", KNRM);
}
void PrintColor::normal() {
  PrintColor::normal(stdout);
}

void PrintColor::red(FILE *f) {
  fprintf(f, "%s", KRED);
}
void PrintColor::red() {
  PrintColor::red(stdout);
}

void PrintColor::green(FILE *f) {
  fprintf(f, "%s", KGRN);
}
void PrintColor::green() {
  PrintColor::green(stdout);
}

void PrintColor::yellow(FILE *f) {
  fprintf(f, "%s", KYEL);
}
void PrintColor::yellow() {
  PrintColor::yellow(stdout);
}

void PrintColor::blue(FILE *f) {
  fprintf(f, "%s", KBLU);
}
void PrintColor::blue() {
  PrintColor::blue(stdout);
}

void PrintColor::magenta(FILE *f) {
  fprintf(f, "%s", KMAG);
}
void PrintColor::magenta() {
  PrintColor::magenta(stdout);
}

void PrintColor::cyan(FILE *f) {
  fprintf(f, "%s", KCYN);
}
void PrintColor::cyan() {
  PrintColor::cyan(stdout);
}

void PrintColor::white(FILE *f) {
  fprintf(f, "%s", KWHT);
}
void PrintColor::white() {
  PrintColor::white(stdout);
}




std::string Str::wide_to_string(const wchar_t *wide) {
  size_t len = wcslen(wide);
  std::vector<char> mbstr(len);

  std::mbstate_t state;
  std::wcsrtombs(&mbstr[0], &wide, mbstr.size(), &state);

  return std::string(&mbstr[0]);
}

void Str::copy_string_to_char(char **dst, const std::string &src) {
  size_t sz = src.size() + 1;
  *dst = (char *)malloc(sz);
  strncpy(*dst, src.c_str(), sz);
}

