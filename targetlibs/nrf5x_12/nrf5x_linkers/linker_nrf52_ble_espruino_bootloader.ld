/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEM_SRAM_SIZE = DEFINED( LD_SRAM_SIZE ) ? LD_SRAM_SIZE : 0x10000 ;
MEM_SRAM_SD_OFFSET = DEFINED( LD_APP_RAM_BASE ) ? LD_APP_RAM_BASE :0x2c40; /* memory allocated for SoftDevice */
MEM_NOINIT_SIZE = 16;

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1f000, LENGTH = 0x55000 /* 0x77000 and above is reserved for the bootloader+fs, then there's saved JS code. see Makefile, secure_dfu_gcc_nrf52.ld,  linker_nrf52_ble_espruino_bootloader.ld and dfu_types.h, BOARD.py  */
  RAM (rwx) :  ORIGIN = 0x20000000 + MEM_SRAM_SD_OFFSET, LENGTH = MEM_SRAM_SIZE - MEM_SRAM_SD_OFFSET - MEM_NOINIT_SIZE
  NOINIT (rwx) :  ORIGIN = 0x20000000 + MEM_SRAM_SIZE - MEM_NOINIT_SIZE , LENGTH = MEM_NOINIT_SIZE
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
