/***************************************************************************//**
 * @file nvm_hal.c
 * @brief Non-Volatile Memory Wear-Leveling driver HAL implementation
 * @version 4.2.1
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include <stdbool.h>
#include "em_msc.h"
#include "nvm.h"
#include "nvm_hal.h"


/*******************************************************************************
 ******************************   CONSTANTS   **********************************
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/* Padding value */
#define NVMHAL_FFFFFFFF      0xffffffffUL

/** @endcond */


/*******************************************************************************
 ***************************   LOCAL FUNCTIONS   *******************************
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */


/**************************************************************************//**
 * @brief  Read a 32-bit word of any alignment. This function checks
           SCB_CCR_UNALIGN_TRP and reads the word as bytes if set to avoid
           generating a unaligned access trap.
 *****************************************************************************/
static uint32_t readUnalignedWord(uint8_t *addr)
{
  /* Check if the unaligned access trap is enabled */
  if (SCB->CCR & SCB_CCR_UNALIGN_TRP_Msk)
  {
    /* Read word as bytes (always aligned) */
    return (*(addr + 3) << 24) | (*(addr + 2) << 16) | (*(addr + 1) << 8) | *addr;
  }
  else
  {
    /* Use unaligned access */
    return *(uint32_t *)addr;
  }
}


/***************************************************************************//**
 * @brief
 *   Convert return type.
 *
 * @details
 *   This function converts between the return type of the emlib and the
 *   NVM API.
 *
 * @param[in] result
 *   Operation result.
 *
 * @return
 *   Returns remapped status code.
 ******************************************************************************/
static Ecode_t returnTypeConvert(MSC_Status_TypeDef result)
{
  /* Direct return from switch gives smallest code size (smaller than if-else).
   * Could try using an offset value to translate. */
  switch (result)
  {
    case mscReturnOk:
      return ECODE_EMDRV_NVM_OK;

    case mscReturnInvalidAddr:
      return ECODE_EMDRV_NVM_ADDR_INVALID;

    case mscReturnUnaligned:
      return ECODE_EMDRV_NVM_ALIGNMENT_INVALID;

    default:
      return ECODE_EMDRV_NVM_ERROR;
  }
}

/** @endcond */

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Initialize NVM driver.
 *
 * @details
 *   This function is run upon initialization, at least once before any other
 *   functions. It can be used to call necessary startup routines before the
 *   hardware can be accessed.
 ******************************************************************************/
void NVMHAL_Init(void)
{
  MSC_Init();
}


/***************************************************************************//**
 * @brief
 *   De-initialize NVM .
 *
 * @details
 *   This function is run when the API deinit function is run. This should be
 *   done before any graceful halts.
 ******************************************************************************/
void NVMHAL_DeInit(void)
{
  MSC_Deinit();
}


/***************************************************************************//**
 * @brief
 *   Read data from NVM.
 *
 * @details
 *   This function is used to read data from the NVM hardware. It should be a
 *   blocking call, since the thread asking for data to be read cannot continue
 *   without the data.
 *
 *   Another requirement is the ability to read unaligned blocks of data with
 *   single byte precision.
 *
 * @param[in] *pAddress
 *   Memory address in hardware for the data to read.
 *
 * @param[in] *pObject
 *   RAM buffer to store the data from NVM.
 *
 * @param[in] len
 *   The length of the data.
 ******************************************************************************/
void NVMHAL_Read(uint8_t *pAddress, void *pObject, uint16_t len)
{
  /* Create a pointer to the void* pBuffer with type for easy movement. */
  uint8_t *pObjectInt = (uint8_t*) pObject;

  while (0 < len) /* While there is more data to fetch. */
  {
    /* Move the data from memory to the buffer. */
    *pObjectInt = *pAddress;
    /* Move active location in buffer, */
    ++pObjectInt;
    /* and in memory. */
    ++pAddress;
    /* More data is read. */
    --len;
  }
}


/***************************************************************************//**
 * @brief
 *   Write data to NVM.
 *
 * @details
 *   This function is used to write data to the NVM. This is a blocking
 *   function.
 *
 * @param[in] *pAddress
 *   NVM address to write to.
 *
 * @param[in] *pObject
 *   Pointer to source data.
 *
 * @param[in] len
 *   The length of the data in bytes.
 *
 * @return
 *   Returns the result of the write operation.
 ******************************************************************************/
Ecode_t NVMHAL_Write(uint8_t *pAddress, void const *pObject, uint16_t len)
{
  /* Used to carry return data. */
  MSC_Status_TypeDef mscStatus = mscReturnOk;
  /* Used as a temporary variable to create the blocks to write when padding to closest word. */
  uint32_t tempWord;

  /* Pointer to let us traverse the given pObject by single bytes. */
  uint8_t *pObjectInt = (uint8_t*)pObject;

  /* Temporary variable to cache length of padding needed. */
  uint8_t padLen;

  /* Get length of pad in front. */
  padLen = (uint32_t) pAddress % sizeof(tempWord);

  if (padLen != 0)
  {
    /* Get first word. */
    tempWord = readUnalignedWord((uint8_t *)pObject);

    /* Shift to offset. */
    tempWord = tempWord << 8 * padLen;
    /* Add nochanging padding. */
    tempWord |= NVMHAL_FFFFFFFF >> (8 * (sizeof(tempWord) - padLen));

    /* Special case for unaligned writes smaller than 1 word. */
    if (len < sizeof(tempWord) - padLen)
    {
      /* Add nochanging padding. */
      tempWord |= NVMHAL_FFFFFFFF << (8 * (padLen + len));
      len       = 0;
    }
    else
    {
      len -= sizeof(tempWord) - padLen;
    }

    /* Align the NVM write address */
    pAddress -= padLen;
    mscStatus = MSC_WriteWord((uint32_t *) pAddress, &tempWord, sizeof(tempWord));

    pObjectInt += sizeof(tempWord) - padLen;
    pAddress   += sizeof(tempWord);
  }


  /* Loop over body. */
  while ((len >= sizeof(tempWord)) && (mscReturnOk == mscStatus))
  {
    tempWord = readUnalignedWord((uint8_t *)pObjectInt);
    mscStatus = MSC_WriteWord((uint32_t *) pAddress, &tempWord, sizeof(tempWord));

    pAddress   += sizeof(tempWord);
    pObjectInt += sizeof(tempWord);
    len        -= sizeof(tempWord);
  }

  /* Pad at the end */
  if ((len > 0) && (mscReturnOk == mscStatus))
  {
    /* Get final word. */
    tempWord = readUnalignedWord((uint8_t *)pObjectInt);

    /* Fill rest of word with padding. */
    tempWord |= NVMHAL_FFFFFFFF << (8 * len);
    mscStatus = MSC_WriteWord((uint32_t *) pAddress, &tempWord, sizeof(tempWord));
  }

  /* Convert between return types, and return. */
  return returnTypeConvert(mscStatus);
}


/***************************************************************************//**
 * @brief
 *   Erase a page in the NVM.
 *
 * @details
 *   This function calls MSC_ErasePage and converts the return status.
 *
 * @param[in] *pAddress
 *   Memory address pointing to the start of the page to erase.
 *
 * @return
 *   Returns the result of the erase operation.
 ******************************************************************************/
Ecode_t NVMHAL_PageErase(uint8_t *pAddress)
{
  /* Call underlying library and convert between return types, and return. */
  return returnTypeConvert(MSC_ErasePage((uint32_t *) pAddress));
}


/***************************************************************************//**
 * @brief
 *   Calculate checksum according to CCITT CRC16.
 *
 * @details
 *   This function calculates a checksum of the supplied buffer.
 *   The checksum is calculated using CCITT CRC16 plynomial x^16+x^12+x^5+1.
 *
 *   This functionality is also present internally in the API, but is duplicated
 *   here to allow for much more efficient calculations specific to the
 *   hardware.
 *
 * @param[in] pChecksum
 *   Pointer to where the checksum should be calculated and stored. This buffer
 *   should be initialized. A good consistent starting point would be
 *   NVM_CHECKSUM_INITIAL.
 *
 * @param[in] pMemory
 *   Pointer to the data you want to calculate a checksum for.
 *
 * @param[in] len
 *   The length of the data.
 ******************************************************************************/
void NVMHAL_Checksum(uint16_t *pChecksum, void *pMemory, uint16_t len)
{
  uint8_t *pointer = (uint8_t *) pMemory;
  uint16_t crc = *pChecksum;

  while(len--)
  {
    crc = (crc >> 8) | (crc << 8);
    crc ^= *pointer++;
    crc ^= (crc & 0xf0) >> 4;
    crc ^= (crc & 0x0f) << 12;
    crc ^= (crc & 0xff) << 5;
  }
  *pChecksum = crc;
}
