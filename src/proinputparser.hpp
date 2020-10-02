#pragma once
#ifndef PRO_INPUT_PARSER_HPP
#define PRO_INPUT_PARSER_HPP

#include <array>
#include <stdexcept>

#include "hidapi_wrapper.hpp"


namespace ProInputParser {
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
    ProInputParser::A, ProInputParser::B, ProInputParser::X, ProInputParser::Y,
    ProInputParser::plus, ProInputParser::minus, 
    ProInputParser::home, ProInputParser::share,
    ProInputParser::L1, ProInputParser::L2, ProInputParser::L3,
    ProInputParser::R1, ProInputParser::R2, ProInputParser::R3,
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
    ProInputParser::axis_lx, ProInputParser::axis_ly, 
    ProInputParser::axis_rx, ProInputParser::axis_ry,
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
    ProInputParser::d_left, ProInputParser::d_right, 
    ProInputParser::d_up, ProInputParser::d_down,
  };

  const char *dpad_name(DPAD dpad);

  uint8_t dpad_bit_position(DPAD dpads, PacketType packet);
  uint8_t dpad_byte_value(DPAD dpads, PacketType packet);
  size_t dpad_data_address(DPAD dpad, PacketType packet);

  void print_exchange_array(size_t packet_len, HidApi::default_packet arr);


  class Parser {
  public:
    Parser(size_t packet_len, HidApi::default_packet data);

    bool is_button_pressed(BUTTONS button) const;

    uint16_t get_axis_status(AXIS axis) const;

    bool is_dpad_pressed(DPAD dpad) const;


    /* Hackishly detects when the controller is trapped in a bad loop.
    Nothing to do here, need to reopen device :(*/
    bool detect_bad_data() const;

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
    HidApi::default_packet dat;
    PacketType type = PacketType::packet_none;
  };
};

#endif
