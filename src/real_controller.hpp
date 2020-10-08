#pragma once
#ifndef PRO__REAL_CONTROLLER_HPP
#define PRO__REAL_CONTROLLER_HPP

#include <array>
#include "real_controller_connection.hpp"
#include "real_controller_parser.hpp"

namespace RealController {
  class Controller {
  public:
    Controller(const HidApi::Enumerate &device_info, unsigned short n_controll);
    Controller(const Controller &other) = delete;
    Controller(Controller &&other) noexcept;

    ~Controller() noexcept;

    Controller &operator=(const Controller &other) = delete;
    Controller &operator=(Controller &&other) noexcept;

    RealController::Parser receive_input();
    RealController::Parser request_input();

    void led(int number = -1);
    void blink();

    void rumble(double high_freq, double low_freq, double amplitude);

    void close();

  private:
    RealController::ControllerConnection connection;
    unsigned short n_controller;

    uint blink_position = 0;
    size_t blink_counter = 0;
    const size_t blink_length = 8;

    bool closed = true;

    const std::array<uint8_t, 8> player_led{0x01, 0x03, 0x07, 0x0f, 0x09, 0x05, 0x0d, 0x06};

    // const std::array<uint8_t, 4> blink_array{{0x05, 0x10, 0x04, 0x08}};
    const std::array<uint8_t, 4> blink_array{{0x01, 0x02, 0x04, 0x08}};
  };
};

#endif
