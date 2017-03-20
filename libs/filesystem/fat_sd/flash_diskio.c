// Use Flash memory with devices with large flash - ESP8266/ESP32
/*
 * @author: Wilberforce - Rhys Williams
 * 
 */

#include "platform_config.h"
#include "jsinteractive.h"

#include "ff.h"
#include "diskio.h"

#define FS_SECTOR_SIZE 4096
#define FS_BLOCK_SIZE 1
#define FS_SECTOR_COUNT 256; // 1024*1024 / 4096 = 256
// Hardcode  page of 4Mb memory
#define FS_FLASH_BASE 0x200000

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
  BYTE drv /* Physical drive number (0) */
  )
{
  NOT_USED(drv);
  jsWarn("Flash Init - disk_initialize %d\n",drv);

  return 0;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
  BYTE drv /* Physical drive number (0) */
  )
{
NOT_USED(drv);
  jsWarn("Flash Init - disk_status %d\n",drv);
  return 0;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
  BYTE drv, /* Physical drive number (0) */
  BYTE *buff, /* Pointer to the data buffer to store read data */
  DWORD sector, /* Start sector number (LBA) */
  UINT count /* Sector count */
  )
{
  uint16_t Transfer_Length;
  uint32_t Memory_Offset;

  Transfer_Length =  count * FS_SECTOR_SIZE;
  Memory_Offset = sector * FS_SECTOR_SIZE;

  jsWarn("Flash disk_read sector: %d, buff: %d len: %d\n", sector, buff, Transfer_Length);
  jshFlashRead( buff, FS_FLASH_BASE+Memory_Offset, Transfer_Length);

  return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
  BYTE drv, /* Physical drive number (0) */
  const BYTE *buff, /* Pointer to the data to be written */
  DWORD sector, /* Start sector number (LBA) */
  UINT count /* Sector count */
  )
{
  uint16_t Transfer_Length;
  uint32_t Memory_Offset;

  Transfer_Length =  count * FS_SECTOR_SIZE;
  Memory_Offset = sector * FS_SECTOR_SIZE;

  jsWarn("Flash disk_write %d %d\n", buff, Transfer_Length);
  jshFlashErasePage(FS_FLASH_BASE+Memory_Offset);
  jshFlashWrite( buff, FS_FLASH_BASE+Memory_Offset,Transfer_Length);  

  return RES_OK;
}


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
  BYTE drv, // Physical drive number (0)
  BYTE ctrl, // Control code
  void *buff // Buffer to send/receive control data
  )
{
  DRESULT res = RES_OK;
  jsWarn("Flash disk_ioctl %d\n",ctrl);

  switch (ctrl) {
  case CTRL_SYNC : /// Make sure that no pending write process
    res = RES_OK;
    jsWarn("Flash disk_ioctl CTRL_SYNC\n");
    break;

  case GET_SECTOR_COUNT :   // Get number of sectors on the disk (DWORD)
    *(DWORD*)buff = FS_SECTOR_COUNT;
    res = RES_OK;
    break;

  case GET_SECTOR_SIZE :   // Get R/W sector size (WORD)
    *(WORD*)buff = FS_SECTOR_SIZE;
    res = RES_OK;
    break;

  case GET_BLOCK_SIZE :     // Get erase block size in unit of sector (DWORD)
    *(DWORD*)buff = FS_BLOCK_SIZE;
    res = RES_OK;
    break;
  }

  return res;
}