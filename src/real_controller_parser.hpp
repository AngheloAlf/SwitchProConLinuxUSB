#pragma once
#ifndef PRO__REAL_CONTROLLER_PARSER_HPP
#define PRO__REAL_CONTROLLER_PARSER_HPP

#include <array>
#include "hidapi_wrapper.hpp"
#include "real_controller_layout.hpp"

namespace RealController {
  void printPacket(size_t packet_len, uint8_t *arr);
  void printPacket(size_t packet_len, HidApi::DefaultPacket arr);

  struct ControllerMAC {
    uint8_t controller_type;
    std::array<uint8_t, 6> mac;
  };

  class Parser {
  public:
    Parser(size_t packet_len, HidApi::DefaultPacket data);

    bool is_button_pressed(Buttons button) const;
    uint16_t get_axis_status(Axis axis) const;
    bool is_dpad_pressed(Dpad dpad) const;

    bool has_button_and_axis_data() const;

    void print() const;

    uint8_t buttons_bit_position(Buttons button) const;
    uint8_t buttons_byte_button_value(Buttons button) const;
    size_t  buttons_data_address(Buttons button) const;

    size_t axis_data_address_high(Axis axis) const;
    size_t axis_data_address_low(Axis axis) const;

    uint8_t dpad_bit_position(Dpad dpad) const;
    uint8_t dpad_byte_value(Dpad dpad) const;
    size_t  dpad_data_address(Dpad dpad) const;
  private:
    size_t len = 0;
    HidApi::DefaultPacket dat;
    PacketType type = PacketType::packet_none;
  };
};

#endif
