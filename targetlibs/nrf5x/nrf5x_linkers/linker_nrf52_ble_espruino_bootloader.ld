/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1f000, LENGTH = 0x55000 /* 0x77000 and above is reserved for the bootloader+fs, then there's saved JS code. see Makefile, secure_dfu_gcc_nrf52.ld,  linker_nrf52_ble_espruino_bootloader.ld and dfu_types.h, BOARD.py  */
  RAM (rwx) :  ORIGIN = 0x20002c40, LENGTH = 0xd3c0
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
