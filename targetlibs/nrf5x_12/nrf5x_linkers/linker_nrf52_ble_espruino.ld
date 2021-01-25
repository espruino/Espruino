/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEM_SRAM_SIZE = DEFINED( LD_SRAM_SIZE ) ? LD_SRAM_SIZE : 0x10000 ;
MEM_SRAM_SD_OFFSET = 0x2c40; /* memory allocated for SoftDevice */

MEMORY
{
  FLASH (rx) : ORIGIN = 0x1f000, LENGTH = 0x60000
  RAM (rwx) :  ORIGIN = 0x20000000 + MEM_SRAM_SD_OFFSET, LENGTH = MEM_SRAM_SIZE - MEM_SRAM_SD_OFFSET /*0xd3c0*/
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
} INSERT AFTER .data;

INCLUDE "gcc_nrf5x_espruino_common.ld"
