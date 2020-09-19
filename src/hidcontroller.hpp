#pragma once
#ifndef HID_CONTROLLER_H
#define HID_CONTROLLER_H

#include "hidapi.h"
#include <array>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <unistd.h>


#define TEST_BAD_DATA_CYCLES 10


class HidController{
private:
  hid_device *controller_ptr;

  unsigned short ven_id;
  unsigned short prod_id;
  unsigned short n_controller;

  uint8_t rumble_counter{0};
  const std::array<uint8_t, 8> player_led{0x01, 0x03, 0x07, 0x0f, 0x09, 0x05, 0x0d, 0x06};
  const std::array<uint8_t, 0> empty{{}};
  const std::array<uint8_t, 2> handshake{{0x80, 0x02}};
  const std::array<uint8_t, 2> switch_baudrate{{0x80, 0x03}};
  const std::array<uint8_t, 2> hid_only_mode{{0x80, 0x04}};
  // const std::array<uint8_t, 4> blink_array{{0x05, 0x10, 0x04, 0x08}};
  const std::array<uint8_t, 4> blink_array{{0x01, 0x02, 0x04, 0x08}};


  uint blink_position = 0;
  size_t blink_counter = 0;
  const size_t blink_length = 8;

  static constexpr uint8_t led_command{0x30};
  static constexpr uint8_t get_input{0x1f};
  //static constexpr uint8_t center{0x7e};
  static constexpr size_t exchange_length{0x400};
  using exchange_array = std::array<uint8_t, exchange_length>;

public:
  HidController(unsigned short vendor_id, unsigned short product_id,
                const wchar_t *serial_number, unsigned short n_controll) {
    controller_ptr = hid_open(vendor_id, product_id, serial_number);
    // controller_ptr = hid_open_path("/dev/input/hidraw0");
 
    //printf("SERIAL NUMBER: %u\n", serial_number);
    if (!controller_ptr) {
      throw std::ios_base::failure("Invalid device pointer.");
    }
    // hid_device_info *info = hid_open(vendor_id, product_id, serial_number);
    // std::cout<< "PATH: " << info->path << std::endl;;

    n_controller = n_controll;
    ven_id = vendor_id;
    prod_id = product_id;

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
    if (controller_ptr) {
      hid_close(controller_ptr);
      //PrintColor::blue();
      // printf("Closed controller nr. %u\n", n_controller);
      //PrintColor::normal();
    }
  }


  exchange_array request_input() {
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



  exchange_array send_rumble(uint8_t large_motor, uint8_t small_motor) {
    std::array<uint8_t, 9> buf{static_cast<uint8_t>(rumble_counter++ & 0xF),
                               0x80,
                               0x00,
                               0x40,
                               0x40,
                               0x80,
                               0x00,
                               0x40,
                               0x40};
    if (large_motor != 0) {
      buf[1] = buf[5] = 0x08;
      buf[2] = buf[6] = large_motor;
    } else if (small_motor != 0) {
      buf[1] = buf[5] = 0x10;
      buf[2] = buf[6] = small_motor;
    }
    exchange_array ret = send_command(0x10, buf);
    print_exchange_array(ret);
    return ret;
  }

  void print_exchange_array(exchange_array arr) {
    bool redcol = false;
    if (arr[0] != 0x30)
      PrintColor::yellow();
    else {
      PrintColor::red();
      redcol = true;
    }
    for (size_t i = 0; i < 20; ++i) {
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

private:

  template <size_t length>
  exchange_array exchange(std::array<uint8_t, length> const &data,
                          bool timed = false) {

    if (hid_write(controller_ptr, data.data(), length) < 0) {
      throw std::system_error(-1, std::generic_category(), 
                              "ERROR: read() returned -1!\n"
                              "Did you disconnect the controller?");
    }

    std::array<uint8_t, exchange_length> ret;
    ret.fill(0);

    bool successful = false;
    if (!timed) {
      successful = hid_read(controller_ptr, ret.data(), exchange_length) >= 0;
    } else {
      successful = hid_read_timeout(controller_ptr, ret.data(), exchange_length, 100) != 0;
    }

    if (!successful) {
      throw std::system_error(-2, std::generic_category());
    }

    return ret;
  }



  template <size_t length>
  exchange_array send_command(uint8_t command,
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
  exchange_array send_subcommand(uint8_t command, uint8_t subcommand,
                                 std::array<uint8_t, length> const &data) {
    std::array<uint8_t, length + 10> buffer{
        static_cast<uint8_t>(rumble_counter++ & 0xF),
        0x00,
        0x01,
        0x40,
        0x40,
        0x00,
        0x01,
        0x40,
        0x40,
        subcommand};
    if (length > 0) {
      memcpy(buffer.data() + 10, data.data(), length);
    }
    return send_command(command, buffer);
  }


  bool try_read_bad_data() {
    auto dat = send_command(get_input, empty);

    if (detect_useless_data(dat[0])) {
      return false;
    }

    if (detect_bad_data(dat[0], dat[1])) {
      // print_exchange_array(dat);
      return true;
    }

    return false;
  }

  /* Hackishly detects when the controller is trapped in a bad loop.
  Nothing to do here, need to reopen device :(*/
  bool detect_bad_data(const uint8_t &dat1, const uint8_t &dat2) {
    return dat2 == 0x01 && dat1 == 0x81;
  }

public:
  /* If this returns true, there is no controller information in this package,
   * we can skip it*/
  bool detect_useless_data(const uint8_t &dat) {
    /*if (dat == 0x30)
      n_bad_data_thirty++;
    if (dat == 0x00)
      n_bad_data_zero++;*/
    return (dat == 0x30 || dat == 0x00);
  }


};

#endif
