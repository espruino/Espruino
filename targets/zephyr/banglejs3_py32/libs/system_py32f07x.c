/**
  ******************************************************************************
  * @file    system_py32f07x.c
  * @author  MCU Application Team
  * @brief   CMSIS Cortex-M0+ Device Peripheral Access Layer System Source File
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

#include "py32f0xx.h"

#if !defined  (HSE_VALUE)
#define HSE_VALUE    24000000U    /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (HSI_VALUE)
#define HSI_VALUE  8000000U   /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */

#if !defined  (LSI_VALUE)
#define LSI_VALUE   32768U      /*!< Value of LSI in Hz*/
#endif /* LSI_VALUE */

#if !defined  (LSE_VALUE)
#define LSE_VALUE  32768U      /*!< Value of LSE in Hz*/
#endif /* LSE_VALUE */


/************************* Miscellaneous Configuration ************************/
/*!< Uncomment the following line if you need to relocate your vector Table in
     Internal SRAM. */
/* #define FORBID_VECT_TAB_MIGRATION */
/* #define VECT_TAB_SRAM */
#define VECT_TAB_OFFSET  0x00 /*!< Vector Table base offset field.
                                   This value must be a multiple of 0x100. */
/******************************************************************************/
/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
/* This variable is updated in three ways:
    1) by calling CMSIS function SystemCoreClockUpdate()
    2) by calling HAL API function HAL_RCC_GetHCLKFreq()
    3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
       Note: If you use this function to configure the system clock; then there
             is no need to call the 2 first functions listed above, since SystemCoreClock
             variable is updated automatically.
*/
uint32_t SystemCoreClock = HSI_VALUE;

const uint32_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint32_t APBPrescTable[8] =  {0, 0, 0, 0, 1, 2, 3, 4};
const uint32_t HSIFreqTable[8] = {4000000U, 8000000U, 16000000U, 22120000U, 24000000U, 4000000U, 4000000U, 4000000U};

/**
 * @brief  Clock functions.
 * @param  none
 * @return none
 */
void SystemCoreClockUpdate(void)             /* Get Core Clock Frequency      */
{
  uint32_t tmp;
  uint32_t hsidiv;
  uint32_t hsifs;
#if defined(RCC_PLL_SUPPORT)
  uint32_t pllmul=0;
  const uint8_t pllMulValue[4] = {2,3,2,2};
#endif /* RCC_PLL_SUPPORT */
  
  /* Get SYSCLK source -------------------------------------------------------*/
  switch (RCC->CFGR & RCC_CFGR_SWS)
  {
  case RCC_SYSCLKSOURCE_STATUS_HSE:  /* HSE used as system clock */
    SystemCoreClock = HSE_VALUE;
    break;

  case RCC_SYSCLKSOURCE_STATUS_LSI:  /* LSI used as system clock */
    SystemCoreClock = LSI_VALUE;
    break;
#if defined(RCC_LSE_SUPPORT)
  case RCC_SYSCLKSOURCE_STATUS_LSE:  /* LSE used as system clock */
    SystemCoreClock = LSE_VALUE;
    break;
#endif /* RCC_LSE_SUPPORT */
#if defined(RCC_PLL_SUPPORT)
  case RCC_SYSCLKSOURCE_STATUS_PLLCLK:  /* PLL used as system clock */   
   pllmul = pllMulValue[((RCC->PLLCFGR & RCC_PLLCFGR_PLLMUL)>>RCC_PLLCFGR_PLLMUL_Pos)] ;
  
   if ((RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) == RCC_PLLCFGR_PLLSRC_HSI) /* HSI used as PLL clock source */
   {
     hsifs = ((READ_BIT(RCC->ICSCR, RCC_ICSCR_HSI_FS)) >> RCC_ICSCR_HSI_FS_Pos);
     SystemCoreClock = pllmul * (HSIFreqTable[hsifs]);
   }
   else   /* HSE used as PLL clock source */
   {
     SystemCoreClock = pllmul * HSE_VALUE;
   }
   break; 
#endif /* RCC_PLL_SUPPORT */
  case RCC_SYSCLKSOURCE_STATUS_HSI:  /* HSI used as system clock */
  default:                /* HSI used as system clock */
    hsifs = ((READ_BIT(RCC->ICSCR, RCC_ICSCR_HSI_FS)) >> RCC_ICSCR_HSI_FS_Pos);
    hsidiv = (1UL << ((READ_BIT(RCC->CR, RCC_CR_HSIDIV)) >> RCC_CR_HSIDIV_Pos));
    SystemCoreClock = (HSIFreqTable[hsifs] / hsidiv);
    break;
  }
  /* Compute HCLK clock frequency --------------------------------------------*/
  /* Get HCLK prescaler */
  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos)];
  /* HCLK clock frequency */
  SystemCoreClock >>= tmp;
}

/**
 * @brief  Setup the microcontroller system.
 *         Initialize the System.
 * @param  none
 * @return none
 */
void SystemInit(void)
{
  /* Set the HSI clock to 8MHz by default */
  /* Set the LSI clock to 32.768KHz by default */
  RCC->ICSCR = (RCC->ICSCR & 0xFE000000) | (0x1 << 13) | *(uint32_t *)(0x1fff3208) | ((*(uint32_t *)(0x1fff3348))<<16);
  
  /* Configure the Vector Table location add offset address ------------------*/
#ifdef VECT_TAB_SRAM
  SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
#else
  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif /* VECT_TAB_SRAM */
}

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
