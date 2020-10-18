#include "virtual_controller_connection.hpp"
using namespace VirtualController;

#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <system_error>

ControllerConnection::ControllerConnection(uint16_t vendor, uint16_t product, uint16_t version, const char *name) {
  closed = false;
  fd = open("/dev/uinput", O_RDWR | O_NONBLOCK);
  if (fd < 0) {
    throw std::system_error(errno, std::generic_category(), "Failed to open uinput device.");
  }

  memset(&dev, 0, sizeof(dev));
  dev.id.bustype = BUS_USB;
  dev.id.vendor  = vendor;
  dev.id.product = product;
  dev.id.version = version;
  strncpy(dev.name, name, UINPUT_MAX_NAME_SIZE);
}
ControllerConnection::~ControllerConnection() noexcept {
  if (closed) {
    return;
  }
  destroy();
}

void ControllerConnection::destroy() noexcept {
  if (!closed) {
    if (created) {
      ioctl(fd, UI_DEV_DESTROY);
    }
    close(fd);
  }
}

void ControllerConnection::setupButtons(const std::vector<uint16_t> &ids) {
  if (closed) {
    return;
  }
  if (created) {
    throw std::runtime_error("ControllerConnection.setupButtons(): Already created.");
  }

  if (ids.size() == 0) {
    return;
  }

  ioctl(fd, UI_SET_EVBIT, EV_KEY);
  for (const uint16_t &id: ids) {
    ioctl(fd, UI_SET_KEYBIT, id);
  }
}

void ControllerConnection::setupAxis(const std::vector<uint16_t> &ids, const std::vector<int32_t> &mins, const std::vector<int32_t> &maxs) {
  if (created) {
    throw std::runtime_error("ControllerConnection.setupAxis(): Already created.");
  }

  if (ids.size() == 0) {
    return;
  }

  ioctl(fd, UI_SET_EVBIT, EV_ABS);
  for (size_t i = 0; i < ids.size(); ++i) {
    struct uinput_abs_setup abs {
      .code = ids[i],
      .absinfo = {
        .value      = (mins[i] + maxs[i])/2,
        .minimum    = mins[i],
        .maximum    = maxs[i],
        .fuzz       = 0,
        .flat       = 0,
        .resolution = 0,
      }
    };
    ioctl(fd, UI_ABS_SETUP, &abs);
  }
}

void ControllerConnection::setupForceFeedback(const std::vector<uint16_t> &ids, uint32_t max_effects) {
  if (created) {
    throw std::runtime_error("ControllerConnection.setupForceFeedback(): Already created.");
  }

  if (ids.size() == 0) {
    return;
  }

  ioctl(fd, UI_SET_EVBIT, EV_FF);
  for (const uint16_t &id: ids) {
    ioctl(fd, UI_SET_FFBIT, id);
  }

  dev.ff_effects_max = max_effects;
}

void ControllerConnection::createController() {
  if (created) {
    throw std::runtime_error("ControllerConnection.createController(): Already created.");
  }

  if (ioctl(fd, UI_DEV_SETUP, &dev) < 0) {
    close(fd);
    throw std::system_error(errno, std::generic_category(), "Failed to setup controller.");
    exit(EXIT_FAILURE);
  }
  if (ioctl(fd, UI_DEV_CREATE) < 0) {
    close(fd);
    throw std::system_error(errno, std::generic_category(), "Failed to create uinput device.");
  }
  created = true;
}




void ControllerConnection::event_axis(uint16_t which, int32_t value) {
  sendEvent(EV_ABS, which, value);
}

void ControllerConnection::event_pressButton(uint16_t which) {
  sendEvent(EV_KEY, which, 1);
}

void ControllerConnection::event_releaseButton(uint16_t which) {
  sendEvent(EV_KEY, which, 1);
}

void ControllerConnection::event_sendReport() {
  sendEvent(EV_SYN, SYN_REPORT, 0);
}

/*
void ControllerConnection::event_recvState() {

}
*/



void ControllerConnection::sendEvent(uint16_t type, uint16_t code, int32_t value) {
  struct input_event event;
  memset(&event, 0, sizeof(event));

  gettimeofday(&event.time, NULL);
  event.type = type;
  event.code = code;
  event.value = value;

  int ret = write(fd, &event, sizeof(event));
  if (ret < 0) {
    throw std::system_error(errno, std::generic_category(), "VirtualController: write error.");
  }
}


