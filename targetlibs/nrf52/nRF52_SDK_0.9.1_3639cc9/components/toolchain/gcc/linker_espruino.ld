/* Linker script to configure memory regions. */

  SEARCH_DIR(.)
  GROUP(-lgcc -lc -lnosys)

  MEMORY
  {
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x80000
    RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x08000
  }

INCLUDE "/home/michael/Documents/Espruino/targetlibs/nrf52/nRF52_SDK_0.9.1_3639cc9/components/toolchain/gcc/linker_common_espruino.ld"