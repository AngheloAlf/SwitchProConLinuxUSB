#ifndef PROCON_DRIVER_H
#define PROCON_DRIVER_H

#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <linux/input.h>
#include <ratio>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <filesystem>

#include "config.hpp"
#include "real_controller.hpp"
#include "real_controller_exceptions.hpp"
#include "virtual_controller.hpp"
#include "utils.hpp"

#define PROCON_DRIVER_VERSION "2.0"


#define PROCON_ID 0x2009
#define NINTENDO_ID 0x057E

#define MAX_N_CONTROLLERS 4

class ProController {
public:
  ProController(unsigned short n_controller, const HidApi::Enumerate &device_info, 
                Config &cfg): config(cfg), hid_ctrl(device_info, n_controller), uinput_ctrl() {
    if (config.force_calibration) {
      read_calibration_from_file = false;
    }

    const char* home = getenv("HOME");
    calibration_path = std::string(home) + calibration_path___;
  }

  void print_sticks() const {
    for (const RealController::Axis &id: RealController::axis_ids) {
      printf("%s %03x ", RealController::axis_name(id), axis_values[id]);
    }
  }

  void print_buttons() const {
    for (const RealController::Buttons &id: RealController::btns_ids) {
      if (buttons_pressed[id]) {
        printf("%s ", RealController::button_name(id));
      }
    }
  }

  void print_dpad() const {
    for (const RealController::Dpad &id: RealController::dpad_ids) {
      if (dpad_pressed[id]) {
        printf("%s ", RealController::dpad_name(id));
      }
    }
  }

  void print_calibration_values() const {
    for (const RealController::Axis &id: RealController::axis_ids) {
      printf("%s %03x,%03x,%03x   ", RealController::axis_name(id), axis_min[id], axis_cen[id], axis_max[id]);
    }
  }

  void poll_input(long double delta_milis) {
    uinput_ctrl.update_time(delta_milis);

    RealController::Parser parser = hid_ctrl.receive_input();
    if (!parser.has_button_and_axis_data()) {
      return;
    }
    update_input_state(parser);

    if (buttons_pressed[RealController::Buttons::home] &&
        buttons_pressed[RealController::Buttons::share]) {
      decalibrate();
      return;
    }

    manage_rumble();

    manage_buttons();
    manage_joysticks();
    manage_dpad();

    return;
  }

  void calibrate_from_file() {
    if (read_calibration_from_file) {
      if (read_calibration_file()) {
        calibrated = true;
      }
    }
  }

  void calibrate() {
    hid_ctrl.blink();

    RealController::Parser parser = hid_ctrl.receive_input();
    if (!parser.has_button_and_axis_data()) {
      return;
    }
    update_input_state(parser);

    if (!share_button_free) {
      if (!buttons_pressed[RealController::Buttons::share]) {
        share_button_free = true;
      }
      return;
    }

    if (perform_calibration(parser)) {
      calibrated = true;
      write_calibration_to_file();
      hid_ctrl.led();
    }
  }

  void decalibrate() {
    for (const RealController::Axis &id: RealController::axis_ids) {
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
    std::ifstream conf(calibration_path + calibration_filename);
    return conf.good();
  }

private:
  bool perform_calibration(const RealController::Parser &parser) {
    for (const RealController::Axis &id: RealController::axis_ids) {
      uint16_t value = parser.get_axis_status(id);
      if (value < axis_min[id]) axis_min[id] = value;
      if (value > axis_max[id]) axis_max[id] = value;
    }

    if (!buttons_pressed[RealController::Buttons::share]) {
      return false;
    }

    for (const RealController::Axis &id: RealController::axis_ids) {
      axis_cen[id] = parser.get_axis_status(id);
    }

    return true;
  }

  bool read_calibration_file() {
    bool file_readed = false;
    std::ifstream myReadFile;
    myReadFile.open(calibration_path + calibration_filename,
                    std::ios::in | std::ios::binary);
    if (myReadFile) {
      for (const RealController::Axis &id: RealController::axis_ids) {
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
    std::filesystem::create_directories(calibration_path);
    std::ofstream calibration_file;
    calibration_file.open(calibration_path + calibration_filename,
                          std::ios::out | std::ios::binary);
    for (const RealController::Axis &id: RealController::axis_ids) {
      calibration_file.write((char *)&axis_min[id], sizeof(uint16_t));
      calibration_file.write((char *)&axis_max[id], sizeof(uint16_t));
      calibration_file.write((char *)&axis_cen[id], sizeof(uint16_t));
    }
    calibration_file.close();
  }


  void manage_rumble() {
    for(const auto &effect: uinput_ctrl.getRumbleEffects()) {
      if (effect->get_remaining() > 0) {
        auto data = effect->get_data();
        if (data.strong) {
          hid_ctrl.rumble(320, 160, data.strong/(double)0x10000);
        }
        else if(data.weak) {
          hid_ctrl.rumble(120, 80, data.weak/(double)0x10000);
        }
      }
    }

    uinput_ctrl.update_state();
  }

  //-------------------------
  //         UINPUT
  //-------------------------

  void manage_dpad() {
    int x = 0, y = 0;
    if (dpad_pressed[RealController::Dpad::d_left]) {
      x = -1;
    } else if (dpad_pressed[RealController::Dpad::d_right]) {
      x = 1;
    }
    if (dpad_pressed[RealController::Dpad::d_down]) {
      y = -1;
    } else if (dpad_pressed[RealController::Dpad::d_up]) {
      y = 1;
    }

    uinput_ctrl.write_single_joystick(y, ABS_HAT0Y);
    uinput_ctrl.write_single_joystick(x, ABS_HAT0X);
    uinput_ctrl.send_report();
  }

  void manage_buttons() {
    for (const RealController::Buttons &id: xbox_btns_ids) {
      if (buttons_pressed[id] && !last_pressed[id]) {
        if (config.found_dribble_cam_value) {
          switch (id) {
          case RealController::Buttons::X:
            uinput_ctrl.button_press(btns_map[RealController::Buttons::X]);
            if (dribble_mode) toggle_dribble_mode(); // toggle off dribble mode
            continue;
          case RealController::Buttons::Y:
            toggle_dribble_mode();
            continue;
          case RealController::Buttons::share:
            uinput_ctrl.button_press(btns_map[RealController::Buttons::Y]);
            continue;
          default:
            break;
          }
        }

        uinput_ctrl.button_press(btns_map[id]);
      }
    }

    for (const RealController::Buttons &id: xbox_btns_ids) {
      if (!buttons_pressed[id] && last_pressed[id]) {
        if (config.found_dribble_cam_value) {
          switch (id) {
          case RealController::Buttons::Y:
            uinput_ctrl.button_release(btns_map[RealController::Buttons::X]);
            continue;
          case RealController::Buttons::share:
            uinput_ctrl.button_release(btns_map[RealController::Buttons::Y]);
            continue;
          default:
            break;
          }
        }

        uinput_ctrl.button_release(btns_map[id]);
      }
    }

    // do triggers here as well
    uinput_ctrl.write_single_joystick(buttons_pressed[RealController::Buttons::L2]*0xFFF, ABS_Z);
    uinput_ctrl.write_single_joystick(buttons_pressed[RealController::Buttons::R2]*0xFFF, ABS_RZ);

    uinput_ctrl.send_report();
  }

  void manage_joysticks() {
    if (dribble_mode) {
      axis_values[RealController::Axis::axis_ry] = Utils::Number::clamp<uint16_t>(axis_values[RealController::Axis::axis_ry] + config.dribble_cam_value - 0x7FF, 0x000, 0xFFF);
    }

    for (const RealController::Axis &id: RealController::axis_ids) {
      uinput_ctrl.write_single_joystick(axis_values[id], axis_map[id]);
    }

    uinput_ctrl.send_report();
  }

  void update_input_state(const RealController::Parser &parser) {
    /// Buttons
    for (const RealController::Buttons &id: RealController::btns_ids) {
      /// Store last state
      last_pressed[id] = buttons_pressed[id];
      /// Update value
      buttons_pressed[id] = parser.is_button_pressed(id);
    }

    /// Axis
    for (const RealController::Axis &id: RealController::axis_ids) {
      axis_values[id] = parser.get_axis_status(id);
    }

    /// dpad
    for (const RealController::Dpad &id: RealController::dpad_ids) {
      /// Store last state
      dpad_last[id] = dpad_pressed[id];
      /// Update value
      dpad_pressed[id] = parser.is_dpad_pressed(id);
    }

    if (config.swap_ab || config.swap_buttons) {
      buttons_pressed[RealController::Buttons::A] = parser.is_button_pressed(RealController::Buttons::B);
      buttons_pressed[RealController::Buttons::B] = parser.is_button_pressed(RealController::Buttons::A);
    }
    if (config.swap_xy || config.swap_buttons) {
      buttons_pressed[RealController::Buttons::X] = parser.is_button_pressed(RealController::Buttons::Y);
      buttons_pressed[RealController::Buttons::Y] = parser.is_button_pressed(RealController::Buttons::X);
    }

    if (config.invert_dx) {
      dpad_pressed[RealController::Dpad::d_left]  = parser.is_dpad_pressed(RealController::Dpad::d_right);
      dpad_pressed[RealController::Dpad::d_right] = parser.is_dpad_pressed(RealController::Dpad::d_left);
    }
    if (config.invert_dy) {
      dpad_pressed[RealController::Dpad::d_up]    = parser.is_dpad_pressed(RealController::Dpad::d_down);
      dpad_pressed[RealController::Dpad::d_down]  = parser.is_dpad_pressed(RealController::Dpad::d_up);
    }

    if (calibrated) {
      map_sticks();
    }

    // Invert axis
    if (config.invert_lx) axis_values[RealController::Axis::axis_lx] = 0xFFF - axis_values[RealController::Axis::axis_lx];
    if (config.invert_ly) axis_values[RealController::Axis::axis_ly] = 0xFFF - axis_values[RealController::Axis::axis_ly];
    if (config.invert_rx) axis_values[RealController::Axis::axis_rx] = 0xFFF - axis_values[RealController::Axis::axis_rx];
    if (config.invert_ry) axis_values[RealController::Axis::axis_ry] = 0xFFF - axis_values[RealController::Axis::axis_ry];
  }

  void map_sticks() {
    for (const RealController::Axis &id: RealController::axis_ids) {
      long double val;
      if (axis_values[id] < axis_cen[id]) {
        val = (long double)(axis_values[id] - axis_min[id]) /
              (long double)(axis_cen[id] - axis_min[id]) / 2.L;
      } else {
        val = (long double)(axis_values[id] - axis_cen[id]) /
              (long double)(axis_max[id] - axis_cen[id]) / 2.L;
        val += 0.5L;
      }
      axis_values[id] = Utils::Number::clamp<uint16_t>(val * 0xFFF, 0x000, 0xFFF);
    }
  }

  void toggle_dribble_mode() {
    dribble_mode = !dribble_mode; 
  }

  std::array<int, 14> make_button_map() const {
    std::array<int, 14> map {0};

    map[RealController::Buttons::A] = BTN_EAST;
    map[RealController::Buttons::B] = BTN_SOUTH;
    map[RealController::Buttons::X] = BTN_WEST;
    map[RealController::Buttons::Y] = BTN_NORTH;

    map[RealController::Buttons::plus]  = BTN_START;
    map[RealController::Buttons::minus] = BTN_SELECT;
    map[RealController::Buttons::home]  = BTN_MODE;
    // map[RealController::Buttons::share] = ;

    map[RealController::Buttons::L1] = BTN_TL;
    map[RealController::Buttons::L2] = BTN_TL2;
    map[RealController::Buttons::L3] = BTN_THUMBL;
    map[RealController::Buttons::R1] = BTN_TR;
    map[RealController::Buttons::R2] = BTN_TR2;
    map[RealController::Buttons::R3] = BTN_THUMBR;

    return map;
  }

  std::array<int, 4> make_axis_map() const {
    std::array<int, 4> map {0};
    map[RealController::Axis::axis_lx] = ABS_X;
    map[RealController::Axis::axis_ly] = ABS_Y;
    map[RealController::Axis::axis_rx] = ABS_RX;
    map[RealController::Axis::axis_ry] = ABS_RY;
    return map;
  }

  std::array<int, 4> make_dpad_map() const {
    std::array<int, 4> map {0};
    map[RealController::Dpad::d_left]  = BTN_DPAD_LEFT;
    map[RealController::Dpad::d_right] = BTN_DPAD_RIGHT;
    map[RealController::Dpad::d_up]    = BTN_DPAD_UP;
    map[RealController::Dpad::d_down]  = BTN_DPAD_DOWN;
    return map;
  }

  std::string calibration_path;
  const std::string calibration_path___ = "/.config/procon_driver/";
  const std::string calibration_filename = "procon_calibration_data.bin";

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
  const std::array<RealController::Buttons, 12> xbox_btns_ids{
    RealController::Buttons::A, RealController::Buttons::B,
    RealController::Buttons::X, RealController::Buttons::Y,
    RealController::Buttons::plus, RealController::Buttons::minus, RealController::Buttons::home, 
    RealController::Buttons::share, /// Allow it to be used for dribble mode.
    RealController::Buttons::L1, /*RealController::Buttons::L2,*/ RealController::Buttons::L3,
    RealController::Buttons::R1, /*RealController::Buttons::R2,*/ RealController::Buttons::R3,
  };
  std::array<bool, 14> buttons_pressed{false};
  std::array<bool, 14> last_pressed{false};

  std::array<int, 4> dpad_map = make_dpad_map();
  std::array<bool, 4> dpad_pressed{false};
  std::array<bool, 4> dpad_last{false};

  bool dribble_mode = false;

  Config &config;
  RealController::Controller hid_ctrl;
  VirtualController::Controller uinput_ctrl;
};

#endif
