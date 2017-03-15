# Building

There are several options to building Espruino on various platforms for the OS and board versions that are avaiable.

To build and run Espruino on the OS that one is using is as simple as the following, if prerequisits are met:

```bash
make clean && make
```

**Note:**

* In general, have a look through the Makefile to see what other options are available
* If you're swapping between compiling for different targets, you need to call `make clean` before you compile for the new target.
* If you have a ld error, check the board name in the BOARDNAME=1 make in the Makefile.
* ```RELEASE=1``` for performance and code size, without it, assertions are kept for debugging.
* ```DEBUG=1``` is available.

## Under Linux

Espruino is easy to build under Linux, for either for Espruino running on Linux or a board.

The current reference OS for building is Ubuntu 16.04.1 LTS, and the following can ensure problem free development:

### Easy Method : provision.sh

Simply run the following with the name of your board to set
your computer up ready for a build:

```bash
source scripts/provision.sh BOARDNAME
```

This should work for common platforms on Linux, but will only set
paths up for your current session. You'll have to run it again
next time you log in.

### for Espruino

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential git python python-pip
sudo pip install --upgrade pip
# User choice for placement of source repos
mkdir -p ~/source/repos/github/espruino
cd ~/source/repos/github/espruino
git clone https://github.com/espruino/Espruino.git
cd Espruino
make clean && make
chmod +x espruino && sudo cp espruino /usr/local/bin
```

### for an example of cross compilation for Puck.js

Having successfully created an native OS Espruino, try a cross compilation.

```bash
sudo apt-get update
sudo pip install nrfutil
sudo apt-get install -y \
  lib32z1 lib32ncurses5
wget https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q3-update/+download/gcc-arm-none-eabi-5_4-2016q3-20160926-linux.tar.bz2
tar xjf gcc-arm-none-eabi-5_4-2016q3-20160926-linux.tar.bz2
sudo mv gcc-arm-none-eabi-5_4-2016q3-20160926 /usr/local
export PATH=/usr/local/gcc-arm-none-eabi-5_4-2016q3/bin:$PATH
cd ~/source/repos/github/espruino/Espruino
make clean && DFU_UPDATE_BUILD=1 BOARD=PUCKJS RELEASE=1 make
ls -l *puckjs*
```

## Under MacOS

* It is possible to build Espruino under MacOS with some effort.
  * PR for an easy Espruino build under MacOS are welcome.
* If you don't have Linux it's much easier to install it in a Virtual Machine (see below).

## Under Windows

It is possible to build Espruino under Windows with the following addition to the Linux explanation:

* Install Bash on Ubuntu on Windows 10 <https://msdn.microsoft.com/da-dk/commandline/wsl/install_guide>
  * After enabling, just use the instructions for Linux

Note:

* there is no access to USB in the present version
  * copy any crosscompile output to one's user directory and either bluetooth or USB the result to the target
* Ubuntu 14.04 LTS is the present version.

Or use a Virtual machine as described below dependent on ones taste.

## Cross compilation

### for Raspberry Pi

Using [RASPBIAN JESSIE WITH PIXEL](https://www.raspberrypi.org/downloads/raspbian/), getting Espruino is easy.

To enable the full power of Espruino on the Pi, [WiringPi](http://wiringpi.com/):

* ```sudo apt-get install wiringpi```

Getting Espruino:

* clone this repository and cd into the directory
  * ```make clean && make```
  * ```chmod +x espruino && sudo cp espruino /usr/local/bin```

### for OpenWRT

Follow the instructions for [OpenWRT build system](https://wiki.openwrt.org/doc/howto/buildroot.exigence)

After a successful OpenWRT build, [OpenWRT Espruino packages](https://github.com/vshymanskyy/OpenWRT-Espruino-packages)

### for STM32 Boards (incl. [Espruino Board](http://www.espruino.com/EspruinoBoard)

```bash
make clean && BOARD=BOARDNAME RELEASE=1 make
```

Where BOARDNAME is one of:

* `PICO_R1_3` for Espruino Pico
* `ESPRUINOBOARD` for Original Espruino
* `ESPRUINOWIFI` for Espruino WiFi

Or choose another board name based on the files in `boards/*.py`

* `BOARD=BOARDNAME RELEASE=1 make serialflash` will flash to /dev/ttyUSB0 using the STM32 serial bootloader (what's needed for the Espruino and HY boards)
* `BOARD=BOARDNAME RELEASE=1 make flash` will flash using st-flash if it's a discovery board, the maple bootloader if using that board, or will copy the binary to `/media/NUCLEO` if using a Nucleo board.

It may complain that there isn't enough space on the chip. This isn't an issue unless you save to flash, but you can fix the error in a few ways:

* Disable the check by adding `TRAVIS=1`
* Change the compile flags from `-O3` to `-Os` in the `Makefile`
* Knock out some functionality (like `USE_GRAPHICS=1`) that you don't need in the `Makefile`

**Note:** Espruino boards contain a special bootloader at `0x08000000` (the default address), with the Espruino binary moved on 10240 bytes to `0x08002800`. To load the Espruino binary onto a board at the correct address, use `BOARD=ESPRUINO_1V3 RELEASE=1 make serialflash`. If you want to make a binary that contains the bootloader as well as Espruino (like the ones that the Espruino Web IDE expects to use) use the script `scripts/create_espruino_image_1v3.sh` which will compile the bootloader *and* Espruino, and then join them together.

### for [Nordic Semiconductor's nRF51/nRF52 series devices](https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy)

Dependant on the board, either usb or bluetooth can be used to program the board or install the bootloader from different devices.

* Bluetooth Low energy
  * the board will appear as `Espruino XYZ` where `XYZ` is the board name
* USB
  * the board appears as a drive to drop a hex on

#### for [Puck.js](http://www.espruino.com/Puck.js)

The Puck.js is based on the nRF52

```bash
make clean && DFU_UPDATE_BUILD=1 BOARD=PUCKJS RELEASE=1 make
```

The resulting file is a zip that has to be transferred to the puck.js via a Bluetooth low energy device.
See <https://www.espruino.com/Puck.js+Quick+Start> for information concerning transferring the zip to the puck.js.

#### for [NRF52-DK](https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF52-DK)

All boards based on the nRF52 have RAM and Flash to support Espruino without feature disablement.

```bash
make clean && BOARD=NRF52832DK RELEASE=1 make
```

#### for [micro:bit](http://microbit.org/)

The micro:bit is based on the nRF51.

* ```make clean && BOARD=MICROBIT RELEASE=1 make```
* Drop the hex generated on the micro:bit drive and it is then an Espruino.

Note:

* At the time of writing, if one uses the [Espruino Web IDE](https://www.espruino.com/Web+IDE), access the Settings->Communications
  * Request board details on connect: false
  * Throttle Send: true

#### for [NRF51-DK](https://www.nordicsemi.com/eng/Products/nRF51-DK)

All boards based on the nRF51 are limited in RAM and Flash so many features are disabled.

```bash
make clean && BOARD=NRF51822DK RELEASE=1 make
```

### for esp8266

In order to compile for the esp8266 on Linux several pre-requisites have to be installed:

* the esp-open-sdk from <https://github.com/pfalcon/esp-open-sdk>, use make STANDALONE=n
* the Espressif SDK (version 1.5.0 with lwip patch as of this writing) from <http://bbs.espressif.com/viewforum.php?f=46> and <http://bbs.espressif.com/viewtopic.php?f=7&t=1528>

To run make you need to pass a couple of environment variables to `make`.  These include:

* `BOARD=ESP8266_BOARD`
* `FLASH_4MB=1` if you have an esp-12
* `ESP8266_SDK_ROOT=<Path to the 1.4 SDK>`
* `PATH=<Path to esp-open-sdk/xtensa-lx106-elf/bin/>`
* `COMPORT=</dev/ttyUSB0|COM1|...>`

The easiest is to place the following lines into a script, adapt it to your needs and then run it.

```bash

#! /bin/bash
export BOARD=ESP8266_BOARD
export FLASH_4MB=1
export ESP8266_SDK_ROOT=/esp8266/esp_iot_sdk_v1.5.0
export PATH=$PATH:/esp8266/esp-open-sdk/xtensa-lx106-elf/bin/
export COMPORT=/dev/ttyUSB0
make clean && make $*

```

* If you do `make flash` it will try to flash your esp8266 module over serial
* If you do `make wiflash` it will try to flash you esp8266 module over wifi, which assumes
  that it's already running Espruino
* You will also get an `espruino_1v00_*_esp8266.tgz` archive, which contains everything you
  need to flash a module (except for esptool.py), including a README_flash.txt

#### Building on Eclipse

When building on Eclipse, update the Makefile properties to include the definitions show above.  The easiest way to achieve
that task is to right-click your Espruino project and select `properties`.  From there, navigate to `C/C++ Build > Environment`.

#### for EMW3165

Note: the emw3165 port is very preliminary and does not include Wifi support at this time.
_The text below is what is planned in order to support Wifi, but it doesn't exist yet._

The EMW3165 port uses WICED, which is an application framework provided by Broadcom for its
wifi chips, such as the BCM43362 used in the EMW3165 module. The module consists of an
STM32F411CE processor and the BCM43362. The WICED framework comes with everything and the kitchen
sink plus a rather complex build process in order to support umpteen different processor and
wifi chip combinations, plus various use-cases. WICED includes FreeRTOS and LwIP plus
proprietary code to manage the Wifi chip.

The strategy employed is to compile portions of WICED into a library using the WICED toolchain
and then linking this into Espruino.

Setting up WICED:

* WICED does not officially support the EMW3165.
* Clone <https://github.com/MXCHIP-EMW/WICED-for-EMW> and follow the instructions there to configure
  WICED and build it. (You will need to sign up for a developer acct with Broadcom.)
* Build the apsta sample program (snippet) using a command-line like
  `./make EMW3165-FreeRTOS-LwIP-snip.apsta download run JTAG=stlink-v2`
* Hook up your emw3165 to an ST-Link-v2 or your preferred STM32 programmer and flash using the
  above command-line. You should see the EMW's access point.
* An alternative program to test with is the "scan" snip as it will also print something on the
  console (works well with the WifiMCU board): `./make EMW3165-FreeRTOS-LwIP-snip.scan ...`

Compiling WICED into a library:

* ... if only this worked ...

Compiling Espruino:

* To compile Espruino you will need to point to the WICED root and include files. This is
  done by specifying a WICED_ROOT environment variable.
* Adapt the pathnames from the following script:

```bash

  WICED_ROOT=/home/emw3165/WICED-for-EMW/WICED-SDK-3.3.1 make $*

```

## Documentation

```bash

sudo pip install markdown
python scripts/build_docs.py

```

This will create a file called ```functions.html``` that is a version of [the reference pages](http://www.espruino.com/Reference),
but based on your source code.

## Virtual Machines under Windows and MacOS

The easiest solution for a Virtual Machine for Windows and MacOS is [VirtualBox](https://www.virtualbox.org/).
A really easy way to provision, ie setup the system for development, is [Vagrant](https://www.vagrantup.com).

Note:

* Windows
  * Installation of <https://git-scm.com/> gives a ssh that can be used with vagrant ssh

### Vagrant and VirtualBox

* For your host OS (Windows or MacOS)
  * Download and install [VirtualBox](https://www.virtualbox.org/)
  * Download and install [Vagrant](https://www.vagrantup.com/downloads.html)
  * Note: for MacOS, the two previous steps can be accomplished easily with [Homebrew Cask](http://caskroom.io)
    * `brew cask install virtualbox vagrant`
* Clone this repository and navigate with the command prompt to the contents
* Install the auto-network plugin
  * `vagrant plugin install vagrant-auto_network`
* Execute
  * `vagrant up`
    * This will take a little while while the box is downloaded, and your virtual machine is provisioned.
  * `vagrant ssh`
    * a ssh session into your new VM will be created.
  * `cd /vagrant && make clean && make`
    * a native OS version of Espruino is now built. See this documentation for further examples
  * To exit the ssh session
    * `exit`.
  * On the host OS, the following are useful vagrant commands
    * `vagrant suspend`
      * will pause the VM
    * `vagrant halt`
      * will stop the VM
    * See Vagrant's ["Getting Started"](https://www.vagrantup.com/docs/getting-started/index.html) page for further information.

### VirtualBox

If one does not wish to use vagrant, then install Virtual Box and use the Linux method.

## USB access

In order to access USB, bluetooth or connected USB devices, one has USB filters to dedicate access to the guest OS.
The easiest method is via the VirtualBox Manager, and select which devices should be dedicated to the VM with the device filter.
Note: VirtualBox Guest Additions from VirtualBox are required.

### To flash Espruino from the VM

* Plug the Espruino board in while holding BTN1, and wait for Windows to finish connecting to the USB device
* Go into the VirtualBox Manager (There's no need to stop your VM)
* Click on `USB`, then click on the icon with the `+` sign (With the tooltip 'Adds a new USB filter ... selected USB device')
* Click on the device labeled `STMicroelectronics STM32 ...`
* Now unplug the Espruino board, wait a few seconds, and plug it back in (holding BTN1 again)
* Go back into the VM, and type `sudo BOARD=ESPRUINO_1V3 RELEASE=1 make serialflash`
* Your board will now be flashed

**Note:** if you want to you can change permissions so you don't need `sudo` by typing `sudo cp misc/45-espruino.rules /etc/udev/rules.d;sudo udevadm control --reload-rules` and then re-inserting the board.
