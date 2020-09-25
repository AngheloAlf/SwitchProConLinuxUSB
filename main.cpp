#include "procon.hpp"
#include "config.hpp"
#include "print_color.hpp"

#include <signal.h>

//#define DEBUG

bool controller_loop = true;
void exit_handler(int ignored){
  (void)ignored;
  controller_loop = false;
  PrintColor::magenta();
  printf("\nExiting...\n");
  PrintColor::normal();
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
  PrintColor::cyan();
  printf("Nintendo Switch Pro-Controller USB uinput driver. ");
  PrintColor::cyan();
  printf("Version: ");
  printf(PROCON_DRIVER_VERSION);
  PrintColor::normal();

  printf("  |\n"
         "-------------------------------------------------------------------"
         "-------\n\n");
  fflush(stdout);
}


void handle_controller(const HidApi::Enumerate &iter, Config &config) {
  unsigned short n_controller = 0;

  ProController controller(n_controller, iter, config);

  PrintColor::green();
  printf("Opened controller!\n");

  if (!controller.needs_first_calibration()) {
    controller.calibrate_from_file();
    PrintColor::green();
    printf("Read calibration data from file!");
    PrintColor::cyan();
    printf("Press 'share' and 'home' to calibrate again or start with "
           "--calibrate or -c.\n");
    PrintColor::green();
    printf("Now entering input mode!\n");
    PrintColor::normal();
  }

  printf("\n");

  while (controller_loop) {
    if (!controller.is_calibrated()) {
      fflush(stdout);
      PrintColor::blue();
      printf("Starting calibration mode.\n");
      PrintColor::cyan();
      printf("Move both control sticks to their maximum positions "
            "(i.e. turn them in a circle once slowly.).\n"
            "Then leave both control sticks at their center and press the " 
            "square 'share' button!\n");
      PrintColor::normal();
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
      PrintColor::green();
      printf("Wrote calibration data to file!\n");
      printf("Calibrated Controller! Now entering input mode!\n");
      PrintColor::normal();
    }

    controller.poll_input();

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
  }
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
    PrintColor::cyan();
    printf("Dribble mode enabled!\n");
    printf("Value: %i\n\n", config.dribble_cam_value);
    PrintColor::normal();
  }

  try {
    HidApi::init();
  }
  catch (const std::runtime_error &e) {
    PrintColor::red();
    printf("%s\n", e.what());
    PrintColor::normal();
    return -1;
  }

  signal(SIGINT, exit_handler);

  try {
    // Don't trust hidapi, returns non-matching devices sometimes
    HidApi::Enumerate iter(NINTENDO_ID, PROCON_ID);
    handle_controller(iter, config);
  } catch (const std::ios_base::failure &e) {
    PrintColor::red();
    printf("%s\n", e.what());

    PrintColor::magenta();
    printf("\nTry unplugging and plugging again the usb to the controller.\n");

    PrintColor::yellow();
    printf("Exiting...\n");
    PrintColor::normal();
  } catch (const std::exception &e) {
    PrintColor::red();
    printf("%s\n", e.what());
    PrintColor::normal();
  }
  #if 0
  catch (const std::runtime_error &e) {
    PrintColor::red();
    printf("No controller found: %s\n", e.what());
    return -1;
  }
  #endif

  try {
   HidApi::exit();
  }
  catch (const std::runtime_error &e) {
    PrintColor::red();
    printf("%s\n", e.what());
    return -1;
  }

  printf("\n");
  return 0;
}
