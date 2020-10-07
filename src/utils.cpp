#include "utils.hpp"
using namespace Utils;

#include <cerrno>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <system_error>

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
  if (wide == nullptr) {
    throw std::invalid_argument("wide_to_string(): `wide` parameter can't be null.");
  }
  size_t len = wcslen(wide);
  std::vector<char> mbstr(len+1);
  mbstr.assign(len+1, 0);

  std::mbstate_t state = std::mbstate_t();
  size_t bytes_written = std::wcsrtombs(&mbstr[0], &wide, len, &state);
  if (bytes_written == static_cast<std::size_t>(-1)) {
    throw std::system_error(errno, std::generic_category());
  }
  mbstr[len] = 0x00;

  return std::string(&mbstr[0]);
}

void Str::copy_string_to_char(char *&dst, const char *src) {
  size_t sz = strlen(src) + 1;
  dst = (char *)malloc(sz);
  if (dst == nullptr) {
    throw std::bad_alloc();
  }
  strncpy(dst, src, sz);
}
void Str::copy_string_to_char(char *&dst, const std::string &src) {
  copy_string_to_char(dst, src.c_str());
}

