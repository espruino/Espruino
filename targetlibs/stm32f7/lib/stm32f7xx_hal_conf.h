/**
  ******************************************************************************
  * @file    stm32f7xx_hal_conf.h
  * @brief   HAL configuration file for Espruino (NUCLEO-F767ZI)
  ******************************************************************************
  */

#ifndef __STM32F7xx_HAL_CONF_H
#define __STM32F7xx_HAL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* ########################## Module Selection ############################## */
/* Only the flash HAL module is enabled (rest uses LL).
   Enable others here only if adding HAL source files to STM32F7.make. */

#define HAL_FLASH_MODULE_ENABLED

/* ########################## HSE/HSI Values adaptation ##################### */
/* NUCLEO-F767ZI: ST-LINK MCO provides 8 MHz to HSE */
#if !defined  (HSE_VALUE)
  #define HSE_VALUE    8000000U
#endif /* HSE_VALUE */

#if !defined  (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    100U
#endif /* HSE_STARTUP_TIMEOUT */

#if !defined  (HSI_VALUE)
  #define HSI_VALUE    16000000U
#endif /* HSI_VALUE */

#if !defined  (LSI_VALUE)
 #define LSI_VALUE    32000U
#endif /* LSI_VALUE */

#if !defined  (LSE_VALUE)
 #define LSE_VALUE    32768U
#endif /* LSE_VALUE */

#if !defined  (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    5000U
#endif /* LSE_STARTUP_TIMEOUT */

#if !defined  (EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE    12288000U
#endif /* EXTERNAL_CLOCK_VALUE */

/* ########################### System Configuration ######################### */
#define  VDD_VALUE                    3300U
#define  TICK_INT_PRIORITY            0x0FU
#define  USE_RTOS                     0U
#define  PREFETCH_ENABLE              1U
#define  ART_ACCELERATOR_ENABLE       1U

#define  USE_HAL_ADC_REGISTER_CALLBACKS         0U
#define  USE_HAL_DAC_REGISTER_CALLBACKS         0U
#define  USE_HAL_DMA_REGISTER_CALLBACKS         0U
#define  USE_HAL_EXTI_REGISTER_CALLBACKS        0U
#define  USE_HAL_FLASH_REGISTER_CALLBACKS       0U
#define  USE_HAL_GPIO_REGISTER_CALLBACKS        0U
#define  USE_HAL_I2C_REGISTER_CALLBACKS         0U
#define  USE_HAL_I2S_REGISTER_CALLBACKS         0U
#define  USE_HAL_PWR_REGISTER_CALLBACKS         0U
#define  USE_HAL_RCC_REGISTER_CALLBACKS         0U
#define  USE_HAL_RTC_REGISTER_CALLBACKS         0U
#define  USE_HAL_SDRAM_REGISTER_CALLBACKS       0U
#define  USE_HAL_SPI_REGISTER_CALLBACKS         0U
#define  USE_HAL_TIM_REGISTER_CALLBACKS         0U
#define  USE_HAL_UART_REGISTER_CALLBACKS        0U
#define  USE_HAL_USART_REGISTER_CALLBACKS       0U
#define  USE_HAL_WWDG_REGISTER_CALLBACKS        0U

/* ########################## Assert Selection ############################## */
/* #define USE_FULL_ASSERT    1 */

/* ################## SPI peripheral configuration ########################## */
#define USE_SPI_CRC     1U

/* Includes ------------------------------------------------------------------*/
/* Note: stm32f7xx_hal.h includes this file, and this file conditionally
   includes the module-specific HAL headers based on what's enabled above.
   Only modules with source files compiled by STM32F7.make should be enabled. */

#ifdef __cplusplus
extern "C" {
#endif

/* assert_param macro for HAL driver parameter checking */
#define assert_param(expr) ((void)0U)

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f7xx_hal_flash.h"
#endif

/* Minimal HAL_GetTick for flash timeout handling */
uint32_t HAL_GetTick(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F7xx_HAL_CONF_H */
