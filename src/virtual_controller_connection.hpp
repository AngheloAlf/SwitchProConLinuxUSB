#pragma once
#ifndef PRO__VIRTUAL_CONTROLLER_CONNECTION_HPP
#define PRO__VIRTUAL_CONTROLLER_CONNECTION_HPP

#include <linux/uinput.h>

#include <cstdint>
#include <vector>

namespace VirtualController {
  class ControllerConnection {
  public:
    ControllerConnection(uint16_t vendor, uint16_t product, uint16_t version, const char *name);
    ControllerConnection(const ControllerConnection &other) = delete;
    ControllerConnection(ControllerConnection &&other) noexcept;

    ~ControllerConnection() noexcept;

    ControllerConnection &operator=(const ControllerConnection &other) = delete;
    ControllerConnection &operator=(ControllerConnection &&other) noexcept;

    void destroy() noexcept ;

    void setupButtons(const std::vector<uint16_t> &ids);
    void setupAxis(const std::vector<uint16_t> &ids, const std::vector<int32_t> &mins, const std::vector<int32_t> &maxs);
    void setupForceFeedback(const std::vector<uint16_t> &ids, uint32_t max_effects);
    void createController();


    void event_axis(uint16_t which, int32_t value);
    void event_pressButton(uint16_t which);
    void event_releaseButton(uint16_t which);
    void event_sendReport();

    //void event_recvState();

  private:
    void sendEvent(uint16_t type, uint16_t code, int32_t value);

    struct uinput_setup dev;
    int fd;
    bool closed = true;
    bool created = false;
  };
};

#endif
