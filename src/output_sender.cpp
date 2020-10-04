#include "output_sender.hpp"
using namespace OutputSender;


Sender::Sender(const HidApi::Enumerate &device_info): hidw(device_info) {
  std::string serial_number = hidw.get_serial_number();
  bluetooth = false;
  if (serial_number.find(':') != std::string::npos) {
    printf("bluetooth!");
    bluetooth = true;
  }
}

/*const HidApi::Device &Sender::Hid() const {
  return hidw;
}*/

bool Sender::Bluetooth() const {
  return bluetooth;
}
bool Sender::Usb() const {
  return !bluetooth;
}

void Sender::setBlocking() {
  if (!hidw.IsBlocking()) {
    hidw.set_blocking();
  }
}
void Sender::setNonBlocking() {
  if (hidw.IsBlocking()) {
    hidw.set_non_blocking();
  }
}

ProInputParser::ControllerMAC Sender::request_mac(int milliseconds) {
  HidApi::default_packet response;
  size_t len = send_uart(Uart::status);
  if (len != 2) {
    // throw ;
  }

  len = hidw.read(response, milliseconds);
  if (len < 10) {
    // throw ;
  }
  if (response[0] != Protocols::nin_response || response[1] != Uart::status) {
    send_reset();
    throw std::runtime_error("USB connection wasn't closed properly.");
  }

  ProInputParser::ControllerMAC data;
  data.controller_type = (response[2] << 8) | response[3];
  for (size_t i = 0; i < 6; ++i) {
    data.mac[i] = response[9-i];
  } 
  return data;
}
void Sender::do_handshake() {
  send_uart(Uart::handshake);
  // TODO: check if controller answers
}
void Sender::increment_baudrate() {
  send_uart(Uart::inc_baudrate);
  // TODO: check if controller answers
}
void Sender::enable_hid_only_mode() {
  send_uart(Uart::hid_only);
  // TODO: check if controller answers
}
void Sender::disable_hid_only_mode() {
  send_uart(Uart::turn_off_hid);
  // TODO: check if controller answers
}
void Sender::send_reset() {
  send_uart(Uart::reset);
  // TODO: check if controller answers
}



void Sender::set_player_leds(uint8_t bitwise) {
  HidApi::generic_packet<1> value {bitwise};
  send_subcommand(SubCmd::set_leds, value, no_rumble, no_rumble);
  // TODO: check if controller answers
}
/*void Sender::get_player_leds() {
}*/

void Sender::toggle_imu(bool en) {
  send_subcommand(SubCmd::en_imu, en ? enable : disable, no_rumble, no_rumble);
  // TODO: check if controller answers
}
void Sender::set_imu_sensitivity(uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t arg4) {
  HidApi::generic_packet<4> imu_args{arg1, arg2, arg3, arg4};
  send_subcommand(SubCmd::set_imu, imu_args, no_rumble, no_rumble);
  // TODO: check if controller answers
}

void Sender::toggle_rumble(bool en) {
  send_subcommand(SubCmd::en_rumble, en ? enable : disable, no_rumble, no_rumble);
  // TODO: check if controller answers
}


void Sender::set_input_report_mode(uint8_t mode) {
  HidApi::generic_packet<1> buff{mode};
  send_subcommand(SubCmd::set_in_report, buff, no_rumble, no_rumble);
  // TODO: check if controller answers
}


void Sender::send_rumble(const HidApi::generic_packet<4> &left_rumble, 
                  const HidApi::generic_packet<4> &right_rumble) {
  send_command_with_rumble_data(Cmd::rumble_only, empty, left_rumble, right_rumble);
  // TODO: check if controller answers
}



size_t Sender::send_uart(Uart uart) {
  HidApi::generic_packet<2> packet {Protocols::nintendo, uart};
  return hidw.write(packet);
}
