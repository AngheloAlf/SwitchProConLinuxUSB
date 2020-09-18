#pragma once
#ifndef UINPUT_CONTROLLER_H
#define UINPUT_CONTROLLER_H

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <linux/uinput.h>


class UInputController{
//private:
public:
  struct uinput_user_dev uinput_device;
  int uinput_version, uinput_rc, uinput_fd;

public:
  struct input_event uinput_event;

  UInputController() {
    uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
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

    write(uinput_fd, &uinput_device, sizeof(uinput_device));

    if (ioctl(uinput_fd, UI_DEV_CREATE)) {
      //return -1;
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


  void uinput_write_single_joystick(const int &val, const int &cod) {

    uinput_event.type = EV_ABS;
    uinput_event.code = cod; // BTN_EAST;
    uinput_event.value = (int)val;

    int ret = write(uinput_fd, &uinput_event, sizeof(uinput_event));
    if (ret < 0) {
      #if 0
      PrintColor::red();
      printf("ERROR: write in write_single_joystic() returned %i\nMaybe try running with sudo\n", ret);
      PrintColor::normal();
      #endif
    }
  }

  void uinput_button_down(const int &cod) {

    // press button
    memset(&uinput_event, 0, sizeof(uinput_event));
    gettimeofday(&uinput_event.time, NULL);
    uinput_event.type = EV_KEY;
    uinput_event.code = cod; // BTN_EAST;
    uinput_event.value = 1;
    int ret = write(uinput_fd, &uinput_event, sizeof(uinput_event));
    if (ret < 0) {
      #if 0
      PrintColor::red();
      printf("ERROR: write in button_down() returned %i\nMaybe try running with sudo\n", ret);
      PrintColor::normal();
      #endif
    }

    // if (ret < 0)
    // {
    //   red();
    //   printf("ERROR: write in button_down() send report returned %i\n", ret);
    //   normal();
    // }

    // printf("PRessed button %u\n", cod);
  }

  void uinput_button_release(const int &cod) {
    // release button
    memset(&uinput_event, 0, sizeof(uinput_event));
    gettimeofday(&uinput_event.time, NULL);
    uinput_event.type = EV_KEY;
    uinput_event.code = cod;
    uinput_event.value = 0;
    write(uinput_fd, &uinput_event, sizeof(uinput_event));

    // send report
    uinput_event.type = EV_SYN;
    uinput_event.code = SYN_REPORT;
    uinput_event.value = 0;
    write(uinput_fd, &uinput_event, sizeof(uinput_event));
  }
//#endif
  
};

#endif
