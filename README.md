Espruino JavaScript for Microcontrollers
========================================
<pre>
 _____                 _
|   __|___ ___ ___ _ _|_|___ ___
|   __|_ -| . |  _| | | |   | . |
|_____|___|  _|_| |___|_|_|_|___|
          |_|      
</pre>
http://www.espruino.com

About
-----

Espruino is a JavaScript interpreter for microcontrollers. It is designed to fit into devices with as little as 128kB Flash and 8kB RAM.

Please support Espruino by [ordering one of our official boards](http://www.espruino.com/Order).


Documentation
------------

It'd be best to browse the [Espruino Website](http://www.espruino.com) and read the [FAQ](http://www.espruino.com/FAQ) first.

There's also a [Reference](http://www.espruino.com/Reference) for JavaScript commands as well as [Tutorials](http://www.espruino.com/Tutorials). However please note that this repository is under heavy development, and the documentation on the Espruino website will match the version [available for download](http://www.espruino.com/Download) but **not** the latest version on GitHub.

Builds for the [Espruino Board](http://www.espruino.com/EspruinoBoard) (built automatically for each Git commit) are [available from here](http://www.espruino.com/binaries/git)

Other documentation of use is:

* [The Forum](http://forum.espruino.com/)
* [FAQ](http://www.espruino.com//FAQ)
* [Troubleshooting](http://www.espruino.com/Troubleshooting)
* [Performance Notes](http://www.espruino.com/Performance)
* [Implementation Notes](http://www.espruino.com/Internals)
* [Debugging Notes](http://www.espruino.com/AdvancedDebug)


License
-------

Please see the [LICENSE](LICENSE) file


Found a Bug?
------------

Please check that:
* It hasn't [already been found](https://github.com/espruino/Espruino/issues) or [been covered on our forum](http://www.espruino.com/Forum)
* You're not just looking at outdated documentation (See the [Building](#Building) section to see how to build documentation)

Please [submit bugs](https://github.com/espruino/Espruino/issues) with clear steps to reproduce them (ideally with a test case for the ```tests``` directory), and if at all possible try and include a patch to fix them. Please be aware that we have a whole bunch of outstanding issues, so if you report something (especially if it doesn't contain a test or a pull request) it may not be fixed for quite some time.


Contributing
------------

Please see [CONTRIBUTING.md](CONTRIBUTING.md)


Current State
-------------

You can download binaries from http://www.espruino.com/Download (these aren't the latest, but are more likely to work with your board)

The only officially supported board is the [Espruino Board](http://www.espruino.com/EspruinoBoard). While Espruino can run on other boards, we make no money from them and so cannot afford to test, fix or support the firmware on them.

If you are a board manufacturer interested in getting your board officially supported, please [Contact Us](http://www.espruino.com/Contact+Us).

* [Espruino Board](http://www.espruino.com/EspruinoBoard) - great support.
* Linux - WORKING
* STM32VLDISCOVERY - WORKING - limited memory so some features removed
* STM32F3DISCOVERY - USB BROKEN
* STM32F4DISCOVERY - WORKING
* STM32F401CDISCOVERY - appears WORKING, but very little testing done
* STM32F429IDISCOVERY - WORKING over serial (A9/A10). No USB and no LCD support
* NRF52832 Preview Development Kit - WORKING with limited functionality. Able to interface with Espruino over BLE (send commands from smartphone or computer) or serial as normal (send commands from chrome IDE or terminal).
* HY STM32 2.4" - WORKING
* HY STM32 2.8" - WORKING - limited memory so some features removed
* HY STM32 3.2" - WORKING
* Olimexino - WORKING - limited memory so some features removed
* Carambola - WORKING - GPIO via filesystem (no SPI or I2C)
* Raspberry Pi - WORKING - GPIO via filesystem (no SPI or I2C)
* Sony SmartWatch - NOT WORKING - USB VCP support for F2 still needed
* MBed platforms - have not worked for a while - full hardware wrapper still required
* ARDUINOMEGA2560 - compiles, but has never worked. Almost certainly due to ints being 16 bits.
* LC-TECH STM32F103RBT6 - WORKING, but with some issues (LED inverted logic, BTN needs pullup to work)
* ST NUCLEO-F401RE - beta status
* ST NUCLEO-F411RE - early alpha status

Building under Linux
------------------
  
Espruino is easy to build under Linux, and it is possible to build under MacOS with some effort. If you don't have Linux it's **much** easier to install it in a Virtual Machine. See the heading **Building under Windows/MacOS with a VM** below for more information.

### Building for STM32 Boards (incl. [Espruino Board](http://www.espruino.com/EspruinoBoard))
  
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

### Building for Nordic Semiconductor's NRF52832 Preview Development Kit.

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

### Building for Linux

Simple: Just run `make`

## Building for Arduino

This does not work right now - these steps are only to get you started!

* `sudo apt-get install gcc-avr avr-libc avrdude`
* `sudo cp -r /usr/share/arduino/hardware/arduino targetlibs/arduino_avr`
* `ARDUINOMEGA2560=1 make`
* You'll then need to flash the binary yourself

### Building for Raspberry Pi

On the Pi, just run `make`.

Or to cross-compile:

```
cd targetlibs
mkdir raspberrypi
cd raspberrypi
git clone git://github.com/raspberrypi/tools.git
sudo apt-get install ia32-libs
```

### Building for Carambola (OpenWRT)

To cross compile,

* Follow instructions at <https://github.com/8devices/carambola> to set toolchain up in ```~/workspace/carambola```
* Run ```CARAMBOLA=1 make```

### Building Documentation

```python scripts/build_docs.py ```

This will create a file called ```functions.html```

### Building under Windows/MacOS with a VM (Vagrant)

* Clone this repository.
* Download and install the correct [VirtualBox](https://www.virtualbox.org/) for your platform. eg. If you have Windows, download 'VirtualBox for Windows Hosts'.
* Download and install the correct [Vagrant](https://www.vagrantup.com/downloads.html) for your platform.
  > If running on MacOS, the two previous steps can be accomplished easily with [Homebrew Cask](http://caskroom.io):  `brew cask install virtualbox vagrant` will do it.
* In your terminal application, navigate to your cloned working copy.
* Execute `vagrant up`.  This will take a little while while the box is downloaded, and your virtual machine is provisioned.
* When it is complete, execute `vagrant ssh`, which will open an ssh session into your new VM. 
* Execute `cd /vagrant && ESPRUINO_1V3=1 RELEASE=1 make` and wait.
* Espruino is now built. See the documentation under **Building under Linux** for more examples.
* To terminate the ssh session, simply execute `exit`.
* `vagrant suspend` will pause the VM, `vagrant halt` will stop it, and `vagrant up` will bring it back up again.  See Vagrant's ["Getting Started"](http://docs.vagrantup.com/v2/getting-started/index.html) page for further information.

Building under Windows/MacOS with a VM
---------------------------------

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


### To flash Espruino from the VM

* Plug the Espruino board in while holding BTN1, and wait for Windows to finish connecting to the USB device
* Go into the VirtualBox Manager (There's no need to stop your VM)
* Click on `USB`, then click on the icon with the `+` sign (With the tooltip 'Adds a new USB filter ... selected USB device')
* Click on the device labelled `STMicroelectronics STM32 ...`
* Now unplug the Espruino board, wait a few seconds, and plug it back in (holding BTN1 again)
* Go back into the VM, and type `sudo ESPRUINO_1V3=1 RELEASE=1 make serialflash` 
* Your board will now be flashed

**Note:** if you want to you can change permissions so you don't need `sudo` by typing `sudo cp misc/45-espruino.rules /etc/udev/rules.d;sudo udevadm control --reload-rules` and then re-inserting the board.



Testing
------

There are a bunch of tests in the `tests` directory. See [`tests/README.md`](tests/README.md) for examples on how to run them.

Modifying
--------

### Directories and Files

* `ChangeLog`:          What's new
* `TODO`:               List of things to do
* `boards/`:            Information on boards, used to auto-generate a lot of the code
* `gen/`:               Auto-Generated Source Files
* `libs/`:              Optional libraries to include in Espruino (Math, Filesystem, Graphics, etc)
* `misc/`:              Other useful things
* `scripts/`:           Scripts for generating files in gen, and for analysing code/compilation/etc
* `src/`:               Main source code
* `targetlibs/`:        Libraries for targeted architectures
* `targets/`:           Specific code for targeted architectures
* `tests/`:             JavaScript Testcases
* `benchmark/`:         JavaScript Benchmarks
* `dist_*`:             files to be copied into distribution zip file

### Adding more devices

Currently there are a bunch of different files to modify. Eventually the plan is to fit everything into boards/BOARDNAME.py and to auto-generate the rest of the config files.

* Most build options handled in `Makefile`
* Extra libraries like USB/LCD/filesystem in `Makefile`
* `boards/*.py` files describe the CPU, available pins, and connections - so the relevant linker script, headers + docs can be created
* `boards/pins/*.csv` are copies of the 'pin definitions' table in the chip's datasheet. They are read in for STM32 chips by the `boards/*.py` files, but they are not required - see `boards/RASPBERRYPI.py` for an example.
* Processor-specific code in `targets/ARCH` - eg. `targets/stm32`, `targets/linux`
* Processor-specific libs in `targetlibs/foo` 
* `src/jshardware.h` is effectively a simple abstraction layer for SPI/I2C/etc

### Adding libraries

* Create `jswrap_mylib.c/h` in `libs/`
* Create library functions (see examples in other jswrap files, also the comments in `scripts/common.py`)

See [libs/README.md](libs/README.md) for a short tutorial on how to add your own libraries.


Using Espruino in your Projects
---------------------------

If you're using Espruino for your own personal projects - go ahead, we hope you have fun - and please let us know what you do with it on http://www.espruino.com/Forum!

However if you're planning on selling the Espruino software on your own board, please talk to us:

* Read the terms of the MPLv2 Licence that Espruino is distributed under, and make sure you comply with it
* MPLv2 dictates that any files that you modify must be made available in source form. New files that you create don't need to be made available (although we'd encourage it!)
* You won't be able to call your board an 'Espruino' board unless it's agreed with us (we own the trademark)
* You must explain clearly in your documentation that your device uses Espruino internally
* If you're profiting from our hard work without contributing anything back, we're not going to very motivated to support you (or your users)
