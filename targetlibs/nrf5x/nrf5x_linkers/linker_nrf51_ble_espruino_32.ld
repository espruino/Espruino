/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1B000, LENGTH = 0x25000
  RAM (rwx) :  ORIGIN = 0x20001E00, LENGTH = 0x6200 /* 32kB - 0x1E000 */
}

INCLUDE "gcc_nrf5x_espruino_common.ld"
