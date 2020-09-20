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
private:
  int uinput_version, uinput_rc, uinput_fd;
  struct uinput_user_dev uinput_device;

public:
  UInputController() {
    uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinput_fd < 0) {
      throw std::system_error(errno, std::generic_category(), "Failed to open uinput device!");
    }
    uinput_rc = ioctl(uinput_fd, UI_GET_VERSION, &uinput_version);

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

    // joysticks
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
    uinput_device.absmax[ABS_HAT0X] = 1;
    uinput_device.absmin[ABS_HAT0Y] = -1;
    uinput_device.absmax[ABS_HAT0Y] = 1;

    if (write(uinput_fd, &uinput_device, sizeof(uinput_device)) < 0) {
      close(uinput_fd);
      throw std::system_error(errno, std::generic_category(), "Failed to open uinput device!");
    }

    if (ioctl(uinput_fd, UI_DEV_CREATE) < 0) {
      close(uinput_fd);
      throw std::system_error(errno, std::generic_category(), "Failed to open uinput device!");
    }

    #if 0
    PrintColor::green();
    printf("Created uinput device!\n");
    PrintColor::normal();
    #endif

  }

  ~UInputController() {
    ioctl(uinput_fd, UI_DEV_DESTROY);

    close(uinput_fd);

    #if 0
    PrintColor::yellow();
    printf("Destroyed uinput device!\n");
    PrintColor::normal();
    #endif

    return;
  }

  void write_single_joystick(const int &val, const int &cod) {
    send_packet(EV_ABS, cod, (int)val);
  }

  void button_press(const int &cod) {
    send_packet(EV_KEY, cod, 1);
  }

  void button_release(const int &cod) {
    send_packet(EV_KEY, cod, 0);
  }

  void send_report() {
    send_packet(EV_SYN, SYN_REPORT, 0);
  }


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
                                   "Maybe try running with sudo.\n");
    }
  }
  
};

#endif
