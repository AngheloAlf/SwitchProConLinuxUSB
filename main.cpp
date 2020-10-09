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
    Utils::PrintColor::red();
    printf("\n\nHard exit.\n");
    Utils::PrintColor::normal();
    fflush(stdout);
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
    return;
  }
  (void)ignored;
  controller_loop = false;
  Utils::PrintColor::magenta();
  printf("\nExiting...\n");
  Utils::PrintColor::normal();
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
  Utils::PrintColor::cyan();
  printf("Nintendo Switch Pro-Controller USB uinput driver. ");
  Utils::PrintColor::cyan();
  printf("Version: ");
  printf(PROCON_DRIVER_VERSION);
  Utils::PrintColor::normal();

  printf("  |\n"
         "-------------------------------------------------------------------"
         "-------\n\n");
  fflush(stdout);
}


ProController *create_ctrl(unsigned short n_controller, const HidApi::Enumerate &iter, Config &config) {
  ProController *controller = nullptr;

  bool opened = false;
  int retries = 0;
  while(!opened){
    try {
      controller = new ProController(n_controller, iter, config);
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

  return controller;
}

void handle_controller(const HidApi::Enumerate &iter, Config &config) {
  unsigned short n_controller = 0;

  ProController *controller = create_ctrl(n_controller, iter, config);

  Utils::PrintColor::green();
  printf("Opened controller!\n");

  if (!controller->needs_first_calibration()) {
    controller->calibrate_from_file();
    Utils::PrintColor::green();
    printf("Read calibration data from file! ");
    Utils::PrintColor::cyan();
    printf("Press 'share' and 'home' to calibrate again or start with "
           "--calibrate or -c.\n");
    Utils::PrintColor::green();
    printf("Now entering input mode!\n");
    Utils::PrintColor::normal();
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

    if (!controller->is_calibrated()) {
      fflush(stdout);
      Utils::PrintColor::blue();
      printf("Starting calibration mode.\n");
      Utils::PrintColor::cyan();
      printf("Move both control sticks to their maximum positions "
            "(i.e. turn them in a circle once slowly.).\n"
            "Then leave both control sticks at their center and press the " 
            "square 'share' button!\n");
      Utils::PrintColor::normal();
      fflush(stdout);
      while (!controller->is_calibrated()) {
        if (!controller_loop) {
          return;
        }
        controller->calibrate();

        if (config.print_axis) {
          controller->print_sticks();
          fflush(stdout);
          printf("\r\e[K");
        }
      }
      Utils::PrintColor::green();
      printf("Wrote calibration data to file!\n");
      printf("Calibrated Controller! Now entering input mode!\n");
      Utils::PrintColor::normal();
    }

    controller->poll_input(delta_milis);

    if (config.print_axis) {
      controller->print_sticks();
      printf("\t");
    }
    if (config.print_buttons) {
      controller->print_buttons();
    }
    if (config.print_dpad) {
      controller->print_dpad();
    }
    if (config.print_axis || config.print_buttons || config.print_dpad) {
      fflush(stdout);
      printf("\r\e[K");
    }

    auto frame_end = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::microseconds((1000 * 1000 / 120)) - (frame_end-frame_start));

    last_start = frame_start;
  }

  delete controller;
}


int main(int argc, char *argv[]) {
  Config config(argc, argv);

  if (config.help) {
    print_help();
    return 0;
  }

  if (config.show_version) {
    std::cout << "Version is " << PROCON_DRIVER_VERSION << std::endl;
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
    Utils::PrintColor::red();
    printf("%s\n", e.what());
    Utils::PrintColor::normal();
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
    Utils::PrintColor::red();
    printf("No controller found:\n  %s\n", e.what());
    Utils::PrintColor::normal();
  }
  catch (const std::ios_base::failure &e) {
    Utils::PrintColor::red();
    printf("%s\n", e.what());

    Utils::PrintColor::magenta();
    printf("\nTry unplugging and plugging again the usb to the controller.\n");

    Utils::PrintColor::yellow();
    printf("Exiting...\n");
    Utils::PrintColor::normal();
  } 
  catch (const HidApi::OpenError &e) {
    Utils::PrintColor::red();
    printf("%s\n", e.what());
    Utils::PrintColor::yellow();
    printf("Unable to create a connection with controller.\n");
    printf("You could try running with sudo.\n");
    Utils::PrintColor::normal();
  }
  catch (const std::exception &e) {
    Utils::PrintColor::red();
    printf("%s\n", e.what());
    Utils::PrintColor::normal();
  }

  try {
    HidApi::exit();
  }
  catch (const HidApi::ExitError &e) {
    Utils::PrintColor::red();
    printf("%s\n", e.what());
    return -1;
  }

  printf("\n");
  return 0;
}
