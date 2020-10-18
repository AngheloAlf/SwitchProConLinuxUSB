#pragma once
#ifndef PRO__VIRTUAL_CONTROLLER_RUMBLE_HPP
#define PRO__VIRTUAL_CONTROLLER_RUMBLE_HPP

#include <cstdint>


namespace VirtualController {
  struct RumbleData{
    uint16_t id=0, type=0;
    uint16_t length=0, delay=0;
    uint16_t strong=0, weak=0;
    uint16_t direction=0;
  };

  class Rumble {
  public:
    bool init(uint16_t id, uint16_t type, uint16_t length, uint16_t delay, uint16_t strong=0, uint16_t weak=0, uint16_t direction=0) noexcept;
    void deinit() noexcept;

    bool startEffect(uint16_t id, uint16_t value);

    void updateTime(long double delta_milis) noexcept;

    int32_t getRemainingTime() const noexcept;
    bool isEnabled() const noexcept;
    const struct RumbleData &getData() const noexcept;

  private:
    struct RumbleData data;

    bool enabled = false;
    int32_t time_delayed = 0;
    int32_t remaining = 0;
  };
};

#endif
