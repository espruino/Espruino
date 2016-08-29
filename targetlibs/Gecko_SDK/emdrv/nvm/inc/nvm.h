/***************************************************************************//**
 * @file nvm.h
 * @brief Non-Volatile Memory Wear-Leveling driver API
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

#ifndef __NVM_H
#define __NVM_H

#include <stdint.h>
#include <stdbool.h>
#include "em_device.h"
#include "nvm_hal.h"
#include "nvm_config.h"
#include "ecode.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup EM_Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup NVM
 * @brief NVM Non-volatile Memory Wear-Leveling driver, see
 *        @ref nvm_doc page for detailed
 *        documentation.
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   DEFINES   ***********************************
 ******************************************************************************/

/** Return/error codes */
#define ECODE_EMDRV_NVM_OK                      ( ECODE_OK )                                    ///< Success return value
#define ECODE_EMDRV_NVM_ADDR_INVALID            ( ECODE_EMDRV_SPIDRV_BASE | 0x00000001 )        ///< Invalid address
#define ECODE_EMDRV_NVM_ALIGNMENT_INVALID       ( ECODE_EMDRV_SPIDRV_BASE | 0x00000002 )        ///< Invalid data alignment
#define ECODE_EMDRV_NVM_DATA_INVALID            ( ECODE_EMDRV_SPIDRV_BASE | 0x00000003 )        ///< Invalid input data or format
#define ECODE_EMDRV_NVM_WRITE_LOCK              ( ECODE_EMDRV_SPIDRV_BASE | 0x00000004 )        ///< A write is currently in progress
#define ECODE_EMDRV_NVM_NO_PAGES_AVAILABLE      ( ECODE_EMDRV_SPIDRV_BASE | 0x00000005 )        ///< Initialization didn't find any pages available to allocate
#define ECODE_EMDRV_NVM_PAGE_INVALID            ( ECODE_EMDRV_SPIDRV_BASE | 0x00000006 )        ///< Could not find the page specified
#define ECODE_EMDRV_NVM_ERROR                   ( ECODE_EMDRV_SPIDRV_BASE | 0x00000007 )        ///< General error

/** All objects are written from RAM. */
#define NVM_WRITE_ALL_CMD         0xff
/** All objects are copied from the old page. */
#define NVM_WRITE_NONE_CMD        0xfe
/** All objects are read to RAM. */
#define NVM_READ_ALL_CMD          0xff

/** Retains the registered erase count when eraseing a page. */
#define NVM_ERASE_RETAINCOUNT    0xffffffffUL

/** Structure defining end of pages table. */
#define NVM_PAGE_TERMINATION    { NULL, 0, (NVM_Object_Ids) 0 }


/*******************************************************************************
 ***************************   PROTOTYPES   ************************************
 ******************************************************************************/

Ecode_t NVM_Init(NVM_Config_t const *nvmConfig);
Ecode_t NVM_Erase(uint32_t eraseCount);
Ecode_t NVM_Write(uint16_t pageId, uint8_t objectId);
Ecode_t NVM_Read(uint16_t pageId, uint8_t objectId);

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#ifndef NVM_FEATURE_WEARLEVELGET_ENABLED
#define NVM_FEATURE_WEARLEVELGET_ENABLED    true
#endif
/** @endcond */
#if (NVM_FEATURE_WEARLEVELGET_ENABLED)
uint32_t NVM_WearLevelGet(void);
#endif

/** @} (end defgroup NVM) */
/** @} (end addtogroup EM_Drivers) */

#ifdef __cplusplus
}
#endif

#endif /* __NVM_H */
