#ifndef PROCON_DRIVER_H
#define PROCON_DRIVER_H

#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/input.h>
#include <linux/uinput.h>
#include <ratio>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.hpp"
#include "print_color.hpp"
#include "hidcontroller.hpp"
#include "uinputcontroller.hpp"

#define PROCON_DRIVER_VERSION "1.0 alpha2"


#define PROCON_ID 0x2009
#define NINTENDO_ID 0x057E

#define MAX_N_CONTROLLERS 4

class ProController {

  enum BUTTONS {
    d_left,
    d_right,
    d_up,
    d_down,
    A,
    B,
    X,
    Y,
    plus,
    minus,
    home,
    share,
    L1,
    L2,
    L3,
    R1,
    R2,
    R3,
    None
  };

  static constexpr uint8_t center{0x7e};

public:
  ProController(unsigned short n_controller, hid_device_info *device_info, 
                Config &cfg): config(cfg){
    if (config.force_calibration) {
      read_calibration_from_file = false;
    }

    bool opened = false;
    while(!opened){
      try {
        hid_ctrl = new HidController(device_info->vendor_id, 
                                    device_info->product_id,
                                    device_info->serial_number, n_controller);
        opened = true;
      } catch (const std::ios_base::failure &e) {
        throw;
      } catch (const std::runtime_error &e) {
        usleep(1000 * 10);
        PrintColor::red();
        printf("%s", e.what());
        PrintColor::normal();
        printf("\n");
      }
    }
    
    uinput_ctrl = new UInputController();
    #if 0
    if (controller.uinput_create() < 0) {
      PrintColor::red();
      printf("Failed to open uinput device!\n");
      PrintColor::normal();
    }
    #endif


  }

  ~ProController(){
    if (hid_ctrl != nullptr) {
      delete hid_ctrl;
    }
    if (uinput_ctrl != nullptr) {
      delete uinput_ctrl;
    }
  }

  static const uint8_t bit_position(ProController::BUTTONS button) {
    switch (button) {
    case d_left:
      return 0x04;
      break;
    case d_right:
      return 0x03;
      break;
    case d_up:
      return 0x02;
      break;
    case d_down:
      return 0x01;
      break;
    case A:
      return 0x04;
      break;
    case B:
      return 0x03;
      break;
    case X:
      return 0x02;
      break;
    case Y:
      return 0x01;
      break;
    case plus:
      return 0x02;
      break;
    case minus:
      return 0x01;
      break;
    case home:
      return 0x05;
      break;
    case share:
      return 0x06;
      break;
    case L1:
      return 0x07;
      break;
    case L2:
      return 0x08;
      break;
    case L3:
      return 0x04;
      break;
    case R1:
      return 0x07;
      break;
    case R2:
      return 0x08;
      break;
    case R3:
      return 0x03;
      break;
    case None:
      return 0x00;
      break;
    default:
      PrintColor::red();
      printf("ERROR: Tried to find bitpos of unknown button!\n");
      PrintColor::normal();
      return 0x00;
      break;
    }
  }

  static const uint8_t byte_button_value(ProController::BUTTONS button) {
    switch (button) {
    case d_left:
      return 0x08;
      break;
    case d_right:
      return 0x04;
      break;
    case d_up:
      return 0x02;
      break;
    case d_down:
      return 0x01;
      break;
    case A:
      return 0x08;
      break;
    case B:
      return 0x04;
      break;
    case X:
      return 0x02;
      break;
    case Y:
      return 0x01;
      break;
    case plus:
      return 0x02;
      break;
    case minus:
      return 0x01;
      break;
    case home:
      return 0x10;
      break;
    case share:
      return 0x20;
      break;
    case L1:
      return 0x40;
      break;
    case L2:
      return 0x80;
      break;
    case L3:
      return 0x08;
      break;
    case R1:
      return 0x40;
      break;
    case R2:
      return 0x80;
      break;
    case R3:
      return 0x04;
      break;
    case None:
      return 0x00;
      break;
    default:
      PrintColor::red();
      printf("ERROR: Tried to find bitpos of unknown button!\n");
      PrintColor::normal();
      return 0x00;
      break;
    }
  }

  static const uint8_t data_address(ProController::BUTTONS button) {
    switch (button) {
    case d_left:
      return 0x0f;
      break;
    case d_right:
      return 0x0f;
      break;
    case d_up:
      return 0x0f;
      break;
    case d_down:
      return 0x0f;
      break;
    case A:
      return 0x0d;
      break;
    case B:
      return 0x0d;
      break;
    case X:
      return 0x0d;
      break;
    case Y:
      return 0x0d;
      break;
    case plus:
      return 0x0e;
      break;
    case minus:
      return 0x0e;
      break;
    case home:
      return 0x0e;
      break;
    case share:
      return 0x0e;
      break;
    case L1:
      return 0x0f;
      break;
    case L2:
      return 0x0f;
      break;
    case L3:
      return 0x0e;
      break;
    case R1:
      return 0x0d;
      break;
    case R2:
      return 0x0d;
      break;
    case R3:
      return 0x0e;
      break;
    case None:
      return 0x00;
      break;
    default:
      PrintColor::red();
      printf("ERROR: Tried to find data adress of unknown button!\n");
      PrintColor::normal();
      return 0x00;
      break;
    }
  }

  // void timer() {

  //   using namespace std;
  //   clock_t now = clock();

  //   double elapsed_secs = double(now - last_time) / CLOCKS_PER_SEC;

  //   last_time = now;

  //   printf("Time for last %u polls: %f seconds\n", n_print_cycle,
  //   elapsed_secs);
  //   printf("Bad 0x00: %u\nBad 0x30: %u\n\n", n_bad_data_thirty,
  //          n_bad_data_zero);

  //   print_cycle_counter = 0;
  //   n_bad_data_thirty = 0;
  //   n_bad_data_zero = 0;
  // }

  // void print_sticks(const uint8_t &data0, const uint8_t &data1, const uint8_t
  // &data2,
  //                   const uint8_t &data3, const uint8_t &data4,
  //                   const uint8_t &data5) {
  //   uint8_t left_x = ((data1 & 0x0F) << 4) | ((data0 & 0xF0) >> 4);
  //   uint8_t left_y = data2;
  //   uint8_t right_x = ((data4 & 0x0F) << 4) | ((data3 & 0xF0) >> 4);
  //   uint8_t right_y = data5;

  //   map_sticks(left_x, left_y, right_x, right_y);

  //   clear_console();
  //   yellow();
  //   printf("left_x %d\n", left_x);
  //   printf("left_y %d\n", left_y);
  //   printf("right_x %d\n", right_x);
  //   printf("right_y %d\n\n", right_y);
  //   normal();

  //   // if(left_x == 0x00 || left_y == 0x00 || right_x == 0x00 || right_y ==
  //   0x00
  //   // ) {
  //   //     return -1;
  //   // }
  //   // return 0;
  // }

  // void print_buttons(const uint8_t &left, const uint8_t &mid, const uint8_t
  // &right) {
  //   // uint8_t left = buttons[0];
  //   // uint8_t mid = buttons[1];
  //   // uint8_t right = buttons[2];

  //   if (left & byte_button_value(d_left))
  //     printf("d_left\n");
  //   if (left & byte_button_value(d_right))
  //     printf("d_right\n");
  //   if (left & byte_button_value(d_up))
  //     printf("d_up\n");
  //   if (left & byte_button_value(d_down))
  //     printf("d_down\n");
  //   if (left & byte_button_value(L1))
  //     printf("L1\n");
  //   if (left & byte_button_value(L2))
  //     printf("L2\n");
  //   if (mid & byte_button_value(L3))
  //     printf("L3\n");
  //   if (mid & byte_button_value(R3))
  //     printf("R3\n");
  //   if (mid & byte_button_value(share))
  //     printf("share\n");
  //   if (mid & byte_button_value(home)) {
  //     printf("home\n");
  //   }
  //   if (mid & byte_button_value(plus))
  //     printf("plus\n");
  //   if (mid & byte_button_value(minus))
  //     printf("minus\n");
  //   if (right & byte_button_value(A))
  //     printf("A\n");
  //   if (right & byte_button_value(B))
  //     printf("B\n");
  //   if (right & byte_button_value(X))
  //     printf("X\n");
  //   if (right & byte_button_value(Y))
  //     printf("Y\n");
  //   if (right & byte_button_value(R1))
  //     printf("R1\n");
  //   if (right & byte_button_value(R2))
  //     printf("R2\n");
  // }

  void clear_console() { system("clear"); }


  int poll_input() {
    // print_cycle_counter++;
    // if(print_cycle_counter++ > n_print_cycle) {
    //     timer();
    // }

    auto dat = hid_ctrl->send_command(hid_ctrl->get_input, hid_ctrl->empty);

    if (hid_ctrl->detect_useless_data(dat[0])) {
      // printf("detected useless data!\n");
      return 0;
    }

    hid_ctrl->send_subcommand(0x1, hid_ctrl->led_command, hid_ctrl->led_calibrated); // XXX way too often

    if (dat[0x0e] & byte_button_value(home) &&
        dat[0x0e] & byte_button_value(share)) {
      decalibrate();
    }

    uinput_manage_buttons(dat[0x0f], dat[0x0e], dat[0x0d]);
    uinput_manage_joysticks(dat[0x10], dat[0x11], dat[0x12], dat[0x13],
                            dat[0x14], dat[0x15]);
    uinput_manage_dpad(dat[0x0f]);

    // print_buttons(dat[0x0f], dat[0x0e], dat[0x0d]);
    // print_sticks(dat[0x10], dat[0x11], dat[0x12], dat[0x13], dat[0x14],
    // dat[0x15]);
    // print_exchange_array(dat);
    return 0;
  }

#ifdef DRIBBLE_MODE
  void toggle_dribble_mode() { dribble_mode = !dribble_mode; }
#endif

  void calibrate() {
    if (read_calibration_from_file) {
      std::ifstream myReadFile;
      uint8_t output[8];
      myReadFile.open("procon_calibration_data",
                      std::ios::in | std::ios::binary);
      if (myReadFile) {

        // while (!myReadFile.eof())

        myReadFile.read((char *)&left_x_min, sizeof(uint8_t));
        myReadFile.read((char *)&left_x_max, sizeof(uint8_t));
        myReadFile.read((char *)&left_y_min, sizeof(uint8_t));
        myReadFile.read((char *)&left_y_max, sizeof(uint8_t));
        myReadFile.read((char *)&right_x_min, sizeof(uint8_t));
        myReadFile.read((char *)&right_x_max, sizeof(uint8_t));
        myReadFile.read((char *)&right_y_min, sizeof(uint8_t));
        myReadFile.read((char *)&right_y_max, sizeof(uint8_t));

        PrintColor::green();
        printf("Read calibration data from file! ");
        PrintColor::cyan();
        printf("Press 'share' and 'home' to calibrate again or start with "
               "--calibrate or -c.\n");
        PrintColor::normal();

        calibrated = true;
        // send_rumble(0,255);
        // send_subcommand(0x1, led_command, led_calibrated);

        return;
      }

      myReadFile.close();
    }


    auto dat = hid_ctrl->send_command(hid_ctrl->get_input, hid_ctrl->empty);

    if (hid_ctrl->detect_useless_data(dat[0])) {
      // printf("detected useless data!\n");
      return;
    }

    // print_buttons(dat[0x0f], dat[0x0e], dat[0x0d]);
    // print_sticks(dat[0x10], dat[0x11], dat[0x12], dat[0x13], dat[0x14],
    // dat[0x15]);
    // print_exchange_array(dat);

    hid_ctrl->send_subcommand(0x1, hid_ctrl->led_command, hid_ctrl->led_calibration); // XXX way too often
    if (!share_button_free) {
      if (!(dat[0x0e] & byte_button_value(share))) {
        share_button_free = true;
      }
    } else {
      bool cal = do_calibrate(dat[0x10], dat[0x11], dat[0x12], dat[0x13],
                              dat[0x14], dat[0x15], dat[0x0e]);
      if (cal) {
        // send_rumble(0,255);
        calibrated = true;
        hid_ctrl->send_subcommand(0x1, hid_ctrl->led_command, hid_ctrl->led_calibrated);
        // printf("finished calibration\n");
        // usleep(1000000);

        // write calibration data to file
        std::ofstream calibration_file;
        calibration_file.open("procon_calibration_data",
                              std::ios::out | std::ios::binary);
        calibration_file.write((char *)&left_x_min, sizeof(uint8_t));
        calibration_file.write((char *)&left_x_max, sizeof(uint8_t));
        calibration_file.write((char *)&left_y_min, sizeof(uint8_t));
        calibration_file.write((char *)&left_y_max, sizeof(uint8_t));
        calibration_file.write((char *)&right_x_min, sizeof(uint8_t));
        calibration_file.write((char *)&right_x_max, sizeof(uint8_t));
        calibration_file.write((char *)&right_y_min, sizeof(uint8_t));
        calibration_file.write((char *)&right_y_max, sizeof(uint8_t));
        calibration_file.close();
        PrintColor::green();
        printf("Wrote calibration data to file!\n");
        PrintColor::normal();
      }
    }

    // std::ofstream out("calibration_data");
    // if (!out)
    // {
    //     return;
    // }

    // printf("wrote text\n");

    // out.close();
  }

  bool do_calibrate(const uint8_t &stick0, const uint8_t &stick1,
                    const uint8_t &stick2, const uint8_t &stick3,
                    const uint8_t &stick4, const uint8_t &stick5,
                    const uint8_t &mid_buttons) {
    uint8_t left_x = ((stick1 & 0x0F) << 4) | ((stick0 & 0xF0) >> 4);
    uint8_t left_y = stick2;
    uint8_t right_x = ((stick4 & 0x0F) << 4) | ((stick3 & 0xF0) >> 4);
    uint8_t right_y = stick5;

    // invert
    if (config.invert_lx) {
      left_x = 255 - left_x;
    }
    if (config.invert_ly) {
      left_y = 255 - left_y;
    }
    if (config.invert_rx) {
      right_x = 255 - right_x;
    }
    if (config.invert_ry) {
      right_y = 255 - right_y;
    }

    left_x_min = (left_x < left_x_min) ? left_x : left_x_min;
    left_y_min = (left_y < left_y_min) ? left_y : left_y_min;
    right_x_min = (right_x < right_x_min) ? right_x : right_x_min;
    right_y_min = (right_y < right_y_min) ? right_y : right_y_min;
    left_x_max = (left_x > left_x_max) ? left_x : left_x_max;
    left_y_max = (left_y > left_y_max) ? left_y : left_y_max;
    right_x_max = (right_x > right_x_max) ? right_x : right_x_max;
    right_y_max = (right_y > right_y_max) ? right_y : right_y_max;

    // clear_console();
    // printf("left_x_min: %u\n", left_x_min);
    // printf("left_y_min: %u\n", left_y_min);
    // printf("right_x_min: %u\n", right_x_min);
    // printf("right_y_min: %u\n", right_y_min);
    // printf("left_x_max: %u\n", left_x_max);
    // printf("left_y_max: %u\n", left_y_max);
    // printf("right_x_max: %u\n", right_x_max);
    // printf("right_y_max: %u\n\n", right_y_max);
    // print_calibration_values();

    return (mid_buttons & byte_button_value(share));
  }

  void print_calibration_values() {
    std::cout << "LX: " << (unsigned int)left_x_min << ","
              << (unsigned int)left_x_max
              << "   LY: " << (unsigned int)left_y_min << ","
              << (unsigned int)left_y_max
              << "   RX: " << (unsigned int)right_x_min << ","
              << (unsigned int)right_x_max
              << "   RY: " << (unsigned int)right_y_min << ","
              << (unsigned int)right_y_max << "                \r";
  }

  void decalibrate() {
    left_x_min = center;
    left_x_max = center;
    left_y_min = center;
    left_x_max = center;
    right_x_min = center;
    right_x_max = center;
    right_y_min = center;
    right_x_max = center;
    calibrated = false;
    hid_ctrl->send_subcommand(0x1, hid_ctrl->led_command, hid_ctrl->led_calibration);
    PrintColor::magenta();
    printf("Controller decalibrated!\n");
    PrintColor::cyan();
    printf("%c[%d;%dmPerform calibration again and press the square 'share' "
           "button!\n%c[%dm",
           27, 1, 36, 27, 0);
    PrintColor::normal();
    read_calibration_from_file = false;
    share_button_free = false;
    // usleep(1000*1000);
  }

  const void map_sticks(uint8_t &left_x, uint8_t &left_y, uint8_t &right_x,
                        uint8_t &right_y) {
    left_x = (uint8_t)(clamp((float)(left_x - left_x_min) /
                             (float)(left_x_max - left_x_min) * 255.f));
    left_y = (uint8_t)(clamp((float)(left_y - left_y_min) /
                             (float)(left_y_max - left_y_min) * 255.f));
    right_x = (uint8_t)(clamp((float)(right_x - right_x_min) /
                              (float)(right_x_max - right_x_min) * 255.f));
    right_y = (uint8_t)(clamp((float)(right_y - right_y_min) /
                              (float)(right_y_max - right_y_min) * 255.f));
  }

  static const float clamp(float inp) {
    if (inp < 0.5f)
      return 0.5f;
    if (inp > 254.5f) {
      return 254.5f;
    }
    return inp;
  }
  static const int clamp_int(int inp) {
    if (inp < 0)
      return 0;
    if (inp > 255) {
      return 255;
    }
    return inp;
  }


  // void blink() {
  //   if (++blink_counter > blink_length) {
  //     blink_counter = 0;
  //     if (++blink_position >= blink_array.size()) {
  //       blink_position = 0;
  //     }
  //   }
  //   std::array<uint8_t,1> blink_command{{blink_array[blink_position]}};
  //   send_subcommand(0x1, led_command, blink_command);
  // }

  //-------------------------
  //         UINPUT
  //-------------------------

  void uinput_manage_dpad(const char &left) {
    bool b_d_left;
    bool b_d_right;
    bool b_d_up;
    bool b_d_down;

    // invert
    if (!config.invert_dx) {
      b_d_left = left & byte_button_value(d_left);
      b_d_right = left & byte_button_value(d_right);
    } else {
      b_d_left = left & byte_button_value(d_right);
      b_d_right = left & byte_button_value(d_left);
    }

    if (!config.invert_dy) {
      b_d_up = left & byte_button_value(d_up);
      b_d_down = left & byte_button_value(d_down);
    } else {
      b_d_up = left & byte_button_value(d_down);
      b_d_down = left & byte_button_value(d_up);
    }

    memset(&uinput_ctrl->uinput_event, 0, sizeof(uinput_ctrl->uinput_event));
    gettimeofday(&uinput_ctrl->uinput_event.time, NULL);

    if (b_d_left) {
      uinput_ctrl->uinput_write_single_joystick(-1, ABS_HAT0X);
    } else if (b_d_right) {
      uinput_ctrl->uinput_write_single_joystick(1, ABS_HAT0X);
    } else if (!b_d_left && !b_d_right) {
      uinput_ctrl->uinput_write_single_joystick(0, ABS_HAT0X);
    }
    if (b_d_down) {
      uinput_ctrl->uinput_write_single_joystick(-1, ABS_HAT0Y);
    } else if (b_d_up) {
      uinput_ctrl->uinput_write_single_joystick(1, ABS_HAT0Y);
    } else if (!b_d_down && !b_d_up) {
      uinput_ctrl->uinput_write_single_joystick(0, ABS_HAT0Y);
    }

    // send report
    uinput_ctrl->uinput_event.type = EV_SYN;
    uinput_ctrl->uinput_event.code = SYN_REPORT;
    uinput_ctrl->uinput_event.value = 0;
    write(uinput_ctrl->uinput_fd, &uinput_ctrl->uinput_event, sizeof(uinput_ctrl->uinput_event));
  }

  void uinput_manage_buttons(const char &left, const char &mid,
                             const char &right) {

    // bool b_d_left = left & byte_button_value(d_left);
    // bool b_d_right = left & byte_button_value(d_right);
    // bool b_d_up = left & byte_button_value(d_up);
    // bool b_d_down = left & byte_button_value(d_down);
    bool b_L1 = left & byte_button_value(L1);
    bool b_L2 = left & byte_button_value(L2);
    bool b_L3 = mid & byte_button_value(L3);
    bool b_R1 = right & byte_button_value(R1);
    bool b_R2 = right & byte_button_value(R2);
    bool b_R3 = mid & byte_button_value(R3);
    bool b_share = mid & byte_button_value(share);
    bool b_home = mid & byte_button_value(home);
    bool b_plus = mid & byte_button_value(plus);
    bool b_minus = mid & byte_button_value(minus);

    bool b_a, b_b, b_x, b_y;
    if (!config.swap_buttons) {
      b_a = right & byte_button_value(A);
      b_b = right & byte_button_value(B);
      b_x = right & byte_button_value(X);
      b_y = right & byte_button_value(Y);
    } else {
      b_a = right & byte_button_value(B);
      b_b = right & byte_button_value(A);
      b_x = right & byte_button_value(Y);
      b_y = right & byte_button_value(X);
    }

    // press
    if (b_a && !last_a)
      uinput_ctrl->uinput_button_down(BTN_EAST);
    if (b_b && !last_b)
      uinput_ctrl->uinput_button_down(BTN_SOUTH);
    if (b_x && !last_x) {
      uinput_ctrl->uinput_button_down(BTN_WEST);
#ifdef DRIBBLE_MODE // toggle off dribble mode
      if (dribble_mode)
        toggle_dribble_mode();
#endif
    }
#ifdef DRIBBLE_MODE // toggle dribble mode
    if (b_y && !last_y)
      toggle_dribble_mode();
    if (b_share && !last_share) // replace button by share
      uinput_ctrl->uinput_button_down(BTN_NORTH);
#else
    if (b_y && !last_y)
      uinput_ctrl->uinput_button_down(BTN_NORTH);
#endif

    // if (b_d_down && !last_d_down)
    //   uinput_ctrl->uinput_button_down(BTN_DPAD_DOWN);
    // if (b_d_up && !last_d_up)
    //   uinput_ctrl->uinput_button_down(BTN_DPAD_UP);
    // if (b_d_left && !last_d_left)
    //   uinput_ctrl->uinput_button_down(BTN_DPAD_LEFT);
    // if (b_d_right && !last_d_right)
    //   uinput_ctrl->uinput_button_down(BTN_DPAD_RIGHT);
    if (b_plus && !last_plus)
      uinput_ctrl->uinput_button_down(BTN_START);
    if (b_minus && !last_minus)
      uinput_ctrl->uinput_button_down(BTN_SELECT);
    if (b_L1 && !last_L1)
      uinput_ctrl->uinput_button_down(BTN_TL);
    // if (b_L2 && !last_L2)
    //   uinput_ctrl->uinput_button_down(BTN_TL2);
    if (b_R1 && !last_R1)
      uinput_ctrl->uinput_button_down(BTN_TR);
    // if (b_R2 && !last_R2)
    //   uinput_ctrl->uinput_button_down(BTN_TR2);
    if (b_L3 && !last_L3)
      uinput_ctrl->uinput_button_down(BTN_THUMBL);
    if (b_R3 && !last_R3)
      uinput_ctrl->uinput_button_down(BTN_THUMBR);
    // if (b_L1 && !last_L1)
    //   uinput_ctrl->uinput_button_down(BTN_TL);
    if (b_home && !last_home)
      uinput_ctrl->uinput_button_down(BTN_MODE);

    // release
    if (!b_a && last_a)
      uinput_ctrl->uinput_button_release(BTN_EAST);
    if (!b_b && last_b)
      uinput_ctrl->uinput_button_release(BTN_SOUTH);
    if (!b_x && last_x)
      uinput_ctrl->uinput_button_release(BTN_WEST);

#ifdef DRIBBLE_MODE
    if (!b_y && last_y)
      uinput_ctrl->uinput_button_release(BTN_WEST);
    if (!b_share && last_share)
      uinput_ctrl->uinput_button_release(BTN_NORTH);
#else
    if (!b_y && last_y)
      uinput_ctrl->uinput_button_release(BTN_NORTH);
#endif

    // if (!b_d_down && last_d_down)
    //   uinput_ctrl->uinput_button_release(BTN_DPAD_DOWN);
    // if (!b_d_up && last_d_up)
    //   uinput_ctrl->uinput_button_release(BTN_DPAD_UP);
    // if (!b_d_left && last_d_left)
    //   uinput_ctrl->uinput_button_release(BTN_DPAD_LEFT);
    // if (!b_d_right && last_d_right)
    //   uinput_ctrl->uinput_button_release(BTN_DPAD_RIGHT);
    if (!b_plus && last_plus)
      uinput_ctrl->uinput_button_release(BTN_START);
    if (!b_minus && last_minus)
      uinput_ctrl->uinput_button_release(BTN_SELECT);
    if (!b_L1 && last_L1)
      uinput_ctrl->uinput_button_release(BTN_TL);
    // if (!b_L2 && last_L2)
    //   uinput_ctrl->uinput_button_release(BTN_TL2);
    if (!b_R1 && last_R1)
      uinput_ctrl->uinput_button_release(BTN_TR);
    // if (!b_R2 && last_R2)
    //   uinput_ctrl->uinput_button_release(BTN_TR2);
    if (!b_L3 && last_L3)
      uinput_ctrl->uinput_button_release(BTN_THUMBL);
    if (!b_R3 && last_R3)
      uinput_ctrl->uinput_button_release(BTN_THUMBR);
    // if (!b_L1 && last_L1)
    //   uinput_ctrl->uinput_button_release(BTN_TL);
    if (!b_home && last_home)
      uinput_ctrl->uinput_button_release(BTN_MODE);

    // last_d_left = b_d_left;
    // last_d_right = b_d_right;
    // last_d_up = b_d_up;
    // last_d_down = b_d_down;
    last_L1 = b_L1;
    // last_L2 = b_L2;
    last_L3 = b_L3;
    last_R1 = b_R1;
    // last_R2 = b_R2;
    last_R3 = b_R3;
    last_share = b_share;
    last_home = b_home;
    last_plus = b_plus;
    last_minus = b_minus;
    last_a = b_a;
    last_b = b_b;
    last_x = b_x;
    last_y = b_y;

    // do triggers here as well
    int val = b_L2 ? 255 : 0;
    uinput_ctrl->uinput_write_single_joystick(val, ABS_Z);
    val = b_R2 ? 255 : 0;
    uinput_ctrl->uinput_write_single_joystick(val, ABS_RZ);

    // send report
    uinput_ctrl->uinput_event.type = EV_SYN;
    uinput_ctrl->uinput_event.code = SYN_REPORT;
    uinput_ctrl->uinput_event.value = 0;
    write(uinput_ctrl->uinput_fd, &uinput_ctrl->uinput_event, sizeof(uinput_ctrl->uinput_event));
  }

  bool calibration_file_exists() {
    std::ifstream conf("procon_calibration_data");
    return conf.good();
  }

  void uinput_manage_joysticks(const char &dat0, const char &dat1,
                               const char &dat2, const char &dat3,
                               const char &dat4, const char &dat5) {
    // extract data
    uint8_t left_x = ((dat1 & 0x0F) << 4) | ((dat0 & 0xF0) >> 4);
    uint8_t left_y = dat2;
    uint8_t right_x = ((dat4 & 0x0F) << 4) | ((dat3 & 0xF0) >> 4);
    uint8_t right_y = dat5;

    // invert
    if (config.invert_lx) {
      left_x = 255 - left_x;
    }
    if (config.invert_ly) {
      left_y = 255 - left_y;
    }
    if (config.invert_rx) {
      right_x = 255 - right_x;
    }
    if (config.invert_ry) {
      right_y = 255 - right_y;
    }

    // map data
    map_sticks(left_x, left_y, right_x, right_y);

    // write uinput
    memset(&uinput_ctrl->uinput_event, 0, sizeof(uinput_ctrl->uinput_event));

    // left_x = 0;
    // left_y = 127;
    // right_x = 255;
    // right_y = 200;
    gettimeofday(&uinput_ctrl->uinput_event.time, NULL);

    uinput_ctrl->uinput_write_single_joystick(left_x, ABS_X);
    uinput_ctrl->uinput_write_single_joystick(left_y, ABS_Y);
    uinput_ctrl->uinput_write_single_joystick(right_x, ABS_RX);
#ifndef DRIBBLE_MODE
    uinput_ctrl->uinput_write_single_joystick(right_y, ABS_RY);
#else
    if (dribble_mode) {
      right_y = clamp_int(right_y + config.dribble_cam_value - 127);
    }
#endif
    uinput_ctrl->uinput_write_single_joystick(right_y, ABS_RY);
    // send report
    uinput_ctrl->uinput_event.type = EV_SYN;
    uinput_ctrl->uinput_event.code = SYN_REPORT;
    uinput_ctrl->uinput_event.value = 0;
    write(uinput_ctrl->uinput_fd, &uinput_ctrl->uinput_event, sizeof(uinput_ctrl->uinput_event));

    // clear_console();
    // printf("left_x: %i\n", (int)left_x);
    // printf("left_y: %i\n", (int)left_y);
    // printf("right_x: %i\n", (int)right_x);
    // printf("right_y: %i\n", (int)right_y);
  }


  std::clock_t last_time;

  std::array<uint8_t, 20> first{{0x0}};
  std::array<uint8_t, 20> second{{0x0}};
  std::array<uint8_t, 20> third{{0x0}};
  std::array<uint8_t, 20> fourth{{0x0}};
  std::array<uint8_t, 20> fifth{{0x0}};
  std::array<uint8_t, 20> sixth{{0x0}};


  // uint blink_position = 0;
  // size_t blink_counter = 0;
  // const size_t blink_length = 50;

  bool bad_data_detected = false;

  uint n_print_cycle = 1000;
  uint print_cycle_counter = 0;
  uint n_bad_data_zero = 0;
  uint n_bad_data_thirty = 0;

  bool calibrated = false;
  bool read_calibration_from_file =
      true; // will be set to false in decalibrate or with flags
  bool share_button_free = false; // used for recalibration (press share & home)
  uint8_t left_x_min = 0x7e;
  uint8_t left_y_min = 0x7e;
  uint8_t right_x_min = 0x7e;
  uint8_t right_y_min = 0x7e;
  uint8_t left_x_max = 0x7e;
  uint8_t left_y_max = 0x7e;
  uint8_t right_x_max = 0x7e;
  uint8_t right_y_max = 0x7e;

  // bool last_d_left = false;
  // bool last_d_right = false;
  // bool last_d_up = false;
  // bool last_d_down = false;
  bool last_L1 = false;
  // bool last_L2 = false;
  bool last_L3 = false;
  bool last_R1 = false;
  // bool last_R2 = false;
  bool last_R3 = false;
  bool last_share = false;
  bool last_home = false;
  bool last_plus = false;
  bool last_minus = false;
  bool last_a = false;
  bool last_b = false;
  bool last_x = false;
  bool last_y = false;

  bool dribble_mode = false;

  Config config;
  HidController *hid_ctrl = nullptr;
  UInputController *uinput_ctrl = nullptr;

};

#endif
