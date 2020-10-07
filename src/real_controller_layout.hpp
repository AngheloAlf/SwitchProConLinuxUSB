#pragma once
#ifndef PRO__REAL_CONTROLLER_LAYOUT_HPP
#define PRO__REAL_CONTROLLER_PARSER_HPP

#include <array>
#include "real_controller_packets.hpp"

namespace RealController {
  enum Buttons {
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
  static const std::array<Buttons, 14> btns_ids = {
    Buttons::A, Buttons::B, Buttons::X, Buttons::Y,
    Buttons::plus, Buttons::minus, 
    Buttons::home, Buttons::share,
    Buttons::L1, Buttons::L2, Buttons::L3,
    Buttons::R1, Buttons::R2, Buttons::R3,
  };

  const char *button_name(Buttons button);

  uint8_t buttons_bit_position(Buttons button, PacketType packet);
  uint8_t buttons_byte_button_value(Buttons button, PacketType packet);
  size_t  buttons_data_address(Buttons button, PacketType packet);


  enum Axis {
    axis_lx,
    axis_ly,
    axis_rx,
    axis_ry,
    axis_none
  };
  const std::array<Axis, 4> axis_ids = {
    Axis::axis_lx, Axis::axis_ly, 
    Axis::axis_rx, Axis::axis_ry,
  };

  const char *axis_name(Axis axis);

  size_t axis_data_address_high(Axis axis, PacketType packet);
  size_t axis_data_address_low(Axis axis, PacketType packet);


  enum Dpad {
    d_left,
    d_right,
    d_up,
    d_down,
    d_none
  };
  static const std::array<Dpad, 4> dpad_ids = {
    Dpad::d_left, Dpad::d_right, 
    Dpad::d_up, Dpad::d_down,
  };

  const char *dpad_name(Dpad dpad);

  uint8_t dpad_bit_position(Dpad dpads, PacketType packet);
  uint8_t dpad_byte_value(Dpad dpads, PacketType packet);
  size_t dpad_data_address(Dpad dpad, PacketType packet);
};

#endif
