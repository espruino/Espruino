Building
========


Under Linux
-----------
  
Espruino is easy to build under Linux, and it is possible to build under MacOS with some effort. If you don't have Linux it's **much** easier to install it in a Virtual Machine. See the heading **Building under Windows/MacOS with a VM** below for more information.

### for STM32 Boards (incl. [Espruino Board](http://www.espruino.com/EspruinoBoard))
  
The (previously suggested) CodeSourcery GCC compiler is no longer available. We'd suggest you use [gcc-arm-none-eabi](https://launchpad.net/gcc-arm-embedded/+download).

Download the compiler, set up your path so you have access to it, and run:

```YOUR_BOARD_NAME=1 RELEASE=1 make```

* See the top of Makefile for board names
* Without `RELEASE=1`, assertions are kept in the code (which is good for debugging, bad for performance + code size)
* `BOARDNAME=1 RELEASE=1 make serialflash` will flash to /dev/ttyUSB0 using the STM32 serial bootloader (what's needed for the Espruino and HY boards)
* `BOARDNAME=1 RELEASE=1 make flash` will flash using st-flash if it's a discovery board, the maple bootloader if using that board, or will copy the binary to `/media/NUCLEO` if using a Nucleo board.

It may complain that there isn't enough space on the chip. This isn't an issue unless you save to flash, but you can fix the error in a few ways:

* Disable the check by adding `TRAVIS=1`
* Change the compile flags from `-O3` to `-Os` in the `Makefile`
* Knock out some functionality (like `USE_GRAPHICS=1`) that you don't need in the `Makefile`
* Try different compilers. `codesourcery-2013.05-23-arm-none-eabi` provides low binary size for `-O3`

**Note:** Espruino boards contain a special bootloader at `0x08000000` (the default address), with the Espruino binary moved on 10240 bytes to `0x08002800`. To load the Espruino binary onto a board at the correct address, use `ESPRUINO_1V3=1 RELEASE=1 make serialflash`. If you want to make a binary that contains the bootloader as well as Espruino (like the ones that the Espruino Web IDE expects to use) use the script `scripts/create_espruino_image_1v3.sh` which will compile the bootloader *and* Espruino, and then join them together.

----

### for Nordic Semiconductor's nRF51/nRF52 series devices

The (previously suggested) CodeSourcery GCC compiler is no longer available. We'd suggest you use [gcc-arm-none-eabi](https://launchpad.net/gcc-arm-embedded/+download).

Download the compiler, set up your path so you have access to it, and run:

```NRF52832DK=1 RELEASE=1 make```

**Note:** This is for the nRF52 devkit, use `NRF51822DK=1` instead for the nRF51 devkit, `MICROBIT=1` for the BBC micro:bit, or check the first 50-ish lines of the `Makefile` for more board options.

To program the nRF52 Development Kit with Espruino, type `NRF52832DK=1 RELEASE=1 make flash` on Linux. This will copy the generated `espruino_xx.xx_nrf52832.hex` file to the nRF devkit's USB flash drive - you can however do that manually if you want to.

Now Espruino is ready to use - you can either use [the Web IDE](http://www.espruino.com/Quick+Start) to connect, or you can use any serial terminal application as long as you connect at 9600 baud (no parity, 1 stop bit).

You can also connect via Bluetooth Low energy - the board will appear as `Espruino XYZ` where `XYZ` is the board name:

* Use the Nordic UART Android app - install it, connect to the device, and then issue commands. Note that you will have to explicitly send a Carriage Return at the end of any command in order or it to execute (eg. `1+2 [newline]` then click send)

* Use Web Bluetooth - see [The BBC micro:bit page](http://www.espruino.com/MicroBit) for more information about this.

----

### for esp8266

In order to compile for the esp8266 on Linux several pre-requisites have to be installed:
- the esp-open-sdk from https://github.com/pfalcon/esp-open-sdk, use make STANDALONE=n
- the Espressif SDK (version 1.5.0 with lwip patch as of this writing) from http://bbs.espressif.com/viewforum.php?f=46 and http://bbs.espressif.com/viewtopic.php?f=7&t=1528

To run make you need to pass a couple of environment variables to `make`.  These include:

* `ESP8266_BOARD=1`
* `FLASH_4MB=1` if you have an esp-12
* `ESP8266_SDK_ROOT=<Path to the 1.4 SDK>`
* `PATH=<Path to esp-open-sdk/xtensa-lx106-elf/bin/>`
* `COMPORT=</dev/ttyUSB0|COM1|...>`

The easiest is to place
the following lines into a script, adapt it to your needs and then run it.
```
#! /bin/bash
export ESP8266_BOARD=1
export FLASH_4MB=1
export ESP8266_SDK_ROOT=/esp8266/esp_iot_sdk_v1.5.0
export PATH=$PATH:/esp8266/esp-open-sdk/xtensa-lx106-elf/bin/
export COMPORT=/dev/ttyUSB0
make $*
```

* If you do `make flash` it will try to flash your esp8266 module over serial
* If you do `make wiflash` it will try to flash you esp8266 module over wifi, which assumes
  that it's already running Espruino
* You will also get an `espruino_1v00_*_esp8266.tgz` archive, which contains everything you
  need to flash a module (except for esptool.py), including a README_flash.txt

####Building on Eclipse
When building on Eclipse, update the Makefile properties to include the definitions show above.  The easiest way to achieve
that task is to right-click your Espruino project and select `properties`.  From there, navigate to `C/C++ Build > Environment`.

----

### for EMW3165

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
- WICED does not officially support the EMW3165.
- Clone https://github.com/MXCHIP-EMW/WICED-for-EMW and follow the instructions there to configure
  WICED and build it. (You will need to sign up for a developer acct with Broadcom.)
- Build the apsta sample program (snippet) using a command-line like
  `./make EMW3165-FreeRTOS-LwIP-snip.apsta download run JTAG=stlink-v2`
- Hook up your emw3165 to an ST-Link-v2 or your preferred STM32 programmer and flash using the
  above command-line. You should see the EMW's access point.
- An alternative program to test with is the "scan" snip as it will also print something on the
  console (works well with the WifiMCU board): `./make EMW3165-FreeRTOS-LwIP-snip.scan ...`

Compiling WICED into a library:
- ... if only this worked ...

Compiling Espruino:
- To compile Espruino you will need to point to the WICED root and include files. This is
  done by specifying a WICED_ROOT environment variable.
- Adapt the pathnames from the following script:
```
  WICED_ROOT=/home/emw3165/WICED-for-EMW/WICED-SDK-3.3.1 make $*
```

----

### for Linux

Simple: Just run `make`

----

### for Raspberry Pi

On the Pi, just run `make`.

Or to cross-compile:

```
cd targetlibs
mkdir raspberrypi
cd raspberrypi
git clone git://github.com/raspberrypi/tools.git
sudo apt-get install ia32-libs
```

----

### for Carambola (OpenWRT)

To cross compile,

* Follow instructions at <https://github.com/8devices/carambola> to set toolchain up in ```~/workspace/carambola```
* Run ```CARAMBOLA=1 make```

----

### Documentation

```python scripts/build_docs.py ```

This will create a file called ```functions.html``` that is a version of [the reference pages](http://www.espruino.com/Reference),
but based on your source code.

----

Building under Windows/MacOS with a VM (Vagrant)
------------------------------------------------

* Clone this repository.
* Download and install the correct [VirtualBox](https://www.virtualbox.org/) for your platform. eg. If you have Windows, download 'VirtualBox for Windows Hosts'.
* Download and install the correct [Vagrant](https://www.vagrantup.com/downloads.html) for your platform.
  > If running on MacOS, the two previous steps can be accomplished easily with [Homebrew Cask](http://caskroom.io):  `brew cask install virtualbox vagrant` will do it.
* In your terminal application, navigate to your cloned working copy.
* Install the auto-network plugin with `vagrant plugin install vagrant-auto_network`
* Execute `vagrant up`.  This will take a little while while the box is downloaded, and your virtual machine is provisioned.
* When it is complete, execute `vagrant ssh`, which will open an ssh session into your new VM. 
* Execute `cd /vagrant && ESPRUINO_1V3=1 RELEASE=1 make` and wait.
* Espruino is now built. See the documentation under **Building under Linux** for more examples.
* To terminate the ssh session, simply execute `exit`.
* `vagrant suspend` will pause the VM, `vagrant halt` will stop it, and `vagrant up` will bring it back up again.  See Vagrant's ["Getting Started"](http://docs.vagrantup.com/v2/getting-started/index.html) page for further information.

----

Building under Windows/MacOS with a VM
--------------------------------------

* Download and install the correct [VirtualBox](https://www.virtualbox.org/) for your platform. eg. If you have Windows, download 'VirtualBox for Windows Hosts'.
* Download the [Ubuntu 14.04 32 bit Desktop ISO Image](http://www.ubuntu.com/download/desktop)
* Run VirtualBox, and click the 'New' button
* Give the new OS a name, choose `Linux` as the type, and `Ubuntu (32 bit)` as the version
* Click `Next`, choose 2048MB of memory, and not to create a hard disk image (ignore the warning). **Note:** We're going to run Ubuntu right from the virtual CD without installing (because it's a bit faster and easier). If you have time you might want to create a hard disk image (you won't need as much memory then) and then choose to install Ubuntu when given the chance.
* Click start, and when prompted for a CD image choose the Ubuntu ISO you downloaded
* Wait until a picture of a keyboard appears at the bottom of the screen, then press enter
* Select a language, and then choose `Try Ubuntu` with the arrow keys
* When it's booted, press Alt-F2, type `gnome-terminal`, then enter. **Note:** You could also just press Ctrl-Alt-F2 to get a faster but less shiny-looking terminal window.
* In the terminal, type: `sudo add-apt-repository ppa:terry.guo/gcc-arm-embedded` and press enter when prompted
* Type `sudo apt-get update`
* Type `sudo apt-get install gcc-arm-none-eabi git` and press 'Y' if prompted
* Type `git clone https://github.com/espruino/Espruino.git`
* Type `cd Espruino`
* Type `ESPRUINO_1V3=1 RELEASE=1 make` and wait
* Espruino is now built. See the documentation under **Building under Linux** for more examples.
* When you exit the VM, make sure you choose `Save State`. If you `Power Off` you will lose everything you've done so far.

There's some more information on how to do this on the forum at http://forum.espruino.com/conversations/151 including links to a pre-made [Amazon EC2 instance](http://forum.espruino.com/conversations/151/?offset=25#comment20326).

----

### To flash Espruino from the VM

* Plug the Espruino board in while holding BTN1, and wait for Windows to finish connecting to the USB device
* Go into the VirtualBox Manager (There's no need to stop your VM)
* Click on `USB`, then click on the icon with the `+` sign (With the tooltip 'Adds a new USB filter ... selected USB device')
* Click on the device labelled `STMicroelectronics STM32 ...`
* Now unplug the Espruino board, wait a few seconds, and plug it back in (holding BTN1 again)
* Go back into the VM, and type `sudo ESPRUINO_1V3=1 RELEASE=1 make serialflash` 
* Your board will now be flashed

**Note:** if you want to you can change permissions so you don't need `sudo` by typing `sudo cp misc/45-espruino.rules /etc/udev/rules.d;sudo udevadm control --reload-rules` and then re-inserting the board.

