#pragma once
#ifndef HID_CONTROLLER_H
#define HID_CONTROLLER_H

#include <array>
#include <stdexcept>
#include <unistd.h>

#include "hidapi_wrapper.hpp"
#include "proinputparser.hpp"

#define TEST_BAD_DATA_CYCLES 10

class HidController{
public:
  HidController(unsigned short vendor_id, unsigned short product_id,
                const wchar_t *serial_number, unsigned short n_controll)
                : hid(vendor_id, product_id, serial_number), 
                  n_controller(n_controll) {
    hid.set_blocking();

    hid.exchange(handshake);
    hid.exchange(switch_baudrate);
    hid.exchange(handshake);

    // the next part will sometimes fail, then need to reopen device via hidapi
    hid.exchange(hid_only_mode, true);

    hid.set_non_blocking();
    usleep(100 * 1000);

    // TEST FOR BAD DATA
    for (size_t i = 0; i < TEST_BAD_DATA_CYCLES; ++i) {
      if (try_read_bad_data()) {
        throw std::runtime_error("Detected bad data stream. Trying again...");
      }
    }
  }

  ~HidController(){
    hid.set_blocking();
    hid.exchange(msg_close);

    // Wait for controller to receive the close packet.
    usleep(1000 * 1000);
  }

  ProInputParser request_input() {
    return send_command(get_input, empty);
  }

  void led(int number = -1){
    std::array<uint8_t, 1> value {
      number < 0 ?
      player_led[n_controller] : 
      static_cast<uint8_t>(number)};
    
    send_subcommand(0x1, led_command, value);
  }

  void blink() {
    if (++blink_counter > blink_length) {
      blink_counter = 0;
      if (++blink_position >= blink_array.size()) {
        blink_position = 0;
      }
    }
    std::array<uint8_t,1> blink_command{{blink_array[blink_position]}};
    send_subcommand(0x1, led_command, blink_command);
  }

  ProInputParser send_rumble(uint8_t large_motor, uint8_t small_motor) {
    std::array<uint8_t, 9> buf{
      static_cast<uint8_t>(rumble_counter++ & 0xF),
      0x80, 0x00, 0x40, 0x40, 0x80, 0x00, 0x40, 0x40};

    if (large_motor != 0) {
      buf[1] = buf[5] = 0x08;
      buf[2] = buf[6] = large_motor;
    } else if (small_motor != 0) {
      buf[1] = buf[5] = 0x10;
      buf[2] = buf[6] = small_motor;
    }
    ProInputParser ret = send_command(0x10, buf);
    ret.print();
    return ret;
  }

private:
  template <size_t length>
  ProInputParser send_command(uint8_t command,
                              std::array<uint8_t, length> const &data) {
    std::array<uint8_t, length + 0x9> buffer;
    buffer.fill(0);
    buffer[0x0] = 0x80;
    buffer[0x1] = 0x92;
    buffer[0x3] = 0x31;
    buffer[0x8] = command;
    if (length > 0) {
      memcpy(buffer.data() + 0x9, data.data(), length);
    }
    return ProInputParser(hid.exchange(buffer));
  }

  template <size_t length>
  ProInputParser send_subcommand(uint8_t command, uint8_t subcommand,
                                 std::array<uint8_t, length> const &data) {
    std::array<uint8_t, length + 10> buffer{
        static_cast<uint8_t>(rumble_counter++ & 0xF),
        0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40,
        subcommand};

    if (length > 0) {
      memcpy(buffer.data() + 10, data.data(), length);
    }
    return send_command(command, buffer);
  }

  bool try_read_bad_data() {
    ProInputParser dat = send_command(get_input, empty);

    if (dat.detect_useless_data()) {
      return false;
    }
    if (dat.detect_bad_data()) {
      // print_exchange_array(dat);
      return true;
    }

    return false;
  }

  HidApi hid;

  unsigned short n_controller;

  uint8_t rumble_counter{0};
  const std::array<uint8_t, 8> player_led{0x01, 0x03, 0x07, 0x0f, 0x09, 0x05, 0x0d, 0x06};
  const std::array<uint8_t, 0> empty{{}};

  const std::array<uint8_t, 2> handshake{{0x80, 0x02}};
  const std::array<uint8_t, 2> switch_baudrate{{0x80, 0x03}};
  /** 
   * Forces the Pro Controller to only talk over USB HID without any timeouts. 
   * This is required for the Pro Controller to not time out and revert to Bluetooth.
   */
  const std::array<uint8_t, 2> hid_only_mode{{0x80, 0x04}};
  const std::array<uint8_t, 2> msg_close{{0x80, 0x05}};

  // const std::array<uint8_t, 4> blink_array{{0x05, 0x10, 0x04, 0x08}};
  const std::array<uint8_t, 4> blink_array{{0x01, 0x02, 0x04, 0x08}};


  uint blink_position = 0;
  size_t blink_counter = 0;
  const size_t blink_length = 8;

  static constexpr uint8_t led_command{0x30};
  static constexpr uint8_t get_input{0x1f};
};

#endif
