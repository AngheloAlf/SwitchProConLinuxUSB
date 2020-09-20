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
  printf(" -c --calibration            force calibration at start\n");
  printf(" -s --swap_buttons           Swap A and B buttons and X and Y "
          "buttons\n");
  printf(" -i --invert-axis [AXIS]     invert axis, possible axis: lx, ly, "
          "rx, ry, dx, dy\n");
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


void handle_controller(hid_device_info *iter, Config &config) {
  unsigned short n_controller = 0;

  ProController controller(n_controller, iter, config);

  PrintColor::green();
  printf("Opened controller!\n");

  if (controller.needs_first_calibration()) {
    PrintColor::blue();
    printf("Now entering calibration mode. \n");
    PrintColor::cyan();
    printf("Move both control sticks to their maximum positions "
           "(i.e. turn them in a circle once slowly.), then press the " 
           "square 'share' button!\n");
    PrintColor::normal();
  } else {
    controller.calibrate();
    PrintColor::green();
    printf("Read calibration data from file! ");
    PrintColor::cyan();
    printf("Press 'share' and 'home' to calibrate again or start with "
           "--calibrate or -c.\n");
    PrintColor::normal();
  }

  while (controller_loop) {
    if (!controller.is_calibrated()) {
      while (!controller.is_calibrated()) {
        if (!controller_loop) {
          return;
        }
        controller.calibrate();
      }
      PrintColor::green();
      printf("Wrote calibration data to file!\n");
      printf("Calibrated Controller! Now entering input mode!\n");
      PrintColor::normal();
    }

    if (controller.poll_input() < 0)
      break;
    
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

  if (hid_init() < 0) {
    PrintColor::red();
    printf("Hid init error...\n");
    PrintColor::normal();
    return -1;
  }
  hid_device_info *iter =
      hid_enumerate(NINTENDO_ID, PROCON_ID); // Don't trust hidapi, returns
                                             // non-matching devices sometimes
                                             // (*const to prevent compiler from
                                             // optimizing away)
  if (iter == nullptr) {
    PrintColor::red();
    printf("No controller found...\n");
    PrintColor::normal();
    return -1;
  }

  signal(SIGINT, exit_handler);

  try {
    handle_controller(iter, config);
  } catch (const std::exception &e) {
    PrintColor::red();
    printf("%s\n", e.what());
    PrintColor::normal();
  }

  hid_free_enumeration(iter);

  if (hid_exit() < 0) {
    PrintColor::red();
    printf("Hid init error...\n");
    PrintColor::normal();
    return -1;
  }

  printf("\n");
  return 0;
}
