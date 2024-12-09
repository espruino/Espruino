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



SD_CardInfo SDCardInfo2;

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
  BYTE drv /* Physical drive number (0) */
  )
{
  NVIC_InitTypeDef NVIC_InitStructure;

  //jsiConsolePrint("SD_Init\n");
  SD_Init();
#ifndef STM32F4
  // FIXME: Do we even need this in EspruinoBoard? seems like SD_GetCardInfo especially just wastes time
  //jsiConsolePrint("SD_GetCardInfo\n");
  SD_GetCardInfo(&SDCardInfo2);
  //jsiConsolePrint("SD_SelectDeselect\n");
  SD_SelectDeselect((uint32_t) (SDCardInfo2.RCA << 16));
  //jsiConsolePrint("SD_EnableWideBusOperation\n");
  SD_EnableWideBusOperation(SDIO_BusWide_4b);
  //jsiConsolePrint("SD_SetDeviceMode\n");
  SD_SetDeviceMode(SD_DMA_MODE);
#endif

  //jsiConsolePrint("NVIC_Init\n");
  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  //NAND_Init();
  return 0;
}


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

#ifndef ESPR_SDIO_FAST_UNALIGNED
DRESULT disk_read_unaligned (
  BYTE drv, /* Physical drive number (0) */
  BYTE *buff, /* Pointer to the data buffer to store read data */
  DWORD sector, /* Start sector number (LBA) */
  UINT count /* Sector count */
  )
{
  uint8_t alignedBuffer[512] __attribute__ ((aligned (8)));
  uint32_t Memory_Offset;
  Memory_Offset = sector * 512;
  while (count--) {
    SD_ReadBlock(Memory_Offset, (uint32_t *)alignedBuffer, 512);
    memcpy(buff, alignedBuffer, 512);
    Memory_Offset += 512;
    buff += 512;
  }
  return RES_OK;
}
#endif

DRESULT disk_read (
  BYTE drv, /* Physical drive number (0) */
  BYTE *buff, /* Pointer to the data buffer to store read data */
  DWORD sector, /* Start sector number (LBA) */
  UINT count /* Sector count */
  )
{
  uint16_t Transfer_Length;
  uint32_t Memory_Offset;

  Transfer_Length =  count * 512;
  Memory_Offset = sector * 512;

#ifdef ESPR_SDIO_FAST_UNALIGNED
/* ESPR_SDIO_FAST_UNALIGNED is a really nasty hack. STM32 can't DMA to unaligned
addresses so what we do is we push the address forward to the next aligned point,
then we memmove it back by the correct amount after DMA. THIS MEANS THAT ANY
BUFFER WE F_READ INTO SHOULD BE 4 BYTES LONGER THAN THE MAX READ REQUEST
*/
  int misalignment = ((size_t)buff)&3;
  BYTE *correctbuff = buff;
  if (misalignment)
    buff += 4-misalignment;
#else
  if ((size_t)buff&3) // unaligned read - must read to aligned buffer because of DMA
    return disk_read_unaligned(drv, buff, sector, count);
#endif
  if (count<=1)
    SD_ReadBlock(Memory_Offset, (uint32_t *)buff, Transfer_Length);
  else
    SD_ReadMultiBlocks(Memory_Offset, (uint32_t *)buff, 512, count);

#ifdef ESPR_SDIO_FAST_UNALIGNED
  if (misalignment) {
    // move data back to correct point
    memmove(correctbuff, buff, Transfer_Length);
  }
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

  Transfer_Length =  count * 512;
  Memory_Offset = sector * 512;
  assert(((size_t)buff&3)==0);
  if (count<=1)
    SD_WriteBlock(Memory_Offset, (uint32_t *)buff, Transfer_Length);
  else
    SD_WriteMultiBlocks(Memory_Offset, (uint32_t *)buff, 512, count);
  //NAND_Write(Memory_Offset, (uint32_t *)buff, Transfer_Length);

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
  //uint32_t status = NAND_READY;

  switch (ctrl) {
  case CTRL_SYNC : /// Make sure that no pending write process
    res = RES_OK;
    break;

  case GET_SECTOR_COUNT : {  // Get number of sectors on the disk (DWORD)
    SD_CardInfo SDCardInfo;
    SD_GetCardInfo(&SDCardInfo);
    *(DWORD*)buff = SDCardInfo.CardCapacity>>9;
    res = RES_OK;
  } break;

  case GET_SECTOR_SIZE :   // Get R/W sector size (WORD)
    *(WORD*)buff = 512;
    res = RES_OK;
    break;

  case GET_BLOCK_SIZE :     // Get erase block size in unit of sector (DWORD)
    *(DWORD*)buff = 32;
    res = RES_OK;
    break;
  }

  return res;
}
