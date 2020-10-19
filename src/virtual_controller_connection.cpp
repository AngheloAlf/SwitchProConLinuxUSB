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

void ControllerConnection::setupForceFeedback(const std::vector<uint16_t> &ids, uint16_t max_effects) {
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
  effects_amount = max_effects;
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


void ControllerConnection::event_recvState(std::vector<Rumble> &effects) {
  struct input_event event;
  ssize_t ret = recvEvent(event);
  while (ret > 0) {
    switch (event.type) {
    case EV_UINPUT:
      handle_EV_UINPUT(event, effects);
      break;

    case EV_FF:
      handle_EV_FF(event, effects);
      break;

    default:
      printf("Unkonwn type: %x %x %i\n", event.type, event.code, event.value);
      break;
    }
    ret = recvEvent(event);
  }
}



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

ssize_t ControllerConnection::recvEvent(struct input_event &event) {
  memset(&event, 0, sizeof(event));

  ssize_t ret = read(fd, &event, sizeof(event));
  /*if (ret == 0) {
    throw std::runtime_error("ERROR: read on virtual controller returned"
                                + std::to_string(ret) + "\n"
                                "Maybe try running with sudo\n"
                                + strerror(errno) + "\n");
  }*/
  return ret;
}

void ControllerConnection::handle_EV_UINPUT(const struct input_event &event, std::vector<Rumble> &effects) {
  struct uinput_ff_upload upload;
  struct uinput_ff_erase erase;
  switch (event.code) {
  case UI_FF_UPLOAD:
    memset(&upload, 0, sizeof(struct uinput_ff_upload));
    upload.request_id = event.value;

    ioctl(fd, UI_BEGIN_FF_UPLOAD, &upload);

    //printf("(UI_FF_UPLOAD): %i\n", event.value);
    //printf("%x %x\n", upload.request_id, upload.retval);
    //printf("\t%x %x %x\n", upload.effect.type, upload.effect.id, upload.effect.direction);
    //printf("\ttrigger: %x %x\n", upload.effect.trigger.button, upload.effect.trigger.interval);
    //printf("\treplay: %i %i\n", upload.effect.replay.length, upload.effect.replay.delay);
    if (upload.effect.type == FF_RUMBLE) {
      //printf("\tmagnitude: %x %x\n", upload.effect.u.rumble.strong_magnitude, upload.effect.u.rumble.weak_magnitude);
    }
    //printf("\n");
    upload.retval = 0; // -1 on error
    //upload.effect.id = 0;

    if (upload.effect.id < effects_amount) {
      struct ff_effect *eff = &upload.effect;
      if (!effects.at(eff->id).init(eff->id, eff->type, eff->replay.length, eff->replay.delay, eff->u.rumble.strong_magnitude, eff->u.rumble.weak_magnitude)) {
        upload.retval = -1;
      }
    }
    else {
      upload.retval = -1;
    }

    ioctl(fd, UI_END_FF_UPLOAD, &upload);
    break;

  case UI_FF_ERASE:
    memset(&erase, 0, sizeof(struct uinput_ff_erase));
    erase.request_id = event.value;

    ioctl(fd, UI_BEGIN_FF_ERASE, &erase);

    //printf("(UI_FF_ERASE): %i\n", event.value);
    //printf("%x %x %x\n", erase.request_id, erase.retval, erase.effect_id);
    //printf("\n");

    if (erase.effect_id < effects_amount) {
      effects.at(erase.effect_id).deinit();
      erase.retval = 0;
    }
    else {
      erase.retval = -1;
    }

    ioctl(fd, UI_END_FF_ERASE, &erase);
    break;

  default:
    printf("(EV_UINPUT) Unkonwn code: %x %i\n", event.code, event.value);
    break;
  }
}

void ControllerConnection::handle_EV_FF(const struct input_event &event, std::vector<Rumble> &effects) {
  //printf("(EV_FF) code: %04x - value: %x\n", event.code, event.value);
  if (event.code < effects_amount) {
    effects.at(event.code).startEffect(event.code, event.value);
  }
}


