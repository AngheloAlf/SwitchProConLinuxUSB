#pragma once
#ifndef PRO_INPUT_PARSER_HPP
#define PRO_INPUT_PARSER_HPP

#include <array>

#include "hidapi_wrapper.hpp"


class ProInputParser{
public:
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

  ProInputParser(HidApi::default_packet data): dat(data) {
    switch (dat[0x00]) {
    case 0x00:
      type = PacketType::zeros;
      break;

    case 0x21:
    case 0x30:
    case 0x31:
      type = PacketType::standard_input_report;
      break;

    case 0x81:
      type = PacketType::packet_req;
      break;

    default:
      /// maybe throw?
      type = PacketType::unknown;
      printf("unknown packet\n");
      print();
      break;
    }
  }

  bool is_button_pressed(BUTTONS button) const {
    uint8_t pos = buttons_data_address(button);
    return dat[pos] & buttons_byte_button_value(button);
  }

  uint16_t get_axis_status(AXIS axis) const {
    size_t high, low;
    high = axis_data_address_high(axis);
    low  = axis_data_address_low(axis);
    switch (axis) {
    case axis_lx:
    case axis_rx:
      return ((dat[high] & 0x0F) << 8) |   dat[low];
    case axis_ly:
    case axis_ry:
      return  (dat[high] << 4)         | ((dat[low] & 0xF0) >> 4);
    default:
      return 0;
    }
  }

  bool is_dpad_pressed(DPAD dpad) const {
    uint8_t pos = dpad_data_address(dpad);
    return dat[pos] & dpad_byte_value(dpad);
  }


  /* Hackishly detects when the controller is trapped in a bad loop.
  Nothing to do here, need to reopen device :(*/
  bool detect_bad_data() const {
    return dat[1] == 0x01 && dat[0] == 0x81;
  }

  /* If this returns true, there is no controller information in this package,
   * we can skip it*/
  bool detect_useless_data() const {
    /*if (dat == 0x30)
      n_bad_data_thirty++;
    if (dat == 0x00)
      n_bad_data_zero++;*/
    return (/*dat[0] == 0x30 ||*/ dat[0] == 0x00);
  }

  void print() const {
    print_exchange_array(dat, type);
  }

  static uint8_t buttons_bit_position(BUTTONS button) {
    switch (button) {
    case Y:
    case minus:
      return 0x01;
    case X:
    case plus:
      return 0x02;
    case B:
    case R3:
      return 0x03;
    case A:
    case L3:
      return 0x04;
    case home:
      return 0x05;
    case share:
      return 0x06;
    case L1:
    case R1:
      return 0x07;
    case L2:
    case R2:
      return 0x08;
    case None:
      return 0x00;
    default:
      throw std::domain_error("ERROR: Tried to find bitpos of unknown button!");
    }
  }

  static uint8_t buttons_byte_button_value(BUTTONS button) {
    switch (button) {
    case Y:
    case minus:
      return 0x01;
    case X:
    case plus:
      return 0x02;
    case B:
    case R3:
      return 0x04;
    case A:
    case L3:
      return 0x08;
    case home:
      return 0x10;
    case share:
      return 0x20;
    case L1:
    case R1:
      return 0x40;
    case L2:
    case R2:
      return 0x80;
    case None:
      return 0x00;
    default:
      throw std::domain_error("ERROR: Tried to find bytepos of unknown button!");
    }
  }

  static size_t buttons_data_address(BUTTONS button, PacketType packet) {
    std::array<size_t, 3> address;
    switch (packet) {
    case PacketType::standard_input_report:
      address[0x00] = 0x03;
      address[0x01] = 0x04;
      address[0x02] = 0x05;
      break;
    case PacketType::packet_req:
      address[0x00] = 0x0d;
      address[0x01] = 0x0e;
      address[0x02] = 0x0f;
      break;
    default:
      throw std::domain_error("Error: This packet type does not contain button data.");
    }
    switch (button) {
    case BUTTONS::A:
    case BUTTONS::B:
    case BUTTONS::X:
    case BUTTONS::Y:
    case BUTTONS::R1:
    case BUTTONS::R2:
      return address[0x00];
    case BUTTONS::plus:
    case BUTTONS::minus:
    case BUTTONS::home:
    case BUTTONS::share:
    case BUTTONS::L3:
    case BUTTONS::R3:
      return address[0x01];
    case BUTTONS::L1:
    case BUTTONS::L2:
      return address[0x02];
    default:
      throw std::domain_error("ERROR: Tried to find address of unknown button!");
    }
  }
  size_t buttons_data_address(BUTTONS button) const {
    return buttons_data_address(button, type);
  }


  static size_t axis_data_address_high(AXIS axis, PacketType packet) {
    std::array<size_t, 4> address;
    switch (packet) {
    case PacketType::standard_input_report:
      address[0x00] = 0x07;
      address[0x01] = 0x08;
      address[0x02] = 0x0A;
      address[0x03] = 0x0B;
      break;
    case PacketType::packet_req:
      address[0x00] = 0x11;
      address[0x01] = 0x12;
      address[0x02] = 0x14;
      address[0x03] = 0x15;
      break;
    default:
      throw std::domain_error("Error: This packet type does not contain axis data.");
    }
    switch (axis) {
    case AXIS::axis_lx:
      return address[0x00];
    case AXIS::axis_ly:
      return address[0x01];
    case AXIS::axis_rx:
      return address[0x02];
    case AXIS::axis_ry:
      return address[0x03];
    default:
      throw std::domain_error("ERROR: Tried to find address of unknown axis.");
    }
  }
  static size_t axis_data_address_low(AXIS axis, PacketType packet) {
    std::array<size_t, 4> address;
    switch (packet) {
    case PacketType::standard_input_report:
      address[0x00] = 0x06;
      address[0x01] = 0x07;
      address[0x02] = 0x09;
      address[0x03] = 0x0A;
      break;
    case PacketType::packet_req:
      address[0x00] = 0x10;
      address[0x01] = 0x11;
      address[0x02] = 0x13;
      address[0x03] = 0x14;
      break;
    default:
      throw std::domain_error("Error: This packet type does not contain axis data.");
    }
    switch (axis) {
    case AXIS::axis_lx:
      return address[0x00];
    case AXIS::axis_ly:
      return address[0x01];
    case AXIS::axis_rx:
      return address[0x02];
    case AXIS::axis_ry:
      return address[0x03];
    default:
      throw std::domain_error("ERROR: Tried to find address of unknown axis.");
    }
  }
  size_t axis_data_address_high(AXIS axis) const {
    return axis_data_address_high(axis, type);
  }
  size_t axis_data_address_low(AXIS axis) const {
    return axis_data_address_low(axis, type);
  }


  static uint8_t dpad_bit_position(DPAD dpad) {
    switch (dpad) {
    case d_left:
      return 0x04;
    case d_right:
      return 0x03;
    case d_up:
      return 0x02;
    case d_down:
      return 0x01;
    case d_none:
      return 0x00;
    default:
      throw std::domain_error("ERROR: Tried to find bitpos of unknown dpad button!");
    }
  }

  static uint8_t dpad_byte_value(DPAD dpad) {
    switch (dpad) {
    case d_left:
      return 0x08;
    case d_right:
      return 0x04;
    case d_up:
      return 0x02;
    case d_down:
      return 0x01;
    case d_none:
      return 0x00;
    default:
      throw std::domain_error("ERROR: Tried to find bytepos of unknown dpad button!");
    }
  }

  static size_t dpad_data_address(DPAD dpad, PacketType packet) {
    std::array<size_t, 1> address;
    switch (packet) {
    case PacketType::standard_input_report:
      address[0x00] = 0x05;
      break;
    case PacketType::packet_req:
      address[0x00] = 0x0f;
      break;
    default:
      throw std::domain_error("Error: This packet type does not contain dpad data.");
    }
    switch (dpad) {
    case DPAD::d_left:
    case DPAD::d_right:
    case DPAD::d_up:
    case DPAD::d_down:
      return address[0x00];
    default:
      throw std::domain_error("ERROR: Tried to find address of unknown dpad button.");
    }
  }
  size_t dpad_data_address(DPAD dpad) const {
    return dpad_data_address(dpad, type);
  }


  static void print_exchange_array(HidApi::default_packet arr, PacketType type) {
    bool redcol = false;
    if (arr[0] != 0x30)
      PrintColor::yellow();
    else {
      PrintColor::red();
      redcol = true;
    }
    size_t len = (type == PacketType::standard_input_report ? 0x32 : 0x20);
    for (size_t i = 0; i < len; ++i) {
      if (arr[i] == 0x00) {
        PrintColor::blue();
      } else {
        if (redcol) {
          PrintColor::red();
        } else {
          PrintColor::yellow();
        }
      }
      printf("%02X ", arr[i]);
    }
    PrintColor::normal();
    printf("\n");
    fflush(stdout);
  }

  static const char *button_name(BUTTONS button) {
    switch (button) {
    case A:
      return "A";
    case B:
      return "B";
    case X:
      return "X";
    case Y:
      return "Y";
    case plus:
      return "plus";
    case minus:
      return "minus";
    case home:
      return "home";
    case share:
      return "share";
    case L1:
      return "L1";
    case L2:
      return "L2";
    case L3:
      return "L3";
    case R1:
      return "R1";
    case R2:
      return "R2";
    case R3:
      return "R3";
    case None:
      return "None";
    default:
      return nullptr;
    }
  }

  static const char *axis_name(AXIS axis) {
    switch (axis) {
    case axis_lx:
      return "axis_lx";
    case axis_ly:
      return "axis_ly";
    case axis_rx:
      return "axis_rx";
    case axis_ry:
      return "axis_ry";
    case axis_none:
      return "axis_none";
    default:
      return nullptr;
    }
  }

  static const char *dpad_name(DPAD dpad) {
    switch (dpad) {
    case d_left:
      return "d_left";
    case d_right:
      return "d_right";
    case d_up:
      return "d_up";
    case d_down:
      return "d_down";
    case d_none:
      return "None";
    default:
      return nullptr;
    }
  }

  static const std::array<BUTTONS, 14> btns_ids;
  static const std::array<AXIS, 4> axis_ids;
  static const std::array<DPAD, 4> dpad_ids;

private:
  HidApi::default_packet dat;
  PacketType type = PacketType::packet_none;
};

const std::array<ProInputParser::BUTTONS, 14> ProInputParser::btns_ids = {
  ProInputParser::A, ProInputParser::B, ProInputParser::X, ProInputParser::Y,
  ProInputParser::plus, ProInputParser::minus, 
  ProInputParser::home, ProInputParser::share,
  ProInputParser::L1, ProInputParser::L2, ProInputParser::L3,
  ProInputParser::R1, ProInputParser::R2, ProInputParser::R3,
};

const std::array<ProInputParser::AXIS, 4> ProInputParser::axis_ids = {
  ProInputParser::axis_lx, ProInputParser::axis_ly, ProInputParser::axis_rx, ProInputParser::axis_ry,
};

const std::array<ProInputParser::DPAD, 4> ProInputParser::dpad_ids = {
  ProInputParser::d_left, ProInputParser::d_right, 
  ProInputParser::d_up, ProInputParser::d_down,
};

#endif
