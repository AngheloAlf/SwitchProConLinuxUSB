#pragma once
#ifndef PRO__REAL_CONTROLLER_EXCEPTIONS_HPP
#define PRO__REAL_CONTROLLER_EXCEPTIONS_HPP

#include <stdexcept>

namespace RealController {
  class ParserError: public std::runtime_error {
  public:
    ParserError();
    ParserError(const std::string& what_arg);
    ParserError(const char* what_arg);

    ~ParserError();

    const char *what() const noexcept;
  
  protected:
    char *str = nullptr;
  };


  class PacketTypeError: public ParserError {
    using ParserError::ParserError;
  };

  class PacketLengthError: public PacketTypeError {
    using PacketTypeError::PacketTypeError;
  };


  class InputError: public ParserError {
    using ParserError::ParserError;
  };


  class ButtonError: public InputError {
    using InputError::InputError;
  };

  class AxisError: public InputError {
    using InputError::InputError;
  };

  class DpadError: public InputError {
    using InputError::InputError;
  };

  class MotionSensorError: public InputError {
    using InputError::InputError;
  };

  class NFCError: public InputError {
    using InputError::InputError;
  };

};

#endif
