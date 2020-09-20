#pragma once
#ifndef CONFIG_H
#define CONFIG_H

// #define DRIBBLE_MODE // game-specific hack. does not belong here!

#include <cstring>
#include <string>
#include <stdexcept>

class Config{
public:
  bool help = false;
  bool force_calibration = false;
  bool show_version = false;
  bool invert_lx = false;
  bool invert_ly = false;
  bool invert_rx = false;
  bool invert_ry = false;
  bool invert_dx = false;
  bool invert_dy = false;
  bool swap_buttons = false;
  bool print_axis = false;
  bool print_buttons = false;
  bool print_dpad = false;

  int dribble_cam_value = 205;
  bool found_dribble_cam_value = false;

  Config(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
      bool option_found = false;
      if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
        help = true;
        option_found = true;
      }
      if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--calibration")) {
        force_calibration = true;
        option_found = true;
      }
      if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
        show_version = true;
        option_found = true;
      }
      if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--invert-axis")) {
        if (i + 1 >= argc) {
          throw std::invalid_argument("Expected axis parameter. use --help for options!");
        }
        option_found = true;
        bool valid_axis_name;
        do {
          valid_axis_name = false;
          if (!strcmp(argv[i+1], "lx")) {
            invert_lx = true;
            valid_axis_name = true;
          } else if (!strcmp(argv[i+1], "ly")) {
            invert_ly = true;
            valid_axis_name = true;
          } else if (!strcmp(argv[i+1], "rx")) {
            invert_rx = true;
            valid_axis_name = true;
          } else if (!strcmp(argv[i+1], "ry")) {
            invert_ry = true;
            valid_axis_name = true;
          } else if (!strcmp(argv[i+1], "dx")) {
            invert_dx = true;
            valid_axis_name = true;
          } else if (!strcmp(argv[i+1], "dy")) {
            invert_dy = true;
            valid_axis_name = true;
          }

          if (valid_axis_name) {
            ++i;
          }
        } while (valid_axis_name && i + 1 < argc);
      }
      if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--swap_buttons")) {
        option_found = true;
        swap_buttons = true;
      }
      if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--print-state")) {
        if (i + 1 >= argc) {
          throw std::invalid_argument("Expected parameter. Use --help for options!");
        }
        option_found = true;
        bool valid_parameter;
        do {
          valid_parameter = true;
          if (!strcmp(argv[i+1], "a")) {
            print_axis = true;
          } else if (!strcmp(argv[i+1], "b")) {
            print_buttons = true;
          } else if (!strcmp(argv[i+1], "d")) {
            print_dpad = true;
          } else {
            valid_parameter = false;
          }

          if (valid_parameter) {
            ++i;
          }
        } while (valid_parameter && i + 1 < argc);
      }
      #ifdef DRIBBLE_MODE
      if (!strcmp(argv[i], "-d")) {
        option_found = true;
        if (i+1 < argc && isdigit(argv[i+1][0])) {
          i++;
          dribble_cam_value = std::stoi(argv[i]);
          if (dribble_cam_value < 0 || dribble_cam_value > 255) {
            throw std::domain_error("Dribble cam value out of range. "
                                    "Expected value in [0, 255], got "
                                    + std::to_string(dribble_cam_value) + ".");
          }
        }
        found_dribble_cam_value = true;
      }
      #endif
      if (!option_found) {
        throw std::invalid_argument("Unknown option " + std::string(argv[i]) + ". For usage, type './procon_driver --help'");
      }
    }

  }

};

#endif
