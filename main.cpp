#include "procon.hpp"
#include "config.hpp"
#include "print_color.hpp"

//#define DEBUG


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

  ProController controller(config);
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


  // OPEN PHASE
  do {
    opened = false;
    bad_data = false;
    if (iter->product_id == PROCON_ID && iter->vendor_id == NINTENDO_ID) {
      // open & test for timeout in read!
      int ret = controller.open_device(iter->vendor_id, iter->product_id,
                                        iter->serial_number, n_controller + 1);
      opened = ret == 0;
      if (!opened) { // read timed out

        if (ret == -1) {
          PrintColor::red();
          printf("Invalid device pointer. Aborting!\n");
          PrintColor::normal();
          return -1;
        }
        PrintColor::magenta();
        printf("Failed to open controller, error code %d, trying again...\n",
                ret);
        PrintColor::normal();
        controller.close_device();
        usleep(1000 * 10);
        continue;
      } else {
        #if 0
        // TEST FOR BAD DATA
        for (size_t i = 0; i < TEST_BAD_DATA_CYCLES; ++i) {
          if (controller.try_read_bad_data() != 0) {
            PrintColor::magenta();
            printf("Detected bad data stream. Trying again...\n");
            PrintColor::normal();
            controller.close_device();
            bad_data = true;
            usleep(1000 * 10);
            break;
          }
        }
        #endif
      }
    }

  } while (!opened || bad_data);

  if (controller.is_opened) {
    PrintColor::green();
    printf("Opened controller!\n");

    if (controller.uinput_create() < 0) {
      PrintColor::red();
      printf("Failed to open uinput device!\n");
      PrintColor::normal();
    }

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
  }

  // controller.u_setup();

  while (true) {
    if (!controller.calibrated) {
      while (!controller.calibrated) {
        controller.calibrate();
      }
      PrintColor::green();
      printf("Calibrated Controller! Now entering input mode!\n");
      PrintColor::normal();
    }

    if (controller.is_opened) {
      if (controller.poll_input() < 0)
        return -1;
    }
  }

  // hid_exit;
  for (short unsigned i = 0; i < MAX_N_CONTROLLERS; ++i) {
    controller.close_device();
    controller.uinput_destroy();
  }
  printf("\n");
  return 0;
}
