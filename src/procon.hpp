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
#include "hidcontroller.hpp"
#include "uinputcontroller.hpp"
#include "proinputparser.hpp"
#include "utils.hpp"

#define PROCON_DRIVER_VERSION "1.0 alpha2"


#define PROCON_ID 0x2009
#define NINTENDO_ID 0x057E

#define MAX_N_CONTROLLERS 4

class ProController {
public:
  ProController(unsigned short n_controller, const HidApi::Enumerate &device_info, 
                Config &cfg): config(cfg){
    if (config.force_calibration) {
      read_calibration_from_file = false;
    }

    bool opened = false;
    int retries = 0;
    while(!opened){
      try {
        hid_ctrl = new HidController(device_info, n_controller);
        opened = true;
      } catch (const HidApi::OpenError &e) {
        throw;
      } catch (const std::runtime_error &e) {
        ++retries;
        if (retries > 10) {
          throw;
        }
        usleep(1000 * 1000);
        Utils::PrintColor::red();
        printf("%s\nRetrying... (%i/%i)\n\n", e.what(), retries, 10);
        Utils::PrintColor::normal();
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
      printf("%s %03x ", ProInputParser::axis_name(id), axis_values[id]);
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
      printf("%s %03x,%03x,%03x   ", ProInputParser::axis_name(id), axis_min[id], axis_cen[id], axis_max[id]);
    }
  }

  void poll_input(long double delta_milis) {
    // print_cycle_counter++;
    // if(print_cycle_counter++ > n_print_cycle) {
    //     timer();
    // }
    auto remaining_arr = uinput_ctrl->update_time(delta_milis);

    try {
      ProInputParser::Parser parser = hid_ctrl->request_input();
      // parser.print();
      if (parser.detect_useless_data()) {
        return;
      }
      if (!parser.has_button_and_axis_data()) {
        return;
      }
      update_input_state(parser);
    }
    catch (const ProInputParser::PacketLengthError &e) {
      /*PrintColor::magenta();
      printf("%s\n", e.what());
      PrintColor::normal();*/
      return;
    }
    catch (const ProInputParser::PacketTypeError &e) {
      /*PrintColor::magenta();
      printf("%s\n", e.what());
      PrintColor::normal();
      return;*/
      throw;
    }

    if (buttons_pressed[ProInputParser::home] &&
        buttons_pressed[ProInputParser::share]) {
      decalibrate();
      return;
    }

    for(const int32_t &remaining: remaining_arr) {
      if (remaining > 0) {
        //printf("%04i ms\n", remaining);
        //printf("%04i ms  %12.2Lf ms\n", remaining, delta_milis);
        hid_ctrl->rumble();
      }
    }

    uinput_ctrl->update_state();

    manage_buttons();
    manage_joysticks();
    manage_dpad();

    // parser.print();
    return;
  }

  void calibrate_from_file() {
    if (read_calibration_from_file) {
      if (read_calibration_file()) {
        calibrated = true;
        // send_rumble(0,255);
        // hid_ctrl->led();
      }
    }
  }

  void calibrate() {
    try {
      hid_ctrl->blink();

      ProInputParser::Parser parser = hid_ctrl->request_input();
      if (parser.detect_useless_data()) {
        return;
      }

      if (!parser.has_button_and_axis_data()) {
        return;
      }
      update_input_state(parser);
      // parser.print();

      if (!share_button_free) {
        if (!buttons_pressed[ProInputParser::share]) {
          share_button_free = true;
        }
        return;
      }

      if (perform_calibration(parser)) {
        // send_rumble(0,255);
        calibrated = true;
        hid_ctrl->led();
        write_calibration_to_file();
        // print_calibration_values();
        // printf("\n");
      }
    }
    catch (const ProInputParser::PacketLengthError &e) {
      /*PrintColor::magenta();
      printf("%s\n", e.what());
      PrintColor::normal();*/
      return;
    }
    catch (const ProInputParser::PacketTypeError &e) {
      /*PrintColor::magenta();
      printf("%s\n", e.what());
      PrintColor::normal();
      return;*/
      throw;
    }
  }

  void decalibrate() {
    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      axis_min[id] = center;
      axis_max[id] = center;
      axis_cen[id] = center;
    }

    calibrated = false;
    read_calibration_from_file = false;
    share_button_free = false;
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
  bool perform_calibration(const ProInputParser::Parser &parser) {
    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      uint16_t value = parser.get_axis_status(id);
      if (value < axis_min[id]) axis_min[id] = value;
      if (value > axis_max[id]) axis_max[id] = value;
    }

    if (!buttons_pressed[ProInputParser::share]) {
      return false;
    }

    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      axis_cen[id] = parser.get_axis_status(id);
    }

    return true;
  }

  bool read_calibration_file() {
    bool file_readed = false;
    std::ifstream myReadFile;
    myReadFile.open(calibration_filename,
                    std::ios::in | std::ios::binary);
    if (myReadFile) {
      for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
        myReadFile.read((char *)&axis_min[id], sizeof(uint16_t));
        myReadFile.read((char *)&axis_max[id], sizeof(uint16_t));
        myReadFile.read((char *)&axis_cen[id], sizeof(uint16_t));
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
      calibration_file.write((char *)&axis_min[id], sizeof(uint16_t));
      calibration_file.write((char *)&axis_max[id], sizeof(uint16_t));
      calibration_file.write((char *)&axis_cen[id], sizeof(uint16_t));
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
    uinput_ctrl->write_single_joystick(buttons_pressed[ProInputParser::L2]*0xFFF, ABS_Z);
    uinput_ctrl->write_single_joystick(buttons_pressed[ProInputParser::R2]*0xFFF, ABS_RZ);

    uinput_ctrl->send_report();
  }

  void manage_joysticks() {
    if (dribble_mode) {
      axis_values[ProInputParser::axis_ry] = clamp_int(axis_values[ProInputParser::axis_ry] + config.dribble_cam_value - 0x7FF);
    }

    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      uinput_ctrl->write_single_joystick(axis_values[id], axis_map[id]);
    }

    uinput_ctrl->send_report();
  }

  void update_input_state(const ProInputParser::Parser &parser) {
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

    if (config.swap_ab || config.swap_buttons) {
      buttons_pressed[ProInputParser::A] = parser.is_button_pressed(ProInputParser::B);
      buttons_pressed[ProInputParser::B] = parser.is_button_pressed(ProInputParser::A);
    }
    if (config.swap_xy || config.swap_buttons) {
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

    if (calibrated) {
      map_sticks();
    }

    // Invert axis
    if (config.invert_lx) axis_values[ProInputParser::axis_lx] = 0xFFF - axis_values[ProInputParser::axis_lx];
    if (config.invert_ly) axis_values[ProInputParser::axis_ly] = 0xFFF - axis_values[ProInputParser::axis_ly];
    if (config.invert_rx) axis_values[ProInputParser::axis_rx] = 0xFFF - axis_values[ProInputParser::axis_rx];
    if (config.invert_ry) axis_values[ProInputParser::axis_ry] = 0xFFF - axis_values[ProInputParser::axis_ry];
  }

  void map_sticks() {
    for (const ProInputParser::AXIS &id: ProInputParser::axis_ids) {
      long double val;
      if (axis_values[id] < axis_cen[id]) {
        val = (long double)(axis_values[id] - axis_min[id]) /
              (long double)(axis_cen[id] - axis_min[id]) / 2.L;
      } else {
        val = (long double)(axis_values[id] - axis_cen[id]) /
              (long double)(axis_max[id] - axis_cen[id]) / 2.L;
        val += 0.5L;
      }
      axis_values[id] = clamp(val * 0xFFF);
    }
  }

  static uint16_t clamp(long double inp) {
    if (inp < 0.5f)
      return 0;
    if (inp > 4094.5f) {
      return 0xFFF;
    }
    return inp;
  }
  static int clamp_int(int inp) {
    if (inp < 0)
      return 0;
    if (inp > 0xFFF) {
      return 0xFFF;
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

  static constexpr uint16_t center{0x7ff};
  std::array<uint16_t, 4> axis_min{center};
  std::array<uint16_t, 4> axis_max{center};
  std::array<uint16_t, 4> axis_cen{center};

  std::array<int, 4> axis_map = make_axis_map();
  std::array<uint16_t, 4> axis_values{center};

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
