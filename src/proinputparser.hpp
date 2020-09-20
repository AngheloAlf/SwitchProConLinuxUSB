#pragma once
#ifndef PRO_INPUT_PARSER_HPP
#define PRO_INPUT_PARSER_HPP

#include <array>

class ProInputParser{
public:
  static constexpr size_t exchange_length{0x400};
  using exchange_array = std::array<uint8_t, exchange_length>;

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
    return (dat[0] == 0x30 || dat[0] == 0x00);
  }

  void print() const {
    print_exchange_array(dat);
  }

  static uint8_t bit_position(BUTTONS button) {
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

  static uint8_t byte_button_value(BUTTONS button) {
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

  static uint8_t data_address(BUTTONS button) {
    switch (button) {
    case A:
    case B:
    case X:
    case Y:
    case R1:
    case R2:
      return 0x0d;
    case plus:
    case minus:
    case home:
    case share:
    case L3:
    case R3:
      return 0x0e;
    case L1:
    case L2:
      return 0x0f;
    case None:
      return 0x00;
    default:
      throw std::domain_error("ERROR: Tried to find adress of unknown button!");
    }
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

  static uint8_t dpad_data_address(DPAD dpad) {
    switch (dpad) {
    case d_left:
    case d_right:
    case d_up:
    case d_down:
      return 0x0f;
    case d_none:
      return 0x00;
    default:
      throw std::domain_error("ERROR: Tried to find adress of unknown dpad button!");
    }
  }


  static void print_exchange_array(exchange_array arr) {
    bool redcol = false;
    if (arr[0] != 0x30)
      PrintColor::yellow();
    else {
      PrintColor::red();
      redcol = true;
    }
    for (size_t i = 0; i < 0x20; ++i) {
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
  exchange_array dat;

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
