#pragma once
#ifndef CONFIG_H
#define CONFIG_H

// #define DRIBBLE_MODE // game-specific hack. does not belong here!

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

#ifdef DRIBBLE_MODE
  int dribble_cam_value = 205;
  bool found_dribble_cam_value = false;
#endif

  Config(int argc, char *argv[]) {

    for (int i = 1; i < argc; ++i) {
      bool option_found = false;
      if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help") {
        help = true;
        option_found = true;
      }
      if (std::string(argv[i]) == "-c" ||
          std::string(argv[i]) == "--calibration") {
        force_calibration = true;
        option_found = true;
      }
      if (std::string(argv[i]) == "--version") {
        show_version = true;
        option_found = true;
      }
      if (std::string(argv[i]) == "--invert-axis" ||
          std::string(argv[i]) == "-i") {
        if (i + 1 >= argc) {
          throw std::invalid_argument("Expected axis parameter. use --help for options!");
        }
        option_found = true;
        bool valid_axis_name;
        do {
          valid_axis_name = false;
          if (std::string(argv[i + 1]) == "lx") {
            invert_lx = true;
            valid_axis_name = true;
          } else if (std::string(argv[i + 1]) == "ly") {
            invert_ly = true;
            valid_axis_name = true;
          } else if (std::string(argv[i + 1]) == "rx") {
            invert_rx = true;
            valid_axis_name = true;
          } else if (std::string(argv[i + 1]) == "ry") {
            invert_ry = true;
            valid_axis_name = true;
          } else if (std::string(argv[i + 1]) == "dx") {
            invert_dx = true;
            valid_axis_name = true;
          } else if (std::string(argv[i + 1]) == "dy") {
            invert_dy = true;
            valid_axis_name = true;
          }

          if (valid_axis_name) {
            ++i;
          }

        } while (valid_axis_name && i + 1 < argc);
      }
      if (std::string(argv[i]) == "--swap_buttons" ||
          std::string(argv[i]) == "-s") {
        option_found = true;
        swap_buttons = true;
      }
#ifdef DRIBBLE_MODE
      if (std::string(argv[i]) == "-d") {
        option_found = true;
        i++;
        dribble_cam_value = std::stoi(argv[i]);
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
