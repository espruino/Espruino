The Make system has the following targets:

* `clean` - Clean the build
* `all` - Compile the build culminating in the ELF executable used for flashing the firmware
* `flash` - Generate the firmware files and flash.  Calls `all` as needed.

In order to compile the ESP8266/Espruino project, changes were required to the Espruino Makefile.  Here is a record of those changes.

1. A new board type called "ESP8266_ESP12" was added.  There will be a board type for each of the ESP modules such as ESP-1, ESP-12, NodeMCU etc.   It is important to note that there is already a variable called ESP8266 which is about piggybacking an ESP8266 onto an Espruino board.
2. A new ESP8266_ESP12.py file was created under boards.
3. The `CCPREFIX` used in the Makefile was added if we are building an ESP8266.  The prefix is `xtensa-lx106-elf-`.
4. The compiler flags we want are:

* -Os
* -std=gnu99
* -fno-builtin
* -Wpointer-arith
* -Wundef
* -Werror
* -Wl,-EL
* -fno-inline-functions
* -nostdlib
* -mlongcalls
* -mtext-section-literals
* -D__ets__
* -DICACHE_FLASH
* -I directories

5. The linker flags we want are:
* -LC:\Espressif\ESP8266_SDK/lib
* -T./ld2/eagle.app.v6.ld
* -nostdlib
* -Wl,--no-check-sections
* -u call_user_start
* -Wl,-static 
* -Wl,--start-group -lc -lgcc -lhal -lphy -lpp -lnet80211 -llwip -lwpa -lmain build/espruino_app.a -Wl,--end-group 

##Linker configuration
The linker uses a file called `eagle.app.v6.ld` to map the address spaces.  By default, the Espressif supplied version maps text code to the second half of a 512K flash chip.  This means that only 256K appears available.  Since our project will need more than 300K of flash for storage, this won't work.  To solve the problem we need to create a copy of the file and edit it (leave the original alone).  Find the line which reads:

    irom0_0_seg : org = 0x40240000, len = 0x3C000

and change it to

    irom0_0_seg : org = 0x40210000, len = 0x60000

This changes the start address of the firmware from `0x4 0000` to `0x1 0000`.   The Makefile assumes that the name of the new template is `0x10000_eagle.app.v6.ld`.

##ESP8266 Specific files
The rules are that one should not modify any of the Espruino supplied files.  Rather, the items that are specific to an ESP8266 go into their own hardware specific files.  The architecture of Espruino allows for just this by providing a folder called `targets/<boardname>`.  For this project, here is where we define ESP8266 specific items.  Specifically, we have:

* `esp8266_board_utils.c`
* `esp8266_board_utils.h`
* `ESP8266_board.h`
* `jshardware.c` - The primary mapping from logical Espruino functions to ESP8266 specific.
* `user_main.c` - The entry point into ESP8266 execution.
* `user_config.h` - The mandatory, but empty, header file (see ESP8266 docs).
* `uart.c` - The Espressif supplied UART code.
* `driver/uart.h` - The Espressif supplied UART code.
* `driver/uart_register.h` - The Espressif supplied UART code.
* `telnet_client.c` - Experimental Telnet server.
* `telnet.h` - Experimental Telnet server.

In addition, we have the ESP8266 networking files located in the folder called `libs/network/esp8266`.  The files there
include:

* `jswrap_esp8266.c` - Description and implementation JS exposed functions.
* `jswrap_esp8266.h` - Header files for JS exposed functions.
* `network_esp8266.c` - Implementation of JSNetwork service for ESP8266.
* `network_esp8266.h` - Definition of JSNetwork service for ESP8266.