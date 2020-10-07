#include "real_controller_layout.hpp"
using namespace RealController;

#include "real_controller_exceptions.hpp"

const char *RealController::button_name(Buttons button) {
  switch (button) {
  case Buttons::A:
    return "A";
  case Buttons::B:
    return "B";
  case Buttons::X:
    return "X";
  case Buttons::Y:
    return "Y";
  case Buttons::plus:
    return "plus";
  case Buttons::minus:
    return "minus";
  case Buttons::home:
    return "home";
  case Buttons::share:
    return "share";
  case Buttons::L1:
    return "L1";
  case Buttons::L2:
    return "L2";
  case Buttons::L3:
    return "L3";
  case Buttons::R1:
    return "R1";
  case Buttons::R2:
    return "R2";
  case Buttons::R3:
    return "R3";
  default:
    throw ButtonError("ButtonError: Tried to get the name of unknown button.");
  }
}

uint8_t RealController::buttons_bit_position(Buttons button, PacketType packet) {
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
    case Buttons::B:
    case Buttons::minus:
      return 0x00;
    case Buttons::A:
    case Buttons::plus:
      return 0x01;
    case Buttons::Y:
    case Buttons::L3:
      return 0x02;
    case Buttons::X:
    case Buttons::R3:
      return 0x03;
    case Buttons::home:
    case Buttons::L1:
      return 0x04;
    case Buttons::share:
    case Buttons::R1:
      return 0x05;
    case Buttons::L2:
      return 0x06;
    case Buttons::R2:
      return 0x07;
    default:
      throw ButtonError("ButtonError: Tried to find bytepos of unknown button.");
    }
  }
  switch (button) {
  case Buttons::Y:
  case Buttons::minus:
    return 0x00;
  case Buttons::X:
  case Buttons::plus:
    return 0x01;
  case Buttons::B:
  case Buttons::R3:
    return 0x02;
  case Buttons::A:
  case Buttons::L3:
    return 0x03;
  case Buttons::home:
    return 0x04;
  case Buttons::share:
    return 0x05;
  case Buttons::L1:
  case Buttons::R1:
    return 0x06;
  case Buttons::L2:
  case Buttons::R2:
    return 0x07;
  default:
    throw ButtonError("ButtonError: Tried to find bitpos of unknown button.");
  }
}

uint8_t RealController::buttons_byte_button_value(Buttons button, PacketType packet) {
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
    case Buttons::B:
    case Buttons::minus:
      return 0x01;
    case Buttons::A:
    case Buttons::plus:
      return 0x02;
    case Buttons::Y:
    case Buttons::L3:
      return 0x04;
    case Buttons::X:
    case Buttons::R3:
      return 0x08;
    case Buttons::home:
    case Buttons::L1:
      return 0x10;
    case Buttons::share:
    case Buttons::R1:
      return 0x20;
    case Buttons::L2:
      return 0x40;
    case Buttons::R2:
      return 0x80;
    default:
      throw ButtonError("ButtonError: Tried to find bytepos of unknown button.");
    }
  }
  switch (button) {
  case Buttons::Y:
  case Buttons::minus:
    return 0x01;
  case Buttons::X:
  case Buttons::plus:
    return 0x02;
  case Buttons::B:
  case Buttons::R3:
    return 0x04;
  case Buttons::A:
  case Buttons::L3:
    return 0x08;
  case Buttons::home:
    return 0x10;
  case Buttons::share:
    return 0x20;
  case Buttons::L1:
  case Buttons::R1:
    return 0x40;
  case Buttons::L2:
  case Buttons::R2:
    return 0x80;
  default:
    throw ButtonError("ButtonError: Tried to find bytepos of unknown button.");
  }
}

size_t  RealController::buttons_data_address(Buttons button, PacketType packet) {
  /// Joycons layout
  /*if (packet == PacketType::normal_ctrl_report) {
    switch (button) {
    case Buttons::A:
    case Buttons::B:
    case Buttons::X:
    case Buttons::Y:
      return 0x01;
    case Buttons::plus:
    case Buttons::minus:
    case Buttons::home:
    case Buttons::share:
    case Buttons::L1:
    case Buttons::L2:
    case Buttons::L3:
    case Buttons::R1:
    case Buttons::R2:
    case Buttons::R3:
      return 0x02;
    }
  }*/
  if (packet == PacketType::normal_ctrl_report) {
    switch (button) {
    case Buttons::A:
    case Buttons::B:
    case Buttons::X:
    case Buttons::Y:
    case Buttons::L1:
    case Buttons::L2:
    case Buttons::R1:
    case Buttons::R2:
      return 0x01;
    case Buttons::plus:
    case Buttons::minus:
    case Buttons::home:
    case Buttons::share:
    case Buttons::L3:
    case Buttons::R3:
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
  case Buttons::A:
  case Buttons::B:
  case Buttons::X:
  case Buttons::Y:
  case Buttons::R1:
  case Buttons::R2:
    return address[0x00];
  case Buttons::plus:
  case Buttons::minus:
  case Buttons::home:
  case Buttons::share:
  case Buttons::L3:
  case Buttons::R3:
    return address[0x01];
  case Buttons::L1:
  case Buttons::L2:
    return address[0x02];
  default:
    throw ButtonError("ButtonError: Tried to find address of unknown button.");
  }
}


const char *RealController::axis_name(Axis axis) {
  switch (axis) {
  case Axis::axis_lx:
    return "axis_lx";
  case Axis::axis_ly:
    return "axis_ly";
  case Axis::axis_rx:
    return "axis_rx";
  case Axis::axis_ry:
    return "axis_ry";
  default:
    throw AxisError("AxisError: Tried to get the name of unknown axis.");
  }
}

size_t RealController::axis_data_address_high(Axis axis, PacketType packet) {
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
  case Axis::axis_lx:
    return address[0];
  case Axis::axis_ly:
    return address[1];
  case Axis::axis_rx:
    return address[2];
  case Axis::axis_ry:
    return address[3];
  default:
    throw AxisError("AxisError: Tried to find address of unknown axis.");
  }
}
size_t RealController::axis_data_address_low(Axis axis, PacketType packet) {
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
  case Axis::axis_lx:
    return address[0];
  case Axis::axis_ly:
    return address[1];
  case Axis::axis_rx:
    return address[2];
  case Axis::axis_ry:
    return address[3];
  default:
    throw AxisError("AxisError: Tried to find address of unknown axis.");
  }
}


const char *RealController::dpad_name(Dpad dpad) {
  switch (dpad) {
  case Dpad::d_left:
    return "d_left";
  case Dpad::d_right:
    return "d_right";
  case Dpad::d_up:
    return "d_up";
  case Dpad::d_down:
    return "d_down";
  default:
    throw DpadError("DpadError: Tried to get the name of unknown dpad.");
  }
}

uint8_t RealController::dpad_bit_position(Dpad dpad, PacketType packet) {
  if (packet != PacketType::standard_input_report && packet != PacketType::packet_req) {
    throw DpadError("AxisError: This packet type (" + std::to_string(packet) + ") does not map dpad to specific bits.");
  }
  switch (dpad) {
  case Dpad::d_down:
    return 0x00;
  case Dpad::d_up:
    return 0x01;
  case Dpad::d_right:
    return 0x02;
  case Dpad::d_left:
    return 0x03;
  default:
    throw DpadError("DpadError: Tried to find bitpos of unknown dpad button.");
  }
}

uint8_t RealController::dpad_byte_value(Dpad dpad, PacketType packet) {
  if (packet != PacketType::standard_input_report && packet != PacketType::packet_req) {
    throw DpadError("AxisError: This packet type (" + std::to_string(packet) + ") does not map dpad to specific bits.");
  }
  switch (dpad) {
  case Dpad::d_down:
    return 0x01;
  case Dpad::d_up:
    return 0x02;
  case Dpad::d_right:
    return 0x04;
  case Dpad::d_left:
    return 0x08;
  default:
    throw DpadError("DpadError: Tried to find bytepos of unknown dpad button.");
  }
}

size_t RealController::dpad_data_address(Dpad dpad, PacketType packet) {
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
  case Dpad::d_down:
  case Dpad::d_up:
  case Dpad::d_right:
  case Dpad::d_left:
    return address[0x00];
  default:
    throw DpadError("DpadError: Tried to find address of unknown dpad button.");
  }
}
