#pragma once
#ifndef PRO__REAL_CONTROLLER_EXCEPTIONS_HPP
#define PRO__REAL_CONTROLLER_EXCEPTIONS_HPP

#include <stdexcept>

namespace RealController {
  class RealControllerError: public std::runtime_error {
  public:
    RealControllerError();
    RealControllerError(const std::string& what_arg);
    RealControllerError(const char* what_arg);

    RealControllerError(const RealControllerError &other);
    RealControllerError(RealControllerError &&other) noexcept;

    ~RealControllerError() noexcept ;

    RealControllerError &operator=(const RealControllerError &other);
    RealControllerError &operator=(RealControllerError &&other) noexcept;

    const char *what() const noexcept;
  
  protected:
    void exit() noexcept ;

    char *str = nullptr;
  };


  class ParserError: public RealControllerError {
    using RealControllerError::RealControllerError;
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
