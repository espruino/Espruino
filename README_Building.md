Building
========


Under Linux
-----------
  
Espruino is easy to build under Linux, and it is possible to build under MacOS with some effort. If you don't have Linux it's **much** easier to install it in a Virtual Machine. See the heading **Building under Windows/MacOS with a VM** below for more information.

### for STM32 Boards (incl. [Espruino Board](http://www.espruino.com/EspruinoBoard))
  
The (previously suggested) CodeSourcery GCC compiler is no longer available. We'd suggest you use [gcc-arm-none-eabi](https://launchpad.net/gcc-arm-embedded/+download).

**YOU'LL NEED GCC 4.8** - On the 4.9 releases from launchpad above, the Original Espruino board's binary will build but USB will not work (this is believed to be due to an out of order register write in ST's USB code caused by an optimisation pass in GCC).

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

### for Nordic Semiconductor's NRF52832 Preview Development Kit.

The (previously suggested) CodeSourcery GCC compiler is no longer available. We'd suggest you use [gcc-arm-none-eabi](https://launchpad.net/gcc-arm-embedded/+download).

Download the compiler, set up your path so you have access to it, and run:

```NRF52832DK=1 BLE_INTERFACE=1 RELEASE=1 make```

Note: If you want to communicate with espruino through wired USB connection remove 'BLE_INTERFACE=1'. This will run Espruino so that you can program it from the chrome IDE as you normally would instead of over BLE.

To program the nRF52 Development Kit with Espruino:

1.) You will need nRFgo Studio to program the nRF52 with .hex files. Download this here: https://www.nordicsemi.com/chi/node_176/2.4GHz-RF/nRFgo-Studio. This will also install all necessary software to program your nRF52 device, as well as nRF5x tools (which is a more advanced way to program the nRF52 with more functionality if you would like).

2.) If you want to use BLE, first you need to download the SoftDevice provided by Nordic to enable BLE. The softdevice can be downloaded here: https://www.nordicsemi.com/eng/Products/Bluetooth-Smart-Bluetooth-low-energy/nRF52-Preview-DK and is in the Downloads tab as S132-SD "S132 nRF52 SoftDevice."

3.) Now open nRFgo Studio. Connect the nRF52 DK to your computer. A device should pop up in nRFgo studio under nRF52 development boards as "Segger XXXXXXX." Select this device. Click erase all (this will erase the device). After this succesfully completes click the Program SoftDevice tab in the upper right part of the screen. Browse for the SoftDevice you downloaded in the File to program field. Once this is selected hit the Program button.

4.) Now that the SoftDevice is programmed, you can program Espruino! (The order, SoftDevice then Espruino is very important!). Click the Program Application tab now and Browse for espruino_1vxx.xx_nrf52832.hex. Select this file and click program.

5.) Now that Espruino is running on the board LED1 should be blinking. This means that Espruino is advertising. Download the nRF toolbox app here: https://www.nordicsemi.com/eng/Products/nRFready-Demo-Apps/nRF-Toolbox-App. Open this on your smart phone, open the UART module, and then click connect. Select Nordic_Espruino. Now you are connected!

6.) Now you can send javascript commands from your smartphone to Espruino! Go to the log in the nRF UART app and try typing 25*4 "ENTER". Note that you need to hit the new line button for espruino to work! Espruino should return >100. Now try digitalWrite(18, 0);"ENTER" This will turn LED2 (pin18 on the nRF52 DK) on! (Note 0 turns led on, 1 turns led off). You can program the device the same way you do in the Espruino IDE, just make sure you send an enter after each command. Experiment with the app and how you can assign a custom script to each button!

-- Note that limited functionality is implemented on the nRF52. Feel free to help! Currently working on NFC touch to pair so you can tap the device with your smartphone to connect without opening the app and then scanning. Also thinking of ways to edit scripts (typing long programs on smartphone is tedious).

----

### for esp8266

In order to compile for the esp8266 on Linux several pre-requisites have to be installed:
- the esp-open-sdk from https://github.com/pfalcon/esp-open-sdk, use make STANDALONE=n
- the Espressif SDK (version 1.4.0 as of this writing) from http://bbs.espressif.com/viewforum.php?f=46

To run make you need to pass a couple of environment variables to `make`.  These include:

* `ESP8266_BOARD = 1`
* `FLASH_4MB` if you have an esp-12
* `ESP8266_SDK_ROOT = <Path to the 1.4 SDK>`
* `COMPORT = </dev/ttyUSB0|COM1|...>`

Ensure that your program Path includes the folders/directories for:

* Xtensa GCC compiler
* esptool
* Python 3.4

The easiest is to place
the following lines into a script, adapt it to your needs and then run it.
```
#! /bin/bash
export ESP8266_BOARD=1
export FLASH_4MB=1
export ESP8266_SDK_ROOT=/esp8266/esp_iot_sdk_v1.4.0
export PATH=$PATH:/esp8266/esp-open-sdk/xtensa-lx106-elf/bin/
export COMPORT=/dev/ttyUSB0
make $*
```

* If you do `make flash` it will try to flash your esp8266 module over serial
* If you do `make wiflash` it will try to flash you esp8266 module over wifi, which assumes
  that it's already running Espruino
* You will also get an `espruino_esp8266_1v00_*.tgz` archive, which contains everything you
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

