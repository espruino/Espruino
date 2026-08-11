/**
  ******************************************************************************
  * @file    py32f07x_hal_flash.c
  * @author  MCU Application Team
  * @brief   FLASH HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the internal FLASH memory:
  *           + Program operations functions
  *           + Memory Control functions
  *           + Peripheral Errors functions
  *
 @verbatim

  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal.h"

/** @defgroup FLASH FLASH
  * @brief FLASH HAL module driver
  * @{
  */

#ifdef HAL_FLASH_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup FLASH_Private_Variables FLASH Private Variables
 * @{
 */
/**
  * @brief  Variable used for Program/Erase sectors under interruption
  */
FLASH_ProcessTypeDef pFlash  = {.Lock = HAL_UNLOCKED, \
                                .ErrorCode = HAL_FLASH_ERROR_NONE, \
                                .ProcedureOnGoing = FLASH_TYPENONE, \
                                .Address = 0U, \
                                .PageOrSector = 0U, \
                                .NbPagesSectorsToErase = 0U
                               };
/**
  * @}
  */
const uint32_t _FlashTimmingParam[8] = {0x1FFF3238, 0x1FFF3260, 0x1FFF3288, 0x1FFF32B0, 0x1FFF32D8, 0x1FFF3238, 0x1FFF3238, 0x1FFF3238};

/**
  * @brief  Option byte program.
  * @note   Enable this configuration, you need to add the corresponding optionbyte FLM file
  */
#ifdef FLASH_OPT_PROGRAM_ENABLED
#if defined ( __GNUC__ ) && !defined (__CC_ARM) /* GNU Compiler */
    const uint32_t u32ICG[] __attribute__((section(".opt_sec"))) =
#elif defined (__CC_ARM)
    const uint32_t u32ICG[] __attribute__((at(OB_BASE))) =
#elif defined (__ICCARM__)
    __root const uint32_t u32ICG[] @ OB_BASE =
#else
    #error "unsupported compiler!!"
#endif
{
    FLASH_OPTR_CONSTANT,
    FLASH_SDKR_CONSTANT,
    0xFFFFFFFF,
    FLASH_WRPR_CONSTANT,
};
#endif /* FLASH_OPT_PROGRAM_ENABLED */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup FLASH_Private_Functions FLASH Private Functions
 * @{
 */
static  void  FLASH_MassErase(void);
static  void  FLASH_Program_Page(uint32_t Address, uint32_t * DataAddress);
static  void  FLASH_PageErase(uint32_t PageAddress);
/**
  * @}
  */

/**
  * @brief  Unlock the FLASH control register access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Unlock(void)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0x00U)
  {
    /* Authorize the FLASH Registers access */
    WRITE_REG(FLASH->KEYR, FLASH_KEY1);
    WRITE_REG(FLASH->KEYR, FLASH_KEY2);

    /* verify Flash is unlock */
    if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0x00U)
    {
      status = HAL_ERROR;
    }
  }

  return status;
}

/**
  * @brief  Lock the FLASH control register access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Lock(void)
{
  HAL_StatusTypeDef status = HAL_ERROR;

  /* Set the LOCK Bit to lock the FLASH Registers access */
  SET_BIT(FLASH->CR, FLASH_CR_LOCK);

  /* verify Flash is locked */
  if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0x00u)
  {
    status = HAL_OK;
  }

  return status;
}

/**
  * @brief  Unlock the FLASH Option Bytes Registers access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_OB_Unlock(void)
{
  HAL_StatusTypeDef status = HAL_ERROR;

  if (READ_BIT(FLASH->CR, FLASH_CR_OPTLOCK) != 0x00U)
  {
    /* Authorizes the Option Byte register programming */
    WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY1);
    WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY2);

    /* verify option bytes are unlocked */
    if (READ_BIT(FLASH->CR, FLASH_CR_OPTLOCK) == 0x00U)
    {
      status = HAL_OK;
    }
  }

  return status;
}

/**
  * @brief  Lock the FLASH Option Bytes Registers access.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_OB_Lock(void)
{
  HAL_StatusTypeDef status = HAL_ERROR;

  /* Set the OPTLOCK Bit to lock the FLASH Option Byte Registers access */
  SET_BIT(FLASH->CR, FLASH_CR_OPTLOCK);

  /* verify option bytes are locked */
  if (READ_BIT(FLASH->CR, FLASH_CR_OPTLOCK) != 0x00u)
  {
    status = HAL_OK;
  }

  return status;
}

/**
  * @brief  Launch the option byte loading.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_OB_Launch(void)
{
  /* Set the bit to force the option byte reloading */
  SET_BIT(FLASH->CR, FLASH_CR_OBL_LAUNCH);

  /* We should not reach here : Option byte launch generates Option byte reset
     so return error */
  return HAL_ERROR;
}

/**
  * @brief  Get the specific FLASH error flag.
  * @retval FLASH_ErrorCode The returned value can be
  *            @arg @ref HAL_FLASH_ERROR_NONE No error set
  *            @arg @ref HAL_FLASH_ERROR_WRP FLASH Write protection error
  *            @arg @ref HAL_FLASH_ERROR_OPTV FLASH Option validity error
  * @note (*) availability depends on devices
  */
uint32_t HAL_FLASH_GetError(void)
{
  return pFlash.ErrorCode;
}

/**
  * @brief  Wait for a FLASH operation to complete.
  * @param  Timeout maximum flash operation timeout
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout)
{
  /* Wait for the FLASH operation to complete by polling on BUSY flag to be reset.
     Even if the FLASH operation fails, the BUSY flag will be reset and an error
     flag will be set */
  uint32_t timeout = HAL_GetTick() + Timeout;

  /* Wait if any operation is ongoing */
  while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != 0x00U)
  {
    if (HAL_GetTick() >= timeout)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Clear SR register */
  FLASH->SR = FLASH_FLAG_SR_CLEAR;

  return HAL_OK;
}

/**
  * @brief  Full erase of FLASH memory Bank
  *
  * @retval None
  */
static void FLASH_MassErase(void)
{
  /* Clean the error context */
  pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;
  /* Only bank1 will be erased*/
  SET_BIT(FLASH->CR, FLASH_CR_MER);
  *(__IO uint32_t *)(0x08000000) = 0x12344321;
}

/**
  * @brief  Page erase of FLASH memory
  *
  * @retval None
  */
static void FLASH_PageErase(uint32_t PageAddress)
{
  /* Clean the error context */
  pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;
  SET_BIT(FLASH->CR, FLASH_CR_PER);
  *(__IO uint32_t *)(PageAddress) = 0xFF;
}

/**
  * @brief  Sector erase of FLASH memory
  *
  * @retval None
  */
static void FLASH_SectorErase(uint32_t SectorAddress)
{
  SET_BIT(FLASH->CR, FLASH_CR_SER);
  *(__IO uint32_t *)(SectorAddress) = 0xFF;
}

/**
  * @brief  Page program of FLASH memory
  *
  * @retval None
  */
static __RAM_FUNC void FLASH_Program_Page(uint32_t Address, uint32_t * DataAddress)
{

  uint8_t index=0;
  uint32_t dest = Address;
  uint32_t * src = DataAddress;
  uint32_t primask_bit;

  SET_BIT(FLASH->CR, FLASH_CR_PG);
  /* Enter critical section */
  primask_bit = __get_PRIMASK();
  __disable_irq();
  /* 64 words*/
  while(index<64U)
  {
    *(uint32_t *)dest = *src;
    src += 1U;
    dest += 4U;
    index++;
    if(index==63)
    {
      SET_BIT(FLASH->CR, FLASH_CR_PGSTRT);
    }
  }

  /* Exit critical section: restore previous priority mask */
  __set_PRIMASK(primask_bit);
}

/**
  * @brief  Perform a mass erase or erase the specified FLASH memory pages
  * @note   To correctly run this function, the @ref HAL_FLASH_Unlock() function
  *         must be called before.
  *         Call the @ref HAL_FLASH_Lock() to disable the flash memory access
  *         (recommended to protect the FLASH memory against possible unwanted operation)
  * @param[in]  pEraseInit pointer to an FLASH_EraseInitTypeDef structure that
  *         contains the configuration information for the erasing.
  *
  * @param[out]  PageSectorError pointer to variable  that
  *         contains the configuration information on faulty page or sector in case of error
  *         (0xFFFFFFFF means that all the pages or sectors have been correctly erased)
  *
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageSectorError)
{
  HAL_StatusTypeDef status = HAL_ERROR;
  uint32_t address = 0U;

  /* Process Locked */
  __HAL_LOCK(&pFlash);
    
  /* Config flash timming */
  __HAL_FLASH_TIMMING_SEQUENCE_CONFIG();

  /* Check the parameters */
  assert_param(IS_FLASH_TYPEERASE(pEraseInit->TypeErase));

  if (pEraseInit->TypeErase == FLASH_TYPEERASE_MASSERASE)
  {
    /* Mass Erase requested for Bank1 */
    /* Wait for last operation to be completed */
    if (FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE) == HAL_OK)
    {
      /*Mass erase to be done*/
      FLASH_MassErase();

      /* Wait for last operation to be completed */
      status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

      /* If the erase operation is completed, disable the MER Bit */
      CLEAR_BIT(FLASH->CR, FLASH_CR_MER);
    }
  }else if(pEraseInit->TypeErase == FLASH_TYPEERASE_PAGEERASE)
  {
    /* Page Erase is requested */
    /* Check the parameters */
    assert_param(IS_FLASH_PROGRAM_ADDRESS(pEraseInit->PageAddress));
    assert_param(IS_FLASH_NB_PAGES(pEraseInit->PageAddress, pEraseInit->NbPages));

    /* Wait for last operation to be completed */
    if (FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE) == HAL_OK)
    {
      /*Initialization of PageSectorError variable*/
      *PageSectorError = 0xFFFFFFFFU;

      /* Erase page by page to be done*/
      for(address = pEraseInit->PageAddress;
          address < ((pEraseInit->NbPages * FLASH_PAGE_SIZE) + pEraseInit->PageAddress);
          address += FLASH_PAGE_SIZE)
      {
        FLASH_PageErase(address);

        /* Wait for last operation to be completed */
        status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
        /* If the erase operation is completed, disable the PER Bit */
        CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
        if (status != HAL_OK)
        {
          /* In case of error, stop erase procedure and return the faulty address */
          *PageSectorError = address;
          break;
        }
      }
    }
  }
  else if(pEraseInit->TypeErase == FLASH_TYPEERASE_SECTORERASE)
  {
    /* Sector Erase is requested */
    /* Check the parameters */
    assert_param(IS_FLASH_PROGRAM_ADDRESS(pEraseInit->SectorAddress));
    assert_param(IS_FLASH_NB_SECTORS(pEraseInit->SectorAddress, pEraseInit->NbSectors));

    /* Wait for last operation to be completed */
    if (FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE) == HAL_OK)
    {
      /*Initialization of PageSectorError variable*/
      *PageSectorError = 0xFFFFFFFFU;

      /* Erase page by page to be done*/
      for(address = pEraseInit->SectorAddress;
          address < ((pEraseInit->NbSectors * FLASH_SECTOR_SIZE) + pEraseInit->SectorAddress);
          address += FLASH_SECTOR_SIZE)
      {
        FLASH_SectorErase(address);
        /* Wait for last operation to be completed */
        status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);
        /* If the erase operation is completed, disable the SER Bit */
        CLEAR_BIT(FLASH->CR, FLASH_CR_SER);
        if (status != HAL_OK)
        {
          /* In case of error, stop erase procedure and return the faulty address */
          *PageSectorError = address;
          break;
        }
      }  
    }
  }

  /* Process Unlocked */
  __HAL_UNLOCK(&pFlash);

  return status;
}


/**
  * @brief  Perform a mass erase or erase the specified FLASH memory pages with interrupt enabled.
  * @param  pEraseInit Pointer to an @ref FLASH_EraseInitTypeDef structure that
  *         contains the configuration information for the erasing.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Erase_IT(FLASH_EraseInitTypeDef *pEraseInit)
{
  HAL_StatusTypeDef status;

  /* Check the parameters */
  assert_param(IS_FLASH_TYPEERASE(pEraseInit->TypeErase));

  /* Process Locked */
  __HAL_LOCK(&pFlash);
    
  /* Config flash timming */
  __HAL_FLASH_TIMMING_SEQUENCE_CONFIG();

  /* Reset error code */
  pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

  /* save procedure for interrupt treatment */
  pFlash.ProcedureOnGoing = pEraseInit->TypeErase;

  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

  if (status != HAL_OK)
  {
    /* Process Unlocked */
    __HAL_UNLOCK(&pFlash);
  }
  else
  {
    /* Enable End of Operation and Error interrupts */
    FLASH->CR |= FLASH_CR_EOPIE | FLASH_CR_ERRIE;

    if (pEraseInit->TypeErase == FLASH_TYPEERASE_MASSERASE)
    {
      /* Set Page to 0 for Interrupt callback management */
      pFlash.PageOrSector = 0;

      /* Proceed to Mass Erase */
      FLASH_MassErase();
    }else if (pEraseInit->TypeErase == FLASH_TYPEERASE_PAGEERASE)
    {
      /* Erase by page to be done */
      pFlash.NbPagesSectorsToErase = pEraseInit->NbPages;
      pFlash.PageOrSector = pEraseInit->PageAddress;

      /*Erase 1st page and wait for IT */
      FLASH_PageErase(pEraseInit->PageAddress);
    }else if (pEraseInit->TypeErase == FLASH_TYPEERASE_SECTORERASE)
    {
      /* Erase by sector to be done */
      pFlash.NbPagesSectorsToErase = pEraseInit->NbSectors;
      pFlash.PageOrSector = pEraseInit->SectorAddress;
        
      FLASH_SectorErase(pEraseInit->SectorAddress);
    }
  }

  /* return status */
  return status;
}

/**
  * @brief  Program of a page at a specified address.
  * @param  Address Specifies the address to be programmed.
  * @param  DataAddr:Page Start Address
  *
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_PageProgram(uint32_t Address, uint32_t * DataAddr )
{
   return HAL_FLASH_Program(FLASH_TYPEPROGRAM_PAGE, Address, DataAddr);
}

/**
  * @brief  Program of a page at a specified address.
  * @param  TypeProgram Indicate the way to program at a specified address.
  *                      This parameter can be a value of @ref FLASH_Type_Program
  * @param  Address Specifies the address to be programmed.
  * @param  DataAddr:Page Start Address
  *
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint32_t * DataAddr )
{
  HAL_StatusTypeDef status = HAL_ERROR;

  /* Process Locked */
  __HAL_LOCK(&pFlash);
    
  /* Config flash timming */
  __HAL_FLASH_TIMMING_SEQUENCE_CONFIG();

  /* Check the parameters */
  assert_param(IS_FLASH_TYPEPROGRAM(TypeProgram));
  assert_param(IS_FLASH_PROGRAM_ADDRESS(Address));

  /* must be the first address of the PAGE */
  if(Address%FLASH_PAGE_SIZE)
  {
    return HAL_ERROR;
  }

  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

  if(status == HAL_OK)
  {
    FLASH_Program_Page(Address, DataAddr);

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
    /* If the program operation is completed, disable the PG Bit */
    CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
    /* In case of error, stop programming procedure */
    if (status != HAL_OK)
    {
      return status;
    }
  }

  /* Process Unlocked */
  __HAL_UNLOCK(&pFlash);

  return status;
}

/**
  * @brief  Program of a page at a specified address with interrupt enabled.
  * @param  Address  Specifies the address to be programmed.
  * @param  DataAddr Specifies the buffer address to be programmed.
  *
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_PageProgram_IT(uint32_t Address, uint32_t *DataAddr)
{
  return HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_PAGE, Address, DataAddr);
}

/**
  * @brief  Program of a page at a specified address with interrupt enabled.
  * @param  TypeProgram Indicate the way to program at a specified address.
  *                      This parameter can be a value of @ref FLASH_Type_Program
  * @param  Address Specifies the address to be programmed.
  * @param  DataAddr Specifies the buffer address to be programmed.
  *
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Program_IT(uint32_t TypeProgram, uint32_t Address, uint32_t *DataAddr)
{
  HAL_StatusTypeDef status;

  /* Check the parameters */
  assert_param(IS_FLASH_TYPEPROGRAM(TypeProgram));
  assert_param(IS_FLASH_PROGRAM_ADDRESS(Address));

  /* Process Locked */
  __HAL_LOCK(&pFlash);
    
  /* Config flash timming */
  __HAL_FLASH_TIMMING_SEQUENCE_CONFIG();

  /* Reset error code */
  pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

  if (status != HAL_OK)
  {
    /* Process Unlocked */
    __HAL_UNLOCK(&pFlash);
  }else
  {
    /* Set internal variables used by the IRQ handler */
    pFlash.ProcedureOnGoing = TypeProgram;
    pFlash.Address = Address;

    /* Enable End of Operation and Error interrupts */
    __HAL_FLASH_ENABLE_IT(FLASH_IT_EOP | FLASH_IT_OPERR);

    FLASH_Program_Page(Address, DataAddr);
  }

  /* return status */
  return status;
}

/**
  * @brief  Set the read protection level.
  * @param  ReadProtectLevel specifies the read protection level.
  *         This parameter can be one of the following values:
  *            @arg @ref OB_RDP_LEVEL_0 No protection
  *            @arg @ref OB_RDP_LEVEL_1 Read protection of the memory
  *
  * @note   Warning: When enabling OB_RDP level 2 it's no more possible to go back to level 1 or 0
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_FLASH_OB_RDP_LevelConfig(uint8_t ReadProtectLevel)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the parameters */
  assert_param(IS_OB_RDP_LEVEL(ReadProtectLevel));

  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  if(status == HAL_OK)
  {
    /* Process Locked */
    __HAL_LOCK(&pFlash);

    /* Clean the error context */
    pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;

    /* If the previous operation is completed, proceed to erase the option bytes */
    SET_BIT(FLASH->OPTR, ReadProtectLevel);

    /* starts to modify Flash Option bytes */
    FLASH->CR|=FLASH_CR_OPTSTRT;

    /* set bit EOPIE */
    FLASH->CR|=FLASH_CR_EOPIE;

    /* trigger program */
    *((__IO uint32_t *)(0x40022080))=0xff;

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

    /* Process Unlocked */
    __HAL_UNLOCK(&pFlash);
  }

  return status;
}

/**
  * @brief  Return the FLASH User Option Byte value.
  * @retval The FLASH User Option Bytes values. It will be a combination of all the following values:
  *           @arg @ref FLASH_OB_USER_IWDG_SW
  *           @arg @ref FLASH_OB_USER_WWDG_SW
  *           @arg @ref FLASH_OB_USER_RESET_CONFIG
  *           @arg @ref FLASH_OB_USER_nBOOT1
  *           @arg @ref FLASH_OB_USER_IWDG_STOP
  */
static uint32_t FLASH_OB_GetUser(void)
{
  uint32_t user = ((FLASH->OPTR & ~FLASH_OPTR_RDP) & OB_USER_ALL);
  return user;
}

/**
  * @brief  Return the FLASH Read Protection level.
  * @retval FLASH ReadOut Protection Status:
  *         This return value can be one of the following values:
  *           @arg @ref OB_RDP_LEVEL_0 No protection
  *           @arg @ref OB_RDP_LEVEL_1 Read protection of the memory
  */
static uint32_t FLASH_OB_GetRDP(void)
{
  uint32_t rdplvl = READ_BIT(FLASH->OPTR, FLASH_OPTR_RDP);

  if (rdplvl == OB_RDP_LEVEL_0)
  {
    return (OB_RDP_LEVEL_0);
  }else
  {
    return rdplvl;
  }
}

/**
  * @brief  Set the BOR Level. 
  * @param  Level specifies the Option Bytes BOR Reset Level.
  *          This parameter can be one of the following values:
  *            @arg OB_BOR_OFF: BOR Disable
  *            @arg OB_BOR_LEVEL_1p7_1p8: Supply voltage ranges from 1.7 to 1.8 V
  *            @arg OB_BOR_LEVEL_1p9_2p0: Supply voltage ranges from 1.9 to 2.0 V
  *            @arg OB_BOR_LEVEL_2p1_2p2: Supply voltage ranges from 2.1 to 2.2 V
  *            @arg OB_BOR_LEVEL_2p3_2p4: Supply voltage ranges from 2.3 to 2.4 V
  *            @arg OB_BOR_LEVEL_2p5_2p6: Supply voltage ranges from 2.5 to 2.6 V
  *            @arg OB_BOR_LEVEL_2p7_2p8: Supply voltage ranges from 2.7 to 2.8 V
  *            @arg OB_BOR_LEVEL_2p9_3p0: Supply voltage ranges from 2.9 to 3.0 V
  *            @arg OB_BOR_LEVEL_3p1_3p2: Supply voltage ranges from 3.1 to 3.2 V
  * @retval HAL Status
  */
static HAL_StatusTypeDef FLASH_OB_BOR_LevelConfig(uint16_t Level)
{
  /* Check the parameters */
  assert_param(IS_OB_BOR_LEVEL(Level));

  /* Set the BOR Level */
  MODIFY_REG(FLASH->SDKR, FLASH_SDKR_BOR_EN  | FLASH_SDKR_BOR_LEV, Level);
  
  return HAL_OK; 
}

/**
  * @brief  Set user & RDP configuration
  * @param  UserType  The FLASH User Option Bytes to be modified.
  *         This parameter can be a combination of @ref FLASH_OB_USER_Type
  * @param  UserConfig  The FLASH User Option Bytes values.
  *         This parameter can be a combination of:
  *           @arg @ref FLASH_OB_USER_RESET_CONFIG
  *           @arg @ref FLASH_OB_USER_IWDG_SW
  *           @arg @ref FLASH_OB_USER_WWDG_SW
  *           @arg @ref FLASH_OB_USER_nBOOT1
  *           @arg @ref FLASH_OB_USER_IWDG_STOP
  * @param  RDPLevel  specifies the read protection level.
  *         This parameter can be one of the following values:
  *           @arg @ref OB_RDP_LEVEL_0 No protection
  *           @arg @ref OB_RDP_LEVEL_1 Memory Read protection
  * @note  (*) availability depends on devices
  * @retval None
  */
static void FLASH_OB_OptrConfig(uint32_t UserType, uint32_t UserConfig, uint32_t RDPLevel)
{
  uint32_t optr;

  /* Check the parameters */
  assert_param(IS_OB_USER_TYPE(UserType));
  assert_param(IS_OB_USER_CONFIG(UserType, UserConfig));
  assert_param(IS_OB_RDP_LEVEL(RDPLevel));

  /* Configure the RDP level in the option bytes register */
  optr = FLASH->OPTR;
  optr &= ~(UserType | FLASH_OPTR_RDP);
  FLASH->OPTR = (optr | UserConfig | RDPLevel);
}

/**
  * @brief  Program option bytes
  * @note   The function @ref HAL_FLASH_Unlock() should be called before to unlock the FLASH interface
  *         The function @ref HAL_FLASH_OB_Unlock() should be called before to unlock the options bytes
  *         The function @ref HAL_FLASH_OB_Launch() should be called after to force the reload of the options bytes
  *         (system reset will occur)
  *
  * @param  pOBInit pointer to an FLASH_OBInitStruct structure that
  *         contains the configuration information for the programming.
  *
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_OBProgram(FLASH_OBProgramInitTypeDef *pOBInit)
{
  uint32_t optr;
  HAL_StatusTypeDef status = HAL_ERROR;

  /* Process Locked */
  __HAL_LOCK(&pFlash);
    
  /* Config flash timming */
  __HAL_FLASH_TIMMING_SEQUENCE_CONFIG();

  /* Check the parameters */
  assert_param(IS_OPTIONBYTE(pOBInit->OptionType));

  /* WRP register */
  if ((pOBInit->OptionType & OPTIONBYTE_WRP) != 0)
  {
    /* Write protection configuration */
    FLASH->WRPR = (uint16_t)(~(pOBInit->WRPSector));
  }
  
  /* SDK register */
  if ((pOBInit->OptionType & OPTIONBYTE_SDK) != 0)
  {
    /* SDK protection configuration */
    FLASH->SDKR = (pOBInit->SDKStartAddr) | (pOBInit->SDKEndAddr<<8);
  }
  /* BOR Level  configuration */
  if((pOBInit->OptionType & OPTIONBYTE_BOR) == OPTIONBYTE_BOR)
  {
    status = FLASH_OB_BOR_LevelConfig(pOBInit->BORLevel);
  }
  
  /* Option register */
  if ((pOBInit->OptionType & (OPTIONBYTE_RDP | OPTIONBYTE_USER)) == (OPTIONBYTE_RDP | OPTIONBYTE_USER))
  {
    /* Fully modify OPTR register with RDP & user data */
    FLASH_OB_OptrConfig(pOBInit->USERType, pOBInit->USERConfig, pOBInit->RDPLevel);
  }
  else if((pOBInit->OptionType & OPTIONBYTE_RDP) != 0x00U)
  {
    /* Only modify RDP so get current user data */
    optr = FLASH_OB_GetUser();
    FLASH_OB_OptrConfig(optr, optr, pOBInit->RDPLevel);
  }
  else if ((pOBInit->OptionType & OPTIONBYTE_USER) != 0x00U)
  {
    /* Only modify user so get current RDP level */
    optr = FLASH_OB_GetRDP();
    FLASH_OB_OptrConfig(pOBInit->USERType, pOBInit->USERConfig, optr);
  }
  else
  {
    /* nothing to do */
  }

  /* starts to modify Flash Option bytes */
  FLASH->CR|=FLASH_CR_OPTSTRT;

  /* set bit EOPIE */
  FLASH->CR|=FLASH_CR_EOPIE;

  /* trigger program */
  *((__IO uint32_t *)(0x40022080))=0xff;

  /* Wait for last operation to be completed */
  status = FLASH_WaitForLastOperation((uint32_t)FLASH_TIMEOUT_VALUE);

  /* Process Unlocked */
  __HAL_UNLOCK(&pFlash);

  return status;
}

/**
  * @brief   Get the Option byte configuration
  * @param  pOBInit pointer to an FLASH_OBInitStruct structure that
  *         contains the configuration information for the programming.
  *
  * @retval None
  */
void HAL_FLASH_OBGetConfig(FLASH_OBProgramInitTypeDef *pOBInit)
{
  pOBInit->OptionType = OPTIONBYTE_ALL;

  /* Get WRP sector */
  pOBInit->WRPSector = (uint16_t)(~FLASH->WRPR);

  /* Get SDK sector */
  pOBInit->SDKStartAddr = (FLASH->SDKR)&0x1F;
  pOBInit->SDKEndAddr = ((FLASH->SDKR)&0x1F00)>>8;

  /*Get RDP Level*/
  if (((FLASH->OPTR)&0xFF) == OB_RDP_LEVEL_0)
  {
    pOBInit->RDPLevel   = OB_RDP_LEVEL_0;
  } else
  {
    pOBInit->RDPLevel   = OB_RDP_LEVEL_1;
  }
  /* Get BOR */
  pOBInit->BORLevel=(FLASH->SDKR)&(FLASH_SDKR_BOR_EN | FLASH_SDKR_BOR_LEV);

  /*Get USER*/
  pOBInit->USERType = OB_USER_ALL;
  pOBInit->USERConfig = (FLASH->OPTR)&(FLASH_OPTR_WWDG_SW   | FLASH_OPTR_IWDG_SW | \
                                       FLASH_OPTR_NRST_MODE | FLASH_OPTR_nBOOT1 | FLASH_OPTR_IWDG_STOP);
}

/**
  * @brief Handle FLASH interrupt request.
  * @retval None
  */
void HAL_FLASH_IRQHandler(void)
{
  uint32_t param = 0xFFFFFFFFU;
  uint32_t error;

  /* Save flash errors. Only ECC detection can be checked here as ECCC
     generates NMI */
  error = (FLASH->SR & FLASH_FLAG_SR_ERROR);

  CLEAR_BIT(FLASH->CR, pFlash.ProcedureOnGoing);

  /* A] Set parameter for user or error callbacks */
  /* check operation was a program or erase */
  if ((pFlash.ProcedureOnGoing & (FLASH_TYPEPROGRAM_PAGE)) != 0x00U)
  {
    /* return adress being programmed */
    param = pFlash.Address;
  }else if ((pFlash.ProcedureOnGoing & (FLASH_TYPEERASE_MASSERASE | FLASH_TYPEERASE_SECTORERASE | FLASH_TYPEERASE_PAGEERASE)) != 0x00U)
  {
    /* return page number being erased (0 for mass erase) */
    param = pFlash.PageOrSector;
  }else
  {
    /* Nothing to do */
  }

  /* B] Check errors */
  if (error != 0x00U)
  {
    /*Save the error code*/
    pFlash.ErrorCode |= error;

    /* clear error flags */
    __HAL_FLASH_CLEAR_FLAG(error);

    /*Stop the procedure ongoing*/
    pFlash.ProcedureOnGoing = FLASH_TYPENONE;

    /* Error callback */
    HAL_FLASH_OperationErrorCallback(param);
  }

  /* C] Check FLASH End of Operation flag */
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP) != 0x00U)
  {
    /* Clear FLASH End of Operation pending bit */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);

    if (pFlash.ProcedureOnGoing == FLASH_TYPEERASE_PAGEERASE)
    {
      /* Nb of pages to erased can be decreased */
      pFlash.NbPagesSectorsToErase--;

      /* Check if there are still pages to erase*/
      if (pFlash.NbPagesSectorsToErase != 0x00U)
      {
        /* Increment page number */
        pFlash.PageOrSector += FLASH_PAGE_SIZE;
        FLASH_PageErase(pFlash.PageOrSector);
      }else
      {
        /* No more pages to erase: stop erase pages procedure */
        pFlash.ProcedureOnGoing = FLASH_TYPENONE;
      }
    }
    else if (pFlash.ProcedureOnGoing == FLASH_TYPEERASE_SECTORERASE)
    {
      /* Nb of sectors to erased can be decreased */
      pFlash.NbPagesSectorsToErase--;

      /* Check if there are still pages to erase*/
      if (pFlash.NbPagesSectorsToErase != 0x00U)
      {
        /* Increment page number */
        pFlash.PageOrSector += FLASH_SECTOR_SIZE;
        FLASH_SectorErase(pFlash.PageOrSector);
      }else
      {
        /* No more pages to erase: stop erase pages procedure */
        pFlash.ProcedureOnGoing = FLASH_TYPENONE;
      }
    }
    else
    {
      /*Stop the ongoing procedure */
      pFlash.ProcedureOnGoing = FLASH_TYPENONE;
    }

    /* User callback */
    HAL_FLASH_EndOfOperationCallback(param);
  }

  if (pFlash.ProcedureOnGoing == FLASH_TYPENONE)
  {
    /* Disable End of Operation and Error interrupts */
    __HAL_FLASH_DISABLE_IT(FLASH_IT_EOP | FLASH_IT_OPERR);

    /* Process Unlocked */
    __HAL_UNLOCK(&pFlash);
  }
}

/**
  * @brief  FLASH end of operation interrupt callback.
  * @param  ReturnValue The value saved in this parameter depends on the ongoing procedure
  *                  Mass Erase: 0
  *                  Page Erase: Page which has been erased
  *                  Program: Address which was selected for data program
  * @retval None
  */
__weak void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(ReturnValue);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_FLASH_EndOfOperationCallback could be implemented in the user file
   */
}

/**
  * @brief  FLASH operation error interrupt callback.
  * @param  ReturnValue The value saved in this parameter depends on the ongoing procedure
  *                 Mass Erase: 0
  *                 Page Erase: Page number which returned an error
  *                 Program: Address which was selected for data program
  * @retval None
  */
__weak void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(ReturnValue);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_FLASH_OperationErrorCallback could be implemented in the user file
   */
}

#endif /* HAL_FLASH_MODULE_ENABLED */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
