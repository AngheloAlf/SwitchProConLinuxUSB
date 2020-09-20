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

  void print_sticks() const {
    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      printf("%s %03i ", ProInputParser::axis_name(id), axis_values[id]);
    }
  }

  void print_buttons() const {
    for (const ProInputParser::BUTTONS &id: ProInputParser::btns_ids) {
      if (buttons_pressed[id]) {
        printf("%s ", ProInputParser::button_name(id));
      }
    }
  }

  void print_dpad() const {
    for (const ProInputParser::DPAD &id: ProInputParser::dpad_ids) {
      if (dpad_pressed[id]) {
        printf("%s ", ProInputParser::dpad_name(id));
      }
    }
  }

  void print_calibration_values() const {
    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      printf("%s %03i,%03i   ", ProInputParser::axis_name(id), axis_min[id], axis_max[id]);
    }
  }

  void poll_input() {
    // print_cycle_counter++;
    // if(print_cycle_counter++ > n_print_cycle) {
    //     timer();
    // }
    hid_ctrl->led();

    ProInputParser parser = hid_ctrl->request_input();
    if (parser.detect_useless_data()) {
      return;
    }
    update_input_state(parser);

    if (buttons_pressed[ProInputParser::home] &&
        buttons_pressed[ProInputParser::share]) {
      decalibrate();
      return;
    }

    manage_buttons();
    manage_joysticks();
    manage_dpad();

    // parser.print();
    return;
  }

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

    ProInputParser parser = hid_ctrl->request_input();
    if (parser.detect_useless_data()) {
      return;
    }

    update_input_state(parser);
    // parser.print();

    if (!share_button_free) {
      if (!parser.is_button_pressed(ProInputParser::share)) {
        share_button_free = true;
      }
    } else {
      if (do_calibrate(parser)) {
        // send_rumble(0,255);
        calibrated = true;
        hid_ctrl->led();
        write_calibration_to_file();
      }
    }
  }

  bool do_calibrate(const ProInputParser &parser) {
    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      uint8_t value = parser.get_axis_status(id);
      if (value < axis_min[id]) axis_min[id] = value;
      if (value > axis_max[id]) axis_max[id] = value;
    }
    // print_calibration_values();

    return buttons_pressed[ProInputParser::share];
  }

  void decalibrate() {
    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      axis_min[id] = center;
      axis_max[id] = center;
    }

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

  bool is_calibrated() const {
    return calibrated;
  }

  bool needs_first_calibration() const {
    return !read_calibration_from_file || !calibration_file_exists();
  }

  bool calibration_file_exists() const {
    std::ifstream conf(calibration_filename);
    return conf.good();
  }

private:
  bool read_calibration_file() {
    bool file_readed = false;
    std::ifstream myReadFile;
    myReadFile.open(calibration_filename,
                    std::ios::in | std::ios::binary);
    if (myReadFile) {
      for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
        myReadFile.read((char *)&axis_min[id], sizeof(uint8_t));
        myReadFile.read((char *)&axis_max[id], sizeof(uint8_t));
      }
      file_readed = true;
    }

    myReadFile.close();
    return file_readed;
  }

  void write_calibration_to_file() {
    std::ofstream calibration_file;
    calibration_file.open(calibration_filename,
                          std::ios::out | std::ios::binary);
    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      calibration_file.write((char *)&axis_min[id], sizeof(uint8_t));
      calibration_file.write((char *)&axis_max[id], sizeof(uint8_t));
    }
    calibration_file.close();
  }

  //-------------------------
  //         UINPUT
  //-------------------------

  void manage_dpad() {
    int x = 0, y = 0;
    if (dpad_pressed[ProInputParser::d_left]) {
      x = -1;
    } else if (dpad_pressed[ProInputParser::d_right]) {
      x = 1;
    }
    if (dpad_pressed[ProInputParser::d_down]) {
      y = -1;
    } else if (dpad_pressed[ProInputParser::d_up]) {
      y = 1;
    }

    uinput_ctrl->write_single_joystick(y, ABS_HAT0Y);
    uinput_ctrl->write_single_joystick(x, ABS_HAT0X);
    uinput_ctrl->send_report();
  }

  void manage_buttons() {
    for (const ProInputParser::BUTTONS &id: xbox_btns_ids) {
      if (buttons_pressed[id] && !last_pressed[id]) {
        if (config.found_dribble_cam_value) {
          switch (id) {
          case ProInputParser::X:
            uinput_ctrl->button_press(btns_map[ProInputParser::X]);
            if (dribble_mode) toggle_dribble_mode(); // toggle off dribble mode
            continue;
          case ProInputParser::Y:
            toggle_dribble_mode();
            continue;
          case ProInputParser::share:
            uinput_ctrl->button_press(btns_map[ProInputParser::Y]);
            continue;
          default:
            break;
          }
        }

        uinput_ctrl->button_press(btns_map[id]);
      }
    }

    for (const ProInputParser::BUTTONS &id: xbox_btns_ids) {
      if (!buttons_pressed[id] && last_pressed[id]) {
        if (config.found_dribble_cam_value) {
          switch (id) {
          case ProInputParser::Y:
            uinput_ctrl->button_release(btns_map[ProInputParser::X]);
            continue;
          case ProInputParser::share:
            uinput_ctrl->button_release(btns_map[ProInputParser::Y]);
            continue;
          default:
            break;
          }
        }

        uinput_ctrl->button_release(btns_map[id]);
      }
    }

    // do triggers here as well
    uinput_ctrl->write_single_joystick(buttons_pressed[ProInputParser::L2]*255, ABS_Z);
    uinput_ctrl->write_single_joystick(buttons_pressed[ProInputParser::R2]*255, ABS_RZ);

    uinput_ctrl->send_report();
  }

  void manage_joysticks() {
    if (dribble_mode) {
      axis_values[ProInputParser::axis_ry] = clamp_int(axis_values[ProInputParser::axis_ry] + config.dribble_cam_value - 127);
    }

    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      uinput_ctrl->write_single_joystick(axis_values[id], axis_map[id]);
    }

    uinput_ctrl->send_report();
  }

  void update_input_state(const ProInputParser &parser) {
    /// Buttons
    for (const ProInputParser::BUTTONS &id: ProInputParser::btns_ids) {
      /// Store last state
      last_pressed[id] = buttons_pressed[id];
      /// Update value
      buttons_pressed[id] = parser.is_button_pressed(id);
    }

    /// Axis
    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      axis_values[id] = parser.get_axis_status(id);
    }

    /// dpad
    for (const ProInputParser::DPAD &id: ProInputParser::dpad_ids) {
      /// Store last state
      dpad_last[id] = dpad_pressed[id];
      /// Update value
      dpad_pressed[id] = parser.is_dpad_pressed(id);
    }

    if (config.swap_buttons) {
      buttons_pressed[ProInputParser::A] = parser.is_button_pressed(ProInputParser::B);
      buttons_pressed[ProInputParser::B] = parser.is_button_pressed(ProInputParser::A);
      buttons_pressed[ProInputParser::X] = parser.is_button_pressed(ProInputParser::Y);
      buttons_pressed[ProInputParser::Y] = parser.is_button_pressed(ProInputParser::X);
    }

    if (config.invert_dx) {
      dpad_pressed[ProInputParser::d_left]  = parser.is_dpad_pressed(ProInputParser::d_right);
      dpad_pressed[ProInputParser::d_right] = parser.is_dpad_pressed(ProInputParser::d_left);
    }
    if (config.invert_dy) {
      dpad_pressed[ProInputParser::d_up]    = parser.is_dpad_pressed(ProInputParser::d_down);
      dpad_pressed[ProInputParser::d_down]  = parser.is_dpad_pressed(ProInputParser::d_up);
    }

    map_sticks();

    // Invert axis
    if (config.invert_lx) axis_values[ProInputParser::axis_lx] = 255 - axis_values[ProInputParser::axis_lx];
    if (config.invert_ly) axis_values[ProInputParser::axis_ly] = 255 - axis_values[ProInputParser::axis_ly];
    if (config.invert_rx) axis_values[ProInputParser::axis_rx] = 255 - axis_values[ProInputParser::axis_rx];
    if (config.invert_ry) axis_values[ProInputParser::axis_ry] = 255 - axis_values[ProInputParser::axis_ry];
  }

  void map_sticks() {
    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      axis_values[id] = clamp((float)(axis_values[id] - axis_min[id]) /
                              (float)(axis_max[id] - axis_min[id]) * 255.f);
    }
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

  void toggle_dribble_mode() {
    dribble_mode = !dribble_mode; 
  }

  std::array<int, 14> make_button_map() const {
    std::array<int, 14> map {0};

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

  std::array<int, 4> make_axis_map() const {
    std::array<int, 4> map {0};
    map[ProInputParser::axis_lx] = ABS_X;
    map[ProInputParser::axis_ly] = ABS_Y;
    map[ProInputParser::axis_rx] = ABS_RX;
    map[ProInputParser::axis_ry] = ABS_RY;
    return map;
  }

  std::array<int, 4> make_dpad_map() const {
    std::array<int, 4> map {0};
    map[ProInputParser::d_left]  = BTN_DPAD_LEFT;
    map[ProInputParser::d_right] = BTN_DPAD_RIGHT;
    map[ProInputParser::d_up]    = BTN_DPAD_UP;
    map[ProInputParser::d_down]  = BTN_DPAD_DOWN;
    return map;
  }

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
  std::array<uint8_t, 4> axis_min{center};
  std::array<uint8_t, 4> axis_max{center};

  std::array<int, 4> axis_map = make_axis_map();
  std::array<uint8_t, 4> axis_values{center};

  std::array<int, 14> btns_map = make_button_map();
  const std::array<ProInputParser::BUTTONS, 12> xbox_btns_ids{
    ProInputParser::A, ProInputParser::B,
    ProInputParser::X, ProInputParser::Y,
    ProInputParser::plus, ProInputParser::minus, ProInputParser::home, 
    ProInputParser::share, /// Allow it to be used for dribble mode.
    ProInputParser::L1, /*ProInputParser::L2,*/ ProInputParser::L3,
    ProInputParser::R1, /*ProInputParser::R2,*/ ProInputParser::R3,
  };
  std::array<bool, 14> buttons_pressed{false};
  std::array<bool, 14> last_pressed{false};

  std::array<int, 4> dpad_map = make_dpad_map();
  std::array<bool, 4> dpad_pressed{false};
  std::array<bool, 4> dpad_last{false};

  bool dribble_mode = false;

  Config config;
  HidController *hid_ctrl = nullptr;
  UInputController *uinput_ctrl = nullptr;
};

#endif
