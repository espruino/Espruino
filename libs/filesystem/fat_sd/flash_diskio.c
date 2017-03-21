// Use Flash memory with devices with large flash - ESP8266/ESP32
/*
 * @author: Wilberforce - Rhys Williams
 * 
 */

#include "jsinteractive.h"

#include "ff.h"
#include "diskio.h"

#define FS_SECTOR_SIZE 4096
#define FS_BLOCK_SIZE 1
// 1MB = 1024*1024 / 4096 = 256
// 0-33 format
#define FS_SECTOR_COUNT 256

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
  jsWarn("Flash Init - disk_initialize %d",drv);

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
  jsWarn("Flash Init - disk_status %d",drv);
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
  NOT_USED(drv);
  uint16_t Transfer_Length;
  uint32_t Memory_Offset;

  Transfer_Length =  count * FS_SECTOR_SIZE;
  Memory_Offset = sector * FS_SECTOR_SIZE + FS_FLASH_BASE;

  jsWarn("Flash disk_read sector: %d, buff: mem: %d buff: %d len: %d", sector, Memory_Offset, buff, Transfer_Length);
  jshFlashRead( buff, Memory_Offset, Transfer_Length);

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
  NOT_USED(drv);
  uint16_t Transfer_Length;
  uint32_t Memory_Offset;

  Transfer_Length =  count * FS_SECTOR_SIZE;
  Memory_Offset = sector * FS_SECTOR_SIZE + FS_FLASH_BASE;

  jsWarn("Flash disk_write sector:  %d, buff: mem: %d buff: %d len: %d", sector, Memory_Offset, buff, Transfer_Length);
  jshFlashErasePage(Memory_Offset);
  jshFlashWrite( buff, Memory_Offset,Transfer_Length);  

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
  NOT_USED(drv);
  DRESULT res = RES_OK;
  jsWarn("Flash disk_ioctl %d",ctrl);

  switch (ctrl) {
  case CTRL_SYNC : /// Make sure that no pending write process
    res = RES_OK;
    jsWarn("Flash disk_ioctl CTRL_SYNC");
    break;

  case GET_SECTOR_COUNT :   // Get number of sectors on the disk (DWORD)
    *(DWORD*)buff = FS_SECTOR_COUNT;
	jsWarn("disk_ioctl FS_SECTOR_COUNT %d",FS_SECTOR_COUNT);
    res = RES_OK;
    break;

  case GET_SECTOR_SIZE :   // Get R/W sector size (WORD)
    *(WORD*)buff = FS_SECTOR_SIZE;
	jsWarn("disk_ioctl FS_SECTOR_SIZE %d",FS_SECTOR_SIZE);	
    res = RES_OK;
    break;

  case GET_BLOCK_SIZE :     // Get erase block size in unit of sector (DWORD)
    *(DWORD*)buff = FS_BLOCK_SIZE;
	jsWarn("disk_ioctl FS_BLOCK_SIZE %d",FS_BLOCK_SIZE);		
    res = RES_OK;
    break;
  }

  return res;
}l