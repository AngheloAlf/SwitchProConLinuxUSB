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
#include "real_controller.hpp"
#include "real_controller_exceptions.hpp"
#include "uinputcontroller.hpp"
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
        hid_ctrl = new RealController::Controller(device_info, n_controller);
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
  ProController(const ProController &other) = delete;
  // ProController(ProController &&other) noexcept ;

  ~ProController() noexcept {
    if (hid_ctrl != nullptr) {
      delete hid_ctrl;
    }
    if (uinput_ctrl != nullptr) {
      delete uinput_ctrl;
    }
  }

  ProController &operator=(const ProController &other) = delete;
  // ProController &operator=(ProController &&other) noexcept;


  void print_sticks() const {
    for (const RealController::AXIS &id: RealController::axis_ids) {
      printf("%s %03x ", RealController::axis_name(id), axis_values[id]);
    }
  }

  void print_buttons() const {
    for (const RealController::BUTTONS &id: RealController::btns_ids) {
      if (buttons_pressed[id]) {
        printf("%s ", RealController::button_name(id));
      }
    }
  }

  void print_dpad() const {
    for (const RealController::DPAD &id: RealController::dpad_ids) {
      if (dpad_pressed[id]) {
        printf("%s ", RealController::dpad_name(id));
      }
    }
  }

  void print_calibration_values() const {
    for (const RealController::AXIS &id: RealController::axis_ids) {
      printf("%s %03x,%03x,%03x   ", RealController::axis_name(id), axis_min[id], axis_cen[id], axis_max[id]);
    }
  }

  void poll_input(long double delta_milis) {
    auto remaining_arr = uinput_ctrl->update_time(delta_milis);

    try {
      RealController::Parser parser = hid_ctrl->receive_input();
      // parser.print();
      if (!parser.has_button_and_axis_data()) {
        return;
      }
      update_input_state(parser);
    }
    catch (const RealController::PacketTypeError &e) {
      /*Utils::PrintColor::magenta();
      printf("%s\n", e.what());
      Utils::PrintColor::normal();
      return;*/
      throw;
    }

    if (buttons_pressed[RealController::home] &&
        buttons_pressed[RealController::share]) {
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

      RealController::Parser parser = hid_ctrl->receive_input();
      if (!parser.has_button_and_axis_data()) {
        return;
      }
      update_input_state(parser);
      // parser.print();

      if (!share_button_free) {
        if (!buttons_pressed[RealController::share]) {
          share_button_free = true;
        }
        return;
      }

      if (perform_calibration(parser)) {
        // send_rumble(0,255);
        calibrated = true;
        write_calibration_to_file();
        hid_ctrl->led();
        // print_calibration_values();
        // printf("\n");
      }
    }
    catch (const RealController::PacketTypeError &e) {
      /*Utils::PrintColor::magenta();
      printf("%s\n", e.what());
      Utils::PrintColor::normal();
      return;*/
      throw;
    }
  }

  void decalibrate() {
    for (const RealController::AXIS &id: RealController::axis_ids) {
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
  bool perform_calibration(const RealController::Parser &parser) {
    for (const RealController::AXIS &id: RealController::axis_ids) {
      uint16_t value = parser.get_axis_status(id);
      if (value < axis_min[id]) axis_min[id] = value;
      if (value > axis_max[id]) axis_max[id] = value;
    }

    if (!buttons_pressed[RealController::share]) {
      return false;
    }

    for (const RealController::AXIS &id: RealController::axis_ids) {
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
      for (const RealController::AXIS &id: RealController::axis_ids) {
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
    for (const RealController::AXIS &id: RealController::axis_ids) {
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
    if (dpad_pressed[RealController::d_left]) {
      x = -1;
    } else if (dpad_pressed[RealController::d_right]) {
      x = 1;
    }
    if (dpad_pressed[RealController::d_down]) {
      y = -1;
    } else if (dpad_pressed[RealController::d_up]) {
      y = 1;
    }

    uinput_ctrl->write_single_joystick(y, ABS_HAT0Y);
    uinput_ctrl->write_single_joystick(x, ABS_HAT0X);
    uinput_ctrl->send_report();
  }

  void manage_buttons() {
    for (const RealController::BUTTONS &id: xbox_btns_ids) {
      if (buttons_pressed[id] && !last_pressed[id]) {
        if (config.found_dribble_cam_value) {
          switch (id) {
          case RealController::X:
            uinput_ctrl->button_press(btns_map[RealController::X]);
            if (dribble_mode) toggle_dribble_mode(); // toggle off dribble mode
            continue;
          case RealController::Y:
            toggle_dribble_mode();
            continue;
          case RealController::share:
            uinput_ctrl->button_press(btns_map[RealController::Y]);
            continue;
          default:
            break;
          }
        }

        uinput_ctrl->button_press(btns_map[id]);
      }
    }

    for (const RealController::BUTTONS &id: xbox_btns_ids) {
      if (!buttons_pressed[id] && last_pressed[id]) {
        if (config.found_dribble_cam_value) {
          switch (id) {
          case RealController::Y:
            uinput_ctrl->button_release(btns_map[RealController::X]);
            continue;
          case RealController::share:
            uinput_ctrl->button_release(btns_map[RealController::Y]);
            continue;
          default:
            break;
          }
        }

        uinput_ctrl->button_release(btns_map[id]);
      }
    }

    // do triggers here as well
    uinput_ctrl->write_single_joystick(buttons_pressed[RealController::L2]*0xFFF, ABS_Z);
    uinput_ctrl->write_single_joystick(buttons_pressed[RealController::R2]*0xFFF, ABS_RZ);

    uinput_ctrl->send_report();
  }

  void manage_joysticks() {
    if (dribble_mode) {
      axis_values[RealController::axis_ry] = clamp_int(axis_values[RealController::axis_ry] + config.dribble_cam_value - 0x7FF);
    }

    for (const RealController::AXIS &id: RealController::axis_ids) {
      uinput_ctrl->write_single_joystick(axis_values[id], axis_map[id]);
    }

    uinput_ctrl->send_report();
  }

  void update_input_state(const RealController::Parser &parser) {
    /// Buttons
    for (const RealController::BUTTONS &id: RealController::btns_ids) {
      /// Store last state
      last_pressed[id] = buttons_pressed[id];
      /// Update value
      buttons_pressed[id] = parser.is_button_pressed(id);
    }

    /// Axis
    for (const RealController::AXIS &id: RealController::axis_ids) {
      axis_values[id] = parser.get_axis_status(id);
    }

    /// dpad
    for (const RealController::DPAD &id: RealController::dpad_ids) {
      /// Store last state
      dpad_last[id] = dpad_pressed[id];
      /// Update value
      dpad_pressed[id] = parser.is_dpad_pressed(id);
    }

    if (config.swap_ab || config.swap_buttons) {
      buttons_pressed[RealController::A] = parser.is_button_pressed(RealController::B);
      buttons_pressed[RealController::B] = parser.is_button_pressed(RealController::A);
    }
    if (config.swap_xy || config.swap_buttons) {
      buttons_pressed[RealController::X] = parser.is_button_pressed(RealController::Y);
      buttons_pressed[RealController::Y] = parser.is_button_pressed(RealController::X);
    }

    if (config.invert_dx) {
      dpad_pressed[RealController::d_left]  = parser.is_dpad_pressed(RealController::d_right);
      dpad_pressed[RealController::d_right] = parser.is_dpad_pressed(RealController::d_left);
    }
    if (config.invert_dy) {
      dpad_pressed[RealController::d_up]    = parser.is_dpad_pressed(RealController::d_down);
      dpad_pressed[RealController::d_down]  = parser.is_dpad_pressed(RealController::d_up);
    }

    if (calibrated) {
      map_sticks();
    }

    // Invert axis
    if (config.invert_lx) axis_values[RealController::axis_lx] = 0xFFF - axis_values[RealController::axis_lx];
    if (config.invert_ly) axis_values[RealController::axis_ly] = 0xFFF - axis_values[RealController::axis_ly];
    if (config.invert_rx) axis_values[RealController::axis_rx] = 0xFFF - axis_values[RealController::axis_rx];
    if (config.invert_ry) axis_values[RealController::axis_ry] = 0xFFF - axis_values[RealController::axis_ry];
  }

  void map_sticks() {
    for (const RealController::AXIS &id: RealController::axis_ids) {
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

    map[RealController::A] = BTN_EAST;
    map[RealController::B] = BTN_SOUTH;
    map[RealController::X] = BTN_WEST;
    map[RealController::Y] = BTN_NORTH;

    map[RealController::plus]  = BTN_START;
    map[RealController::minus] = BTN_SELECT;
    map[RealController::home]  = BTN_MODE;
    // map[RealController::share] = ;

    map[RealController::L1] = BTN_TL;
    map[RealController::L2] = BTN_TL2;
    map[RealController::L3] = BTN_THUMBL;
    map[RealController::R1] = BTN_TR;
    map[RealController::R2] = BTN_TR2;
    map[RealController::R3] = BTN_THUMBR;

    return map;
  }

  std::array<int, 4> make_axis_map() const {
    std::array<int, 4> map {0};
    map[RealController::axis_lx] = ABS_X;
    map[RealController::axis_ly] = ABS_Y;
    map[RealController::axis_rx] = ABS_RX;
    map[RealController::axis_ry] = ABS_RY;
    return map;
  }

  std::array<int, 4> make_dpad_map() const {
    std::array<int, 4> map {0};
    map[RealController::d_left]  = BTN_DPAD_LEFT;
    map[RealController::d_right] = BTN_DPAD_RIGHT;
    map[RealController::d_up]    = BTN_DPAD_UP;
    map[RealController::d_down]  = BTN_DPAD_DOWN;
    return map;
  }

  const std::string calibration_filename = "procon_calibration_data";

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
  const std::array<RealController::BUTTONS, 12> xbox_btns_ids{
    RealController::A, RealController::B,
    RealController::X, RealController::Y,
    RealController::plus, RealController::minus, RealController::home, 
    RealController::share, /// Allow it to be used for dribble mode.
    RealController::L1, /*RealController::L2,*/ RealController::L3,
    RealController::R1, /*RealController::R2,*/ RealController::R3,
  };
  std::array<bool, 14> buttons_pressed{false};
  std::array<bool, 14> last_pressed{false};

  std::array<int, 4> dpad_map = make_dpad_map();
  std::array<bool, 4> dpad_pressed{false};
  std::array<bool, 4> dpad_last{false};

  bool dribble_mode = false;

  Config config;
  RealController::Controller *hid_ctrl = nullptr;
  UInputController *uinput_ctrl = nullptr;
};

#endif
