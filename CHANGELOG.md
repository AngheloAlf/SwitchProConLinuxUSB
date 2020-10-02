# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Map the home button to the Xbox guide button.
- Blink the leds during the axis calibration.
- Option to print the state of the axis (`-p a`), buttons (`-p b`) and/or dpad (`-p d`).
  - If enabled, the raw state of the axis will be printed during the calibration.
- Option to only swap A and B buttons (`--swap-ab`) and option to only swap X and Y buttons (`--swap-xy`).
- Send a 'close connection' message to controller when exiting the program.
  - This way the led turns off at exit.
  - This message is sended only in USB mode.
- Improve axis precision from 8 bits to 12 bits.
- Bluetooth compatibility.
- Exceptions.
- A changelog.

### Changed

- A _lot_ of code cleanup.
- The calibration process now stores the center position of the axis.
- `--swap_buttons` to `--swap-buttons`.
- Improved the comunication protocol to the pro controller.
- Lowered avareage response time from 15ms to 5ms.
- Hidapi backend, from libusb to hidraw.
- Error handling.

### Fixed

- Some memory leaks related to hidapi.
- "Can't connect to controller" error when trying to reopen the program after a successful execution.

## [1.0 alpha2] - 2019-05-26

### Added

- Version of the original project.
- Disguise a Pro-Controller as an Xbox360 controller.
- Allow to perform calibration on both axis.
- Allow to invert both axis.
- Allow swaping A with B and X with Y.

[Unreleased]: https://github.com/AngheloAlf/SwitchProConLinuxUSB/compare/master...AngheloAlf:new_features
[1.0 alpha2]: https://github.com/AngheloAlf/SwitchProConLinuxUSB/tree/64e7d35563c4141ced78a3130de772ea55fc426d
