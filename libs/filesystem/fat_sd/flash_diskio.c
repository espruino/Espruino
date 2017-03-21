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
#define FS_SECTOR_COUNT 256

// Hardcode  page of 4Mb memory
 uint32_t fs_flash_base=0;
 int read_on=true;
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
  if ( fs_flash_base == 0 ) {
	jsError("Need to set Flash address with E.flashFatFs");
	return FR_NOT_READY;
  }
  return RES_OK;
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
  NOT_USED(drv);
  uint16_t Transfer_Length;
  uint32_t Memory_Offset;

  Transfer_Length =  count * FS_SECTOR_SIZE;
  Memory_Offset = sector * FS_SECTOR_SIZE + fs_flash_base;

  jsWarn("Flash disk_read sector: %d, buff: mem: %d buff: %d len: %d", sector, Memory_Offset, buff, Transfer_Length);
  if ( ( read_on && sector > 10 ) || ( sector <= 10 ) ){
	jshFlashRead( buff, Memory_Offset, Transfer_Length);
  }
   else {
     jsWarn("reading disabled...%d", read_on );
   }
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
  Memory_Offset = sector * FS_SECTOR_SIZE + fs_flash_base;

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
}


/****/

extern void jsfsReportError(const char *msg, FRESULT res);
extern bool jsfsInit();

int flashFatFsInit( FATFS * jsfsFAT, int addr, int format) {
	FRESULT res;
	// sanity check here?
	fs_flash_base=addr;
    if ( format == 1 ) {
        jsError("E.flashFatFs formatting...");
		memset(jsfsFAT, 0, sizeof(FATFS));
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
		jsWarn("jsfsInit: %d", jsfsInit());
	}

	if ( format == 20 ) {
		read_on=false;
		jsWarn("Disable Read %d", read_on );		
	}

	if ( format == 21 ) {
		read_on=true;
		jsWarn("Enable Read %d", read_on );		
	}

	
	if ( format == 4 ) {
        DWORD fre_clust;
		FATFS* ptr=jsfsFAT;
		res = f_getfree("", &fre_clust, &ptr);
		jsWarn("fre_clust: %d", fre_clust);
	}
	return true;
}