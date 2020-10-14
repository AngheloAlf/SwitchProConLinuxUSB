#pragma once
#ifndef PRO__VIRTUAL_CONTROLLER_HPP
#define PRO__VIRTUAL_CONTROLLER_HPP

#include <array>
#include <cstdint>
#include <linux/uinput.h>
#include "rumbledata.hpp"

namespace VirtualController {
  class Controller {
  public:
    static constexpr uint16_t max_effects{2};

    Controller() ;
    Controller(const Controller &other) = delete;
    Controller(Controller &&other) noexcept;

    ~Controller() noexcept ;

    Controller &operator=(const Controller &other) = delete;
    Controller &operator=(Controller &&other) noexcept;

    void write_single_joystick(int val, int cod);

    void button_press(int cod);

    void button_release(int cod);

    void send_report();

    void update_state();

    void update_time(long double delta_milis);

    std::array<const RumbleData *, max_effects> getRumbleEffects();

  private:
    void send_packet(unsigned short type, unsigned short code, int value);

    int get_packet(struct input_event &uinput_event);

    void handle_EV_UINPUT(const struct input_event &uinput_event);

    void handle_EV_FF(const struct input_event &uinput_event);

    int uinput_version, uinput_rc, uinput_fd;
    std::array<RumbleData, max_effects> rumble_effects;
    bool closed = true;
  };
};

#endif
