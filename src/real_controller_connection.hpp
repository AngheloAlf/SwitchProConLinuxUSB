#pragma once
#ifndef PRO__REAL_CONTROLLER_CONNECTION_HPP
#define PRO__REAL_CONTROLLER_CONNECTION_HPP

#include <cstring>
#include "hidapi_wrapper.hpp"
#include "real_controller_parser.hpp"
#include "real_controller_packets.hpp"

namespace RealController {
  const HidApi::GenericPacket<0> empty  {{}};

  const HidApi::GenericPacket<1> disable{{0x00}};
  const HidApi::GenericPacket<1> enable {{0x01}};

  const HidApi::GenericPacket<4> no_rumble{0x00, 0x01, 0x40, 0x40};

  class ControllerConnection {
  public:
    ControllerConnection(const HidApi::Enumerate &device_info);
    ControllerConnection(const ControllerConnection &other) = delete;
    ControllerConnection(ControllerConnection &&other) noexcept;

    ~ControllerConnection() noexcept;

    ControllerConnection &operator=(const ControllerConnection &other) = delete;
    ControllerConnection &operator=(ControllerConnection &&other) noexcept;

    bool Bluetooth() const;
    bool Usb() const;

    void setBlocking();
    void setNonBlocking();

    RealController::ControllerMAC request_mac(int milliseconds=100);
    void do_handshake();
    void increment_baudrate();
    void enable_hid_only_mode();
    void disable_hid_only_mode();
    void send_reset();


    template <size_t length>
    size_t receive_input(HidApi::GenericPacket<length> &buffer, int milliseconds=-1) {
      // send_subcommand(SubCmd::zero, empty, no_rumble, no_rumble);
      return hidw.read(buffer, milliseconds);
    }

    template <size_t length>
    size_t request_input(HidApi::GenericPacket<length> &buffer) {
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


    void send_rumble(const HidApi::GenericPacket<4> &left_rumble, 
                     const HidApi::GenericPacket<4> &right_rumble);


  private:
    size_t send_uart(Uart uart);

    template <size_t length>
    size_t send_uart(const HidApi::GenericPacket<length> &data){
      HidApi::GenericPacket<length + 8> packet;
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
                        HidApi::GenericPacket<length> const &data) {
      HidApi::GenericPacket<length + 1> buffer;
      buffer[0] = command;
      if (length > 0) {
        memcpy(buffer.data() + 1, data.data(), length);
      }

      // ProInputParser::printPacket(length + 1, buffer.data());
      if (bluetooth) {
        return hidw.write(buffer);
      }
      return send_uart(buffer);
    }

    template <size_t length>
    size_t send_command_with_rumble_data(Cmd command,
                           const HidApi::GenericPacket<length> &data, 
                           const HidApi::GenericPacket<4> &left_rumble, 
                           const HidApi::GenericPacket<4> &right_rumble) {
      HidApi::GenericPacket<length + 9> buffer;
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
                           const HidApi::GenericPacket<length> &data, 
                           const HidApi::GenericPacket<4> &left_rumble, 
                           const HidApi::GenericPacket<4> &right_rumble) {
      HidApi::GenericPacket<length + 1> buffer;
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
