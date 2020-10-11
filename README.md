# SwitchProConLinuxUSB

This repository aims to provide a uinput driver for the Nintendo Switch Pro Controller when connected via USB.

![License](https://img.shields.io/badge/license-MIT-green)
// TODO: add more badges

## Features

- Map the inputs of a connected Pro Controller to a virtual Microsoft Xbox 360 controller.
  - Allows swapping A-B and/or X-Y buttons to match the written layout.
  - Allows inverting each the axis (and dpad) individually.
- Simple rumble support.
- Option to print buttons pressing and axis to a terminal.
- Option to calibrate each axis in case of problems.
- Low response times.
- Experimental bluetooth support.
  - It is not enabled by default. See the section [Enable experimental bluetooth support](#Enable-experimental-bluetooth-support).
  - See [Known issues](#known-issues).
- At the time, only one controller is supported.

## Usage

// TODO

### Invert axis and swap buttons

If you're having trouble with inverted axis, use `./procon_driver --help` to see how to invert the axis.

There's also an option to run with A and B as well as X and Y buttons switched, if you prefer the button output as they're written on the pad as opposed to XBox layout.

## Building from source

### Dependencies

(TODO: check if those are really needed.)

The following is needed to build the driver:

- libudev
- autotools, autoconf and libtool
- cmake
- hidapi

On a Debian-based distro (Ubuntu, Pop!_Os, etc) you can install these in a terminal with:

```bash
sudo apt install libudev-dev libusb-1.0-0-dev libfox-1.6-dev
sudo apt install autotools-dev autoconf automake libtool
sudo apt install cmake
sudo apt install libhidapi-dev
```

As a one liner:

```bash
sudo apt install libudev-dev libusb-1.0-0-dev libfox-1.6-dev autotools-dev autoconf automake libtool cmake libhidapi-dev
```

### Build the driver

1. Download the source code (via `git clone` or zip file) to a folder.
2. Navigate to that folder in a terminal.
3. Run the script `install_rules.sh` with root privileges to install the udev rules.
    - Those are required to be able to run the driver without root privileges.
4. Run the script `cbuild.sh` to build the driver.
5. The driver will be in the build folder.

The following is an example of the steps above:

```bash
git clone https://github.com/AngheloAlf/SwitchProConLinuxUSB.git procon_driver
cd procon_driver/
sudo ./install_rules.sh
./cbuild.sh
```

### Enable experimental bluetooth support

Assuming you already have the source code, you have to edit the file `CMakeLists.txt` and change the `hidapi-libusb` line to `hidapi-hidraw` line, then recompile the driver.

## Planned

- Support for multiple controller at the same time.
- Joy-cons support.
  - Wired (charging grip) and bluetooth.
- IMU sensors (accelerometer and gyroscope) support.
  - Probably will expose them as new axis for each one.
- Button re-mapping.
- Fix the known issues and bugs.
- Find currently unknown issues.
- Add more bugs to fix later.

### Would be nice, but unlikely

- Non-official and third party controllers support.
  - If you have one, and you are willing to help, open an issue!
- Some kind of scanning mode, or controller hot-plugging.
- Play MIDI files using the hd rumble.

### Changelog

See [Changelog](./blob/CHANGELOG.md).

## User support

In case you have any problem using this driver, first see the [Known issues](#known-issues) section. If your problem is not listed there, or you have new info on some problem, just open an [Issue](./issues) describing your problem and your system.

### Known issues

- If the driver was recently closed and then reopened, it could struggle to connect to the controller. Just keep trying, or replug the cable.

#### Experimental bluetooth support issues

- The driver is not compatible with Steam.
  - Trying to use this driver when Steam is open, makes the controller unusable in both platforms.
  - This happens because Steam sees the controller and tries to connect with it.
- The controller randomly disconnects.

## Contributing

Contributing is highly appreciated. Just open a new [Issue](./issues) or [Pull request](./pulls).

You can also propose new features in the [Issues](./issues) section.

### Thanks

- Everyone that has helped in <https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering>.
- The original project: <https://github.com/FrotBot/SwitchProConLinuxUSB>.
  - Which is based and inspired on <https://github.com/MTCKC/ProconXInput/tree/v0.1.0-alpha2>.
- Everyone in <https://gitlab.com/fabis_cafe/game-devices-udev> for the udev rules.

## License

[MIT](./blob/master/License.txt)
