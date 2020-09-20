#pragma once
#ifndef HID_CONTROLLER_H
#define HID_CONTROLLER_H

#include "hidapi.h"
#include <array>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <unistd.h>

#include "proinputparser.hpp"

#define TEST_BAD_DATA_CYCLES 10

class HidController{
public:
  HidController(unsigned short vendor_id, unsigned short product_id,
                const wchar_t *serial_number, unsigned short n_controll)
                : ven_id(vendor_id), prod_id(product_id), 
                  n_controller(n_controll) {
    controller_ptr = hid_open(vendor_id, product_id, serial_number);
    // controller_ptr = hid_open_path("/dev/input/hidraw0");
 
    //printf("SERIAL NUMBER: %u\n", serial_number);
    if (!controller_ptr) {
      throw std::ios_base::failure("Can't connect to controller. Invalid device pointer");
    }
    // hid_device_info *info = hid_open(vendor_id, product_id, serial_number);
    // std::cout<< "PATH: " << info->path << std::endl;;

    // if (false)
    // { //!exchange(handshake)) { //need std::optional
    //     red();
    //     printf("ERROR: exchange handshake failed!\n");
    //     normal();
    // }

    // set_non_blocking();

    exchange(switch_baudrate);
    exchange(handshake);

    // the next part will sometimes fail, then need to reopen device via hidapi
    exchange(hid_only_mode, true);

    usleep(100 * 1000);

    // TEST FOR BAD DATA
    for (size_t i = 0; i < TEST_BAD_DATA_CYCLES; ++i) {
      if (try_read_bad_data()) {
        throw std::runtime_error("Detected bad data stream. Trying again...");
      }
    }
  }

  ~HidController(){
    if (controller_ptr != nullptr) {
      hid_close(controller_ptr);
    }
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

  void set_non_blocking() {
    if (hid_set_nonblocking(controller_ptr, 1) < 0) {
      throw std::runtime_error("Couldn't set controller " + 
                               std::to_string(n_controller) + 
                               " to non-blocking.");
    }
  }

  void set_blocking() {
    if (hid_set_nonblocking(controller_ptr, 0) < 0) {
      throw std::runtime_error("Couldn't set controller " + 
                               std::to_string(n_controller) + 
                               " to blocking.");
    }
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
  ProInputParser exchange(std::array<uint8_t, length> const &data,
                          bool timed = false) {
    if (hid_write(controller_ptr, data.data(), length) < 0) {
      throw std::system_error(-1, std::generic_category(), 
                              "ERROR: read() returned -1!\n"
                              "Did you disconnect the controller?");
    }

    ProInputParser::exchange_array ret;
    ret.fill(0);

    bool successful = false;
    if (!timed) {
      successful = hid_read(controller_ptr, ret.data(), ProInputParser::exchange_length) >= 0;
    } else {
      successful = hid_read_timeout(controller_ptr, ret.data(), ProInputParser::exchange_length, 100) != 0;
    }

    if (!successful) {
      throw std::system_error(-2, std::generic_category());
    }

    return ProInputParser(ret);
  }

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
    return exchange(buffer);
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

  hid_device *controller_ptr = nullptr;

  unsigned short ven_id;
  unsigned short prod_id;
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
  // const std::array<uint8_t, 4> blink_array{{0x05, 0x10, 0x04, 0x08}};
  const std::array<uint8_t, 4> blink_array{{0x01, 0x02, 0x04, 0x08}};


  uint blink_position = 0;
  size_t blink_counter = 0;
  const size_t blink_length = 8;

  static constexpr uint8_t led_command{0x30};
  static constexpr uint8_t get_input{0x1f};
};

#endif
