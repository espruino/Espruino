/***************************************************************************//**
 * @file  msddmedia.h
 * @brief Media interface for Mass Storage class Device (MSD).
 * @version 4.2.1
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#ifndef __MSDDMEDIA_H
#define __MSDDMEDIA_H

/* NOTE: Only use MSD_SRAM_MEDIA or MSD_FLASH_MEDIA on STK3700         */
#define MSD_SRAM_MEDIA          0   /* 96K "disk" in internal SRAM     */
#define MSD_FLASH_MEDIA         3   /* 512K "disk" in internal FLASH   */

/* NOTE: Don't use the following three options on STK3700              */
#define MSD_PSRAM_MEDIA         1   /* 4M "disk" in external PSRAM     */
#define MSD_SDCARD_MEDIA        2   /* External micro SD-Card "disk"   */
#define MSD_NORFLASH_MEDIA      4   /* 16M "disk" in external NORFLASH */

#if !defined( MSD_MEDIA )
#define MSD_MEDIA  MSD_FLASH_MEDIA  /* Select media type */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*** MSD Media Function prototypes ***/

bool     MSDDMEDIA_CheckAccess( MSDD_CmdStatus_TypeDef *pCmd, uint32_t lba, uint32_t sectors );
void     MSDDMEDIA_Flush( void );
uint32_t MSDDMEDIA_GetSectorCount( void );
bool     MSDDMEDIA_Init( void );
void     MSDDMEDIA_Read(  MSDD_CmdStatus_TypeDef *pCmd, uint8_t *data, uint32_t sectors );
void     MSDDMEDIA_Write( MSDD_CmdStatus_TypeDef *pCmd, uint8_t *data, uint32_t sectors );

#ifdef __cplusplus
}
#endif

#endif /* __MSDDMEDIA_H */
