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

[![Build Status](https://travis-ci.org/espruino/Espruino.svg?branch=master)](https://travis-ci.org/espruino/Espruino)

About
-----

Espruino is a JavaScript interpreter for microcontrollers. It is designed for devices with as little as 128kB Flash and 8kB RAM.

Please support Espruino by [ordering one of our official boards](http://www.espruino.com/Order) or [donating](http://www.espruino.com/Download)


Documentation
------------

If you have an Espruino board, please read [the Quick Start Guide](http://www.espruino.com/Quick+Start) first.

Browse the [Espruino Website](http://www.espruino.com) (try using search in the top right), and read the [FAQ](http://www.espruino.com/FAQ).

There's also a [Reference](http://www.espruino.com/Reference) for JavaScript commands as well as [Tutorials](http://www.espruino.com/Tutorials). However the documentation on the Espruino website will match the version [available for download](http://www.espruino.com/Download) but **not** the latest version on GitHub.

Builds for the [Espruino Board](http://www.espruino.com/EspruinoBoard) and [Pico Board](http://www.espruino.com/Pico) (built automatically for each Git commit) are [available from here](http://www.espruino.com/binaries/git)

Other documentation of use is:

* [The Forum](http://forum.espruino.com/)
* [FAQ](http://www.espruino.com/FAQ)
* [Troubleshooting](http://www.espruino.com/Troubleshooting)
* [Performance Notes](http://www.espruino.com/Performance)
* [Implementation Notes](http://www.espruino.com/Internals)
* [Build Process Notes](README_BuildProcess.md)
* [Building Espruino](README_Building.md)
* [Making your own libraries](libs/README.md)
* [Hardware Debugging Notes](http://www.espruino.com/AdvancedDebug)


Support / Bugs
--------------

First, please try and check that your problem hasn't [already been found](https://github.com/espruino/Espruino/issues) or [covered on our forum](http://www.espruino.com/Forum).

[Submit bugs](https://github.com/espruino/Espruino/issues) with clear steps to reproduce them: a **small** test case (not your whole program), and an actual and expected result. If you can't come up with these, please [post on the forum](http://www.espruino.com/Forum) first as it may just be something in your code that we can help out with.

Work on Espruino is supported by [sales of our boards](http://www.espruino.com/Order).

**If your board isn't made by us but came pre-installed with Espruino then you should contact the manufacturers.**

We try and support users of the boards we sell, but if you bought a non-official board your issue may not get addressed. In this case, please consider [donating](http://www.espruino.com/Download) to help cover the time it takes to fix problems (even so, we can't guarantee to fix every problem).


License
-------

Please see the [LICENSE](LICENSE) file


Building
--------

Check out [the page on building Espruino](README_Building.md)


Testing
-------

There are a bunch of tests in the `tests` directory. See [`tests/README.md`](tests/README.md) for examples on how to run them.


Current State
-------------

The [officially supported boards](http://www.espruino.com/Order) are the best supported. They come pre-installed with Espruino and you are able to easily download and flash the latest versions of Espruino to them.

While Espruino can run on other boards, we make no money from them and so cannot afford to test, fix or support the firmware on them. We're dependent on the community.

You can download binaries from http://www.espruino.com/Download

If you are a board manufacturer interested in getting your board officially supported, please [check out this page](http://www.espruino.com/Business).

For a list of supported boards, please see the [boards folder](https://github.com/espruino/Espruino/tree/master/boards).

Main supported platforms are:

* STM32 (F1, F3, F4, L4)
* nRF52
* nRF51
* ESP8266
* ESP32
* Linux

Espruino has been ported to other boards and platforms (such as EFM32 and SAMD), but these have a habit of being contributed and then never maintained. All boards that this has happened to reside in the [UNMAINTAINED_BOARDS](https://github.com/espruino/Espruino/tree/UNMAINTAINED_BOARDS) branch.


Modification
------------

**Check out [the documentation on the build process](README_BuildProcess.md) first** - this should
clear up a lot of potential questions about the Espruino architecture.

Please see [CONTRIBUTING.md](CONTRIBUTING.md) for some hints about code style/etc.

You can auto-build documentation for all source files - see [doxygen/README.md](doxygen/README.md)

Any more questions? [ask on the forum.](http://www.espruino.com/Forum)

### Porting to new devices

If you're using an existing architecture everything can be done from `boards/BOARDNAME.py`. See a similar board's `.py` file as an example.

However for a new architecture there are a bunch of different files to modify.

* `boards/*.py` files describe the CPU, available pins, and connections - so the relevant linker script, headers + docs can be created
* `boards/pins/*.csv` are copies of the 'pin definitions' table in the chip's datasheet. They are read in for STM32 chips by the `boards/*.py` files, but they are not required - see `boards/MICROBIT.py` for an example.
* Global build options are handled in `Makefile`
* The `make` directory contains arch-specific Makefile fragments
* Extra libraries like USB/LCD/filesystem are in `Makefile`
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
