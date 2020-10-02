#pragma once
#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdio>
#include <string>
#include <sstream>
#include <iomanip>


namespace Utils {

  namespace PrintColor{
    void normal(FILE *f);
    void normal();

    void red(FILE *f);
    void red();

    void green(FILE *f);
    void green();

    void yellow(FILE *f);
    void yellow();

    void blue(FILE *f);
    void blue();

    void magenta(FILE *f);
    void magenta();

    void cyan(FILE *f);
    void cyan();

    void white(FILE *f);
    void white();
  };

  namespace Str{
    std::string wide_to_string(const wchar_t *wide);

    void copy_string_to_char(char *&dst, const std::string &src);

    /// https://stackoverflow.com/a/5100745/6292472
    template<typename T>
    std::string to_hexstr(T i) {
      std::stringstream stream;
      stream << "0x" 
             << std::setfill('0') << std::setw(sizeof(T)*2) 
             << std::hex << i;
      return stream.str();
    }
  };

};

#endif
