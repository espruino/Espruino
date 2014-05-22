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

It'd probably help to browse the [Espruino Website](http://www.espruino.com). read the [FAQ](http://www.espruino.com/FAQ).

There's also the auto-generated [Reference](http://www.espruino.com/Reference) for JavaScript commands as well as the [Tutorials](http://www.espruino.com/Tutorials) on the website. However please note that this repository is under heavy development, and the documentation on the Espruino website will match the version [available for download](http://www.espruino.com/Download) but **not** the latest version on GitHub.

Builds for the [Espruino Board](http://www.espruino.com/EspruinoBoard) (built automatically for each Git commit) are [available from here](http://www.espruino.com/binaries/git)

Other Documentation
------------------

As well as above, please see:

* [The Forum](http://forum.espruino.com/)
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

Please [submit bugs](https://github.com/espruino/Espruino/issues) with clear steps to reproduce them (ideally a test case for the ```tests``` directory), and if at all possible try and include a patch to fix them. Please be aware that we have a whole bunch of outstanding issues (some quite large), so if you report something (especially if it doesn't contain a test or a pull request) it may not be fixed for quite some time.


Contributing
------------

Please see [CONTRIBUTING.md](CONTRIBUTING.md)


Current State
-------------

You can download binaries from http://www.espruino.com/Download (these aren't the latest, but are more likely to work with your board)

Please note that this is BETA. We've been working hard on the Espruino Board support but we haven't had time to check the other boards properly.

* [Espruino Board](http://www.espruino.com/EspruinoBoard) - awesome.
* Linux - working
* STM32VLDISCOVERY - WORKING
* STM32F3DISCOVERY - WORKING
* STM32F4DISCOVERY - WORKING
* STM32F429IDISCOVERY - WORKING over serial (A9/A10). No USB and no LCD support
* HY STM32 2.4" - NOT WORKING - appears to crash after startup
* HY STM32 2.8" - WORKING, but screen is not black at startup
* HY STM32 3.2" - WORKING
* Olimexino - WORKING
* Carambola - ?
* Raspberry Pi - WORKING - only GPIO via filesystem (no SPI or I2C)
* Sony SmartWatch - USB VCP support still needed
* MBed platforms - have not worked for a while - hardware wrapper still needed
* Arduino - has never worked. Compiles but doesn't even get past init
* LC-TECH STM32F103RBT6 - WORKING, but with some issues (LED inverted logic, BTN needs pullup to work)


Building
--------
  
Espruino is easy to build under Linux, and it is possible to build under MacOS. We'd strongly suggest that you DO NOT TRY AND BUILD UNDER WINDOWS, and instead use a Virtual Machine. There's a good post on this here: http://forum.espruino.com/conversations/151

### For ARM Boards (incl. [Espruino Board](http://www.espruino.com/EspruinoBoard))
  
We suggest that you use the CodeSourcery GCC compiler, but paths in Makefile may need changing...

```  BOARDNAME=1 RELEASE=1 make```

* See the top of Makefile for board names
* Without `RELEASE=1`, assertions are kept in the code (which is good for debugging, bad for performance + code size)
* `BOARDNAME=1 RELEASE=1 make serialflash` will flash to /dev/ttyUSB0 using the STM32 serial bootloader (what's needed for Espruino + HY boards)
* `BOARDNAME=1 RELEASE=1 make flash` will flash using st-flash if discovery, or maple bootloader if using that board

It may complain that there isn't enough space on the chip. This isn't an issue unless you save to flash, but you can fix the error in a few ways:

* Disable the check
* Change the compile flags from `-O3` to `-Os`
* Knock out some functionality (like `USE_GRAPHICS=1`) that you don't need
* Try different compilers. `codesourcery-2013.05-23-arm-none-eabi` provides low binary size for `-O3`

**Note:** Espruino boards contain a special bootloader at `0x08000000` (the default address), with the Espruino binary moved upwards 10kb to `0x08002800`. To load the Espruino binary onto a board at the correct address, use `ESPRUINO_1V3=1 make serialflash`. If you want to make a binary that contains the bootloader as well as Espruino (like the ones on the Espruino website) use `scripts/create_espruino_image_1v3.sh`.

### Linux

Just run `make`

## Arduino (VERY beta)

* Ensure that `targets/arduino/utility` is symlinked to `src`
* Symlink `...arduino_workspace/libraries/Espruino` to `targets/arduino`

### Raspberry Pi

On the PI:

```
cd targetlibs
mkdir raspberrypi
cd raspberrypi
git clone git://github.com/raspberrypi/tools.git
sudo apt-get install ia32-libs
```

### Carambola (OpenWRT)

* Follow instructions at <https://github.com/8devices/carambola> to set toolchain up in ```~/workspace/carambola```
* Run ```CARAMBOLA=1 make```

### Documentation

```  python scripts/build_docs.py ```

This will create a file called ```functions.html```

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
* `misc/`:              random other stuff
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
* Linker Scripts are in `linker/`
* `boards/*.py` files handle loading the list of available pins so the relevant headers + docs can be created
* Processor-specific code in `targets/stm32`, `targets/linux`, etc.
* Processor-specific libs in `targetlibs/foo` 
* `src/jshardware.h` is effectively a simple abstraction layer for SPI/I2C/etc
* `targets/stm32/jshardware.c` also has flash-size-specific defines
* `libs/fat_sd` and `libs/lcd` still have some device-specific defines in too

### Adding libraries

* Create `jswrap_mylib.c/h` in `libs/`
* Create library functions (see examples in other jswrap files, also the comments in `scripts/common.py`)


Using Espruino in your Projects
---------------------------

If you're using Espruino for your own personal projects - go ahead, we hope you have fun - and please let us know what you do with it on http://www.espruino.com/Forum!

However if you're planning on selling the Espruino software on your own board, please talk to us:

* Read the terms of the MPLv2 Licence that Espruino is distributed under, and make sure you comply with it
* MPLv2 dictates that any files that you modify must be made available in source form. New files that you create don't need to be made available (although we'd encourage it!)
* You won't be able to call your board an 'Espruino' board unless it's agreed with us (we own the trademark)
* You must explain clearly in your documentation that your device uses Espruino internally
* If you're profiting from our hard work without contributing anything back, we're not going to very motivated to support you (or your users)

