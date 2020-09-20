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
#include <ratio>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.hpp"
#include "print_color.hpp"
#include "hidcontroller.hpp"
#include "uinputcontroller.hpp"
#include "proinputparser.hpp"

#define PROCON_DRIVER_VERSION "1.0 alpha2"


#define PROCON_ID 0x2009
#define NINTENDO_ID 0x057E

#define MAX_N_CONTROLLERS 4

class ProController {

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

    btns_map = make_button_map();
  }

  ~ProController(){
    if (hid_ctrl != nullptr) {
      delete hid_ctrl;
    }
    if (uinput_ctrl != nullptr) {
      delete uinput_ctrl;
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

  // void print_sticks(const ProInputParser &parser) {
  //   uint8_t left_x;
  //   uint8_t left_y;
  //   uint8_t right_x;
  //   uint8_t right_y;
  //   parser.get_joystick_data(left_x, left_y, right_x, right_y);

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

  // void print_buttons(const ProInputParser &parser) {

  //   if (parser.is_button_pressed(ProInputParser::d_left))
  //     printf("d_left\n");
  //   if (parser.is_button_pressed(ProInputParser::d_right))
  //     printf("d_right\n");
  //   if (parser.is_button_pressed(ProInputParser::d_up))
  //     printf("d_up\n");
  //   if (parser.is_button_pressed(ProInputParser::d_down))
  //     printf("d_down\n");
  //   if (parser.is_button_pressed(ProInputParser::L1))
  //     printf("L1\n");
  //   if (parser.is_button_pressed(ProInputParser::L2))
  //     printf("L2\n");
  //   if (parser.is_button_pressed(ProInputParser::L3))
  //     printf("L3\n");
  //   if (parser.is_button_pressed(ProInputParser::R3))
  //     printf("R3\n");
  //   if (parser.is_button_pressed(ProInputParser::share))
  //     printf("share\n");
  //   if (parser.is_button_pressed(ProInputParser::home)) {
  //     printf("home\n");
  //   }
  //   if (parser.is_button_pressed(ProInputParser::plus))
  //     printf("plus\n");
  //   if (parser.is_button_pressed(ProInputParser::minus))
  //     printf("minus\n");
  //   if (parser.is_button_pressed(ProInputParser::A))
  //     printf("A\n");
  //   if (parser.is_button_pressed(ProInputParser::B))
  //     printf("B\n");
  //   if (parser.is_button_pressed(ProInputParser::X))
  //     printf("X\n");
  //   if (parser.is_button_pressed(ProInputParser::Y))
  //     printf("Y\n");
  //   if (parser.is_button_pressed(ProInputParser::R1))
  //     printf("R1\n");
  //   if (parser.is_button_pressed(ProInputParser::R2))
  //     printf("R2\n");
  // }

  void clear_console() { system("clear"); }

  int poll_input() {
    // print_cycle_counter++;
    // if(print_cycle_counter++ > n_print_cycle) {
    //     timer();
    // }

    auto dat = hid_ctrl->request_input();

    if (hid_ctrl->detect_useless_data(dat[0])) {
      // printf("detected useless data!\n");
      return 0;
    }

    hid_ctrl->led();

    ProInputParser input_parser(dat);

    if (input_parser.is_button_pressed(ProInputParser::home) &&
        input_parser.is_button_pressed(ProInputParser::share)) {
      decalibrate();
    }

    uinput_manage_buttons(input_parser);
    uinput_manage_joysticks(input_parser);
    uinput_manage_dpad(input_parser);

    // print_buttons(input_parser);
    // print_sticks(input_parser);
    // print_exchange_array(dat);
    return 0;
  }

#ifdef DRIBBLE_MODE
  void toggle_dribble_mode() { dribble_mode = !dribble_mode; }
#endif

  void calibrate() {
    if (read_calibration_from_file) {
      if (read_calibration_file()) {
        calibrated = true;
        // send_rumble(0,255);
        // hid_ctrl->led();

        return;
      }
    }

    hid_ctrl->blink();

    auto dat = hid_ctrl->request_input();

    if (hid_ctrl->detect_useless_data(dat[0])) {
      // printf("detected useless data!\n");
      return;
    }

    ProInputParser parser(dat);
    // print_buttons(parser);
    // print_sticks(parser);
    // print_exchange_array(dat);

    if (!share_button_free) {
      if (!parser.is_button_pressed(ProInputParser::share)) {
        share_button_free = true;
      }
    } else {
      bool cal = do_calibrate(parser);
      if (cal) {
        // send_rumble(0,255);
        calibrated = true;
        hid_ctrl->led();
        // printf("finished calibration\n");
        // usleep(1000000);

        // write calibration data to file
        write_calibration_to_file();
      }
    }
  }

  bool do_calibrate(const ProInputParser &parser) {
    uint8_t left_x;
    uint8_t left_y;
    uint8_t right_x;
    uint8_t right_y;
    parser.get_joystick_data(left_x, left_y, right_x, right_y);

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

    return parser.is_button_pressed(ProInputParser::share);
  }

  void print_calibration_values() {
    std::cout << "LX: " << (unsigned int)left_x_min << ","
              << (unsigned int)left_x_max
              << "   LY: " << (unsigned int)left_y_min << ","
              << (unsigned int)left_y_max
              << "   RX: " << (unsigned int)right_x_min << ","
              << (unsigned int)right_x_max
              << "   RY: " << (unsigned int)right_y_min << ","
              << (unsigned int)right_y_max << "                \n";
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
    PrintColor::magenta();
    printf("Controller decalibrated!\n");
    PrintColor::cyan();
    printf("Perform calibration again and press the square 'share' "
           "button!\n");
    PrintColor::normal();
    read_calibration_from_file = false;
    share_button_free = false;
    // usleep(1000*1000);
  }

  bool is_calibrated() {
    return calibrated;
  }

  bool needs_first_calibration() {
    return !read_calibration_from_file || !calibration_file_exists();
  }

  bool calibration_file_exists() {
    std::ifstream conf(calibration_filename);
    return conf.good();
  }

  bool read_calibration_file() {
    bool file_readed = false;
    std::ifstream myReadFile;
    myReadFile.open(calibration_filename,
                    std::ios::in | std::ios::binary);
    if (myReadFile) {
      myReadFile.read((char *)&left_x_min, sizeof(uint8_t));
      myReadFile.read((char *)&left_x_max, sizeof(uint8_t));
      myReadFile.read((char *)&left_y_min, sizeof(uint8_t));
      myReadFile.read((char *)&left_y_max, sizeof(uint8_t));
      myReadFile.read((char *)&right_x_min, sizeof(uint8_t));
      myReadFile.read((char *)&right_x_max, sizeof(uint8_t));
      myReadFile.read((char *)&right_y_min, sizeof(uint8_t));
      myReadFile.read((char *)&right_y_max, sizeof(uint8_t));

      file_readed = true;
    }

    myReadFile.close();
    return file_readed;
  }

  void write_calibration_to_file(){
    std::ofstream calibration_file;
    calibration_file.open(calibration_filename,
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
  }

  void map_sticks(uint8_t &left_x, uint8_t &left_y, uint8_t &right_x,
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

  static float clamp(float inp) {
    if (inp < 0.5f)
      return 0.5f;
    if (inp > 254.5f) {
      return 254.5f;
    }
    return inp;
  }
  static int clamp_int(int inp) {
    if (inp < 0)
      return 0;
    if (inp > 255) {
      return 255;
    }
    return inp;
  }


  static std::array<int, 18> make_button_map() {
    std::array<int, 18> map {0};

    map[ProInputParser::d_left]  = BTN_DPAD_LEFT;
    map[ProInputParser::d_right] = BTN_DPAD_RIGHT;
    map[ProInputParser::d_up]    = BTN_DPAD_UP;
    map[ProInputParser::d_down]  = BTN_DPAD_DOWN;

    map[ProInputParser::A] = BTN_EAST;
    map[ProInputParser::B] = BTN_SOUTH;
    map[ProInputParser::X] = BTN_WEST;
    map[ProInputParser::Y] = BTN_NORTH;

    map[ProInputParser::plus]  = BTN_START;
    map[ProInputParser::minus] = BTN_SELECT;
    map[ProInputParser::home]  = BTN_MODE;
    // map[ProInputParser::share] = ;

    map[ProInputParser::L1] = BTN_TL;
    map[ProInputParser::L2] = BTN_TL2;
    map[ProInputParser::L3] = BTN_THUMBL;
    map[ProInputParser::R1] = BTN_TR;
    map[ProInputParser::R2] = BTN_TR2;
    map[ProInputParser::R3] = BTN_THUMBR;

    return map;
  }

  //-------------------------
  //         UINPUT
  //-------------------------

  void uinput_manage_dpad(const ProInputParser &parser) {
    bool b_d_left  = parser.is_button_pressed(ProInputParser::d_left);
    bool b_d_right = parser.is_button_pressed(ProInputParser::d_right);
    bool b_d_up    = parser.is_button_pressed(ProInputParser::d_up);
    bool b_d_down  = parser.is_button_pressed(ProInputParser::d_down);

    // invert
    if (config.invert_dx) {
      b_d_left  = parser.is_button_pressed(ProInputParser::d_right);
      b_d_right = parser.is_button_pressed(ProInputParser::d_left);
    }
    if (config.invert_dy) {
      b_d_up    = parser.is_button_pressed(ProInputParser::d_down);
      b_d_down  = parser.is_button_pressed(ProInputParser::d_up);
    }

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

    uinput_ctrl->send_report();
  }

  void uinput_manage_buttons(const ProInputParser &parser) {
    std::array<bool, 18> buttons_pressed{false};
    for (const ProInputParser::BUTTONS &id: btns_ids) {
      buttons_pressed[id] = parser.is_button_pressed(id);
    }

    if (config.swap_buttons) {
      buttons_pressed[ProInputParser::A] = parser.is_button_pressed(ProInputParser::B);
      buttons_pressed[ProInputParser::B] = parser.is_button_pressed(ProInputParser::A);
      buttons_pressed[ProInputParser::X] = parser.is_button_pressed(ProInputParser::Y);
      buttons_pressed[ProInputParser::Y] = parser.is_button_pressed(ProInputParser::X);
    }

    for (const ProInputParser::BUTTONS &id: xbox_btns_ids) {
      if (buttons_pressed[id] && !last_pressed[id]) {
        uinput_ctrl->uinput_button_down(btns_map[id]);
      }
    }
    #if 0
    // press
    if (buttons_pressed[ProInputParser::X] && !last_pressed[ProInputParser::X]) {
      uinput_ctrl->uinput_button_down(BTN_WEST);
#ifdef DRIBBLE_MODE // toggle off dribble mode
      if (dribble_mode)
        toggle_dribble_mode();
#endif
    }
#ifdef DRIBBLE_MODE // toggle dribble mode
    if (buttons_pressed[ProInputParser::Y] && !last_pressed[ProInputParser::Y])
      toggle_dribble_mode();
    if (buttons_pressed[ProInputParser::share] && !last_pressed[ProInputParser::share]) // replace button by share
      uinput_ctrl->uinput_button_down(BTN_NORTH);
#else
    if (buttons_pressed[ProInputParser::Y] && !last_pressed[ProInputParser::Y])
      uinput_ctrl->uinput_button_down(BTN_NORTH);
#endif
    #endif


    for (const ProInputParser::BUTTONS &id: xbox_btns_ids) {
      if (!buttons_pressed[id] && last_pressed[id]) {
        uinput_ctrl->uinput_button_release(btns_map[id]);
      }
    }
    #if 0
#ifdef DRIBBLE_MODE
    if (!buttons_pressed[ProInputParser::Y] && last_pressed[ProInputParser::Y])
      uinput_ctrl->uinput_button_release(BTN_WEST);
    if (!buttons_pressed[ProInputParser::share] && last_pressed[ProInputParser::share])
      uinput_ctrl->uinput_button_release(BTN_NORTH);
#else
    if (!buttons_pressed[ProInputParser::Y] && last_pressed[ProInputParser::Y])
      uinput_ctrl->uinput_button_release(BTN_NORTH);
#endif
    #endif

    for(const ProInputParser::BUTTONS &id: btns_ids){
      last_pressed[id] = buttons_pressed[id];
    }

    // do triggers here as well
    int val;
    val = buttons_pressed[ProInputParser::L2] ? 255 : 0;
    uinput_ctrl->uinput_write_single_joystick(val, ABS_Z);
    val = buttons_pressed[ProInputParser::R2] ? 255 : 0;
    uinput_ctrl->uinput_write_single_joystick(val, ABS_RZ);

    uinput_ctrl->send_report();
  }

  void uinput_manage_joysticks(const ProInputParser &parser) {
    // extract data
    uint8_t left_x;
    uint8_t left_y;
    uint8_t right_x;
    uint8_t right_y;
    parser.get_joystick_data(left_x, left_y, right_x, right_y);

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

    // left_x = 0;
    // left_y = 127;
    // right_x = 255;
    // right_y = 200;

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
    uinput_ctrl->send_report();

    // clear_console();
    // printf("left_x: %i\n", (int)left_x);
    // printf("left_y: %i\n", (int)left_y);
    // printf("right_x: %i\n", (int)right_x);
    // printf("right_y: %i\n", (int)right_y);
  }

private:
  const std::string calibration_filename = "procon_calibration_data";

  uint n_print_cycle = 1000;
  uint print_cycle_counter = 0;
  uint n_bad_data_zero = 0;
  uint n_bad_data_thirty = 0;

  bool calibrated = false;
  bool read_calibration_from_file =
      true; // will be set to false in decalibrate or with flags
  bool share_button_free = false; // used for recalibration (press share & home)

  static constexpr uint8_t center{0x7e};
  uint8_t left_x_min  = center;
  uint8_t left_y_min  = center;
  uint8_t right_x_min = center;
  uint8_t right_y_min = center;
  uint8_t left_x_max  = center;
  uint8_t left_y_max  = center;
  uint8_t right_x_max = center;
  uint8_t right_y_max = center;

  std::array<int, 18> btns_map;
  const std::array<ProInputParser::BUTTONS, 14> btns_ids{
    ProInputParser::A, ProInputParser::B,
    ProInputParser::X, ProInputParser::Y,
    ProInputParser::plus, ProInputParser::minus,
    ProInputParser::home, ProInputParser::share,
    ProInputParser::L1, ProInputParser::L2, ProInputParser::L3,
    ProInputParser::R1, ProInputParser::R2, ProInputParser::R3,
  };
  const std::array<ProInputParser::BUTTONS, 11> xbox_btns_ids{
    ProInputParser::A, ProInputParser::B,
    ProInputParser::X, ProInputParser::Y,
    ProInputParser::plus, ProInputParser::minus,
    ProInputParser::home, /*ProInputParser::share,*/
    ProInputParser::L1, /*ProInputParser::L2,*/ ProInputParser::L3,
    ProInputParser::R1, /*ProInputParser::R2,*/ ProInputParser::R3,
  };
  const std::array<ProInputParser::BUTTONS, 4> dpad_ids{
    ProInputParser::d_left, ProInputParser::d_right,
    ProInputParser::d_up, ProInputParser::d_down,
  };
  std::array<bool, 18> last_pressed{false};

  bool dribble_mode = false;

  Config config;
  HidController *hid_ctrl = nullptr;
  UInputController *uinput_ctrl = nullptr;
};

#endif
