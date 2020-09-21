# SwitchProConLinuxUSB

This repository aims to provide a uinput driver for the Nintendo Switch Pro Controller when connected via USB.
Currently only one controller is supported!

## Dependencies

This repo needs:

- libudev
- autotools, autoconf and libtool
- cmake
- hidapi

On Ubuntu you can install these in a terminal with:

```bash
sudo apt install libudev-dev libusb-1.0-0-dev libfox-1.6-dev
sudo apt install autotools-dev autoconf automake libtool
sudo apt install cmake
sudo apt install libhidapi-dev
```

## Installation

Create install folder for Pro Controller driver and enter it, e.g.

```bash
mkdir ~/procon_driver
cd ~/procon_driver
```

You can download the ZIP file through your browser and extract it, or you can use git. If you don't already have it:

```bash
sudo apt install git
```

Clone the repository here:

```bash
git clone https://github.com/FrotBot/SwitchProConLinuxUSB.git .
```

install and build the driver:

```bash
bash install.sh
```

Reboot your PC once to make the udev rules work.

Open the terminal once more and navigate to the build directory in the install folder:

``` bash
cd ~/procon_driver/build
```

Start the driver!

```bash
./procon_driver
```

Follow instructions on screen and enjoy your games.

(You'll need to reopen the executable from the last step everytime you use the driver.)

On newer kernel versions, uinput devices need root privileges, so if you get error messages try to run

```bash
sudo ./procon_driver
```

## Invert axes and swap buttons

If you're having trouble with inverted axes, use `./procon_driver --help` to see how to invert the axes.

There's also an option to run with A and B as well as X and Y buttons switched, if you prefer the button output as they're written on the pad as opposed to XBox layout.

## Thanks

This project took heavy inspiration and some constants from this project:
<https://github.com/MTCKC/ProconXInput/tree/v0.1.0-alpha2>

And also the original project: <https://github.com/FrotBot/SwitchProConLinuxUSB>
