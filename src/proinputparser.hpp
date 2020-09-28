#pragma once
#ifndef PRO_INPUT_PARSER_HPP
#define PRO_INPUT_PARSER_HPP

#include <array>

#include "hidapi_wrapper.hpp"


namespace ProInputParser {
  enum PacketType {
    zeros,                  /// A packet with all data zero'ed.
    standard_input_report,  /// Standard input report format. [0] == x21 || x30 || x31
    packet_req,             /// ?
    unknown,                /// For unrecognized packets.
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

  enum AXIS {
    axis_lx,
    axis_ly,
    axis_rx,
    axis_ry,
    axis_none
  };

  enum DPAD {
    d_left,
    d_right,
    d_up,
    d_down,
    d_none
  };

  class Parser {
  public:
    Parser(HidApi::default_packet data);

    bool is_button_pressed(BUTTONS button) const;

    uint16_t get_axis_status(AXIS axis) const;

    bool is_dpad_pressed(DPAD dpad) const;


    /* Hackishly detects when the controller is trapped in a bad loop.
    Nothing to do here, need to reopen device :(*/
    bool detect_bad_data() const;

    /* If this returns true, there is no controller information in this package,
    * we can skip it*/
    bool detect_useless_data() const;

    void print() const;

    static uint8_t buttons_bit_position(BUTTONS button);

    static uint8_t buttons_byte_button_value(BUTTONS button);

    static size_t buttons_data_address(BUTTONS button, PacketType packet);
    size_t buttons_data_address(BUTTONS button) const;


    static size_t axis_data_address_high(AXIS axis, PacketType packet);
    static size_t axis_data_address_low(AXIS axis, PacketType packet);
    size_t axis_data_address_high(AXIS axis) const;
    size_t axis_data_address_low(AXIS axis) const;


    static uint8_t dpad_bit_position(DPAD dpad);

    static uint8_t dpad_byte_value(DPAD dpad);

    static size_t dpad_data_address(DPAD dpad, PacketType packet);
    size_t dpad_data_address(DPAD dpad) const;


    static void print_exchange_array(HidApi::default_packet arr, PacketType type);

    static const char *button_name(BUTTONS button);

    static const char *axis_name(AXIS axis);

    static const char *dpad_name(DPAD dpad);

    static const std::array<BUTTONS, 14> btns_ids;
    static const std::array<AXIS, 4> axis_ids;
    static const std::array<DPAD, 4> dpad_ids;

  private:
    HidApi::default_packet dat;
    PacketType type = PacketType::packet_none;
  };
};

#endif
