// USED FOR SDIO-BASED SD CARDS
/*
 * @author: ickle 
 * @source: http://www.61ic.com/code/archiver/?tid-27986.html
 */

#include "platform_config.h"
#include "jsinteractive.h"

#include "ff.h"
#include "diskio.h"
#include "sdio_sdcard.h"


#ifdef FLASH_FS

#define FS_SECTOR_SIZE 4096
#define FS_BLOCK_SIZE 1
#define FS_SECTOR_COUNT 256; // 1024*1024 / 4096 = 256
// Hardcode last page of 4Mb memory
#define FS_FLASH_BASE 0x200000
#else
#define FS_SECTOR_SIZE 512
#define FS_BLOCK_SIZE 32
#define FS_SECTOR_COUNT 131072; // 4*1024*32 = 131072
#endif

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/


#ifndef FLASH_FS
SD_CardInfo SDCardInfo2;

DSTATUS disk_initialize (
  BYTE drv /* Physical drive number (0) */
  )
{
  NVIC_InitTypeDef NVIC_InitStructure;

  //jsiConsolePrint("SD_Init\n");
  SD_Init();
  //jsiConsolePrint("SD_GetCardInfo\n");
  SD_GetCardInfo(&SDCardInfo2);
  //jsiConsolePrint("SD_SelectDeselect\n");
  SD_SelectDeselect((uint32_t) (SDCardInfo2.RCA << 16));
  //jsiConsolePrint("SD_EnableWideBusOperation\n");
  SD_EnableWideBusOperation(SDIO_BusWide_4b);
  //jsiConsolePrint("SD_SetDeviceMode\n");
  SD_SetDeviceMode(SD_DMA_MODE);
  //jsiConsolePrint("NVIC_Init\n");
  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  return 0;
}
#else

DSTATUS disk_initialize (
  BYTE drv /* Physical drive number (0) */
  )
{
  jsiConsolePrint("SD_Init Flash Init\n");

  return 0;
}
#endif

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
  BYTE drv /* Physical drive number (0) */
  )
{
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

  #ifdef FLASH_FS
  jsiConsolePrint("Flash disk_read %d %d\n", buff, Transfer_Length);
  jshFlashRead(FS_FLASH_BASE+addr, buff, Transfer_Length);
  #else
  SD_ReadBlock(Memory_Offset, (uint32_t *)buff, Transfer_Length);
  #endif

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

  #ifdef FLASH_FS
  jsiConsolePrint("Flash disk_write %d %d\n", buff, Transfer_Length);
  jshFlashErasePage(FS_FLASH_BASE+addr);
  jshFlashWrite(FS_FLASH_BASE+addr, buff, Transfer_Length);  
  #else
  SD_WriteBlock(Memory_Offset, (uint32_t *)buff, Transfer_Length);
  #endif

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
  uint32_t status = SD_NO_TRANSFER;
  //uint32_t status = NAND_READY;

  switch (ctrl) {
  case CTRL_SYNC : /// Make sure that no pending write process
  #ifdef FLASH_FS  
    res = RES_OK;
    jsiConsolePrint("Flash disk_ioctl CTRL_SYNC\n");
  #else
    status = SD_GetTransferState();
    if (status == SD_NO_TRANSFER)
      //status = FSMC_NAND_GetStatus();
      //if (status == NAND_READY)
    {res = RES_OK;}
    else{res = RES_ERROR;}
   #endif
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