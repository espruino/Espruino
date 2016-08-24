/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1c000, LENGTH = 0x5e000 /* 0x7A000 and above is reserved for the bootloader. see Makefile, dfu_gcc_nrf52.ld,  linker_nrf52_ble_espruino_bootloader.ld and dfu_types.h        */
  RAM (rwx) :  ORIGIN = 0x200025e0, LENGTH = 0xda20
  /* FIXME - we seem to have to start with 256 bytes less RAM than the figures below */
  /* length is just 65536 - origin */
  /* PERIPH 1 CENTRAL 0 - RAM (rwx) :  ORIGIN = 0x20001E00, LENGTH = 0xE200 */
  /* PERIPH 1 CENTRAL 1 - RAM (rwx) :  ORIGIN = 0x200023C8, LENGTH = 0xDC38 */ 
}

SECTIONS
{
  .fs_data_out ALIGN(4):
  {
    PROVIDE( __start_fs_data = .);
    KEEP(*(fs_data))
    PROVIDE( __stop_fs_data = .);
  } = 0
}

INCLUDE "gcc_nrf5x_espruino_common.ld"
