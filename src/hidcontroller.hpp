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

  //bool is_opened = false;


  uint8_t rumble_counter{0};
  const std::array<uint8_t, 1> led_calibration{{0xff}};
  const std::array<uint8_t, 1> led_calibrated{{0x01}};
  const std::array<uint8_t, 0> empty{{}};
  const std::array<uint8_t, 2> handshake{{0x80, 0x02}};
  const std::array<uint8_t, 2> switch_baudrate{{0x80, 0x03}};
  const std::array<uint8_t, 2> hid_only_mode{{0x80, 0x04}};
  // const std::array<uint8_t, 4> blink_array{{0x05, 0x10}};//, 0x04, 0x08}};



  static constexpr uint8_t led_command{0x30};
  static constexpr uint8_t get_input{0x1f};
  static constexpr uint8_t center{0x7e};
  static constexpr size_t exchange_length{0x400};
  using exchange_array = std::array<uint8_t, exchange_length>;

public:
  HidController(unsigned short vendor_id, unsigned short product_id,
                const wchar_t *serial_number, unsigned short n_controll) {
    controller_ptr = hid_open(vendor_id, product_id, serial_number);
    // controller_ptr = hid_open_path("/dev/input/hidraw0");
    //is_opened = true;


    //printf("SERIAL NUMBER: %u\n", serial_number);
    if (!controller_ptr) {
      throw std::ios_base::failure("Invalid device pointer.");
      //return -1;
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
    //int read_failed;
    exchange(hid_only_mode, true/*, &read_failed*/);
    /*if (read_failed < 0) {
      throw std::system_error(read_failed, std::generic_category());
      //return -2;
    }*/

    /// TODO:
    ///send_subcommand(0x1, led_command, led_calibration);

    usleep(100 * 1000);



    // TEST FOR BAD DATA
    for (size_t i = 0; i < TEST_BAD_DATA_CYCLES; ++i) {
      if (try_read_bad_data()) {
        throw std::runtime_error("Detected bad data stream. Trying again...");
        //PrintColor::magenta();
        //printf("Detected bad data stream. Trying again...\n");
        //PrintColor::normal();
        //controller.close_device();
        //bad_data = true;
        //usleep(1000 * 10);
        //break;
      }
    }
  }

  ~HidController(){
    /*
    if (!is_opened)
      return;
    is_opened = false;
    */
    if (controller_ptr) {
      hid_close(controller_ptr);
      //PrintColor::blue();
      // printf("Closed controller nr. %u\n", n_controller);
      //PrintColor::normal();
    }
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


  void set_non_blocking() {
    if (hid_set_nonblocking(controller_ptr, 1) < 0) {
      throw std::runtime_error("Couldn't set controller " + std::to_string(n_controller) + " to non-blocking.");
      /// TODO: change to exception
      /*printf("%sERROR: Couldn't set controller %u to non-blocking%s\n", KRED,
             n_controller, KNRM);*/
    }
  }

  void set_blocking() {
    if (hid_set_nonblocking(controller_ptr, 0) < 0) {
      throw std::runtime_error("Couldn't set controller " + std::to_string(n_controller) + " to blocking.");
      /// TODO: change to exception
      /*printf("%sERROR: Couldn't set controller %u to blocking%s\n", KRED,
             n_controller, KNRM);*/
    }
  }


private:

  template <size_t length>
  exchange_array exchange(std::array<uint8_t, length> const &data,
                          bool timed = false/*, int *status = nullptr*/) {

    /// Constructor already handles this.
    /*
    if (!controller_ptr) {
      red();
      printf("ERROR: controller_ptr is nullptr!\n");
      normal();
      return {};
    }
    */

    if (hid_write(controller_ptr, data.data(), length) < 0) {
      throw std::system_error(-1, "ERROR: read() returned -1!\n"
                                  "Did you disconnect the controller?");
      /*red();
      printf(
          "ERROR: read() returned -1!\nDid you disconnect the controller?\n");
      normal();
      throw - 1;
      return {};*/
    }

    std::array<uint8_t, exchange_length> ret;
    ret.fill(0);
    if (!timed)
      hid_read(controller_ptr, ret.data(), exchange_length); /// TODO: check return value
    else {

      if (hid_read_timeout(controller_ptr, ret.data(), exchange_length, 100) ==
          0) {
        // failed to read!
        //if (status) {
          throw std::system_error(-2, std::generic_category());
          //*status = -1;
          //return {};
        //}
      }
    }
    /*if (status) {
      *status = 0;
    }*/
    return ret;
  }



  bool try_read_bad_data() {

    /*
    if (!controller_ptr) {
      printf("%sERROR: Controller pointer is nullptr%s\n", KRED, KNRM);
      return -1;
    }
    */

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
