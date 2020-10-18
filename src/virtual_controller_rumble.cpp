#include "virtual_controller_rumble.hpp"
using namespace VirtualController;

#include <stdexcept>

bool Rumble::init(uint16_t id, uint16_t type, uint16_t length, uint16_t delay, uint16_t strong, uint16_t weak, uint16_t direction) noexcept {
  if (enabled) return false;
  data.id        = id;
  data.type      = type;
  data.length    = length;
  data.delay     = delay;
  data.strong    = strong;
  data.weak      = weak;
  data.direction = direction;

  enabled         = true;
  time_delayed    = 0;
  remaining       = 0;
  return true;
}

void Rumble::deinit() noexcept {
  enabled = false;
}

bool Rumble::startEffect(uint16_t id, uint16_t value) {
  if (!enabled) {
    return false;
  }
  if (data.id != id) {
    throw std::invalid_argument("Rumble.startEffect(): Invalid id. " 
                                "Expected: '" + std::to_string(data.id)
                                + "'. Received: '" + 
                                std::to_string(id) + "'.");
  }

  if (value == 0) {
    time_delayed = 0;
    remaining = 0;
    return true;
  }

  if (remaining <= 0) {
    if (time_delayed < 0) time_delayed = 0;
    time_delayed += data.delay;
  }

  if (remaining < 0) remaining = 0;
  remaining += data.length;
  return true;
}

void Rumble::updateTime(long double delta_milis) noexcept {
  if (!enabled) {
    return;
  }

  if (time_delayed > delta_milis) {
    time_delayed -= delta_milis;
    return;
  }
  else {
    delta_milis -= time_delayed;
    time_delayed = 0;
  }

  if (remaining < delta_milis) {
    remaining = 0;
    return;
  }
  remaining -= delta_milis;
}

int32_t Rumble::getRemainingTime() const noexcept {
  if (!enabled) {
      return 0;
  }
  if (time_delayed > 0) {
      return 0;
  }
  return remaining;
}

bool Rumble::isEnabled() const noexcept {
  return enabled;
}

const struct RumbleData &Rumble::getData() const noexcept {
  return data;
}
