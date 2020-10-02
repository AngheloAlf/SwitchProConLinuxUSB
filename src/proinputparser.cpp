#include "proinputparser.hpp"
using namespace ProInputParser;

#include "utils.hpp"


ParserError::ParserError(): std::runtime_error("ParserError: Unspecified error") {
  std::string aux = std::string("ParserError: ") + "Unspecified error";
  Utils::Str::copy_string_to_char(str, aux);
}

ParserError::ParserError(const std::string& what_arg): std::runtime_error(what_arg) {
  std::string aux = std::string("ParserError: ") + what_arg;
  Utils::Str::copy_string_to_char(str, aux);
}
ParserError::ParserError(const char* what_arg): std::runtime_error(what_arg) {
  std::string aux = std::string("ParserError: ") + what_arg;
  Utils::Str::copy_string_to_char(str, aux);
}

ParserError::~ParserError() {
  if (str != nullptr) {
    free(str);
    str = nullptr;
  }
}


PacketLengthError::PacketLengthError(): PacketTypeError() {
}

PacketLengthError::PacketLengthError(const std::string& what_arg): PacketTypeError(what_arg) {
}
PacketLengthError::PacketLengthError(const char* what_arg): PacketTypeError(what_arg) {
}


const char *ParserError::what() const noexcept {
  return str;
}

const char *ProInputParser::button_name(BUTTONS button) {
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

uint8_t ProInputParser::buttons_bit_position(BUTTONS button) {
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

uint8_t ProInputParser::buttons_byte_button_value(BUTTONS button) {
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

size_t  ProInputParser::buttons_data_address(BUTTONS button, PacketType packet) {
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


const char *ProInputParser::axis_name(AXIS axis) {
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

size_t ProInputParser::axis_data_address_high(AXIS axis, PacketType packet) {
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
    throw AxisError("AxisError: This packet type (" + std::to_string(packet) + ") does not contain axis data.");
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
    throw AxisError("AxisError: Tried to find address of unknown axis.");
  }
}
size_t ProInputParser::axis_data_address_low(AXIS axis, PacketType packet) {
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
    throw AxisError("AxisError: This packet type (" + std::to_string(packet) + ") does not contain axis data.");
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
    throw AxisError("AxisError: Tried to find address of unknown axis.");
  }
}


const char *ProInputParser::dpad_name(DPAD dpad) {
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

uint8_t ProInputParser::dpad_bit_position(DPAD dpad) {
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

uint8_t ProInputParser::dpad_byte_value(DPAD dpad) {
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

size_t ProInputParser::dpad_data_address(DPAD dpad, PacketType packet) {
  std::array<size_t, 1> address;
  switch (packet) {
  case PacketType::standard_input_report:
    address[0x00] = 0x05;
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


void ProInputParser::print_exchange_array(size_t packet_len, HidApi::default_packet arr) {
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


Parser::Parser(size_t packet_len, HidApi::default_packet data): len(packet_len), dat(data) {
  type = PacketType::unknown;

  if (packet_len == 0) {
    throw PacketLengthError("PacketLengthError: Packet can't have zero length.");
  }

  switch (dat[0x00]) {
  case 0x00:
    type = PacketType::zeros;
    break;

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

bool Parser::is_button_pressed(BUTTONS button) const {
  uint8_t pos = buttons_data_address(button);
  return dat[pos] & buttons_byte_button_value(button);
}

uint16_t Parser::get_axis_status(AXIS axis) const {
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

bool Parser::is_dpad_pressed(DPAD dpad) const {
  uint8_t pos = dpad_data_address(dpad);
  return dat[pos] & dpad_byte_value(dpad);
}


/* Hackishly detects when the controller is trapped in a bad loop.
Nothing to do here, need to reopen device :(*/
bool Parser::detect_bad_data() const {
  return dat[1] == 0x01 && dat[0] == 0x81;
}

/* If this returns true, there is no controller information in this package,
  * we can skip it*/
bool Parser::detect_useless_data() const {
  /*if (dat == 0x30)
    n_bad_data_thirty++;
  if (dat == 0x00)
    n_bad_data_zero++;*/
  return (/*dat[0] == 0x30 ||*/ dat[0] == 0x00);
}

bool Parser::has_button_and_axis_data() const {
  return type == PacketType::standard_input_report || type == PacketType::packet_req;
}

void Parser::print() const {
  print_exchange_array(len, dat);
}

size_t Parser::buttons_data_address(BUTTONS button) const {
  return ProInputParser::buttons_data_address(button, type);
}

size_t Parser::axis_data_address_high(AXIS axis) const {
  return ProInputParser::axis_data_address_high(axis, type);
}
size_t Parser::axis_data_address_low(AXIS axis) const {
  return ProInputParser::axis_data_address_low(axis, type);
}


size_t Parser::dpad_data_address(DPAD dpad) const {
  return ProInputParser::dpad_data_address(dpad, type);
}
