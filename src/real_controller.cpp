#include "real_controller.hpp"
using namespace RealController;


Controller::Controller(const HidApi::Enumerate &device_info, unsigned short n_controll)
              : connection(device_info), n_controller(n_controll) {
  closed = false;
  connection.setBlocking();

  if (!connection.Bluetooth()) {
    RealController::ControllerMAC mac = connection.request_mac();
    printf("controller_type: %02x\n", mac.controller_type);
    printf("mac: %02x", mac.mac[0]);
    for (size_t i = 1; i < 6; ++i) {
      printf(":%02x", mac.mac[i]);
    }
    printf("\n");

    connection.do_handshake();
    connection.increment_baudrate();
    connection.do_handshake();

    connection.enable_hid_only_mode();
  }

  connection.toggle_rumble(true);
  connection.toggle_imu(true);

  // connection.set_imu_sensitivity(0x03, 0x00, 0x00, 0x01);

  connection.set_input_report_mode(0x30);

  #if 0
  if (connection.Bluetooth()) {
    //HidApi::GenericPacket<1> msg_increase_datarate_bt{{0x31}};
    //send_subcommand(SubCmd::set_in_report, msg_increase_datarate_bt);
    connection.set_input_report_mode(0x31);
    receive_input().print();
  }
  #endif

  connection.setNonBlocking();
  // usleep(100 * 1000);

  led();
}

Controller::Controller(Controller &&other) noexcept: 
  connection(std::move(other.connection)), n_controller(std::move(other.n_controller)), 
  blink_position(std::move(other.blink_position)), blink_counter(std::move(other.blink_counter)), 
  closed(std::move(other.closed)) {
}

Controller::~Controller() noexcept {
  if (closed) {
    return;
  }

  try {
    close();
    // Wait for controller to receive the close packet.
    //usleep(1000 * 1000);
  }
  catch (const HidApi::HidApiError &e) {
  }
}

Controller &Controller::operator=(Controller &&other) noexcept {
  std::swap(connection, other.connection);
  std::swap(n_controller, other.n_controller);
  std::swap(closed, other.closed);
  std::swap(blink_position, other.blink_position);
  std::swap(blink_counter, other.blink_counter);
  return *this;
}

RealController::Parser Controller::receive_input() {
  /// TODO: do a waiting loop if a zero packet is received or something.
  HidApi::DefaultPacket buff;
  size_t len;
  do {
    len = connection.receive_input(buff, 16);
  } while (len == 0);
  return RealController::Parser(len, buff);
}

RealController::Parser Controller::request_input() {
  HidApi::DefaultPacket buff;
  size_t len = connection.request_input(buff);
  return RealController::Parser(len, buff);
}


void Controller::led(int number){
  uint8_t bitwise = player_led[n_controller];
  if (number >= 0) {
    bitwise = static_cast<uint8_t>(number);
  }

  connection.set_player_leds(bitwise);
  // connection.set_player_leds(bitwise);
}

void Controller::blink() {
  if (++blink_counter > blink_length) {
    blink_counter = 0;
    if (++blink_position >= blink_array.size()) {
      blink_position = 0;
    }
  }

  connection.set_player_leds(blink_array[blink_position]);
}

void Controller::send_rumble(uint8_t large_motor, uint8_t small_motor) {
  HidApi::GenericPacket<4> left {0x80, 0x00, 0x40, 0x40};
  HidApi::GenericPacket<4> right{0x80, 0x00, 0x40, 0x40};

  if (large_motor != 0) {
    left[1] = right[1] = 0x08;
    left[2] = right[2] = large_motor;
  } else if (small_motor != 0) {
    left[1] = right[1] = 0x10;
    left[2] = right[2] = small_motor;
  }

  connection.send_rumble(left, right);
}

void Controller::rumble(/*int frequency, int intensity*/) {
  HidApi::GenericPacket<4> default_rumble{0x00, 0x01, 0x40, 0x40};
  connection.send_rumble(default_rumble, default_rumble);
}

void Controller::close() {
  if (closed) {
    return;
  }
  closed = true;
  connection.setBlocking();
  connection.toggle_rumble(false);
  connection.toggle_imu(false);

  if (!connection.Bluetooth()) {
    connection.disable_hid_only_mode();
    connection.send_reset();
  }
}