/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1d000, LENGTH = 0x23000
  RAM (rwx) :  ORIGIN = 0x20002200, LENGTH = 0x1e00
}

INCLUDE "gcc_nrf51_common.ld"