/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1B000, LENGTH = 0x65000 /*0x61000 Set asside some pages for espruino to save to*/
  RAM (rwx) :  ORIGIN = 0x20001F00, LENGTH = 0x6100
}

INCLUDE "gcc_nrf5x_espruino_common.ld"
