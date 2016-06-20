/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1B000, LENGTH = 0x65000 /*0x61000 Set aside some pages for espruino to save to*/
  RAM (rwx) :  ORIGIN = 0x200024C8, LENGTH = 0xDB38
  /* FIXME - we seem to have to start with 256 bytes less RAM than the figures below */
  /* length is just 65536 - origin */
  /* PERIPH 1 CENTRAL 0 - RAM (rwx) :  ORIGIN = 0x20001E00, LENGTH = 0xE200 */
  /* PERIPH 1 CENTRAL 1 - RAM (rwx) :  ORIGIN = 0x200023C8, LENGTH = 0xDC38 */ 
}

INCLUDE "gcc_nrf5x_espruino_common.ld"
