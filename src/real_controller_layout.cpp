#include "real_controller_layout.hpp"
using namespace RealController;

#include "real_controller_exceptions.hpp"

const char *RealController::button_name(BUTTONS button) {
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
  default:
    throw ButtonError("ButtonError: Tried to get the name of unknown button.");
  }
}

uint8_t RealController::buttons_bit_position(BUTTONS button, PacketType packet) {
  /// Joycons layout
  /*if (packet == PacketType::normal_ctrl_report) {
    switch (button) {
    case home:
      return 0x04;
    case share:
      return 0x05;
    case L1:
    case R1:
      return 0x06;
    case L2:
    case R2:
      return 0x07;
    }
  }*/
  if (packet == PacketType::normal_ctrl_report) {
    switch (button) {
    case B:
    case minus:
      return 0x00;
    case A:
    case plus:
      return 0x01;
    case Y:
    case L3:
      return 0x02;
    case X:
    case R3:
      return 0x03;
    case home:
    case L1:
      return 0x04;
    case share:
    case R1:
      return 0x05;
    case L2:
      return 0x06;
    case R2:
      return 0x07;
    default:
      throw ButtonError("ButtonError: Tried to find bytepos of unknown button.");
    }
  }
  switch (button) {
  case Y:
  case minus:
    return 0x00;
  case X:
  case plus:
    return 0x01;
  case B:
  case R3:
    return 0x02;
  case A:
  case L3:
    return 0x03;
  case home:
    return 0x04;
  case share:
    return 0x05;
  case L1:
  case R1:
    return 0x06;
  case L2:
  case R2:
    return 0x07;
  default:
    throw ButtonError("ButtonError: Tried to find bitpos of unknown button.");
  }
}

uint8_t RealController::buttons_byte_button_value(BUTTONS button, PacketType packet) {
  /// Joycons layout
  /*if (packet == PacketType::normal_ctrl_report) {
    switch (button) {
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
    }
  }*/
  if (packet == PacketType::normal_ctrl_report) {
    switch (button) {
    case B:
    case minus:
      return 0x01;
    case A:
    case plus:
      return 0x02;
    case Y:
    case L3:
      return 0x04;
    case X:
    case R3:
      return 0x08;
    case home:
    case L1:
      return 0x10;
    case share:
    case R1:
      return 0x20;
    case L2:
      return 0x40;
    case R2:
      return 0x80;
    default:
      throw ButtonError("ButtonError: Tried to find bytepos of unknown button.");
    }
  }
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
  default:
    throw ButtonError("ButtonError: Tried to find bytepos of unknown button.");
  }
}

size_t  RealController::buttons_data_address(BUTTONS button, PacketType packet) {
  /// Joycons layout
  /*if (packet == PacketType::normal_ctrl_report) {
    switch (button) {
    case BUTTONS::A:
    case BUTTONS::B:
    case BUTTONS::X:
    case BUTTONS::Y:
      return 0x01;
    case BUTTONS::plus:
    case BUTTONS::minus:
    case BUTTONS::home:
    case BUTTONS::share:
    case BUTTONS::L1:
    case BUTTONS::L2:
    case BUTTONS::L3:
    case BUTTONS::R1:
    case BUTTONS::R2:
    case BUTTONS::R3:
      return 0x02;
    }
  }*/
  if (packet == PacketType::normal_ctrl_report) {
    switch (button) {
    case BUTTONS::A:
    case BUTTONS::B:
    case BUTTONS::X:
    case BUTTONS::Y:
    case BUTTONS::L1:
    case BUTTONS::L2:
    case BUTTONS::R1:
    case BUTTONS::R2:
      return 0x01;
    case BUTTONS::plus:
    case BUTTONS::minus:
    case BUTTONS::home:
    case BUTTONS::share:
    case BUTTONS::L3:
    case BUTTONS::R3:
      return 0x02;
    default:
      throw ButtonError("ButtonError: Tried to find address of unknown button.");
    }
  }
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
    throw ButtonError("ButtonError: This packet type (" + std::to_string(packet) + ") does not contain button data.");
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
    throw ButtonError("ButtonError: Tried to find address of unknown button.");
  }
}


const char *RealController::axis_name(AXIS axis) {
  switch (axis) {
  case axis_lx:
    return "axis_lx";
  case axis_ly:
    return "axis_ly";
  case axis_rx:
    return "axis_rx";
  case axis_ry:
    return "axis_ry";
  default:
    throw AxisError("AxisError: Tried to get the name of unknown axis.");
  }
}

size_t RealController::axis_data_address_high(AXIS axis, PacketType packet) {
  std::array<size_t, 4> address;
  switch (packet) {
  case PacketType::standard_input_report:
    address[0x00] = 0x07;
    address[0x01] = 0x08;
    address[0x02] = 0x0A;
    address[0x03] = 0x0B;
    break;
  case PacketType::normal_ctrl_report:
    address[0x00] = 0x05;
    address[0x01] = 0x07;
    address[0x02] = 0x09;
    address[0x03] = 0x0B;
    break;
  case PacketType::packet_req:
    address[0x00] = 0x11;
    address[0x01] = 0x12;
    address[0x02] = 0x14;
    address[0x03] = 0x15;
    break;
  default:
    throw AxisError("AxisError: This packet type (" + std::to_string(packet) + ") does not contain axis data.");
  }
  switch (axis) {
  case AXIS::axis_lx:
    return address[0];
  case AXIS::axis_ly:
    return address[1];
  case AXIS::axis_rx:
    return address[2];
  case AXIS::axis_ry:
    return address[3];
  default:
    throw AxisError("AxisError: Tried to find address of unknown axis.");
  }
}
size_t RealController::axis_data_address_low(AXIS axis, PacketType packet) {
  std::array<size_t, 4> address;
  switch (packet) {
  case PacketType::standard_input_report:
    address[0x00] = 0x06;
    address[0x01] = 0x07;
    address[0x02] = 0x09;
    address[0x03] = 0x0A;
    break;
  case PacketType::normal_ctrl_report:
    address[0x00] = 0x04;
    address[0x01] = 0x06;
    address[0x02] = 0x08;
    address[0x03] = 0x0A;
    break;
  case PacketType::packet_req:
    address[0x00] = 0x10;
    address[0x01] = 0x11;
    address[0x02] = 0x13;
    address[0x03] = 0x14;
    break;
  default:
    throw AxisError("AxisError: This packet type (" + std::to_string(packet) + ") does not contain axis data.");
  }
  switch (axis) {
  case AXIS::axis_lx:
    return address[0];
  case AXIS::axis_ly:
    return address[1];
  case AXIS::axis_rx:
    return address[2];
  case AXIS::axis_ry:
    return address[3];
  default:
    throw AxisError("AxisError: Tried to find address of unknown axis.");
  }
}


const char *RealController::dpad_name(DPAD dpad) {
  switch (dpad) {
  case d_left:
    return "d_left";
  case d_right:
    return "d_right";
  case d_up:
    return "d_up";
  case d_down:
    return "d_down";
  default:
    throw DpadError("DpadError: Tried to get the name of unknown dpad.");
  }
}

uint8_t RealController::dpad_bit_position(DPAD dpad, PacketType packet) {
  if (packet != PacketType::standard_input_report && packet != PacketType::packet_req) {
    throw DpadError("AxisError: This packet type (" + std::to_string(packet) + ") does not map dpad to specific bits.");
  }
  switch (dpad) {
  case d_down:
    return 0x00;
  case d_up:
    return 0x01;
  case d_right:
    return 0x02;
  case d_left:
    return 0x03;
  default:
    throw DpadError("DpadError: Tried to find bitpos of unknown dpad button.");
  }
}

uint8_t RealController::dpad_byte_value(DPAD dpad, PacketType packet) {
  if (packet != PacketType::standard_input_report && packet != PacketType::packet_req) {
    throw DpadError("AxisError: This packet type (" + std::to_string(packet) + ") does not map dpad to specific bits.");
  }
  switch (dpad) {
  case d_down:
    return 0x01;
  case d_up:
    return 0x02;
  case d_right:
    return 0x04;
  case d_left:
    return 0x08;
  default:
    throw DpadError("DpadError: Tried to find bytepos of unknown dpad button.");
  }
}

size_t RealController::dpad_data_address(DPAD dpad, PacketType packet) {
  std::array<size_t, 1> address;
  switch (packet) {
  case PacketType::standard_input_report:
    address[0x00] = 0x05;
    break;
  case PacketType::normal_ctrl_report:
    address[0x00] = 0x03;
    break;
  case PacketType::packet_req:
    address[0x00] = 0x0f;
    break;
  default:
    throw DpadError("DpadError: This packet type (" + std::to_string(packet) + ") does not contain dpad data.");
  }
  switch (dpad) {
  case DPAD::d_down:
  case DPAD::d_up:
  case DPAD::d_right:
  case DPAD::d_left:
    return address[0x00];
  default:
    throw DpadError("DpadError: Tried to find address of unknown dpad button.");
  }
}
