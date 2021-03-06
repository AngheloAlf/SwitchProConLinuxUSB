#include "real_controller_connection.hpp"
using namespace RealController;


ControllerConnection::ControllerConnection(const HidApi::Enumerate &device_info): hidw(device_info) {
  std::string serial_number = hidw.get_serial_number();
  bluetooth = false;
  if (serial_number.find(':') != std::string::npos) {
    bluetooth = true;
  }
}
ControllerConnection::ControllerConnection(ControllerConnection &&other) noexcept: hidw(std::move(other.hidw)),
  timing_counter(std::move(other.timing_counter)), bluetooth(std::move(other.bluetooth)) {
}

ControllerConnection::~ControllerConnection() noexcept {
}

ControllerConnection &ControllerConnection::operator=(ControllerConnection &&other) noexcept {
  std::swap(hidw, other.hidw);
  std::swap(timing_counter, other.timing_counter);
  std::swap(bluetooth, other.bluetooth);
  return *this;
}


bool ControllerConnection::Bluetooth() const {
  return bluetooth;
}
bool ControllerConnection::Usb() const {
  return !bluetooth;
}

void ControllerConnection::setBlocking() {
  if (!hidw.IsBlocking()) {
    hidw.set_blocking();
  }
}
void ControllerConnection::setNonBlocking() {
  if (hidw.IsBlocking()) {
    hidw.set_non_blocking();
  }
}

RealController::ControllerMAC ControllerConnection::request_mac(int milliseconds) {
  HidApi::DefaultPacket response;
  size_t len = send_uart(Uart::status);
  if (len != 2) {
    // throw ;
  }

  len = hidw.read(response, milliseconds);
  if (len < 10) {
    // throw ;
  }
  if (response[0] != Protocols::nin_response || response[1] != Uart::status
      || response[2] != 0) {
    // RealController::printPacket(len, response);
    send_reset();
    throw std::runtime_error("USB connection wasn't closed properly.");
  }

  RealController::ControllerMAC data;
  data.controller_type = response[3];
  for (size_t i = 0; i < 6; ++i) {
    data.mac[i] = response[9-i];
  } 
  return data;
}
void ControllerConnection::do_handshake() {
  HidApi::DefaultPacket response;
  size_t len = send_uart(Uart::handshake);
  if (len != 2) {
    // throw ;
  }

  len = hidw.read(response);
  if (len < 2) {
    // throw ;
  }
  if (response[0] != Protocols::nin_response || response[1] != Uart::handshake) {
    send_reset(); // ?
    throw std::runtime_error(/*"USB connection wasn't closed properly."*/"..");
  }
}
void ControllerConnection::increment_baudrate() {
  HidApi::DefaultPacket response;
  size_t len = send_uart(Uart::inc_baudrate);
  if (len != 2) {
    // throw ;
  }

  len = hidw.read(response);
  if (len < 2) {
    // throw ;
  }
  if (response[0] != Protocols::nin_response || response[1] != Uart::inc_baudrate) {
    send_reset(); // ?
    throw std::runtime_error(/*"USB connection wasn't closed properly."*/"...");
  }
}
void ControllerConnection::enable_hid_only_mode() {
  HidApi::DefaultPacket response;
  size_t len = send_uart(Uart::hid_only);
  if (len != 2) {
    // throw ;
  }

  len = hidw.read(response);
}
void ControllerConnection::disable_hid_only_mode() {
  HidApi::DefaultPacket response;
  size_t len = send_uart(Uart::turn_off_hid);
  if (len != 2) {
    // throw ;
  }

  len = hidw.read(response);
}
void ControllerConnection::send_reset() {
  HidApi::DefaultPacket response;
  size_t len = send_uart(Uart::reset);
  if (len != 2) {
    // throw ;
  }

  len = hidw.read(response);
}



void ControllerConnection::set_player_leds(uint8_t bitwise) {
  HidApi::GenericPacket<1> value {bitwise};
  send_subcommand(SubCmd::set_leds, value, no_rumble, no_rumble);

  HidApi::DefaultPacket response;
  hidw.read(response);
}
/*void ControllerConnection::get_player_leds() {
}*/

void ControllerConnection::set_input_report_mode(uint8_t mode) {
  HidApi::GenericPacket<1> buff{mode};
  send_subcommand(SubCmd::set_in_report, buff, no_rumble, no_rumble);

  HidApi::DefaultPacket response;
  hidw.read(response);
}

void ControllerConnection::toggle_imu(bool en) {
  send_subcommand(SubCmd::en_imu, en ? enable : disable, no_rumble, no_rumble);

  HidApi::DefaultPacket response;
  hidw.read(response);
}
void ControllerConnection::set_imu_sensitivity(uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t arg4) {
  HidApi::GenericPacket<4> imu_args{arg1, arg2, arg3, arg4};
  send_subcommand(SubCmd::set_imu, imu_args, no_rumble, no_rumble);

  HidApi::DefaultPacket response;
  hidw.read(response);
}

void ControllerConnection::toggle_rumble(bool en) {
  send_subcommand(SubCmd::en_rumble, en ? enable : disable, no_rumble, no_rumble);

  HidApi::DefaultPacket response;
  hidw.read(response);
}


void ControllerConnection::send_rumble(const HidApi::GenericPacket<4> &left_rumble, 
                  const HidApi::GenericPacket<4> &right_rumble) {
  send_command_with_rumble_data(Cmd::rumble_only, empty, left_rumble, right_rumble);
  // TODO: check if controller answers
}



size_t ControllerConnection::send_uart(Uart uart) {
  HidApi::GenericPacket<2> packet {Protocols::nintendo, (uint8_t)uart};
  return hidw.write(packet);
}
