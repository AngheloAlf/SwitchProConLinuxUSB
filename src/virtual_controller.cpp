#include "virtual_controller.hpp"
using namespace VirtualController;

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <system_error>


Controller::Controller() {
  closed = false;
  uinput_fd = open("/dev/uinput", O_RDWR | O_NONBLOCK);
  if (uinput_fd < 0) {
    throw std::system_error(errno, std::generic_category(), "Failed to open uinput device!");
  }
  uinput_rc = ioctl(uinput_fd, UI_GET_VERSION, &uinput_version);

  struct uinput_user_dev uinput_device;
  memset(&uinput_device, 0, sizeof(uinput_device));

  uinput_device.id.bustype = BUS_USB;
  uinput_device.id.vendor = 0x045e;  // Microsoft
  uinput_device.id.product = 0x028e; // XBOX 360
  uinput_device.id.version = 0x110;  // dunno but xboxdrv uses this
  strncpy(uinput_device.name, "Switch ProController disguised as XBox360",
          UINPUT_MAX_NAME_SIZE);

  // buttons
  ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);

  ioctl(uinput_fd, UI_SET_KEYBIT, BTN_EAST);
  ioctl(uinput_fd, UI_SET_KEYBIT, BTN_SOUTH);
  ioctl(uinput_fd, UI_SET_KEYBIT, BTN_NORTH);
  ioctl(uinput_fd, UI_SET_KEYBIT, BTN_WEST);
  ioctl(uinput_fd, UI_SET_KEYBIT, BTN_MODE);
  ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TL);
  ioctl(uinput_fd, UI_SET_KEYBIT, BTN_TR);
  ioctl(uinput_fd, UI_SET_KEYBIT, BTN_THUMBL);
  ioctl(uinput_fd, UI_SET_KEYBIT, BTN_THUMBR);
  ioctl(uinput_fd, UI_SET_KEYBIT, BTN_START);
  ioctl(uinput_fd, UI_SET_KEYBIT, BTN_SELECT);

  // sticks
  ioctl(uinput_fd, UI_SET_EVBIT, EV_ABS);

  ioctl(uinput_fd, UI_SET_ABSBIT, ABS_X);
  ioctl(uinput_fd, UI_SET_ABSBIT, ABS_Y);
  ioctl(uinput_fd, UI_SET_ABSBIT, ABS_RX);
  ioctl(uinput_fd, UI_SET_ABSBIT, ABS_RY);
  ioctl(uinput_fd, UI_SET_ABSBIT, ABS_Z);  // L2
  ioctl(uinput_fd, UI_SET_ABSBIT, ABS_RZ); // R2
  ioctl(uinput_fd, UI_SET_ABSBIT, ABS_HAT0X);
  ioctl(uinput_fd, UI_SET_ABSBIT, ABS_HAT0Y);

  uinput_device.absmin[ABS_X] = 0;
  uinput_device.absmax[ABS_X] = 0xFFF;
  uinput_device.absmin[ABS_Y] = 0;
  uinput_device.absmax[ABS_Y] = 0xFFF;
  uinput_device.absmin[ABS_RX] = 0;
  uinput_device.absmax[ABS_RX] = 0xFFF;
  uinput_device.absmin[ABS_RY] = 0;
  uinput_device.absmax[ABS_RY] = 0xFFF;
  uinput_device.absmin[ABS_Z] = 0;
  uinput_device.absmax[ABS_Z] = 0xFFF;
  uinput_device.absmin[ABS_RZ] = 0;
  uinput_device.absmax[ABS_RZ] = 0xFFF;
  uinput_device.absmin[ABS_HAT0X] = -1;
  uinput_device.absmax[ABS_HAT0X] =  1;
  uinput_device.absmin[ABS_HAT0Y] = -1;
  uinput_device.absmax[ABS_HAT0Y] =  1;

  // rumble
  ioctl(uinput_fd, UI_SET_EVBIT, EV_FF);

  ioctl(uinput_fd, UI_SET_FFBIT, FF_RUMBLE);
  //ioctl(uinput_fd, UI_SET_FFBIT, FF_PERIODIC);
  //ioctl(uinput_fd, UI_SET_FFBIT, FF_SQUARE);
  //ioctl(uinput_fd, UI_SET_FFBIT, FF_TRIANGLE);
  //ioctl(uinput_fd, UI_SET_FFBIT, FF_SINE);
  //ioctl(uinput_fd, UI_SET_FFBIT, FF_GAIN);

  uinput_device.ff_effects_max = max_effects;

  if (write(uinput_fd, &uinput_device, sizeof(uinput_device)) < 0) {
    close(uinput_fd);
    throw std::system_error(errno, std::generic_category(), "Failed to set axis and rumble data!");
  }

  if (ioctl(uinput_fd, UI_DEV_CREATE) < 0) {
    close(uinput_fd);
    throw std::system_error(errno, std::generic_category(), "Failed to create uinput device!");
  }
}

Controller::Controller(Controller &&other) noexcept:
  uinput_version(std::move(other.uinput_version)), uinput_rc(std::move(other.uinput_rc)),
  uinput_fd(std::move(other.uinput_fd)), rumble_effects(std::move(other.rumble_effects)),
  closed(std::move(other.closed)) {
}

Controller::~Controller() noexcept {
  if (!closed) {
    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);
  }
}

Controller &Controller::operator=(Controller &&other) noexcept {
  std::swap(uinput_version, other.uinput_version);
  std::swap(uinput_rc, other.uinput_rc);
  std::swap(uinput_fd, other.uinput_fd);
  std::swap(rumble_effects, other.rumble_effects);
  std::swap(closed, other.closed);
  return *this;
}

void Controller::write_single_joystick(int val, int cod) {
  send_packet(EV_ABS, cod, (int)val);
}

void Controller::button_press(int cod) {
  send_packet(EV_KEY, cod, 1);
}

void Controller::button_release(int cod) {
  send_packet(EV_KEY, cod, 0);
}

void Controller::send_report() {
  send_packet(EV_SYN, SYN_REPORT, 0);
}

void Controller::update_state() {
  struct input_event uinput_event;
  int ret = get_packet(uinput_event);
  while (ret > 0) {
    switch (uinput_event.type) {
    case EV_UINPUT:
      handle_EV_UINPUT(uinput_event);
      break;

    case EV_FF:
      handle_EV_FF(uinput_event);
      break;

    default:
      printf("Unkonwn type: %x %x %i\n", uinput_event.type, uinput_event.code, uinput_event.value);
      break;
    }
    ret = get_packet(uinput_event);
  }
}

void Controller::update_time(long double delta_milis) {
  for(RumbleData &rum: rumble_effects) {
    rum.update_time(delta_milis);
  }
}

std::array<const RumbleData *, Controller::max_effects> Controller::getRumbleEffects() {
  std::array<const RumbleData *, max_effects> arr;
  for (size_t i = 0; i < max_effects; ++i) {
    arr[i] = &rumble_effects[i];
  }
  return arr;
}

void Controller::send_packet(unsigned short type, unsigned short code, int value){
  struct input_event uinput_event;
  memset(&uinput_event, 0, sizeof(uinput_event));

  gettimeofday(&uinput_event.time, NULL);

  uinput_event.type = type;
  uinput_event.code = code;
  uinput_event.value = value;

  int ret = write(uinput_fd, &uinput_event, sizeof(uinput_event));
  if (ret < 0) {
    throw std::runtime_error("ERROR: write on virtual controller returned"
                                + std::to_string(ret) + "\n"
                                "Maybe try running with sudo.\n"
                                + strerror(errno) + "\n");
  }
}

int Controller::get_packet(struct input_event &uinput_event){
  memset(&uinput_event, 0, sizeof(uinput_event));

  int ret = read(uinput_fd, &uinput_event, sizeof(uinput_event));
  if (ret == 0) {
    throw std::runtime_error("ERROR: read on virtual controller returned"
                                + std::to_string(ret) + "\n"
                                "Maybe try running with sudo\n"
                                + strerror(errno) + "\n");
  }
  return ret;
}

void Controller::handle_EV_UINPUT(const struct input_event &uinput_event) {
  struct uinput_ff_upload upload;
  struct uinput_ff_erase erase;
  switch (uinput_event.code) {
  case UI_FF_UPLOAD:
    memset(&upload, 0, sizeof(struct uinput_ff_upload));
    upload.request_id = uinput_event.value;

    ioctl(uinput_fd, UI_BEGIN_FF_UPLOAD, &upload);

    //printf("(UI_FF_UPLOAD): %i\n", uinput_event.value);
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

    if (upload.effect.id < max_effects) {
      struct ff_effect *eff = &upload.effect;
      if (!rumble_effects.at(eff->id).init(eff->id, eff->type, eff->replay.length, eff->replay.delay, eff->u.rumble.strong_magnitude, eff->u.rumble.weak_magnitude)) {
        upload.retval = -1;
      }
    }
    else {
      upload.retval = -1;
    }

    ioctl(uinput_fd, UI_END_FF_UPLOAD, &upload);
    break;

  case UI_FF_ERASE:
    memset(&erase, 0, sizeof(struct uinput_ff_erase));
    erase.request_id = uinput_event.value;

    ioctl(uinput_fd, UI_BEGIN_FF_ERASE, &erase);

    //printf("(UI_FF_ERASE): %i\n", uinput_event.value);
    //printf("%x %x %x\n", erase.request_id, erase.retval, erase.effect_id);
    //printf("\n");

    if (erase.effect_id < max_effects) {
      rumble_effects.at(erase.effect_id).deinit();
      erase.retval = 0;
    }
    else {
      erase.retval = -1;
    }

    ioctl(uinput_fd, UI_END_FF_ERASE, &erase);
    break;

  default:
    printf("(EV_UINPUT) Unkonwn code: %x %i\n", uinput_event.code, uinput_event.value);
    break;
  }
}

void Controller::handle_EV_FF(const struct input_event &uinput_event) {
  //printf("(EV_FF) code: %04x - value: %x\n", uinput_event.code, uinput_event.value);
  if (uinput_event.code < max_effects) {
    rumble_effects.at(uinput_event.code).start_effect(uinput_event.code, uinput_event.value);
  }
}
