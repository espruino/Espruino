/**
  ******************************************************************************
  * @file    py32f07x_hal_flash.h
  * @author  MCU Application Team
  * @brief   Header file of FLASH HAL module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PY32F07X_HAL_FLASH_H
#define __PY32F07X_HAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

/** @addtogroup FLASH
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup FLASH_Exported_Types FLASH Exported Types
  * @{
  */

/**
  * @brief  FLASH Erase structure definition
  */
typedef struct
{
  uint32_t TypeErase;        /*!< Mass erase or page erase.
                                  This parameter can be a value of @ref FLASH_Type_Erase */
  uint32_t PageAddress;      /*!< PageAdress: Initial FLASH page address to erase when mass erase and sector erase is disabled
                                  This parameter must be a number between Min_Data = FLASH_BASE and Max_Data = FLASH_END */
  uint32_t NbPages;          /*!< Number of pages to be erased.
                                  This parameter must be a value between 1 and (FLASH_PAGE_NB - value of initial page)*/
  uint32_t SectorAddress;    /*!< PageAdress: Initial FLASH page address to erase when mass erase and page erase is disabled
                                  This parameter must be a number between Min_Data = FLASH_BASE and Max_Data = FLASH_BANK1_END */    
  uint32_t NbSectors;        /*!< Number of sectors to be erased.
                                  This parameter must be a value between 1 and (FLASH_SECTOR_NB - value of initial sector)*/
    
} FLASH_EraseInitTypeDef;

/**
  * @brief  FLASH Option Bytes PROGRAM structure definition
  */
typedef struct
{
  uint32_t  OptionType;       /*!< OptionType: Option byte to be configured.
                                   This parameter can be a value of @ref FLASH_Option_Type */

  uint32_t  WRPSector;        /*!< WRPSector: This bitfield specifies the sector (s) which are write protected.
                                   This parameter can be a combination of @ref FLASH_Option_Bytes_Write_Protection */

  uint32_t  SDKStartAddr;     /*!< SDK Start address (used for FLASH_SDKR). It represents first address of start block
                                   to protect. Make sure this parameter is multiple of SDK granularity: 4096 Bytes.*/

  uint32_t  SDKEndAddr;       /*!< SDK End address (used for FLASH_SDKR). It represents first address of end block
                                   to protect. Make sure this parameter is multiple of SDK granularity: 4096 Bytes.*/

  uint32_t  RDPLevel;         /*!< RDPLevel: Set the read protection level.
                                   This parameter can be a value of @ref FLASH_OB_Read_Protection */
  
  uint32_t  BORLevel;         /*!< Set the BOR Level.
                                   This parameter can be a value of @ref FLASH_OB_USER_BOR_LEVEL */

  uint32_t  USERType;         /*!< User option byte(s) to be configured (used for OPTIONBYTE_USER).
                                   This parameter can be a combination of @ref FLASH_OB_USER_Type */

  uint32_t  USERConfig;       /*!< Value of the user option byte (used for OPTIONBYTE_USER).
                                   This parameter can be a combination of
                                   @ref FLASH_OB_USER_RESET_CONFIG,
                                   @ref FLASH_OB_USER_IWDG_SW,
                                   @ref FLASH_OB_USER_WWDG_SW,
                                   @ref FLASH_OB_USER_nBOOT1
                                   @ref FLASH_OB_USER_IWDG_STOP */
} FLASH_OBProgramInitTypeDef;

/**
* @brief  FLASH handle Structure definition
*/
typedef struct
{
  HAL_LockTypeDef   Lock;              /* FLASH locking object */
  uint32_t          ErrorCode;         /* FLASH error code */
  uint32_t          ProcedureOnGoing;  /* Internal variable to indicate which procedure is ongoing or not in IT context */
  uint32_t          Address;           /* Internal variable to save address selected for program in IT context */
  uint32_t          PageOrSector;      /* Internal variable to define the current page or sector which is erasing in IT context */
  uint32_t          NbPagesSectorsToErase;    /* Internal variable to save the remaining pages to erase in IT context */
} FLASH_ProcessTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/** @defgroup FLASH_Exported_Constants FLASH Exported Constants
  * @{
  */

/**
  * @}
  */

/** @defgroup FLASH_Latency FLASH Latency
  * @{
  */
#define FLASH_LATENCY_0                 0x00000000UL                               /*!< FLASH Zero wait state SYSCLK<=24MHz */
#define FLASH_LATENCY_1                 FLASH_ACR_LATENCY_0                        /*!< FLASH One wait state 24MHz<SYSCLK<=48MHz */
#define FLASH_LATENCY_2                 FLASH_ACR_LATENCY_1                        /*!< FLASH Two wait state 48MHz<SYSCLK<=72MHz */


/**
  * @}
  */
  
/** @defgroup FLASH_Type_Erase FLASH erase type
  * @{
  */
#define FLASH_TYPEERASE_MASSERASE       (0x01U)  /*!<Flash mass erase activation*/
#define FLASH_TYPEERASE_PAGEERASE       (0x02U)  /*!<Flash Pages erase activation*/
#define FLASH_TYPEERASE_SECTORERASE     (0x03U)
/**
  * @}
  */

/** @defgroup FLASH_Flags FLASH Flags Definition
  * @{
  */
#define FLASH_FLAG_BSY                  FLASH_SR_BSY      /*!< FLASH Operation Busy flag */
#define FLASH_FLAG_OPTVERR              FLASH_SR_OPTVERR  /*!< FLASH Option validity error flag */
#define FLASH_FLAG_WRPERR               FLASH_SR_WRPERR   /*!< FLASH Write protection error flag */
#define FLASH_FLAG_EOP                  FLASH_SR_EOP      /*!< FLASH End of operation flag */

#define FLASH_FLAG_ALL_ERRORS           (FLASH_FLAG_WRPERR | FLASH_FLAG_OPTVERR)
/**
  * @}
  */

/** @defgroup FLASH_Interrupt_definition FLASH Interrupts Definition
  * @brief FLASH Interrupt definition
  * @{
  */
#define FLASH_IT_EOP                    FLASH_CR_EOPIE              /*!< End of FLASH Operation Interrupt source */
#define FLASH_IT_OPERR                  FLASH_CR_ERRIE              /*!< Error Interrupt source */
/**
  * @}
  */

/** @defgroup FLASH_Error FLASH Error
  * @{
  */
#define HAL_FLASH_ERROR_NONE            0x00000000U
#define HAL_FLASH_ERROR_WRP             FLASH_FLAG_WRPERR
#define HAL_FLASH_ERROR_OPTV            FLASH_FLAG_OPTVERR
/**
  * @}
  */

/** @defgroup FLASH_PROGRAM_ERASE_CLOCK FLASH Program and Erase Clock
  * @{
  */
#define FLASH_PROGRAM_ERASE_CLOCK_4MHZ        0x00000000U           /*!< 4MHz */
#define FLASH_PROGRAM_ERASE_CLOCK_8MHZ        0x00000001U           /*!< 8MHz */
#define FLASH_PROGRAM_ERASE_CLOCK_16MHZ       0x00000002U           /*!< 16MHz */
#define FLASH_PROGRAM_ERASE_CLOCK_22p12MHZ    0x00000003U           /*!< 22.12MHz */
#define FLASH_PROGRAM_ERASE_CLOCK_24MHZ       0x00000004U           /*!< 24MHz */
/**
  * @}
  */


/** @defgroup FLASH_Option_Bytes_Write_Protection FLASH Option Bytes Write Protection
  * @{
  */
#define OB_WRP_SECTOR_0            ((uint32_t)0x00000001U) /* Write protection of Sector0 */
#define OB_WRP_SECTOR_1            ((uint32_t)0x00000002U) /* Write protection of Sector1 */
#define OB_WRP_SECTOR_2            ((uint32_t)0x00000004U) /* Write protection of Sector2 */
#define OB_WRP_SECTOR_3            ((uint32_t)0x00000008U) /* Write protection of Sector3 */
#define OB_WRP_SECTOR_4            ((uint32_t)0x00000010U) /* Write protection of Sector4 */
#define OB_WRP_SECTOR_5            ((uint32_t)0x00000020U) /* Write protection of Sector5 */
#define OB_WRP_SECTOR_6            ((uint32_t)0x00000040U) /* Write protection of Sector6 */
#define OB_WRP_SECTOR_7            ((uint32_t)0x00000080U) /* Write protection of Sector7 */
#define OB_WRP_SECTOR_8            ((uint32_t)0x00000100U) /* Write protection of Sector8 */
#define OB_WRP_SECTOR_9            ((uint32_t)0x00000200U) /* Write protection of Sector9 */
#define OB_WRP_SECTOR_10           ((uint32_t)0x00000400U) /* Write protection of Sector10 */
#define OB_WRP_SECTOR_11           ((uint32_t)0x00000800U) /* Write protection of Sector11 */
#define OB_WRP_SECTOR_12           ((uint32_t)0x00001000U) /* Write protection of Sector12 */
#define OB_WRP_SECTOR_13           ((uint32_t)0x00002000U) /* Write protection of Sector13 */
#define OB_WRP_SECTOR_14           ((uint32_t)0x00004000U) /* Write protection of Sector14 */
#define OB_WRP_SECTOR_15           ((uint32_t)0x00008000U) /* Write protection of Sector15 */

#define OB_WRP_Pages0to31          ((uint32_t)0x00000001U) /* Write protection from page0   to page31 */
#define OB_WRP_Pages32to63         ((uint32_t)0x00000002U) /* Write protection from page32  to page63 */
#define OB_WRP_Pages64to95         ((uint32_t)0x00000004U) /* Write protection from page64  to page95 */
#define OB_WRP_Pages96to127        ((uint32_t)0x00000008U) /* Write protection from page96  to page127 */
#define OB_WRP_Pages128to159       ((uint32_t)0x00000010U) /* Write protection from page128 to page159 */
#define OB_WRP_Pages160to191       ((uint32_t)0x00000020U) /* Write protection from page160 to page191 */
#define OB_WRP_Pages192to223       ((uint32_t)0x00000040U) /* Write protection from page192 to page223 */
#define OB_WRP_Pages224to255       ((uint32_t)0x00000080U) /* Write protection from page224 to page255 */
#define OB_WRP_Pages256to287       ((uint32_t)0x00000100U) /* Write protection from page256 to page287 */
#define OB_WRP_Pages288to319       ((uint32_t)0x00000200U) /* Write protection from page288 to page319 */
#define OB_WRP_Pages320to351       ((uint32_t)0x00000400U) /* Write protection from page320 to page351 */
#define OB_WRP_Pages352to383       ((uint32_t)0x00000800U) /* Write protection from page352 to page383 */
#define OB_WRP_Pages384to415       ((uint32_t)0x00001000U) /* Write protection from page384 to page415 */
#define OB_WRP_Pages416to447       ((uint32_t)0x00002000U) /* Write protection from page416 to page447 */
#define OB_WRP_Pages448to479       ((uint32_t)0x00004000U) /* Write protection from page448 to page479 */
#define OB_WRP_Pages480to511       ((uint32_t)0x00008000U) /* Write protection from page480 to page511 */

#define OB_WRP_AllPages            ((uint32_t)0x0000FFFFU) /*!< Write protection of all Sectors */
/**
  * @}
  */

/** @defgroup FLASH_OB_Read_Protection FLASH Option Bytes Read Protection
  * @{
  */
#define OB_RDP_LEVEL_0         ((uint8_t)0xAAU)
#define OB_RDP_LEVEL_1         ((uint8_t)0x55U)

/**
  * @}
  */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_Type FLASH User Option Type
  * @{
  */
#define OB_USER_IWDG_SW         FLASH_OPTR_IWDG_SW
#define OB_USER_WWDG_SW         FLASH_OPTR_WWDG_SW
#define OB_USER_NRST_MODE       FLASH_OPTR_NRST_MODE
#define OB_USER_nBOOT1          FLASH_OPTR_nBOOT1
#define OB_USER_IWDG_STOP       FLASH_OPTR_IWDG_STOP
#define OB_USER_ALL             ( OB_USER_IWDG_SW | OB_USER_WWDG_SW | OB_USER_NRST_MODE | OB_USER_nBOOT1 | OB_USER_IWDG_STOP)

/**
  * @}
  */

/** @defgroup FLASH_Type_Program FLASH type program
  * @{
  */
#define FLASH_TYPEPROGRAM_PAGE       (0x01U)  /*!<Program 128bytes at a specified address.*/
/**
  * @}
  */
  
/** @defgroup FLASH_OB_USER_BOR_ENABLE FLASH Option Bytes BOR Level
  * @{
  */
#define OB_BOR_DISABLE                  0x00000000U        /*!< BOR Reset set to default */
#define OB_BOR_ENABLE                   FLASH_SDKR_BOR_EN  /*!< Use option byte to define BOR thresholds */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_BOR_LEVEL FLASH Option Bytes BOR Level
  * @{
  */
#define OB_BOR_OFF              ((uint32_t)(OB_BOR_DISABLE                                                                    )) /*!< BOR OFF   */
#define OB_BOR_LEVEL_1p7_1p8    ((uint32_t)(OB_BOR_ENABLE                                                                     )) /*!< BOR Reset threshold levels for 1.7V - 1.8V VDD power supply    */
#define OB_BOR_LEVEL_1p9_2p0    ((uint32_t)(OB_BOR_ENABLE                                               | FLASH_SDKR_BOR_LEV_0)) /*!< BOR Reset threshold levels for 1.9V - 2.0V VDD power supply    */
#define OB_BOR_LEVEL_2p1_2p2    ((uint32_t)(OB_BOR_ENABLE                        | FLASH_SDKR_BOR_LEV_1                       )) /*!< BOR Reset threshold levels for 2.1V - 2.2V VDD power supply    */
#define OB_BOR_LEVEL_2p3_2p4    ((uint32_t)(OB_BOR_ENABLE                        | FLASH_SDKR_BOR_LEV_1 | FLASH_SDKR_BOR_LEV_0)) /*!< BOR Reset threshold levels for 2.3V - 2.4V VDD power supply    */
#define OB_BOR_LEVEL_2p5_2p6    ((uint32_t)(OB_BOR_ENABLE | FLASH_SDKR_BOR_LEV_2                                              )) /*!< BOR Reset threshold levels for 2.5V - 2.6V VDD power supply    */
#define OB_BOR_LEVEL_2p7_2p8    ((uint32_t)(OB_BOR_ENABLE | FLASH_SDKR_BOR_LEV_2                        | FLASH_SDKR_BOR_LEV_0)) /*!< BOR Reset threshold levels for 2.7V - 2.8V VDD power supply    */
#define OB_BOR_LEVEL_2p9_3p0    ((uint32_t)(OB_BOR_ENABLE | FLASH_SDKR_BOR_LEV_2 | FLASH_SDKR_BOR_LEV_1                       )) /*!< BOR Reset threshold levels for 2.9V - 3.0V VDD power supply    */
#define OB_BOR_LEVEL_3p1_3p2    ((uint32_t)(OB_BOR_ENABLE | FLASH_SDKR_BOR_LEV_2 | FLASH_SDKR_BOR_LEV_1 | FLASH_SDKR_BOR_LEV_0)) /*!< BOR Reset threshold levels for 3.1V - 3.2V VDD power supply    */
  
/**
  * @}
  */

/** @defgroup FLASH_Option_Type FLASH Option Type
  * @{
  */
#define OPTIONBYTE_WRP            ((uint32_t)0x01U)  /*!<WRP option byte configuration*/
#define OPTIONBYTE_SDK            ((uint32_t)0x02U)  /*!<SDK option byte configuration*/
#define OPTIONBYTE_RDP            ((uint32_t)0x04U)  /*!<RDP option byte configuration*/
#define OPTIONBYTE_USER           ((uint32_t)0x08U)  /*!<USER option byte configuration*/
#define OPTIONBYTE_BOR            ((uint32_t)0x10U)  /*!<BOR option byte configuration*/
#define OPTIONBYTE_ALL            (OPTIONBYTE_WRP  | \
                                   OPTIONBYTE_SDK  | \
                                   OPTIONBYTE_RDP  | \
                                   OPTIONBYTE_USER  | \
                                   OPTIONBYTE_BOR)

/**
  * @}
  */

/** @defgroup FLASH_WRP_State FLASH WRP State
  * @{
  */
#define OB_WRPSTATE_DISABLE        ((uint32_t)0x00U)  /*!<Disable the write protection of the desired sectors*/
#define OB_WRPSTATE_ENABLE         ((uint32_t)0x01U)  /*!<Enable the write protection of the desired sectors*/

/**
  * @}
  */

/** @defgroup FLASH_OB_USER_IWDG_SW FLASH Option Bytes IWatchdog
  * @{
  */

#define OB_IWDG_SW                     FLASH_OPTR_IWDG_SW  /*!< Software IWDG selected */
#define OB_IWDG_HW                     0x00000000U         /*!< Hardware IWDG selected */

/**
  * @}
  */

/** @defgroup FLASH_OB_USER_WWDG_SW FLASH Option Bytes WWatchdog
  * @{
  */

#define OB_WWDG_SW                     FLASH_OPTR_WWDG_SW  /*!< Software WWDG selected */
#define OB_WWDG_HW                     0x00000000U         /*!< Hardware WWDG selected */

/**
  * @}
  */

/** @defgroup FLASH_OB_USER_RESET_CONFIG FLASH Option Bytes User reset config bit
  * @{
  */
#define OB_RESET_MODE_RESET            0x00000000U           /*!< Reset pin is in Reset input mode only */
#define OB_RESET_MODE_GPIO             FLASH_OPTR_NRST_MODE  /*!< Reset pin is in GPIO mode mode only */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_nBOOT1 FLASH Option Bytes BOOT Bit1 Setup
  * @{
  */
#define OB_BOOT1_SRAM                  0x00000000U        /*!< BOOT Bit 1 Reset */
#define OB_BOOT1_SYSTEM                FLASH_OPTR_nBOOT1  /*!< BOOT Bit 1 Set */
/**
  * @}
  */

/** @defgroup FLASH_OB_USER_IWDG_STOP FLASH IWDG Counter Freeze in STOP
  * @{
  */
#define OB_IWDG_STOP_FREEZE      ((uint32_t)0x00000000U) /*!< Freeze IWDG counter in STOP mode */
#define OB_IWDG_STOP_ACTIVE      ((uint32_t)FLASH_OPTR_IWDG_STOP) /*!< IWDG counter active in STOP mode */
/**
  * @}
  */


/**
  * @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup FLASH_Exported_Macros FLASH Exported Macros
  *  @brief macros to control FLASH features
  *  @{
  */

/**
  * @brief  Set the FLASH Latency.
  * @param  __LATENCY__ FLASH Latency
  *         This parameter can be one of the following values :
  *     @arg @ref FLASH_LATENCY_0  FLASH Zero wait state
  *     @arg @ref FLASH_LATENCY_1  FLASH One wait state
  *     @arg @ref FLASH_LATENCY_2  FLASH Two wait state
  * @retval None
  */
#define __HAL_FLASH_SET_LATENCY(__LATENCY__)    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, (__LATENCY__))

/**
  * @brief  Get the FLASH Latency.
  * @retval FLASH Latency
  *         Returned value can be one of the following values :
  *     @arg @ref FLASH_LATENCY_0  FLASH Zero wait state
  *     @arg @ref FLASH_LATENCY_1  FLASH One wait state
  *     @arg @ref FLASH_LATENCY_2  FLASH Two wait state
  *
  */
#define __HAL_FLASH_GET_LATENCY()               READ_BIT(FLASH->ACR, FLASH_ACR_LATENCY)




/** @defgroup FLASH_Interrupt FLASH Interrupts Macros
 *  @brief macros to handle FLASH interrupts
 * @{
 */

/**
  * @brief  Enable the specified FLASH interrupt.
  * @param  __INTERRUPT__ FLASH interrupt
  *         This parameter can be any combination of the following values:
  *     @arg @ref FLASH_IT_EOP End of FLASH Operation Interrupt
  *     @arg @ref FLASH_IT_OPERR Error Interrupt
  * @note (*) availability depends on devices
  * @retval none
  */
#define __HAL_FLASH_ENABLE_IT(__INTERRUPT__)    SET_BIT(FLASH->CR, (__INTERRUPT__))

/**
  * @brief  Disable the specified FLASH interrupt.
  * @param  __INTERRUPT__ FLASH interrupt
  *         This parameter can be any combination of the following values:
  *     @arg @ref FLASH_IT_EOP End of FLASH Operation Interrupt
  *     @arg @ref FLASH_IT_OPERR Error Interrupt
  * @note (*) availability depends on devices
  * @retval none
  */
#define __HAL_FLASH_DISABLE_IT(__INTERRUPT__)   CLEAR_BIT(FLASH->CR, (__INTERRUPT__))

/**
  * @brief  Check whether the specified FLASH flag is set or not.
  * @param  __FLAG__ specifies the FLASH flag to check.
  *   This parameter can be one of the following values:
  *     @arg @ref FLASH_FLAG_EOP FLASH End of Operation flag
  *     @arg @ref FLASH_FLAG_WRPERR FLASH Write protection error flag
  *     @arg @ref FLASH_FLAG_OPTVERR FLASH Option validity error flag
  *     @arg @ref FLASH_FLAG_BSY FLASH write/erase operations in progress flag
  *     @arg @ref FLASH_FLAG_ALL_ERRORS FLASH All errors flags
  * @note (*) availability depends on devices
  * @retval The new state of FLASH_FLAG (SET or RESET).
  */
#define __HAL_FLASH_GET_FLAG(__FLAG__)                    (READ_BIT(FLASH->SR,   (__FLAG__)) == (__FLAG__))

/**
  * @brief  Clear the FLASHs pending flags.
  * @param  __FLAG__ specifies the FLASH flags to clear.
  *   This parameter can be any combination of the following values:
  *     @arg @ref FLASH_FLAG_EOP FLASH End of Operation flag
  *     @arg @ref FLASH_FLAG_WRPERR FLASH Write protection error flag
  *     @arg @ref FLASH_FLAG_OPTVERR FLASH Option validity error flag
  *     @arg @ref FLASH_FLAG_ALL_ERRORS FLASH All errors flags
  * @retval None
  */
#define __HAL_FLASH_CLEAR_FLAG(__FLAG__)                do {  WRITE_REG(FLASH->SR, (__FLAG__)); \
                                                           } while(0U)
/**
  * @}
  */

#define __HAL_FLASH_TIME_REG_SET(__EPPARA0__,__EPPARA1__,__EPPARA2__,__EPPARA3__,__EPPARA4__)           \
                                                        do {                                            \
                                                            FLASH->TS0  = (__EPPARA0__)&0xFF;           \
                                                            FLASH->TS1  = ((__EPPARA0__)>>16)&0x1FF;    \
                                                            FLASH->TS3  = ((__EPPARA0__)>>8)&0xFF;      \
                                                            FLASH->TS2P = (__EPPARA1__)&0xFF;           \
                                                            FLASH->TPS3 = ((__EPPARA1__)>>16)&0x7FF;    \
                                                            FLASH->PERTPE = (__EPPARA2__)&0x1FFFF;      \
                                                            FLASH->SMERTPE = (__EPPARA3__)&0x1FFFF;     \
                                                            FLASH->PRGTPE = (__EPPARA4__)&0xFFFF;       \
                                                            FLASH->PRETPE = ((__EPPARA4__)>>16)&0x3FFF; \
                                                         } while(0U)

#define __HAL_FLASH_IS_INVALID_TIMMING_SEQUENCE(_INDEX_)  (((FLASH->TS0)     !=  ((*(uint32_t *)(_FlashTimmingParam[_INDEX_]))&0xFF))          ||  \
                                                           ((FLASH->TS1)     != (((*(uint32_t *)(_FlashTimmingParam[_INDEX_]))>>16)&0x1FF))    ||  \
                                                           ((FLASH->TS3)     != (((*(uint32_t *)(_FlashTimmingParam[_INDEX_]))>>8)&0xFF))      ||  \
                                                           ((FLASH->TS2P)    !=  ((*(uint32_t *)(_FlashTimmingParam[_INDEX_]+8))&0xFF))        ||  \
                                                           ((FLASH->TPS3)    != (((*(uint32_t *)(_FlashTimmingParam[_INDEX_]+8))>>16)&0x7FF))  ||  \
                                                           ((FLASH->PERTPE)  !=  ((*(uint32_t *)(_FlashTimmingParam[_INDEX_]+16))&0x1FFFF))     ||  \
                                                           ((FLASH->SMERTPE) !=  ((*(uint32_t *)(_FlashTimmingParam[_INDEX_]+24))&0x1FFFF))    ||  \
                                                           ((FLASH->PRGTPE)  !=  ((*(uint32_t *)(_FlashTimmingParam[_INDEX_]+32))&0xFFFF))     ||  \
                                                           ((FLASH->PRETPE)  != (((*(uint32_t *)(_FlashTimmingParam[_INDEX_]+32))>>16)&0x3FFF)))

#define __HAL_FLASH_TIMMING_SEQUENCE_CONFIG() do{                                                                            \
                                                uint32_t tmpreg = (RCC->ICSCR & RCC_ICSCR_HSI_FS) >> RCC_ICSCR_HSI_FS_Pos;   \
                                                if (__HAL_FLASH_IS_INVALID_TIMMING_SEQUENCE(tmpreg))                         \
                                                {                                                                            \
                                                  __HAL_FLASH_TIME_REG_SET((*(uint32_t *)(_FlashTimmingParam[tmpreg])),      \
                                                                           (*(uint32_t *)(_FlashTimmingParam[tmpreg]+8)),    \
                                                                           (*(uint32_t *)(_FlashTimmingParam[tmpreg]+16)),    \
                                                                           (*(uint32_t *)(_FlashTimmingParam[tmpreg]+24)),   \
                                                                           (*(uint32_t *)(_FlashTimmingParam[tmpreg]+32)));  \
                                                }                                                                            \
                                              }while(0U)

/* Include FLASH HAL Extended module */
/* Exported variables --------------------------------------------------------*/
/** @defgroup FLASH_Exported_Variables FLASH Exported Variables
  * @{
  */
extern FLASH_ProcessTypeDef pFlash;
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup FLASH_Exported_Functions
  * @{
  */

/* Program operation functions  ***********************************************/
/** @addtogroup FLASH_Exported_Functions_Group1
  * @{
  */
HAL_StatusTypeDef  HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint32_t *DataAddr );
HAL_StatusTypeDef  HAL_FLASH_Program_IT(uint32_t TypeProgram, uint32_t Address, uint32_t *DataAddr);
HAL_StatusTypeDef  HAL_FLASH_PageProgram(uint32_t Address, uint32_t *DataAddr );
HAL_StatusTypeDef  HAL_FLASH_PageProgram_IT(uint32_t Address, uint32_t *DataAddr);
/* FLASH IRQ handler method */
void               HAL_FLASH_IRQHandler(void);
/* Callbacks in non blocking modes */
void               HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue);
void               HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue);
HAL_StatusTypeDef  HAL_FLASH_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError);
HAL_StatusTypeDef  HAL_FLASH_Erase_IT(FLASH_EraseInitTypeDef *pEraseInit);

#define HAL_FLASHEx_Erase        HAL_FLASH_Erase
#define HAL_FLASHEx_Erase_IT     HAL_FLASH_Erase_IT
/**
  * @}
  */

/* Peripheral Control functions  **********************************************/
/** @addtogroup FLASH_Exported_Functions_Group2
  * @{
  */
HAL_StatusTypeDef  HAL_FLASH_Unlock(void);
HAL_StatusTypeDef  HAL_FLASH_Lock(void);
/* Option bytes control */
HAL_StatusTypeDef  HAL_FLASH_OB_Unlock(void);
HAL_StatusTypeDef  HAL_FLASH_OB_Lock(void);
HAL_StatusTypeDef  HAL_FLASH_OB_Launch(void);
HAL_StatusTypeDef  HAL_FLASH_OBProgram(FLASH_OBProgramInitTypeDef *pOBInit);
void               HAL_FLASH_OBGetConfig(FLASH_OBProgramInitTypeDef *pOBInit);
HAL_StatusTypeDef  HAL_FLASH_OB_RDP_LevelConfig(uint8_t ReadProtectLevel);
/**
  * @}
  */

/* Peripheral State functions  ************************************************/
/** @addtogroup FLASH_Exported_Functions_Group3
  * @{
  */
uint32_t HAL_FLASH_GetError(void);
/**
  * @}
  */

/**
  * @}
  */

/* Private types --------------------------------------------------------*/
/** @defgroup FLASH_Private_types FLASH Private Types
  * @{
  */
HAL_StatusTypeDef  FLASH_WaitForLastOperation(uint32_t Timeout);
/**
  * @}
  */

/* Private constants --------------------------------------------------------*/
/** @defgroup FLASH_Private_Constants FLASH Private Constants
  * @{
  */
#define FLASH_TIMEOUT_VALUE             1000U          /*!< FLASH Execution Timeout, 1 s */

#define FLASH_TYPENONE                  0x00000000U    /*!< No Programmation Procedure On Going */

#define FLASH_FLAG_SR_ERROR             (FLASH_FLAG_OPTVERR  | FLASH_FLAG_WRPERR)     /*!< All SR error flags */

#define FLASH_FLAG_SR_CLEAR             (FLASH_FLAG_SR_ERROR | FLASH_SR_EOP)
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/** @defgroup FLASH_Private_Macros FLASH Private Macros
 *  @{
 */
#define IS_FLASH_MAIN_MEM_ADDRESS(__ADDRESS__)         (((__ADDRESS__) >= (FLASH_BASE)) && ((__ADDRESS__) <= (FLASH_BASE + FLASH_SIZE - 1UL)))

#define IS_FLASH_PROGRAM_MAIN_MEM_ADDRESS(__ADDRESS__) (((__ADDRESS__) >= (FLASH_BASE)) && ((__ADDRESS__) <= (FLASH_BASE + FLASH_SIZE - 8UL)))

#define IS_FLASH_PROGRAM_ADDRESS(__ADDRESS__)          (IS_FLASH_PROGRAM_MAIN_MEM_ADDRESS(__ADDRESS__))

#define IS_FLASH_NB_PAGES(__ADDRESS__, __VALUE__)      (((__ADDRESS__) >= (FLASH_BASE)) && ((__ADDRESS__ + (__VALUE__*FLASH_PAGE_SIZE)) <= (FLASH_BASE + FLASH_SIZE - 1UL)))

#define IS_FLASH_NB_SECTORS(__ADDRESS__, __VALUE__)    (((__ADDRESS__) >= (FLASH_BASE)) && ((__ADDRESS__ + (__VALUE__*FLASH_SECTOR_SIZE)) <= (FLASH_BASE + FLASH_SIZE - 1UL)))

#define IS_FLASH_FAST_PROGRAM_ADDRESS(__ADDRESS__)     (((__ADDRESS__) >= (FLASH_BASE)) && ((__ADDRESS__) <= (FLASH_BASE + FLASH_SIZE - 256UL)))

#define IS_FLASH_PAGE(__PAGE__)                        ((__PAGE__) < FLASH_PAGE_NB)

#define IS_FLASH_BANK(__BANK__)                        ((__BANK__) == 0x00UL)

#define IS_FLASH_TYPEERASE(__VALUE__)                  (((__VALUE__) == FLASH_TYPEERASE_PAGEERASE) || \
                                                        ((__VALUE__) == FLASH_TYPEERASE_SECTORERASE) || \
                                                        ((__VALUE__) == FLASH_TYPEERASE_MASSERASE))

#define IS_FLASH_TYPEPROGRAM(__VALUE__)                ((__VALUE__) == FLASH_TYPEPROGRAM_PAGE)

#define IS_FLASH_TIMECONFIG_CLOCK(__VALUE__)           (((__VALUE__) == FLASH_PROGRAM_ERASE_CLOCK_4MHZ) || \
                                                        ((__VALUE__) == FLASH_PROGRAM_ERASE_CLOCK_8MHZ) || \
                                                        ((__VALUE__) == FLASH_PROGRAM_ERASE_CLOCK_16MHZ) || \
                                                        ((__VALUE__) == FLASH_PROGRAM_ERASE_CLOCK_22p12MHZ) || \
                                                        ((__VALUE__) == FLASH_PROGRAM_ERASE_CLOCK_24MHZ))

#define IS_OPTIONBYTE(__VALUE__)                       ((((__VALUE__) & OPTIONBYTE_ALL) != 0x00U) && \
                                                       (((__VALUE__) & ~OPTIONBYTE_ALL) == 0x00U))

#define IS_OB_RDP_LEVEL(__LEVEL__)                     (((__LEVEL__) == OB_RDP_LEVEL_0)   ||\
                                                        ((__LEVEL__) == OB_RDP_LEVEL_1))
                                                        
#define IS_OB_BOR_LEVEL(__LEVEL__)                     (((__LEVEL__) == OB_BOR_OFF)   ||\
                                                        ((__LEVEL__) == OB_BOR_LEVEL_1p7_1p8) || \
                                                        ((__LEVEL__) == OB_BOR_LEVEL_1p9_2p0) || \
                                                        ((__LEVEL__) == OB_BOR_LEVEL_2p1_2p2) || \
                                                        ((__LEVEL__) == OB_BOR_LEVEL_2p3_2p4) || \
                                                        ((__LEVEL__) == OB_BOR_LEVEL_2p5_2p6) || \
                                                        ((__LEVEL__) == OB_BOR_LEVEL_2p7_2p8) || \
                                                        ((__LEVEL__) == OB_BOR_LEVEL_2p9_3p0) || \
                                                        ((__LEVEL__) == OB_BOR_LEVEL_3p1_3p2))                                                        

#define IS_OB_USER_TYPE(__TYPE__)                      ((((__TYPE__) & OB_USER_ALL) != 0x00U) && \
                                                        (((__TYPE__) & ~OB_USER_ALL) == 0x00U))

#define IS_OB_USER_CONFIG(__TYPE__,__CONFIG__)         ((~(__TYPE__) & (__CONFIG__)) == 0x00U)

#if defined(FLASH_PCROP_SUPPORT)
#define IS_OB_PCROP_CONFIG(__CONFIG__)                 (((__CONFIG__) & ~(OB_PCROP_ZONE_A | OB_PCROP_ZONE_B | OB_PCROP_RDP_ERASE)) == 0x00U)
#endif

#if defined(FLASH_SECURABLE_MEMORY_SUPPORT)
#define IS_OB_SEC_BOOT_LOCK(__VALUE__)                 (((__VALUE__) == OB_BOOT_ENTRY_FORCED_NONE) || ((__VALUE__) == OB_BOOT_ENTRY_FORCED_FLASH))

#define IS_OB_SEC_SIZE(__VALUE__)                      ((__VALUE__) < (FLASH_PAGE_NB + 1U))
#endif

#define IS_FLASH_LATENCY(__LATENCY__)                  (((__LATENCY__) == FLASH_LATENCY_0) || \
                                                        ((__LATENCY__) == FLASH_LATENCY_1) || \
                                                        ((__LATENCY__) == FLASH_LATENCY_2))

#define IS_WRPSTATE(__VALUE__)                         (((__VALUE__) == OB_WRPSTATE_DISABLE) || \
                                                        ((__VALUE__) == OB_WRPSTATE_ENABLE))

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __PY32F07X_HAL_FLASH_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
