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
  HidController(const HidApi::Enumerate &device_info, unsigned short n_controll)
                : hid(device_info), n_controller(n_controll) {
    hid.set_blocking();

    send_uart(Uart::handshake);
    send_uart(Uart::inc_baudrate);
    send_uart(Uart::handshake);

    // the next part will sometimes fail, then need to reopen device via hidapi
    send_uart(Uart::hid_only);

    send_subcommand(SubCmd::en_rumble, enable);
    // send_subcommand(SubCmd::en_imu, enable);

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
    try {
      close();
    }
    catch (const HidApi::HidApiError &e) {
    }

    // Wait for controller to receive the close packet.
    usleep(1000 * 1000);
  }

  ProInputParser request_input() {
    return send_command(Cmd::get_input, empty);
  }

  void led(int number = -1){
    HidApi::generic_packet<1> value {
      number < 0 ?
      player_led[n_controller] : 
      static_cast<uint8_t>(number)};
    
    send_subcommand(SubCmd::set_leds, value);
  }

  void blink() {
    if (++blink_counter > blink_length) {
      blink_counter = 0;
      if (++blink_position >= blink_array.size()) {
        blink_position = 0;
      }
    }
    HidApi::generic_packet<1> blink_command{{blink_array[blink_position]}};
    send_subcommand(SubCmd::set_leds, blink_command);
  }

  ProInputParser send_rumble(uint8_t large_motor, uint8_t small_motor) {
    HidApi::generic_packet<9> buf{
      static_cast<uint8_t>(rumble_counter++ & 0xF),
      0x80, 0x00, 0x40, 0x40, 0x80, 0x00, 0x40, 0x40};

    if (large_motor != 0) {
      buf[1] = buf[5] = 0x08;
      buf[2] = buf[6] = large_motor;
    } else if (small_motor != 0) {
      buf[1] = buf[5] = 0x10;
      buf[2] = buf[6] = small_motor;
    }
    ProInputParser ret = send_command(Cmd::rumble_only, buf);
    ret.print();
    return ret;
  }

  ProInputParser rumble(/*int frequency, int intensity*/) {
    HidApi::generic_packet<8> buf;

    buf[0] = buf[0+4] = 0x00;
    buf[1] = buf[1+4] = 0x01;
    buf[2] = buf[2+4] = 0x40;
    buf[3] = buf[3+4] = 0x40;

    ProInputParser ret = send_command(Cmd::rumble_only, buf);
    //ret.print();
    return ret;
  }

  void close() {
    send_uart(Uart::turn_off_hid);
  }

private:
  enum Protocols {
    zero_one      = 0x01,
    one_zero      = 0x10,
    nintendo      = 0x80,
  };

  enum Uart {
    status        = 0x01,
    handshake     = 0x02,
    inc_baudrate  = 0x03,
    hid_only      = 0x04,
    turn_off_hid  = 0x05,
    //reset         = 0x06,
    //prehand_cmd   = 0x91,
    uart_cmd      = 0x92,
  };

  enum Cmd {
    sub_command   = 0x01,
    rumble_only   = 0x10,
    //nfc_ir_req    = 0x11,
    get_input     = 0x1f, // ?
  };

  enum SubCmd {
    //req_dev_info  = 0x02,
    //set_in_report = 0x03, /// Set input report mode
    set_leds      = 0x30,
    get_leds      = 0x31,
    //set_home_led  = 0x38,
    en_imu        = 0x40,
    en_rumble     = 0x48,
    //get_voltage   = 0x50, /// Get regullated voltage. Useful to know battery status
  };

  HidApi::default_packet send_uart(Uart uart){
    HidApi::generic_packet<0x02> packet {Protocols::nintendo, uart};
    return hid.exchange(packet);
  }

  template <size_t length>
  HidApi::default_packet 
  send_uart(const HidApi::generic_packet<length> &data){
    HidApi::generic_packet<length + 0x08> packet;
    packet.fill(0);
    packet[0x00] = Protocols::nintendo;
    packet[0x01] = Uart::uart_cmd;
    packet[0x02] = 0x00; // length?
    packet[0x03] = 0x31; // length?
    if (length > 0) {
      memcpy(packet.data() + 0x8, data.data(), length);
    }
    return hid.exchange(packet);
  }

  template <size_t length>
  ProInputParser send_command(Cmd command,
                              HidApi::generic_packet<length> const &data) {
    HidApi::generic_packet<length + 0x01> buffer;
    buffer.fill(0);
    buffer[0x00] = command;
    if (length > 0) {
      memcpy(buffer.data() + 0x01, data.data(), length);
    }
    return ProInputParser(send_uart(buffer));
  }

  template <size_t length>
  ProInputParser send_subcommand(SubCmd subcommand,
                                 HidApi::generic_packet<length> const &data) {
    HidApi::generic_packet<length + 10> buffer{
      static_cast<uint8_t>(rumble_counter++ & 0xF),
      0x00, 0x01, 0x40, 0x40, 0x00, 0x01, 0x40, 0x40,
      subcommand};

    if (length > 0) {
      memcpy(buffer.data() + 10, data.data(), length);
    }
    return send_command(Cmd::sub_command, buffer);
  }

  bool try_read_bad_data() {
    ProInputParser dat = request_input();

    if (dat.detect_useless_data()) {
      return false;
    }
    if (dat.detect_bad_data()) {
      // print_exchange_array(dat);
      return true;
    }

    return false;
  }

  HidApi::Device hid;

  unsigned short n_controller;

  uint8_t rumble_counter{0};
  const std::array<uint8_t, 8> player_led{0x01, 0x03, 0x07, 0x0f, 0x09, 0x05, 0x0d, 0x06};

  const HidApi::generic_packet<0> empty{{}};
  const HidApi::generic_packet<1> enable{{0x01}};

  // const std::array<uint8_t, 4> blink_array{{0x05, 0x10, 0x04, 0x08}};
  const std::array<uint8_t, 4> blink_array{{0x01, 0x02, 0x04, 0x08}};

  uint blink_position = 0;
  size_t blink_counter = 0;
  const size_t blink_length = 8;
};

#endif
