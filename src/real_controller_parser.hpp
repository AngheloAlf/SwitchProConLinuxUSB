#pragma once
#ifndef PRO_INPUT_PARSER_HPP
#define PRO_INPUT_PARSER_HPP

#include <array>
#include <stdexcept>

#include "hidapi_wrapper.hpp"


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
  public:
    PacketLengthError();
    PacketLengthError(const std::string& what_arg);
    PacketLengthError(const char* what_arg);

    using PacketTypeError::what;
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


  enum PacketType {
    unknown = -1,           /// For unrecognized packets.
    standard_input_report,  /// Standard input report format. [0] == x21 || x30 || x31
    normal_ctrl_report,     /// [0] == x3F
    packet_req,             /// ?
    packet_none,
  };

  enum BUTTONS {
    A,
    B,
    X,
    Y,
    plus,
    minus,
    home,
    share,
    L1,
    L2,
    L3,
    R1,
    R2,
    R3,
    None
  };
  static const std::array<BUTTONS, 14> btns_ids = {
    BUTTONS::A, BUTTONS::B, BUTTONS::X, BUTTONS::Y,
    BUTTONS::plus, BUTTONS::minus, 
    BUTTONS::home, BUTTONS::share,
    BUTTONS::L1, BUTTONS::L2, BUTTONS::L3,
    BUTTONS::R1, BUTTONS::R2, BUTTONS::R3,
  };

  const char *button_name(BUTTONS button);

  uint8_t buttons_bit_position(BUTTONS button, PacketType packet);
  uint8_t buttons_byte_button_value(BUTTONS button, PacketType packet);
  size_t  buttons_data_address(BUTTONS button, PacketType packet);


  enum AXIS {
    axis_lx,
    axis_ly,
    axis_rx,
    axis_ry,
    axis_none
  };
  const std::array<AXIS, 4> axis_ids = {
    AXIS::axis_lx, AXIS::axis_ly, 
    AXIS::axis_rx, AXIS::axis_ry,
  };

  const char *axis_name(AXIS axis);

  size_t axis_data_address_high(AXIS axis, PacketType packet);
  size_t axis_data_address_low(AXIS axis, PacketType packet);


  enum DPAD {
    d_left,
    d_right,
    d_up,
    d_down,
    d_none
  };
  static const std::array<DPAD, 4> dpad_ids = {
    DPAD::d_left, DPAD::d_right, 
    DPAD::d_up, DPAD::d_down,
  };

  const char *dpad_name(DPAD dpad);

  uint8_t dpad_bit_position(DPAD dpads, PacketType packet);
  uint8_t dpad_byte_value(DPAD dpads, PacketType packet);
  size_t dpad_data_address(DPAD dpad, PacketType packet);

  void printPacket(size_t packet_len, uint8_t *arr);
  void printPacket(size_t packet_len, HidApi::DefaultPacket arr);

  struct ControllerMAC {
    uint8_t controller_type;
    std::array<uint8_t, 6> mac;
  };

  class Parser {
  public:
    Parser(size_t packet_len, HidApi::DefaultPacket data);

    bool is_button_pressed(BUTTONS button) const;
    uint16_t get_axis_status(AXIS axis) const;
    bool is_dpad_pressed(DPAD dpad) const;

    bool has_button_and_axis_data() const;

    void print() const;

    uint8_t buttons_bit_position(BUTTONS button) const;
    uint8_t buttons_byte_button_value(BUTTONS button) const;
    size_t  buttons_data_address(BUTTONS button) const;

    size_t axis_data_address_high(AXIS axis) const;
    size_t axis_data_address_low(AXIS axis) const;

    uint8_t dpad_bit_position(DPAD dpads) const;
    uint8_t dpad_byte_value(DPAD dpads) const;
    size_t  dpad_data_address(DPAD dpad) const;
  private:
    size_t len = 0;
    HidApi::DefaultPacket dat;
    PacketType type = PacketType::packet_none;
  };
};

#endif
