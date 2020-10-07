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
