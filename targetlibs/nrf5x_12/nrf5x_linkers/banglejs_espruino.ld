/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1f000, LENGTH = 0x5B000 /* We can go right up to the bootloader at 0x7A000  */
  RAM (rwx) :  ORIGIN = 0x20002c40, LENGTH = 0xd3b0
  NOINIT (rwx) :  ORIGIN = 0x2000fff0, LENGTH = 16
}

SECTIONS
{
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(.pwr_mgmt_data))
    PROVIDE(__stop_pwr_mgmt_data = .);
  } > RAM
  .noinit (NOLOAD) :
  {
    . = ALIGN(4);
    __noinit_start__ = .;
    *(.noinit*)
    __noinit_end__ = .;
  } > NOINIT
} INSERT AFTER .data;

INCLUDE "gcc_nrf5x_espruino_common.ld"
