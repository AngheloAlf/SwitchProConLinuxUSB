#include "real_controller_exceptions.hpp"
using namespace RealController;

#include "utils.hpp"

RealControllerError::RealControllerError(): std::runtime_error("ParserError: Unspecified error") {
  std::string aux = std::string("ParserError: ") + "Unspecified error";
  Utils::Str::copy_string_to_char(str, aux);
}

RealControllerError::RealControllerError(const std::string& what_arg): std::runtime_error(what_arg) {
  std::string aux = std::string("ParserError: ") + what_arg;
  Utils::Str::copy_string_to_char(str, aux);
}
RealControllerError::RealControllerError(const char* what_arg): std::runtime_error(what_arg) {
  std::string aux = std::string("ParserError: ") + what_arg;
  Utils::Str::copy_string_to_char(str, aux);
}

RealControllerError::RealControllerError(const RealControllerError &other): std::runtime_error(other.str), str(nullptr) {
  Utils::Str::copy_string_to_char(str, other.str);
}
RealControllerError::RealControllerError(RealControllerError &&other) noexcept: std::runtime_error(other.str), str(nullptr) {
  std::swap(str, other.str);
}

RealControllerError::~RealControllerError() {
  exit();
}


RealControllerError &RealControllerError::operator=(const RealControllerError &other) {
  exit();
  Utils::Str::copy_string_to_char(str, other.str);
  return *this;
}
RealControllerError &RealControllerError::operator=(RealControllerError &&other) noexcept {
  exit();
  std::swap(str, other.str);
  return *this;
}


const char *RealControllerError::what() const noexcept {
  return str;
}

void RealControllerError::exit() noexcept {
  if (str != nullptr) {
    free(str);
    str = nullptr;
  }
}
