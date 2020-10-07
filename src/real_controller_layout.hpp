#pragma once
#ifndef PRO__REAL_CONTROLLER_LAYOUT_HPP
#define PRO__REAL_CONTROLLER_PARSER_HPP

#include <array>
#include "real_controller_packets.hpp"

namespace RealController {
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
};

#endif
