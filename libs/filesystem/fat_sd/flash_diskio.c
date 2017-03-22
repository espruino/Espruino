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

// Hardcode last 1Mb of 4Mb flash
 uint32_t fs_flash_base = 0x300000;
 

// 1MB = 1024*1024 / 4096 = 256
#define FS_SECTOR_COUNT 256
// Set sector size as the same a flash block size
#define FS_SECTOR_SIZE 4096
// Cluster = 1 sector
#define FS_BLOCK_SIZE 1

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

// Need size, readonly , automount?
int flashFatFsInit( FATFS * jsfsFAT, int addr, int format) {
	FRESULT res;
	// sanity check here?
	fs_flash_base=addr;
    if ( format == 1 ) {
        jsError("E.flashFatFs formatting...");
		if ((res = f_mount(jsfsFAT, "", 0)) != FR_OK) {
		  jsfsReportError("Unable to format", res);
		  return false;
		}		
        //res = f_mkfs("/", 1, 0);
        res = f_mkfs("", 1, 0);  /* path,  Partitioning rule 0:FDISK, 1:SFD super floppy , size of allocation unit */
        if (res != FR_OK) {
            jsError("[f_mkfs] Error %d\r\n", res);
            jsfsReportError("Format error:",res);
        }
    }
	if ( format == 2 ) {
		jsDebug("jsfsInit: %d", jsfsInit());
	}
	
	if ( format == 4 ) {
        DWORD fre_clust;
		FATFS* ptr=jsfsFAT;
		res = f_getfree("", &fre_clust, &ptr);
		jsDebug("fre_clust: %d", fre_clust);
	}
	return true;
}