#pragma once
#ifndef HID_CONTROLLER_H
#define HID_CONTROLLER_H

#include <array>
#include <stdexcept>
#include <unistd.h>

#include "output_sender.hpp"
#include "proinputparser.hpp"

#define TEST_BAD_DATA_CYCLES 10

class HidController{
public:
  HidController(const HidApi::Enumerate &device_info, unsigned short n_controll)
                : sender(device_info), n_controller(n_controll) {
    bluetooth = sender.Bluetooth();
    sender.setBlocking();

    if (!bluetooth) {
      ProInputParser::ControllerMAC mac = sender.request_mac();
      printf("controller_type: %02x\n", mac.controller_type);
      printf("mac: %02x", mac.mac[0]);
      for (size_t i = 1; i < 6; ++i) {
        printf(":%02x", mac.mac[i]);
      }
      printf("\n");

      sender.do_handshake();
      sender.increment_baudrate();
      sender.do_handshake();

      sender.enable_hid_only_mode();
    }

    sender.toggle_rumble(true);
    sender.toggle_imu(true);

    // sender.set_imu_sensitivity(0x03, 0x00, 0x00, 0x01);

    sender.set_input_report_mode(0x30);

    #if 0
    if (bluetooth) {
      //HidApi::generic_packet<1> msg_increase_datarate_bt{{0x31}};
      //send_subcommand(SubCmd::set_in_report, msg_increase_datarate_bt);
      sender.set_input_report_mode(0x31);
      receive_input().print();
    }
    #endif

    sender.setNonBlocking();
    usleep(100 * 1000);

    led();
  }

  ~HidController(){
    try {
      close();
    }
    catch (const HidApi::HidApiError &e) {
    }

    // Wait for controller to receive the close packet.
    usleep(1000 * 1000);
  }

  ProInputParser::Parser receive_input() {
    /// TODO: do a waiting loop if a zero packet is received or something.
    HidApi::default_packet buff;
    size_t len;
    do {
      len = sender.receive_input(buff, 16);
    } while (len == 0);
    return ProInputParser::Parser(len, buff);
  }

  ProInputParser::Parser request_input() {
    HidApi::default_packet buff;
    size_t len = sender.request_input(buff);
    return ProInputParser::Parser(len, buff);
  }


  void led(int number = -1){
    uint8_t bitwise = player_led[n_controller];
    if (number >= 0) {
      bitwise = static_cast<uint8_t>(number);
    }

    sender.set_player_leds(bitwise);
    // sender.set_player_leds(bitwise);
  }

  void blink() {
    if (++blink_counter > blink_length) {
      blink_counter = 0;
      if (++blink_position >= blink_array.size()) {
        blink_position = 0;
      }
    }

    sender.set_player_leds(blink_array[blink_position]);
  }

  void send_rumble(uint8_t large_motor, uint8_t small_motor) {
    HidApi::generic_packet<4> left {0x80, 0x00, 0x40, 0x40};
    HidApi::generic_packet<4> right{0x80, 0x00, 0x40, 0x40};

    if (large_motor != 0) {
      left[1] = right[1] = 0x08;
      left[2] = right[2] = large_motor;
    } else if (small_motor != 0) {
      left[1] = right[1] = 0x10;
      left[2] = right[2] = small_motor;
    }

    sender.send_rumble(left, right);
  }

  void rumble(/*int frequency, int intensity*/) {
    HidApi::generic_packet<4> default_rumble{0x00, 0x01, 0x40, 0x40};
    sender.send_rumble(default_rumble, default_rumble);
  }

  void close() {
    if (closed) {
      return;
    }
    closed = true;
    sender.setBlocking();
    sender.toggle_rumble(false);
    sender.toggle_imu(false);

    if (!bluetooth) {
      sender.disable_hid_only_mode();
      sender.send_reset();
    }
  }

private:
  OutputSender::Sender sender;
  bool bluetooth = false;

  unsigned short n_controller;
  bool closed = false;

  const std::array<uint8_t, 8> player_led{0x01, 0x03, 0x07, 0x0f, 0x09, 0x05, 0x0d, 0x06};

  // const std::array<uint8_t, 4> blink_array{{0x05, 0x10, 0x04, 0x08}};
  const std::array<uint8_t, 4> blink_array{{0x01, 0x02, 0x04, 0x08}};

  uint blink_position = 0;
  size_t blink_counter = 0;
  const size_t blink_length = 8;
};

#endif
