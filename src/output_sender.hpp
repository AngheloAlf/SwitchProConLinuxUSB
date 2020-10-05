#pragma once
#ifndef OUTPUT_PARSER_HPP
#define OUTPUT_PARSER_HPP

#include <cstring>
#include "hidapi_wrapper.hpp"
#include "proinputparser.hpp"


namespace OutputSender {
  enum Protocols {
    zero_one      = 0x01,
    one_zero      = 0x10,
    nintendo      = 0x80,
    nin_response  = 0x81,
  };

  enum Uart {
    status        = 0x01,
    handshake     = 0x02,
    inc_baudrate  = 0x03,
    hid_only      = 0x04,
    turn_off_hid  = 0x05,
    reset         = 0x06,
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
    zero          = 0x00, // ??
    //req_dev_info  = 0x02,
    set_in_report = 0x03, /// Set input report mode
    set_leds      = 0x30,
    get_leds      = 0x31,
    //set_home_led  = 0x38,
    en_imu        = 0x40,
    set_imu       = 0x41,
    en_rumble     = 0x48,
    //get_voltage   = 0x50, /// Get regullated voltage.
  };

  const HidApi::generic_packet<0> empty{{}};
  const HidApi::generic_packet<1> disable{{0x00}};
  const HidApi::generic_packet<1> enable {{0x01}};

  const HidApi::generic_packet<4> no_rumble{0x00, 0x01, 0x40, 0x40};

  class Sender {
  public:
    Sender(const HidApi::Enumerate &device_info);

    // const HidApi::Device &Hid() const;

    bool Bluetooth() const;
    bool Usb() const;

    void setBlocking();
    void setNonBlocking();

    ProInputParser::ControllerMAC request_mac(int milliseconds=100);
    void do_handshake();
    void increment_baudrate();
    void enable_hid_only_mode();
    void disable_hid_only_mode();
    void send_reset();


    template <size_t length>
    size_t receive_input(HidApi::generic_packet<length> &buffer, int milliseconds=-1) {
      // send_subcommand(SubCmd::zero, empty, no_rumble, no_rumble);
      return hidw.read(buffer, milliseconds);
    }

    template <size_t length>
    size_t request_input(HidApi::generic_packet<length> &buffer) {
      bool was_blocking = hidw.IsBlocking();
      setBlocking();

      send_command(Cmd::get_input, empty);
      size_t len = hidw.read(buffer);

      if (!was_blocking) {
        hidw.set_non_blocking();
      }

      return len;
    }


    void set_player_leds(uint8_t bitwise);
    //void get_player_leds();

    void set_input_report_mode(uint8_t mode);

    void toggle_imu(bool en);
    void set_imu_sensitivity(uint8_t arg1, uint8_t arg2, uint8_t arg3, uint8_t arg4);

    void toggle_rumble(bool en);


    void send_rumble(const HidApi::generic_packet<4> &left_rumble, 
                     const HidApi::generic_packet<4> &right_rumble);


  private:
    size_t send_uart(Uart uart);

    template <size_t length>
    size_t send_uart(const HidApi::generic_packet<length> &data){
      HidApi::generic_packet<length + 8> packet;
      packet.fill(0);
      packet[0] = Protocols::nintendo;
      packet[1] = Uart::uart_cmd;
      packet[2] = 0x00; // length?
      packet[3] = 0x31; // length?
      if (length > 0) {
        memcpy(packet.data() + 8, data.data(), length);
      }
      return hidw.write(packet);
    }

    template <size_t length>
    size_t send_command(Cmd command,
                        HidApi::generic_packet<length> const &data) {
      HidApi::generic_packet<length + 1> buffer;
      buffer[0] = command;
      if (length > 0) {
        memcpy(buffer.data() + 1, data.data(), length);
      }

      // ProInputParser::print_exchange_array(length + 1, buffer.data());
      if (bluetooth) {
        return hidw.write(buffer);
      }
      return send_uart(buffer);
    }

    template <size_t length>
    size_t send_command_with_rumble_data(Cmd command,
                           const HidApi::generic_packet<length> &data, 
                           const HidApi::generic_packet<4> &left_rumble, 
                           const HidApi::generic_packet<4> &right_rumble) {
      HidApi::generic_packet<length + 9> buffer;
      buffer[0] = timing_counter = (timing_counter + 1) & 0x0F;
      for(size_t i = 0; i < 4; ++i){
        buffer[1+i] =  left_rumble[i];
        buffer[5+i] = right_rumble[i];
      }

      if (length > 0) {
        memcpy(buffer.data() + 9, data.data(), length);
      }
      return send_command(command, buffer);
    }

    template <size_t length>
    size_t send_subcommand(SubCmd subcommand,
                           const HidApi::generic_packet<length> &data, 
                           const HidApi::generic_packet<4> &left_rumble, 
                           const HidApi::generic_packet<4> &right_rumble) {
      HidApi::generic_packet<length + 1> buffer;
      buffer[0] = subcommand;
      if (length > 0) {
        memcpy(buffer.data() + 1, data.data(), length);
      }
      return send_command_with_rumble_data(Cmd::sub_command, buffer, left_rumble, right_rumble);
    }

    HidApi::Device hidw;
    uint8_t timing_counter = 0x0F;
    bool bluetooth = false;
  };
};

#endif
