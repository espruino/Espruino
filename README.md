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

NOTE: This software is beta and is provided as-is, and won't be considered even remotely final until we've released the Espruino Board. As such, don't expect support, and do expect it to change rapidly and without warning.

About
-----

It'd probably help to read the [FAQ](http://www.espruino.com/FAQ), and specifically the page about [Performance](http://www.espruino.com/Performance) as it contains information about how Espruino itself works. There's also the auto-generated [Reference](http://www.espruino.com/Reference) for JavaScript commands.


License
-------

Please see the [LICENSE](LICENSE) file

Contributing
------------

Please see [CONTRIBUTING.md](CONTRIBUTING.md)

Current State
-------------

Please note that this is BETA. We've been working hard on the Espruino Board support but we haven't had time to check the other boards properly.

* Espruino Board - working
* Linux - working
* STM32VLDISCOVERY - ?
* STM32F3DISCOVERY - ?
* STM32F4DISCOVERY - ?
* HY STM32 2.4" - ?
* HY STM32 2.8" - ?
* HY STM32 3.2" - ?
* Olimexino - ?
* Carambola - ?
* Raspberry Pi - ?
* Sony SmartWatch - USB VCP support still needed
* MBed platforms - have not worked for a while - hardware wrapper still needed
* Arduino - has never worked. Compiles but doesn't even seem to get past init

Using
-----

If you're using Espruino for your own personal projects - go ahead, we hope you have fun - and please let us know what you do with it on http://www.espruino.com/Forum!

However if you're planning on including the Espruino software in your commercial product, please read the following:

* If you sell a board with the Espruino software on, you cannot call it 'Espruino' but you must explain clearly that it uses 'Espruino' internally (we own the trademark).
* If you're not willing to support us, we won't support you (or your users). Please contact us about this.


Building
--------
  
We suggest that you use the CodeSourcery GCC compiler, but paths in Makefile may need changing...

```  BOARDNAME=1 RELEASE=1 make```

* See the top of Makefile for board names
* Without `RELEASE=1`, assertions are kept in the code (which is good for debugging, bad for performance + code size)
* `BOARDNAME=1 RELEASE=1 make serialflash` will flash to /dev/ttyUSB0 using the STM32 serial bootloader (what's needed for Espruino + HY boards)
* `BOARDNAME=1 RELEASE=1 make flash` will flash using st-flash if discovery, or maple bootloader if using that board

Directories and Files
---------------------

* `ChangeLog`:          What's new
* `TODO`:               List of things to do
* `boards/`:            Information on boards, used to auto-generate a lot of the code
* `code/`:              Example JavaScript code
* `gen/`:               Auto-Generated Source Files
* `libs/`:              Optional libraries to include in Espruino (Math, Filesystem, Graphics, etc)
* `linker/`:            Linker files for various processors
* `misc/`:              random other stuff
* `scripts/`:           Scripts for generating files in gen, and for analysing code/compilation/etc
* `src/`:               Main source code
* `targetlibs/`:        Libraries for targeted architectures
* `targets/`:           Specific code for targeted architectures
* `tests/`:             Testcases
* `dist_*`:             files to be copied into distribution zip file

Adding more devices
-------------------

Currently there are a bunch of different files to modify. Eventually the plan is to fit everything into boards/BOARDNAME.py and to auto-generate the rest of the config files.

* Most build options handled in Makefile
* Extra libraries like USB/LCD/filesystem in Makefile
* Linker Scripts are in linker/
* boards/*.py files handle loading the list of available pins so the relevant headers + docs can be created
* Processor-specific code in targets/stm32, targets/linux, etc.
* Processor-specific libs in targetlibs/foo 
* src/jshardware.h is effectively a simple abstraction layer for SPI/I2C/etc
* targets/stm32/jshardware.c also has flash-size-specific defines
* libs/fat_sd and libs/lcd still have some device-specific defines in too
* If you're low on flash, you might want to modify check_size.sh

Adding libraries
-------------------

* Create `jswrap_mylib.c/h` in `libs/`
* Create library functions (see examples in other jswrap files, also the comments in `scripts/common.py`)


Arduino Compile (beta)
----------------------
* Ensure that `targets/arduino/utility` is symlinked to `src`
* Symlink `...arduino_workspace/libraries/Espruino` to `targets/arduino`

Cross Compile for Raspberry Pi
------------------------------
```
cd targetlibs
mkdir raspberrypi
cd raspberrypi
git clone git://github.com/raspberrypi/tools.git
sudo apt-get install ia32-libs
```

Cross Compile for Carambola (OpenWRT)
-------------------------------------
* Follow instructions at <https://github.com/8devices/carambola> to set toolchain up in ```~/workspace/carambola```
* Run ```CARAMBOLA=1 make```
