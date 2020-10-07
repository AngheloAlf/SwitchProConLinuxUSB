#include "real_controller_exceptions.hpp"
using namespace RealController;

#include "utils.hpp"

ParserError::ParserError(): std::runtime_error("ParserError: Unspecified error") {
  std::string aux = std::string("ParserError: ") + "Unspecified error";
  Utils::Str::copy_string_to_char(str, aux);
}

ParserError::ParserError(const std::string& what_arg): std::runtime_error(what_arg) {
  std::string aux = std::string("ParserError: ") + what_arg;
  Utils::Str::copy_string_to_char(str, aux);
}
ParserError::ParserError(const char* what_arg): std::runtime_error(what_arg) {
  std::string aux = std::string("ParserError: ") + what_arg;
  Utils::Str::copy_string_to_char(str, aux);
}

ParserError::~ParserError() {
  if (str != nullptr) {
    free(str);
    str = nullptr;
  }
}

const char *ParserError::what() const noexcept {
  return str;
}
