#pragma once
#ifndef PRO_INPUT_PARSER_HPP
#define PRO_INPUT_PARSER_HPP

#include <array>

class ProInputParser{
public:
  static constexpr size_t exchange_length{0x400};
  using exchange_array = std::array<uint8_t, exchange_length>;

  enum BUTTONS {
    d_left,
    d_right,
    d_up,
    d_down,
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
    arix_none,
  };

  ProInputParser(exchange_array data): dat(data) {

  }

  bool is_button_pressed(BUTTONS button) const {
    uint8_t pos = data_address(button);
    return dat[pos] & byte_button_value(button);
  }

  uint8_t get_axis_status(AXIS axis) const {
    switch (axis) {
    case axis_lx:
      return ((dat[0x11] & 0x0F) << 4) | ((dat[0x10] & 0xF0) >> 4);
    case axis_ly:
      return dat[0x12];
    case axis_rx:
      return ((dat[0x14] & 0x0F) << 4) | ((dat[0x13] & 0xF0) >> 4);
    case axis_ry:
      return dat[0x15];
    default:
      return 0;
    }
  }

  static uint8_t bit_position(BUTTONS button) {
    switch (button) {
    case d_left:
      return 0x04;
      break;
    case d_right:
      return 0x03;
      break;
    case d_up:
      return 0x02;
      break;
    case d_down:
      return 0x01;
      break;
    case A:
      return 0x04;
      break;
    case B:
      return 0x03;
      break;
    case X:
      return 0x02;
      break;
    case Y:
      return 0x01;
      break;
    case plus:
      return 0x02;
      break;
    case minus:
      return 0x01;
      break;
    case home:
      return 0x05;
      break;
    case share:
      return 0x06;
      break;
    case L1:
      return 0x07;
      break;
    case L2:
      return 0x08;
      break;
    case L3:
      return 0x04;
      break;
    case R1:
      return 0x07;
      break;
    case R2:
      return 0x08;
      break;
    case R3:
      return 0x03;
      break;
    case None:
      return 0x00;
      break;
    default:
      #if 0
      PrintColor::red();
      printf("ERROR: Tried to find bitpos of unknown button!\n");
      PrintColor::normal();
      return 0x00;
      #endif
      break;
    }
  }

  static uint8_t byte_button_value(BUTTONS button) {
    switch (button) {
    case d_left:
      return 0x08;
      break;
    case d_right:
      return 0x04;
      break;
    case d_up:
      return 0x02;
      break;
    case d_down:
      return 0x01;
      break;
    case A:
      return 0x08;
      break;
    case B:
      return 0x04;
      break;
    case X:
      return 0x02;
      break;
    case Y:
      return 0x01;
      break;
    case plus:
      return 0x02;
      break;
    case minus:
      return 0x01;
      break;
    case home:
      return 0x10;
      break;
    case share:
      return 0x20;
      break;
    case L1:
      return 0x40;
      break;
    case L2:
      return 0x80;
      break;
    case L3:
      return 0x08;
      break;
    case R1:
      return 0x40;
      break;
    case R2:
      return 0x80;
      break;
    case R3:
      return 0x04;
      break;
    case None:
      return 0x00;
      break;
    default:
      #if 0
      PrintColor::red();
      printf("ERROR: Tried to find bitpos of unknown button!\n");
      PrintColor::normal();
      #endif
      return 0x00;
      break;
    }
  }

  static uint8_t data_address(BUTTONS button) {
    switch (button) {
    case d_left:
      return 0x0f;
      break;
    case d_right:
      return 0x0f;
      break;
    case d_up:
      return 0x0f;
      break;
    case d_down:
      return 0x0f;
      break;
    case A:
      return 0x0d;
      break;
    case B:
      return 0x0d;
      break;
    case X:
      return 0x0d;
      break;
    case Y:
      return 0x0d;
      break;
    case plus:
      return 0x0e;
      break;
    case minus:
      return 0x0e;
      break;
    case home:
      return 0x0e;
      break;
    case share:
      return 0x0e;
      break;
    case L1:
      return 0x0f;
      break;
    case L2:
      return 0x0f;
      break;
    case L3:
      return 0x0e;
      break;
    case R1:
      return 0x0d;
      break;
    case R2:
      return 0x0d;
      break;
    case R3:
      return 0x0e;
      break;
    case None:
      return 0x00;
      break;
    default:
      #if 0
      PrintColor::red();
      printf("ERROR: Tried to find data adress of unknown button!\n");
      PrintColor::normal();
      #endif
      return 0x00;
      break;
    }
  }


private:
  exchange_array dat;

};


#endif
