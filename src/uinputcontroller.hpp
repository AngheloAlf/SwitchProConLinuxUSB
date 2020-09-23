#pragma once
#ifndef UINPUT_CONTROLLER_H
#define UINPUT_CONTROLLER_H

#include <linux/uinput.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <errno.h>
#include <system_error>


class UInputController{
public:
  UInputController() {
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
    uinput_device.absmax[ABS_X] = 255;
    uinput_device.absmin[ABS_Y] = 0;
    uinput_device.absmax[ABS_Y] = 255;
    uinput_device.absmin[ABS_RX] = 0;
    uinput_device.absmax[ABS_RX] = 255;
    uinput_device.absmin[ABS_RY] = 0;
    uinput_device.absmax[ABS_RY] = 255;
    uinput_device.absmin[ABS_Z] = 0;
    uinput_device.absmax[ABS_Z] = 255;
    uinput_device.absmin[ABS_RZ] = 0;
    uinput_device.absmax[ABS_RZ] = 255;
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

    uinput_device.ff_effects_max = 2;

    if (write(uinput_fd, &uinput_device, sizeof(uinput_device)) < 0) {
      close(uinput_fd);
      throw std::system_error(errno, std::generic_category(), "Failed to set axis data!");
    }

    if (ioctl(uinput_fd, UI_DEV_CREATE) < 0) {
      close(uinput_fd);
      throw std::system_error(errno, std::generic_category(), "Failed to create uinput device!");
    }
  }

  ~UInputController() {
    ioctl(uinput_fd, UI_DEV_DESTROY);

    close(uinput_fd);
  }

  void write_single_joystick(int val, int cod) {
    send_packet(EV_ABS, cod, (int)val);
  }

  void button_press(int cod) {
    send_packet(EV_KEY, cod, 1);
  }

  void button_release(int cod) {
    send_packet(EV_KEY, cod, 0);
  }

  void send_report() {
    send_packet(EV_SYN, SYN_REPORT, 0);
  }

  void update_state() {
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
  uint16_t magnitude = 0, r_length = 0;
  int32_t length_remaining = 0;
private:
  void send_packet(unsigned short type, unsigned short code, int value){
    struct input_event uinput_event;
    memset(&uinput_event, 0, sizeof(uinput_event));

    gettimeofday(&uinput_event.time, NULL);

    uinput_event.type = type;
    uinput_event.code = code;
    uinput_event.value = value;

    int ret = write(uinput_fd, &uinput_event, sizeof(uinput_event));
    if (ret < 0) {
      throw std::ios_base::failure("ERROR: write on virtual controller returned"
                                   + std::to_string(ret) + "\n"
                                   "Maybe try running with sudo.\n"
                                   + strerror(errno) + "\n");
    }
  }

  int get_packet(struct input_event &uinput_event){
    memset(&uinput_event, 0, sizeof(uinput_event));

    int ret = read(uinput_fd, &uinput_event, sizeof(uinput_event));
    if (ret == 0) {
      throw std::ios_base::failure("ERROR: read on virtual controller returned"
                                   + std::to_string(ret) + "\n"
                                   "Maybe try running with sudo\n"
                                   + strerror(errno) + "\n");
    }
    return ret;
  }

  void handle_EV_UINPUT(const struct input_event &uinput_event) {
    struct uinput_ff_upload upload;
    struct uinput_ff_erase erase;
    switch (uinput_event.code) {
    case UI_FF_UPLOAD:
      memset(&upload, 0, sizeof(struct uinput_ff_upload));
      upload.request_id = uinput_event.value;

      ioctl(uinput_fd, UI_BEGIN_FF_UPLOAD, &upload);

      printf("(UI_FF_UPLOAD): %i\n", uinput_event.value);
      printf("%x %x\n", upload.request_id, upload.retval);
      printf("\t%x %x %x\n", upload.effect.type, upload.effect.id, upload.effect.direction);
      printf("\ttrigger: %x %x\n", upload.effect.trigger.button, upload.effect.trigger.interval);
      printf("\treplay: %i %i\n", upload.effect.replay.length, upload.effect.replay.delay);
      if (upload.effect.type == FF_RUMBLE) {
        printf("\tmagnitude: %x %x\n", upload.effect.u.rumble.strong_magnitude, upload.effect.u.rumble.weak_magnitude);
      }
      printf("\n");
      upload.retval = 0; // -1 on error
      //upload.effect.id = 0;

      /// TODO: enable
      if (!magnitude) magnitude = upload.effect.u.rumble.strong_magnitude;
      if (!magnitude) magnitude = upload.effect.u.rumble.weak_magnitude;
      r_length = upload.effect.replay.length;

      ioctl(uinput_fd, UI_END_FF_UPLOAD, &upload);
      break;

    case UI_FF_ERASE:
      memset(&erase, 0, sizeof(struct uinput_ff_erase));
      erase.request_id = uinput_event.value;

      ioctl(uinput_fd, UI_BEGIN_FF_ERASE, &erase);

      printf("(UI_FF_ERASE): %i\n", uinput_event.value);
      printf("%x %x %x\n", erase.request_id, erase.retval, erase.effect_id);
      printf("\n");

      /// TODO: disable

      ioctl(uinput_fd, UI_END_FF_ERASE, &erase);
      break;

    default:
      printf("(EV_UINPUT) Unkonwn code: %x %i\n", uinput_event.code, uinput_event.value);
      break;
    }
  }

  void handle_EV_FF(const struct input_event &uinput_event) {
    printf("(EV_FF) code: %04x - value: %x\n", uinput_event.code, uinput_event.value);
    if (uinput_event.value) {
      length_remaining += r_length;
    }
  }

  int uinput_version, uinput_rc, uinput_fd;
};

#endif
