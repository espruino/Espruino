/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 *  Copyright (C) 2016 by Rhys Williams (wilberforce)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * This file is designed to be parsed during the build process
 *
 * Contains built-in functions for using Flash memory as disk on devices with
 * large flash - e.g ESP32 / ESP8266
 * ----------------------------------------------------------------------------
 */

#include "jsinteractive.h"

#include "ff.h"
#include "diskio.h"
#include "jswrap_file.h"
#include "flash_diskio.h"

 uint32_t fs_flash_base     = FS_FLASH_BASE;
 uint16_t fs_flash_sectors  = FS_SECTOR_COUNT;
 uint8_t  fs_flash_readonly = false;

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
  if (drv != 0)
     return STA_NODISK;
  //jsDebug("Flash Disk Init %d",drv);
  return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
  BYTE drv /* Physical drive number (0) */
  )
{
  if (drv != 0)
     return STA_NODISK;
  return RES_OK;
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
  if (drv != 0)
     return STA_NODISK;
 
  uint16_t size;
  uint32_t addr;

  size =  count * FS_SECTOR_SIZE;
  addr = sector * FS_SECTOR_SIZE + fs_flash_base;
  
  jsDebug("Flash disk_read sector: %d, buff: mem: %d buff: %d len: %d", sector, addr, buff, size);
  jshFlashRead( buff, addr, size);
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
  if (drv != 0)
     return STA_NODISK;
  uint16_t size;
  uint32_t addr;

  size =  count * FS_SECTOR_SIZE;
  addr = sector * FS_SECTOR_SIZE + fs_flash_base;

  jsDebug("Flash disk_write sector:  %d, buff: mem: %d buff: %d len: %d", sector, addr, buff, size);
  jshFlashErasePage(addr);
  jshFlashWrite( buff, addr,size);  

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
  if (drv != 0)
     return STA_NODISK;
  DRESULT res = RES_OK;

  switch (ctrl) {
  case CTRL_SYNC : /// Make sure that no pending write process
    res = RES_OK;
    jsDebug("Flash disk_ioctl CTRL_SYNC");
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

/*-----------------------------------------------------------------------*/
// store settings of base addr, sectors, read only
// used to change default flash areas
/*-----------------------------------------------------------------------*/
uint8_t flashFatFsInit( uint32_t addr, uint16_t sectors, uint8_t readonly, uint8_t format ) {
  fs_flash_base = addr;
  fs_flash_sectors = sectors;
  fs_flash_readonly = readonly;
  return true;
}