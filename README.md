Espruino JavaScript for Microcontrollers
========================================
<pre>
 _____                 _
|   __|___ ___ ___ _ _|_|___ ___
|   __|_ -| . |  _| | | |   | . |
|_____|___|  _|_| |___|_|_|_|___|
          |_|      
</pre>
http://www.espruino.com &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; [![Join the chat at https://gitter.im/espruino/Espruino](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/espruino/Espruino?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)


About
-----

Espruino is a JavaScript interpreter for microcontrollers. It is designed to fit into devices with as little as 128kB Flash and 8kB RAM.

Please support Espruino by [ordering one of our official boards](http://www.espruino.com/Order).


Documentation
------------

If you have an Espruino board, please read [the Quick Start Guide](http://www.espruino.com/Quick+Start) first.

Browse the [Espruino Website](http://www.espruino.com) (try using search in the top right), and read the [FAQ](http://www.espruino.com/FAQ).

There's also a [Reference](http://www.espruino.com/Reference) for JavaScript commands as well as [Tutorials](http://www.espruino.com/Tutorials). However the documentation on the Espruino website will match the version [available for download](http://www.espruino.com/Download) but **not** the latest version on GitHub.

Builds for the [Espruino Board](http://www.espruino.com/EspruinoBoard) and [Pico Board](http://www.espruino.com/Pico) (built automatically for each Git commit) are [available from here](http://www.espruino.com/binaries/git)

Other documentation of use is:

* [The Forum](http://forum.espruino.com/)
* [FAQ](http://www.espruino.com//FAQ)
* [Troubleshooting](http://www.espruino.com/Troubleshooting)
* [Performance Notes](http://www.espruino.com/Performance)
* [Implementation Notes](http://www.espruino.com/Internals)
* [Build Process Notes](README_BuildProcess.md)
* [Building Espruino](README_Building.md)
* [Making your own libraries](libs/README.md)
* [Hardware Debugging Notes](http://www.espruino.com/AdvancedDebug)


License
-------

Please see the [LICENSE](LICENSE) file


Found a Bug?
------------

Please check that it hasn't [already been found](https://github.com/espruino/Espruino/issues) or [been covered on our forum](http://www.espruino.com/Forum)

Please [submit bugs](https://github.com/espruino/Espruino/issues) with clear steps to reproduce them (ideally with a test case for the ```tests``` directory). Please be aware that we're very busy, so if you report something (especially if it doesn't contain a test or a pull request) it may not be fixed for quite some time.


Building 
--------

Check out [the page on building Espruino](README_Building.md)


Testing
-------

There are a bunch of tests in the `tests` directory. See [`tests/README.md`](tests/README.md) for examples on how to run them.


Current State
-------------

The officially supported boards are the [Original Espruino Board](http://www.espruino.com/EspruinoBoard) and the  [Espruino Pico Board](http://www.espruino.com/Pico). The [Web IDE](http://www.espruino.com/webide) is able to automatically download and flash the latest version of Espruino for you.

While Espruino can run on other boards, we make no money from them and so cannot afford to test, fix or support the firmware on them. We're dependent on the community.

You can download binaries from http://www.espruino.com/Download (these aren't the latest, but are more likely to work with your board).

If you are a board manufacturer interested in getting your board officially supported, please [Contact Us](http://www.espruino.com/Contact+Us).

* [Original Espruino Board](http://www.espruino.com/EspruinoBoard) - great support.
* [Espruino Pico Board](http://www.espruino.com/Pico) - great support.
* Linux - WORKING
* [STM32VLDISCOVERY](http://www.espruino.com/ReferenceSTM32VLDISCOVERY) - WORKING - limited memory so some features removed
* [STM32F3DISCOVERY](http://www.espruino.com/ReferenceSTM32F3DISCOVERY) - USB BROKEN
* [STM32F4DISCOVERY](http://www.espruino.com/ReferenceSTM32F4DISCOVERY) - WORKING
* STM32F401CDISCOVERY - appears WORKING, but very little testing done
* STM32F429IDISCOVERY - WORKING over serial (A9/A10). No USB and no LCD support
* NRF52832 Preview Development Kit - WORKING with limited functionality. Able to interface with Espruino over BLE (send commands from smartphone or computer) or serial as normal (send commands from chrome IDE or terminal).
* [HY STM32 2.4"](http://www.espruino.com/ReferenceHYSTM32_24) - WORKING
* [HY STM32 2.8"](http://www.espruino.com/ReferenceHYSTM32_28) - WORKING - limited memory so some features removed
* [HY STM32 3.2"](http://www.espruino.com/ReferenceHYSTM32_32) - WORKING
* [Olimexino STM32 / Leaflabs Maple](http://www.espruino.com/ReferenceOLIMEXINO_STM32) - WORKING - limited memory so some features removed
* Carambola - WORKING - GPIO via filesystem (no I2C)
* Raspberry Pi - WORKING - GPIO via filesystem or wiringPi (no I2C)
* Sony SmartWatch - NOT WORKING - USB VCP support for F2 still needed
* MBed platforms - have not worked for a while - full hardware wrapper still required
* LC-TECH STM32F103RBT6 - WORKING, but with some issues (LED inverted logic, BTN needs pullup to work)
* [ST NUCLEO-F401RE](http://www.espruino.com/ReferenceNUCLEOF401RE) - WORKING
* ST NUCLEO-F411RE - WORKING
* ESP8266 - Beta - Reasonably stable, but expect to find issues
* NRF51/52 - Alpha - very early in development (GPIO, but not peripherals, no sleep or save)
* Arduino (AVR) - NOT POSSIBLE due to the Hardward architecture of AVRs, even though it would fit into an ATMEGA2560. If `avr-gcc` ever gains an easy way to emulate Von Neumann architecture then it might be portable, but for now it isn't.
* Arduino (ARM) - very doable, but no work has been done on this.


Modification
------------

**Check out [the documentation on the build process](README_BuildProcess.md) first** - this should 
clear up a lot of potential questions about the Espruino architecture.

Please see [CONTRIBUTING.md](CONTRIBUTING.md) for some hints about code style/etc.

You can auto-build documentation for all source files - see [doxygen/README.md](doxygen/README.md)

Any more questions? [ask on the forum.](http://www.espruino.com/Forum)

### Porting to new devices

Currently there are a bunch of different files to modify. Eventually the plan is to fit everything into `boards/BOARDNAME.py` and to auto-generate the rest of the config files.

* Most build options handled in `Makefile`
* Extra libraries like USB/LCD/filesystem in `Makefile`
* `boards/*.py` files describe the CPU, available pins, and connections - so the relevant linker script, headers + docs can be created
* `boards/pins/*.csv` are copies of the 'pin definitions' table in the chip's datasheet. They are read in for STM32 chips by the `boards/*.py` files, but they are not required - see `boards/MICROBIT.py` for an example.
* Processor-specific code in `targets/ARCH` - eg. `targets/stm32`, `targets/linux`
* Processor-specific libs (like the SDK) in `targetlibs/ARCH`
* `src/jshardware.h` is effectively a simple abstraction layer for SPI/I2C/etc, which should be implemented in `targets/ARCH/jshardware.c`

### Adding libraries

* Create `jswrap_mylib.c/h` in `libs/`
* Create library functions (see examples in other jswrap files, also the comments in `scripts/common.py`)

See [libs/README.md](libs/README.md) for a short tutorial on how to add your own libraries.


Using Espruino in your Projects
---------------------------

If you're using Espruino for your own personal projects - go ahead, we hope you have fun - and please let us know what you do with it on http://www.espruino.com/Forum!

If you're planning on selling the Espruino software on your own board, please:

* Let us know, we might be able to help.
* Read the terms of the MPLv2 Licence that Espruino is distributed under, and make sure you comply with it
* MPLv2 dictates that any files that you modify must be made available in source form. New files that you create don't need to be made available (although we'd encourage it!)
* You won't be able to call your board an 'Espruino' board unless it's agreed with us (we own the trademark)
* You must explain clearly in your documentation that your device uses Espruino internally
* Please don't fork Espruino - improvements get very hard to share, and in the long run everyone loses.
* Please give something back to the project - be it code improvements, documentation or support.

We spend a *lot* of time supporting Espruino on the forums, but can only do
so because we make money from the sales of Espruino boards. If your users request
support from us then we have absolutely no obligation to help them. However, we'll
be a lot more motivated if you're actively helping to improve Espruino for all its
users (not just your own).

