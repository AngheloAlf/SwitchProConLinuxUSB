#include "real_controller_parser.hpp"
using namespace RealController;

#include "real_controller_exceptions.hpp"
#include "utils.hpp"

void RealController::printPacket(size_t packet_len, uint8_t *arr) {
  bool redcol = false;
  if (arr[0] != 0x30) {
    Utils::PrintColor::yellow();
  }
  else {
    Utils::PrintColor::red();
    redcol = true;
  }
  for (size_t i = 0; i < packet_len; ++i) {
    if (arr[i] == 0x00) {
      Utils::PrintColor::blue();
    } else {
      if (redcol) {
        Utils::PrintColor::red();
      } else {
        Utils::PrintColor::yellow();
      }
    }
    printf("%02X ", arr[i]);
  }
  Utils::PrintColor::normal();
  printf("\n");
  fflush(stdout);
}

void RealController::printPacket(size_t packet_len, HidApi::DefaultPacket arr) {
  printPacket(packet_len, arr.data());
}

Parser::Parser(size_t packet_len, HidApi::DefaultPacket data): len(packet_len), dat(data) {
  type = PacketType::unknown;

  if (packet_len == 0) {
    throw PacketLengthError("PacketLengthError: Packet can't have zero length.");
  }

  switch (dat[0x00]) {
  case 0x21: // ?
  case 0x30:
  case 0x31: // ?
    if (len < 13) {
      type = PacketType::unknown;
      printf("invalid_packet\n");
      print();
      break;
    }
    type = PacketType::standard_input_report;
    break;

  case 0x3F:
    type = PacketType::normal_ctrl_report;
    // print();
    break;

  case 0x81:
    type = PacketType::packet_req;
    break;

  default:
    type = PacketType::unknown;
    break;
  }

  if (type == PacketType::unknown) {
    type = PacketType::unknown;
    printf("unknown packet\n");
    print();
    throw PacketTypeError("PacketTypeError: Unrecognized packet (" + Utils::Str::to_hexstr(dat[0x00]) + ")." );
  }
}

bool Parser::is_button_pressed(Buttons button) const {
  uint8_t pos = buttons_data_address(button);
  return dat[pos] & buttons_byte_button_value(button);
}

uint16_t Parser::get_axis_status(Axis axis) const {
  size_t high, low;
  high = axis_data_address_high(axis);
  low  = axis_data_address_low(axis);
  uint16_t value = (dat[high] << 8) | dat[low];

  if (type == PacketType::normal_ctrl_report) {
    /// This packet reports the y-axis inverted.
    return value >> 4;
  }
  switch (axis) {
  case Axis::axis_lx:
  case Axis::axis_rx:
    return value & 0x0FFF;
  case Axis::axis_ly:
  case Axis::axis_ry:
    return value >> 4;
  default:
    return 0x07FF;
  }
}

bool Parser::is_dpad_pressed(Dpad dpad) const {
  size_t pos = dpad_data_address(dpad);
  uint8_t byte = dat[pos];
  if (type == PacketType::normal_ctrl_report) {
    switch (dpad) {
    case Dpad::d_down:
      switch (byte) {
      case 5:
      case 4:
      case 3:
        return true;
      default:
        return false;
      }

    case Dpad::d_up:
      switch (byte) {
      case 7:
      case 0:
      case 1:
        return true;
      default:
        return false;
      }

    case Dpad::d_right:
      switch (byte) {
      case 1:
      case 2:
      case 3:
        return true;
      default:
        return false;
      }

    case Dpad::d_left:
      switch (byte) {
      case 7:
      case 6:
      case 5:
        return true;
      default:
        return false;
      }

    default:
      throw DpadError("DpadError: Tried to get state of unknown dpad button.");
    }
  }
  return byte & dpad_byte_value(dpad);
}


bool Parser::has_button_and_axis_data() const {
  switch (type) {
  case PacketType::standard_input_report:
  case PacketType::normal_ctrl_report:
  case PacketType::packet_req:
    return true;

  default:
    return false;
  }
}

void Parser::print() const {
  printPacket(len, dat);
}

uint8_t Parser::buttons_bit_position(Buttons button) const {
  return RealController::buttons_bit_position(button, type);
}
uint8_t Parser::buttons_byte_button_value(Buttons button) const {
  return RealController::buttons_byte_button_value(button, type);
}
size_t  Parser::buttons_data_address(Buttons button) const {
  return RealController::buttons_data_address(button, type);
}

size_t Parser::axis_data_address_high(Axis axis) const {
  return RealController::axis_data_address_high(axis, type);
}
size_t Parser::axis_data_address_low(Axis axis) const {
  return RealController::axis_data_address_low(axis, type);
}


uint8_t Parser::dpad_bit_position(Dpad dpad) const {
  return RealController::dpad_bit_position(dpad, type);
}
uint8_t Parser::dpad_byte_value(Dpad dpad) const {
  return RealController::dpad_byte_value(dpad, type);
}
size_t  Parser::dpad_data_address(Dpad dpad) const {
  return RealController::dpad_data_address(dpad, type);
}
