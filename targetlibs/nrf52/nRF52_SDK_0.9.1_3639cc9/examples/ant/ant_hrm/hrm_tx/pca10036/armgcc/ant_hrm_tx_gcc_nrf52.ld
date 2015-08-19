/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x12000, LENGTH = 0x6e000
  RAM (rwx) :  ORIGIN = 0x20000a00, LENGTH = 0x7600
}

INCLUDE "nrf5x_common.ld"