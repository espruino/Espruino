/***************************************************************************//**
 * @file nvm_hal.h
 * @brief Non-Volatile Memory Wear-Leveling driver HAL
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


#ifndef __NVMHAL_H
#define __NVMHAL_H

#include "em_device.h"
#include <stdbool.h>
#include "nvm.h"
#include "ecode.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

void NVMHAL_Init(void);
void NVMHAL_DeInit(void);
void NVMHAL_Read(uint8_t *pAddress, void *pObject, uint16_t len);
Ecode_t NVMHAL_Write(uint8_t *pAddress, void const *pObject, uint16_t len);
Ecode_t NVMHAL_PageErase(uint8_t *pAddress);
void NVMHAL_Checksum(uint16_t *checksum, void *pMemory, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __NVMHAL_H */
