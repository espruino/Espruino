/**
  ******************************************************************************
  * @file    py32f07x_hal_conf.h
  * @author  MCU Application Team
  * @brief   HAL configuration file.
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
#ifndef __PY32F07X_HAL_CONF_H
#define __PY32F07X_HAL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* ########################## Module Selection ############################## */
/**
  * @brief This is the list of modules to be used in the HAL driver
  */
#define HAL_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED
/* #define HAL_CRC_MODULE_ENABLED */
/* #define HAL_COMP_MODULE_ENABLED */
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
/* #define HAL_IWDG_MODULE_ENABLED */
/* #define HAL_WWDG_MODULE_ENABLED */
/* #define HAL_TIM_MODULE_ENABLED */
#define HAL_DMA_MODULE_ENABLED
/* #define HAL_LPTIM_MODULE_ENABLED */
#define HAL_PWR_MODULE_ENABLED
/* #define HAL_I2C_MODULE_ENABLED */
#define HAL_UART_MODULE_ENABLED
/* #define HAL_USART_MODULE_ENABLED */
#define HAL_SPI_MODULE_ENABLED
#define HAL_RTC_MODULE_ENABLED
/* #define HAL_LCD_MODULE_ENABLED */
#define HAL_EXTI_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
/* #define HAL_CTC_MODULE_ENABLED */
/* #define HAL_CAN_MODULE_ENABLED */
/* #define HAL_DAC_MODULE_ENABLED */
/* #define HAL_OPA_MODULE_ENABLED */
/* #define HAL_DIV_MODULE_ENABLED */
/* #define HAL_I2S_MODULE_ENABLED */
/* #define HAL_IRDA_MODULE_ENABLED */
/* #define HAL_SMARTCARD_MODULE_ENABLED */
/* #define HAL_USB_MODULE_ENABLED */

/* ########################## Oscillator Values adaptation ####################*/

#if !defined  (HSI_VALUE)
  #define HSI_VALUE              ((uint32_t)8000000)     /*!< Value of the Internal oscillator in Hz */
#endif /* HSI_VALUE */

/**
  * @brief Adjust the value of External High Speed oscillator (HSE) used in your application.
  *        This value is used by the RCC HAL module to compute the system frequency
  */
#if !defined  (HSE_VALUE)
  #define HSE_VALUE              ((uint32_t)24000000) /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    ((uint32_t)200)   /*!< Time out for HSE start up, in ms */
#endif /* HSE_STARTUP_TIMEOUT */

/**
  * @brief Internal Low Speed Internal oscillator (LSI) value.
  */
#if !defined  (LSI_VALUE)
 #define LSI_VALUE               ((uint32_t)32768)    /*!< LSI Typical Value in Hz */
#endif /* LSI_VALUE */                                /*!< Value of the Internal Low Speed oscillator in Hz
                                                       The real value may vary depending on the variations
                                                       in voltage and temperature. */

/**
  * @brief Adjust the value of External Low Speed oscillator (LSE) used in your application.
  *        This value is used by the RCC HAL module to compute the system frequency
  */
#if !defined  (LSE_VALUE)
  #define LSE_VALUE              ((uint32_t)32768) /*!< Value of the External oscillator in Hz*/
#endif /* LSE_VALUE */

#if !defined  (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    ((uint32_t)5000)   /*!< Time out for LSE start up, in ms */
#endif /* LSE_STARTUP_TIMEOUT */

/* Tip: To avoid modifying this file each time you need to use different HSE,
   ===  you can define the HSE value in your toolchain compiler preprocessor. */

/* ########################### System Configuration ######################### */
/**
  * @brief This is the HAL system configuration section
  */
#define  VDD_VALUE               ((uint32_t)3300) /*!< Value of VDD in mv */
#define  PRIORITY_HIGHEST        0
#define  PRIORITY_HIGH           1
#define  PRIORITY_LOW            2
#define  PRIORITY_LOWEST         3
#define  TICK_INT_PRIORITY       ((uint32_t)PRIORITY_LOWEST)    /*!< tick interrupt priority (lowest by default)  */
#define  USE_RTOS                0
#define  PREFETCH_ENABLE         0

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT       1U */

/* ################## SPI peripheral configuration ########################## */

/* CRC FEATURE: Use to activate CRC feature inside HAL SPI Driver
 * Activated: CRC code is present inside driver
 * Deactivated: CRC code cleaned from driver
 */

#define USE_SPI_CRC                     0U

/* Includes ------------------------------------------------------------------*/
/**
  * @brief Include module's header file
  */
#ifdef HAL_MODULE_ENABLED
 #include "py32f07x_hal.h"
#endif /* HAL_MODULE_ENABLED */

#ifdef HAL_RCC_MODULE_ENABLED
 #include "py32f07x_hal_rcc.h"
#endif /* HAL_RCC_MODULE_ENABLED */

#ifdef HAL_EXTI_MODULE_ENABLED
 #include "py32f07x_hal_exti.h"
#endif /* HAL_EXTI_MODULE_ENABLED */

#ifdef HAL_GPIO_MODULE_ENABLED
 #include "py32f07x_hal_gpio.h"
#endif /* HAL_GPIO_MODULE_ENABLED */

#ifdef HAL_CORTEX_MODULE_ENABLED
 #include "py32f07x_hal_cortex.h"
#endif /* HAL_CORTEX_MODULE_ENABLED */

#ifdef HAL_DMA_MODULE_ENABLED
  #include "py32f07x_hal_dma.h"
#endif /* HAL_DMA_MODULE_ENABLED */

#ifdef HAL_ADC_MODULE_ENABLED
 #include "py32f07x_hal_adc.h"
#endif /* HAL_ADC_MODULE_ENABLED */

#ifdef HAL_CRC_MODULE_ENABLED
 #include "py32f07x_hal_crc.h"
#endif /* HAL_CRC_MODULE_ENABLED */

#ifdef HAL_COMP_MODULE_ENABLED
#include "py32f07x_hal_comp.h"
#endif /* HAL_COMP_MODULE_ENABLED */

#ifdef HAL_FLASH_MODULE_ENABLED
 #include "py32f07x_hal_flash.h"
#endif /* HAL_FLASH_MODULE_ENABLED */

#ifdef HAL_I2C_MODULE_ENABLED
 #include "py32f07x_hal_i2c.h"
#endif /* HAL_I2C_MODULE_ENABLED */

#ifdef HAL_IWDG_MODULE_ENABLED
 #include "py32f07x_hal_iwdg.h"
#endif /* HAL_IWDG_MODULE_ENABLED */

#ifdef HAL_PWR_MODULE_ENABLED
 #include "py32f07x_hal_pwr.h"
#endif /* HAL_PWR_MODULE_ENABLED */

#ifdef HAL_RTC_MODULE_ENABLED
 #include "py32f07x_hal_rtc.h"
#endif /* HAL_RTC_MODULE_ENABLED */

#ifdef HAL_SPI_MODULE_ENABLED
 #include "py32f07x_hal_spi.h"
#endif /* HAL_SPI_MODULE_ENABLED */

#ifdef HAL_TIM_MODULE_ENABLED
 #include "py32f07x_hal_tim.h"
#endif /* HAL_TIM_MODULE_ENABLED */

#ifdef HAL_LPTIM_MODULE_ENABLED
 #include "py32f07x_hal_lptim.h"
#endif /* HAL_LPTIM_MODULE_ENABLED */

#ifdef HAL_UART_MODULE_ENABLED
 #include "py32f07x_hal_uart.h"
#endif /* HAL_UART_MODULE_ENABLED */

#ifdef HAL_WWDG_MODULE_ENABLED
 #include "py32f07x_hal_wwdg.h"
#endif /* HAL_WWDG_MODULE_ENABLED */

#ifdef HAL_LCD_MODULE_ENABLED
 #include "py32f07x_hal_lcd.h"
#endif /* HAL_LCD_MODULE_ENABLED */

#ifdef HAL_USART_MODULE_ENABLED
 #include "py32f07x_hal_usart.h"
#endif /* HAL_USART_MODULE_ENABLED */

#ifdef HAL_CTC_MODULE_ENABLED
 #include "py32f07x_hal_ctc.h"
#endif /* HAL_CTC_MODULE_ENABLED */

#ifdef HAL_CAN_MODULE_ENABLED
 #include "py32f07x_hal_can.h"
#endif /* HAL_CAN_MODULE_ENABLED */

#ifdef HAL_DAC_MODULE_ENABLED
 #include "py32f07x_hal_dac.h"
#endif /* HAL_DAC_MODULE_ENABLED */

#ifdef HAL_OPA_MODULE_ENABLED
 #include "py32f07x_hal_opa.h"
#endif /* HAL_OPA_MODULE_ENABLED */

#ifdef HAL_DIV_MODULE_ENABLED
 #include "py32f07x_hal_div.h"
#endif /* HAL_DIV_MODULE_ENABLED */

#ifdef HAL_I2S_MODULE_ENABLED
 #include "py32f07x_hal_i2s.h"
#endif /* HAL_I2S_MODULE_ENABLED */

#ifdef HAL_SMARTCARD_MODULE_ENABLED
 #include "py32f07x_hal_smartcard.h"
#endif /* HAL_SMARTCARD_MODULE_ENABLED */

#ifdef HAL_IRDA_MODULE_ENABLED
 #include "py32f07x_hal_irda.h"
#endif /* HAL_IRDA_MODULE_ENABLED */

#ifdef HAL_USB_MODULE_ENABLED
 #include "py32f07x_hal_usb.h"
#endif /* HAL_USB_MODULE_ENABLED */

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* __PY32F07X_HAL_CONF_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/