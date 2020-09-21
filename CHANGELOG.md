# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Map the home button to the Xbox guide button.
- Blink the leds during the sticks calibration.
- Option to print the state of the sticks, buttons and/or dpad.
  - If enabled, the raw state of the sticks will be printed during the calibration.
- Exceptions.
- A changelog.

### Changed

- A _lot_ of code cleanup.
- The calibration now stores the center position of the sticks.

### Fixed

- Some memory leaks related to hidapi.

## [1.0 alpha2] - 2019-05-26

### Added

- Version of the original project.
- Disguise a Pro-Controller as an Xbox360 controller.
- Allow to perform calibration on both sticks.
- Allow to invert both sticks.
- Allow swaping A with B and X with Y.

[Unreleased]: https://github.com/AngheloAlf/SwitchProConLinuxUSB/compare/master...AngheloAlf:new_features
[1.0 alpha2]: https://github.com/AngheloAlf/SwitchProConLinuxUSB/tree/64e7d35563c4141ced78a3130de772ea55fc426d
