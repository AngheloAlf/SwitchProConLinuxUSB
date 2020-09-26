#pragma once
#ifndef RUMBLE_DATA_H
#define RUMBLE_DATA_H

#include <cstdint>
#include <string>
#include <stdexcept>


class RumbleData{
public:
    struct rumble_data{
        uint16_t id=0, type=0;
        uint16_t length=0, delay=0;
        uint16_t strong=0, weak=0;
        uint16_t direction=0;
    };

    bool init(uint16_t id, uint16_t type, uint16_t length, uint16_t delay, uint16_t strong=0, uint16_t weak=0, uint16_t direction=0) {
        if (enabled) return false;
        data.id        = id;
        data.type      = type;
        data.length    = length;
        data.delay     = delay;
        data.strong    = strong;
        data.weak      = weak;
        data.direction = direction;

        enabled         = true;
        remaining       = 0;
        return true;
    }

    void deinit() {
        enabled = false;
    }

    bool start_effect(uint16_t id, uint16_t value) {
        if (!enabled) {
            return false;
        }
        if (data.id != id) {
            throw std::invalid_argument("RumbleData.start_effect(): Invalid id. " 
                                        "Expected: '" + std::to_string(data.id)
                                        + "'. Received: '" + 
                                        std::to_string(id) + "'.");
        }

        if (value == 0) {
            remaining = 0;
            return true;
        }

        if (remaining < 0) remaining = 0;
        remaining += data.length;
        return true;
    }

    void update_time(long double delta_milis) {
        if (!enabled) {
            return;
        }
        if (remaining < delta_milis) {
            remaining = 0;
            return;
        }
        remaining -= delta_milis;
    }

    int32_t get_remaining() const {
        if (!enabled) {
            return 0;
        }
        return remaining;
    }

    bool is_enabled() const {
        return enabled;
    }

    const struct rumble_data &get_data() const {
        return data;
    }

private:
    struct rumble_data data;

    bool enabled=false;
    int32_t remaining = 0;
};

#endif
