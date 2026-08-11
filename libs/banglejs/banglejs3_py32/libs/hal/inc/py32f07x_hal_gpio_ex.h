/**
  ******************************************************************************
  * @file    py32f07x_hal_gpio_ex.h
  * @author  MCU Application Team
  * @brief   Header file of GPIO HAL Extended module.
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
#ifndef __PY32F07X_HAL_GPIO_EX_H
#define __PY32F07X_HAL_GPIO_EX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f07x_hal_def.h"

/** @addtogroup PY32F07x_HAL_Driver
  * @{
  */

/** @defgroup GPIOEx GPIOEx
  * @brief GPIO Extended HAL module driver
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/** @defgroup GPIOEx_Exported_Constants GPIOEx Exported Constants
  * @{
  */

/** @defgroup GPIOEx_Alternate_function_selection GPIOEx Alternate function selection
  * @{
  */
/**
  * @brief   AF 0 selection
  */
#define GPIO_AF0_SWJ             ((uint8_t)0x00)  /*!< SWJ (SWD) Alternate Function mapping */
#define GPIO_AF0_EVENOUT         ((uint8_t)0x00)  /*!< EVENOUT Alternate Function mapping   */
#define GPIO_AF0_TIM3            ((uint8_t)0x00)  /*!< TIM3 Alternate Function mapping      */
#define GPIO_AF0_TIM15           ((uint8_t)0x00)  /*!< TIM15 Alternate Function mapping     */
#define GPIO_AF0_TIM14           ((uint8_t)0x00)  /*!< TIM14 Alternate Function mapping     */
#define GPIO_AF0_TIM17           ((uint8_t)0x00)  /*!< TIM17 Alternate Function mapping     */
#define GPIO_AF0_SPI1            ((uint8_t)0x00)  /*!< SPI1 Alternate Function mapping      */
#define GPIO_AF0_SPI2            ((uint8_t)0x00)  /*!< SPI2 Alternate Function mapping      */ 
#define GPIO_AF0_I2S1            ((uint8_t)0x00)  /*!< I2S1 Alternate Function mapping      */
#define GPIO_AF0_I2S2            ((uint8_t)0x00)  /*!< I2S2 Alternate Function mapping      */
#define GPIO_AF0_MCO             ((uint8_t)0x00)  /*!< MCO Alternate Function mapping       */
#define GPIO_AF0_USART1          ((uint8_t)0x00)  /*!< USART1 Alternate Function mapping    */
#define GPIO_AF0_USART4          ((uint8_t)0x00)  /*!< USART4 Alternate Function mapping    */
#define GPIO_AF0_CTC             ((uint8_t)0x00)  /*!< CTC Alternate Function mapping       */
//#define GPIO_AF0_IR

/**
  * @brief   AF 1 selection
  */
#define GPIO_AF1_USART1          ((uint8_t)0x01)  /*!< USART1 Alternate Function mapping    */
#define GPIO_AF1_USART2          ((uint8_t)0x01)  /*!< USART2 Alternate Function mapping    */
#define GPIO_AF1_USART3          ((uint8_t)0x01)  /*!< USART3 Alternate Function mapping    */
#define GPIO_AF1_TIM3            ((uint8_t)0x01)  /*!< TIM3 Alternate Function mapping      */
#define GPIO_AF1_TIM15           ((uint8_t)0x01)  /*!< TIM15 Alternate Function mapping     */
#define GPIO_AF1_I2C1            ((uint8_t)0x01)  /*!< I2C1 Alternate Function mapping      */
#define GPIO_AF1_I2C2            ((uint8_t)0x01)  /*!< I2C2 Alternate Function mapping      */
#define GPIO_AF1_SPI2            ((uint8_t)0x01)  /*!< SPI2 Alternate Function mapping      */
#define GPIO_AF1_I2S             ((uint8_t)0x01)  /*!< I2S Alternate Function mapping       */
#define GPIO_AF1_EVENTOUT        ((uint8_t)0x01)  /*!< EVENTOUT Alternate Function mapping  */

/**
  * @brief   AF 2 selection
  */
#define GPIO_AF2_TIM1            ((uint8_t)0x02)  /*!< TIM1 Alternate Function mapping      */
#define GPIO_AF2_TIM2            ((uint8_t)0x02)  /*!< TIM2 Alternate Function mapping      */
#define GPIO_AF2_TIM16           ((uint8_t)0x02)  /*!< TIM16 Alternate Function mapping     */
#define GPIO_AF2_TIM17           ((uint8_t)0x02)  /*!< TIM17 Alternate Function mapping     */
#define GPIO_AF2_EVENTOUT        ((uint8_t)0x02)  /*!< EVENTOUT Alternate Function mapping  */

/**
  * @brief   AF 3 selection
  */
#define GPIO_AF3_EVENTOUT        ((uint8_t)0x03)  /*!< EVENTOUT Alternate Function mapping  */
#define GPIO_AF3_I2C1            ((uint8_t)0x03)  /*!< I2C1 Alternate Function mapping      */
#define GPIO_AF3_TIM15           ((uint8_t)0x03)  /*!< TIM15 Alternate Function mapping     */

/**
  * @brief   AF 4 selection
  */
#define GPIO_AF4_USART4          ((uint8_t)0x04)  /*!< USART4 Alternate Function mapping    */
#define GPIO_AF4_TIM14           ((uint8_t)0x04)  /*!< TIM14 Alternate Function mapping     */
#define GPIO_AF4_USART3          ((uint8_t)0x04)  /*!< USART3 Alternate Function mapping    */
#define GPIO_AF4_CTC             ((uint8_t)0x04)  /*!< CTC Alternate Function mapping       */
#define GPIO_AF4_CAN             ((uint8_t)0x04)  /*!< CAN Alternate Function mapping       */
#define GPIO_AF4_USART1          ((uint8_t)0x04)  /*!< USART1 Alternate Function mapping    */

/**
  * @brief   AF 5 selection
  */
#define GPIO_AF5_TIM15           ((uint8_t)0x05)  /*!< TIM15 Alternate Function mapping     */
#define GPIO_AF5_TIM16           ((uint8_t)0x05)  /*!< TIM16 Alternate Function mapping     */
#define GPIO_AF5_TIM17           ((uint8_t)0x05)  /*!< TIM17 Alternate Function mapping     */
#define GPIO_AF5_SPI2            ((uint8_t)0x05)  /*!< SPI2 Alternate Function mapping      */
#define GPIO_AF5_I2S2            ((uint8_t)0x05)  /*!< I2S2 Alternate Function mapping      */
#define GPIO_AF5_I2C2            ((uint8_t)0x05)  /*!< I2C2 Alternate Function mapping      */

/**
  * @brief   AF 6 selection
  */
#define GPIO_AF6_I2C1            ((uint8_t)0x06)  /*!< I2C1 Alternate Function mapping      */
#define GPIO_AF6_EVENTOUT        ((uint8_t)0x06)  /*!< EVENTOUT Alternate Function mapping  */

/**
  * @brief   AF 7 selection
  */
#define GPIO_AF7_COMP1           ((uint8_t)0x07)  /*!< COMP1 Alternate Function mapping     */
#define GPIO_AF7_COMP2           ((uint8_t)0x07)  /*!< COMP2 Alternate Function mapping     */
#define GPIO_AF7_COMP3           ((uint8_t)0x07)  /*!< COMP3 Alternate Function mapping     */
#define GPIO_AF7_EVENTOUT        ((uint8_t)0x07)  /*!< EVENTOUT Alternate Function mapping  */

/**
  * @brief   AF 8 selection
  */
#define GPIO_AF8_SPI1            ((uint8_t)0x08)  /*!< SPI1 Alternate Function mapping      */
#define GPIO_AF8_I2S1            ((uint8_t)0x08)  /*!< I2S1 Alternate Function mapping      */
#define GPIO_AF8_SPI2            ((uint8_t)0x08)  /*!< SPI2 Alternate Function mapping      */
#define GPIO_AF8_I2S2            ((uint8_t)0x08)  /*!< I2S2 Alternate Function mapping      */
#define GPIO_AF8_MCO             ((uint8_t)0x08)  /*!< MCO Alternate Function mapping       */

/**
  * @brief   AF 9 selection
  */
#define GPIO_AF9_SPI2            ((uint8_t)0x09)  /*!< SPI2 Alternate Function mapping      */
#define GPIO_AF9_I2S2            ((uint8_t)0x09)  /*!< I2S2 Alternate Function mapping      */
#define GPIO_AF9_I2S1            ((uint8_t)0x09)  /*!< I2S1 Alternate Function mapping      */
#define GPIO_AF9_MCO             ((uint8_t)0x09)  /*!< MCO Alternate Function mapping       */
#define GPIO_AF9_USART1          ((uint8_t)0x09)  /*!< USART1 Alternate Function mapping    */
#define GPIO_AF9_USART2          ((uint8_t)0x09)  /*!< USART2 Alternate Function mapping    */
#define GPIO_AF9_USART3          ((uint8_t)0x09)  /*!< USART3 Alternate Function mapping    */

/**
  * @brief   AF 10 selection
  */
#define GPIO_AF10_USART1         ((uint8_t)0x0A)  /*!< USART1 Alternate Function mapping    */
#define GPIO_AF10_USART3         ((uint8_t)0x0A)  /*!< USART3 Alternate Function mapping    */
#define GPIO_AF10_USART4         ((uint8_t)0x0A)  /*!< USART4 Alternate Function mapping    */

/**
  * @brief   AF 11 selection
  */
#define GPIO_AF11_TIM1           ((uint8_t)0x0B)  /*!< TIM1 Alternate Function mapping      */
#define GPIO_AF11_TIM2           ((uint8_t)0x0B)  /*!< TIM2 Alternate Function mapping      */
#define GPIO_AF11_TIM14          ((uint8_t)0x0B)  /*!< TIM14 Alternate Function mapping     */
#define GPIO_AF11_TIM15          ((uint8_t)0x0B)  /*!< TIM15 Alternate Function mapping     */
#define GPIO_AF11_COMP3          ((uint8_t)0x0B)  /*!< COMP3 Alternate Function mapping     */

/**
  * @brief   AF 12 selection
  */
#define GPIO_AF12_PVD            ((uint8_t)0x0C)  /*!< PVD Alternate Function mapping       */
#define GPIO_AF12_IR             ((uint8_t)0x0C)  /*!< IR Alternate Function mapping        */

/**
  * @brief   AF 13 selection
  */
#define GPIO_AF13_I2C1           ((uint8_t)0x0D)  /*!< I2C1 Alternate Function mapping      */
#define GPIO_AF13_I2C2           ((uint8_t)0x0D)  /*!< I2C2 Alternate Function mapping      */
#define GPIO_AF13_USRAT1         ((uint8_t)0x0D)  /*!< USRAT1 Alternate Function mapping    */
#define GPIO_AF13_TIM1           ((uint8_t)0x0D)  /*!< TIM1 Alternate Function mapping      */

/**
  * @brief   AF 14 selection
  */
#define GPIO_AF14_TIM1           ((uint8_t)0x0E)  /*!< TIM1 Alternate Function mapping      */

#define IS_GPIO_AF(AF)          ((AF) <= (uint8_t)0x0f)
/**
  * @}
  */

/**
  * @}
  */

#if defined(GPIOC)
#define GPIO_GET_INDEX(__GPIOx__)    (((__GPIOx__) == (GPIOA))? 0uL :\
                                      ((__GPIOx__) == (GPIOB))? 1uL :\
                                      ((__GPIOx__) == (GPIOC))? 2uL : 3uL)

#else
#define GPIO_GET_INDEX(__GPIOx__)    (((__GPIOx__) == (GPIOA))? 0uL :\
                                      ((__GPIOx__) == (GPIOB))? 1uL : 2uL)
#endif

/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __PY32F07X_HAL_GPIO_EX_H */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
