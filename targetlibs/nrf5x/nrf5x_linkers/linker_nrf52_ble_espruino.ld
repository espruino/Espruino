/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1c000, LENGTH = 0x5e000 /* TODO: Set aside space for interpreter to save to.*/
  RAM (rwx) :  ORIGIN = 0x20002080, LENGTH = 0xcf80 /* TODO: Reserve space for stack so no collisions happen. */
  /* FIXME - we seem to have to start with 256 bytes less RAM than the figures below */
  /* length is just 65536 - origin */
  /* PERIPH 1 CENTRAL 0 - RAM (rwx) :  ORIGIN = 0x20001E00, LENGTH = 0xE200 */
  /* PERIPH 1 CENTRAL 1 - RAM (rwx) :  ORIGIN = 0x200023C8, LENGTH = 0xDC38 */ 
}

SECTIONS
{
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
} INSERT AFTER .data;

INCLUDE "gcc_nrf5x_espruino_common.ld"
