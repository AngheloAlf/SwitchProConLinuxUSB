#include "procon.hpp"
#include "config.hpp"
#include "utils.hpp"

#include <chrono>
#include <thread>
#include <signal.h>

//#define DEBUG

bool controller_loop = true;
void exit_handler(int ignored){
  if (controller_loop == false) {
    fflush(stdout);
    Utils::PrintColor::red(stdout, "\n\nHard exit.\n");
    fflush(stdout);
    signal(SIGINT, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
    raise(SIGINT);
    return;
  }
  (void)ignored;
  controller_loop = false;
  Utils::PrintColor::magenta(stdout, "\nExiting...\n");
  fflush(stdout);
}

void print_help() {
  printf("Usage: procon_driver [OPTIONS]\nOptions are:\n");
  printf(" -h --help                   get help on usage at start\n");
  printf(" -v --version                show version and exits\n");
  printf(" -c --calibration            force calibration at start\n");
  printf(" -s --swap-buttons           Swap A and B buttons and X and Y "
          "buttons\n");
  printf("    --swap-ab                Swap A and B buttons\n");
  printf("    --swap-xy                Swap X and Y buttons\n");
  printf(" -i --invert-axis [AXIS]     invert axis, possible axis: lx, ly, "
          "rx, ry, dx, dy\n");
  printf(" -p --print-state [TYPE]     Enables printing the state of TYPE. "
         "Possible TYPEs: a (axis), b (buttons), d (dpad)\n");
#ifdef DRIBBLE_MODE
  printf(" -d [VALUE]                  Enables dribble mode. If a parameter is"
         " given, it is used as the dribble cam value. Range 0 to 255\n");
#endif

  printf("\nIf you are experiencing an error, try running the program as root.\n");
  printf("\n");
}

void print_header(){
  printf("\n--------------------------------------------------------------------"
         "------\n");
  printf("| ");
  Utils::PrintColor::cyan(stdout, "Nintendo Switch Pro-Controller USB uinput driver. ");
  Utils::PrintColor::blue(stdout, "Version: " PROCON_DRIVER_VERSION);
  printf("  |\n"
         "-------------------------------------------------------------------"
         "-------\n\n");
}


void handle_controller(const HidApi::Enumerate &iter, Config &config) {
  unsigned short n_controller = 0;

  ProController controller(n_controller, iter, config);

  Utils::PrintColor::green();
  printf("Opened controller!\n");

  if (!controller.needs_first_calibration()) {
    controller.calibrate_from_file();
    Utils::PrintColor::green(stdout, "Read calibration data from file! ");
    Utils::PrintColor::cyan(stdout, "Press 'share' and 'home' to calibrate again or start with --calibrate or -c.\n");
    Utils::PrintColor::green(stdout, "Now entering input mode!\n");
  }

  printf("\n");

  auto last_start = std::chrono::steady_clock::now();
  long double delta_milis = 16;
  while (controller_loop) {
    auto frame_start = std::chrono::steady_clock::now();
    delta_milis = ((frame_start-last_start) / std::chrono::microseconds(1000));
    /*printf("%05.2Lf ms", delta_milis);
    fflush(stdout);
    printf("\r\e[K");*/

    if (!controller.is_calibrated()) {
      Utils::PrintColor::blue(stdout, "Starting calibration mode.\n");
      Utils::PrintColor::cyan(stdout, "Move both control sticks to their maximum positions "
            "(i.e. turn them in a circle once slowly.).\n"
            "Then leave both control sticks at their center and press the " 
            "square 'share' button!\n");
      while (!controller.is_calibrated()) {
        if (!controller_loop) {
          return;
        }
        controller.calibrate();

        if (config.print_axis) {
          controller.print_sticks();
          fflush(stdout);
          printf("\r\e[K");
        }
      }
      Utils::PrintColor::green(stdout, "Wrote calibration data to file!\n"
                                       "Calibrated Controller! Now entering input mode!\n");
    }

    controller.poll_input(delta_milis);

    if (config.print_axis) {
      controller.print_sticks();
      printf("\t");
    }
    if (config.print_buttons) {
      controller.print_buttons();
    }
    if (config.print_dpad) {
      controller.print_dpad();
    }
    if (config.print_axis || config.print_buttons || config.print_dpad) {
      fflush(stdout);
      printf("\r\e[K");
    }

    auto frame_end = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::microseconds((1000 * 1000 / 120)) - (frame_end-frame_start));

    last_start = frame_start;
  }
}


int main(int argc, char *argv[]) {
  Config config(argc, argv);

  if (config.help) {
    print_help();
    return 0;
  }

  if (config.show_version) {
    printf("procon_driver %s\n", PROCON_DRIVER_VERSION);
    return 0;
  }

  print_header();

  if (config.found_dribble_cam_value) {
    Utils::PrintColor::cyan();
    printf("Dribble mode enabled!\n");
    printf("Value: %i\n\n", config.dribble_cam_value);
    Utils::PrintColor::normal();
  }

  try {
    HidApi::init();
  }
  catch (const HidApi::InitError &e) {
    Utils::PrintColor::red(stdout, "Can't init hidapi\n");
    Utils::PrintColor::red(stderr,  ("  " + std::string(e.what()) + "\n").c_str());
    return -1;
  }

  signal(SIGINT, exit_handler);
  signal(SIGHUP, exit_handler);

  try {
    // Don't trust hidapi, returns non-matching devices sometimes
    HidApi::Enumerate iter(NINTENDO_ID, PROCON_ID);
    handle_controller(iter, config);
  }
  catch (const HidApi::EnumerateError &e) {
    Utils::PrintColor::red(stdout, "No controller found.\nTry plugging/connecting the controller again.\n");
    Utils::PrintColor::red(stderr, ("  " + std::string(e.what()) + "\n").c_str());
  }
  catch (const std::ios_base::failure &e) {
    Utils::PrintColor::magenta(stdout, "Try unplugging and plugging again the usb to the controller.\n");
    Utils::PrintColor::red(stderr, ("  " + std::string(e.what()) + "\n").c_str());
  } 
  catch (const HidApi::OpenError &e) {
    Utils::PrintColor::yellow(stdout, "Unable to create a connection with controller.\n");
    Utils::PrintColor::yellow(stdout, "You could try running with sudo.\n");
    Utils::PrintColor::red(stderr, ("  " + std::string(e.what()) + "\n").c_str());
  }
  catch (const std::exception &e) {
    Utils::PrintColor::red(stdout, "Unexpected error.\n");
    Utils::PrintColor::red(stderr, ("  " + std::string(e.what()) + "\n").c_str());
  }

  try {
    HidApi::exit();
  }
  catch (const HidApi::ExitError &e) {
    Utils::PrintColor::red(stderr, ("  " + std::string(e.what()) + "\n").c_str());
    return -1;
  }

  Utils::PrintColor::yellow(stdout, "Exiting...\n");
  printf("\n");
  return 0;
}
