#include "procon.hpp"
#include "config.hpp"
#include "print_color.hpp"

#include  <signal.h>

//#define DEBUG

bool controller_loop = true;
void exit_handler(int ignored){
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
  printf("\nIf you are experiencing an error, try running the program as root.");

#ifdef DRIBBLE_MODE
  printf(" -d [VALUE]                  pass parameter for dribble cam. Range "
          "0 to 255\n");
#endif
  printf("\n");
}

void print_header(){
  printf("\n--------------------------------------------------------------------"
         "------\n");
  printf("| ");
  printf("%c[%d;%dmNintendo Switch Pro-Controller USB uinput driver"
         ".%c[%dm ",
         27, 1, 32, 27, 0);
  printf("%c[%d;%dmVersion: ", 27, 1, 36);
  printf(PROCON_DRIVER_VERSION);
  printf("%c[%dm ", 27, 0);

  printf("%s "
         "|\n-------------------------------------------------------------------"
         "-------",
         KNRM);
  printf("\n\n%s", KNRM);
#ifdef DRIBBLE_MODE
  printf("%c[%d;%dmDribble mode enabled!%c[%dm \n\n", 27, 1, 36, 27, 0);
// if(found_dribble_cam_value) {
//   printf("VALUE: %i", dribble_cam_value);
// }
#endif
  fflush(stdout);
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

  hid_init();
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

  unsigned short n_controller = 0;
  bool controller_found = false;

  bool opened = false;
  bool bad_data = false;


  ProController controller(n_controller, iter, config);

  hid_free_enumeration(iter);


  PrintColor::green();
  printf("Opened controller!\n");

  if (!controller.read_calibration_from_file ||
      !controller.calibration_file_exists()) {
    PrintColor::blue();
    printf("Now entering calibration mode. \n");
    PrintColor::cyan();
    printf("%c[%d;%dmMove both control sticks to their maximum positions (i.e. turn them in a circle once slowly.), then press the "
            "square 'share' button!\n%c[%dm",
            27, 1, 36, 27, 0);
    PrintColor::normal();
  }

  signal(SIGINT, exit_handler);

  try {
    while (controller_loop) {
      if (!controller.calibrated) {
        while (!controller.calibrated) {
          controller.hid_ctrl->blink();
          controller.calibrate();
        }
        PrintColor::green();
        printf("Calibrated Controller! Now entering input mode!\n");
        PrintColor::normal();
      }

      if (controller.poll_input() < 0)
        return -1;
      
    }
  } catch (const std::runtime_error &e) {
    PrintColor::red();
    printf("%s\n", e.what());
    PrintColor::normal();
  }

  printf("\n");
  return 0;
}
