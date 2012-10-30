/**
  ******************************************************************************
  * @file    platform_config.h
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    29-June-2012
  * @brief   Evaluation board specific configuration file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#if defined(STM32F100)
 #include "stm32f10x.h"
  #include "stm32f10x_adc.h"
  #include "stm32f10x_gpio.h"
  #include "stm32f10x_usart.h"
  #include "stm32f10x_flash.h"
  #include "stm32f10x_tim.h"
//  #include "stm32f10x_rtc.h"
#elif defined(STM32F103)
 #include "stm32f10x.h"
  #include "stm32f10x_adc.h"
  #include "stm32f10x_gpio.h"
  #include "stm32f10x_usart.h"
  #include "stm32f10x_flash.h"
  #include "stm32f10x_tim.h"
//  #include "stm32f10x_rtc.h"
#elif defined(STM32F407)
 #include "stm32f4xx.h"
  #include "stm32f4xx_adc.h"
  #include "stm32f4xx_gpio.h"
  #include "stm32f4xx_usart.h"
  #include "stm32f4xx_flash.h"
  #include "stm32f4xx_syscfg.h"
  #include "stm32f4xx_tim.h"
//  #include "stm32f4xx_rtc.h"
#else
 #error UNKNOWN CHIP
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

// PININDEX is the index in the IOPIN_DATA array

// SYSTICK is the counter that counts up and that we use as the real-time clock
// The smaller this is, the longer we spend in interrupts, but also the more we can sleep!
//#define SYSTICK_RANGE 0x1000000 // the Maximum (it is a 24 bit counter) - on Olimexino this is about 0.6 sec
#define SYSTICK_RANGE 0x100000 // 16x faster
#define SYSTICKS_BEFORE_USB_DISCONNECT 32

#define DEFAULT_BUSY_PIN_INDICATOR -1 // no indicator
#define DEFAULT_SLEEP_PIN_INDICATOR -1 // no indicator

  #define USART1_PIN_RX                   GPIO_Pin_10
  #define USART1_PIN_TX                   GPIO_Pin_9
  #define USART1_PORT                     GPIOA
  #define USART2_PIN_RX                   GPIO_Pin_3
  #define USART2_PIN_TX                   GPIO_Pin_2
  #define USART2_PORT                     GPIOA
  #define USART3_PIN_RX                   GPIO_Pin_11
  #define USART3_PIN_TX                   GPIO_Pin_10
  #define USART3_PORT                     GPIOB



#if defined(OLIMEXINO_STM32)
  #define DEFAULT_CONSOLE_DEVICE              EV_SERIAL1

  #define IOBUFFERMASK 31 // (max 255) amount of items in event buffer - events take ~9 bytes each
  #define TXBUFFERMASK 31 // (max 255)

  #define LED1_PORT                           GPIOA
  #define LED1_PIN                            GPIO_Pin_1
  #define LED1_PININDEX                       13
  #define LED2_PORT                           GPIOA
  #define LED2_PIN                            GPIO_Pin_5
  #define LED2_PININDEX                       3
  #define BTN_PORT                            GPIOC // phew - is connected to BOOT0
  #define BTN_PIN                             GPIO_Pin_9
  #define BTN_PININDEX                        38


  #define USB_DISCONNECT                      GPIOC  
  #define USB_DISCONNECT_PIN                  GPIO_Pin_12
  #define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOC

  #define USARTS                          3

#elif defined(STM32VLDISCOVERY)
  #define DEFAULT_CONSOLE_DEVICE              EV_SERIAL1

  #define IOBUFFERMASK 31 // amount of items in event buffer - events take ~9 bytes each
  #define TXBUFFERMASK 31 // (max 255)

  #define LED1_PORT                           GPIOC
  #define LED1_PIN                            GPIO_Pin_8
  #define LED1_PININDEX                       ((2*16)+8)
  #define LED2_PORT                           GPIOC
  #define LED2_PIN                            GPIO_Pin_9
  #define LED2_PININDEX                       ((2*16)+9)
  #define BTN_PORT                            GPIOA
  #define BTN_PIN                             GPIO_Pin_0
  #define BTN_PININDEX                        ((0*16)+0)

  #define USARTS                          3

#elif defined(STM32F4DISCOVERY)
  #define DEFAULT_CONSOLE_DEVICE              EV_SERIAL2

  // we have loads of memory
  #define IOBUFFERMASK 63 // amount of items in event buffer - events take ~9 bytes each
  #define TXBUFFERMASK 127 // (max 255)

  #define LED1_PORT                           GPIOD
  #define LED1_PIN                            GPIO_Pin_12
  #define LED1_PININDEX                       ((3*16)+12)
  #define LED2_PORT                           GPIOD
  #define LED2_PIN                            GPIO_Pin_13
  #define LED2_PININDEX                       ((3*16)+13)
  #define LED3_PORT                           GPIOD
  #define LED3_PIN                            GPIO_Pin_14
  #define LED3_PININDEX                       ((3*16)+14)
  #define LED4_PORT                           GPIOD
  #define LED4_PIN                            GPIO_Pin_15
  #define LED4_PININDEX                       ((3*16)+15)
  #define BTN_PORT                            GPIOA
  #define BTN_PIN                             GPIO_Pin_0
  #define BTN_PININDEX                        ((0*16)+0)

  #define USARTS                          6
  #define USART4_PIN_RX                   GPIO_Pin_11
  #define USART4_PIN_TX                   GPIO_Pin_10
  #define USART4_PORT                     GPIOC
  #define USART5_PIN_RX                   GPIO_Pin_2
  #define USART5_PORT_RX                  GPIOD
  #define USART5_PIN_TX                   GPIO_Pin_12
  #define USART5_PORT_TX                  GPIOC
  #define USART6_PIN_RX                   GPIO_Pin_7
  #define USART6_PIN_TX                   GPIO_Pin_6
  #define USART6_PORT                     GPIOC

#else
  #error UNKNOWN BOARD
#endif 

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __PLATFORM_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
