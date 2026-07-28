/**
  ******************************************************************************
  * @file    py32f072xb.h
  * @brief   CMSIS Cortex-M0+ Device Peripheral Access Layer Header File.
    *          This file contains all the peripheral register's definitions, bits
  *          definitions and memory mapping for PY32F072 devices.
  * @version v1.0.1
  *
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

/** @addtogroup CMSIS_Device
  * @{
  */

/** @addtogroup py32f072xb
  * @{
  */

#ifndef __PY32F072XB_H
#define __PY32F072XB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @addtogroup Configuration_section_for_CMSIS
  * @{
  */

/**
  * @brief Configuration of the Cortex-M0+ Processor and Core Peripherals
   */
#define __CM0PLUS_REV             0 /*!< Core Revision r0p0                            */
#define __MPU_PRESENT             0 /*!< PY32F072 do not provide MPU                   */
#define __VTOR_PRESENT            1 /*!< Vector  Table  Register supported             */
#define __NVIC_PRIO_BITS          2 /*!< PY32F072 uses 2 Bits for the Priority Levels  */
#define __Vendor_SysTickConfig    0 /*!< Set to 1 if different SysTick Config is used  */

/**
  * @}
  */

/** @addtogroup Peripheral_interrupt_number_definition
  * @{
  */

/**
 * @brief PY32F072 Interrupt Number Definition, according to the selected device
 *        in @ref Library_configuration_section
 */

/*!< Interrupt Number Definition */
typedef enum
{
  /******  Cortex-M0+ Processor Exceptions Numbers ***************************************************************/
  NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                                          */
  HardFault_IRQn              = -13,    /*!< 3 Cortex-M Hard Fault Interrupt                                   */
  SVC_IRQn                    = -5,     /*!< 11 Cortex-M SV Call Interrupt                                     */
  PendSV_IRQn                 = -2,     /*!< 14 Cortex-M Pend SV Interrupt                                     */
  SysTick_IRQn                = -1,     /*!< 15 Cortex-M System Tick Interrupt                                 */
  /******  PY32F0 specific Interrupt Numbers *********************************************************************/
  WWDG_IRQn                   = 0,      /*!< Window WatchDog Interrupt                                         */
  PVD_IRQn                    = 1,      /*!< PVD through EXTI Line detection Interrupt(EXTI line 16)           */
  RTC_IRQn                    = 2,      /*!< RTC interrupt through the EXTI line 19                            */
  FLASH_IRQn                  = 3,      /*!< FLASH global Interrupt                                            */
  RCC_CTC_IRQn                = 4,      /*!< RCC and CTC global Interrupts                                     */
  EXTI0_1_IRQn                = 5,      /*!< EXTI 0 and 1 Interrupts                                           */
  EXTI2_3_IRQn                = 6,      /*!< EXTI Line 2 and 3 Interrupts                                      */
  EXTI4_15_IRQn               = 7,      /*!< EXTI Line 4 to 15 Interrupts                                      */
  LCD_IRQn                    = 8,      /*!< LCD global Interrupt                                              */
  DMA1_Channel1_IRQn          = 9,      /*!< DMA1 Channel 1 Interrupt                                          */
  DMA1_Channel2_3_IRQn        = 10,     /*!< DMA1 Channel 2 and Channel 3 Interrupts                           */
  DMA1_Channel4_5_6_7_IRQn    = 11,     /*!< DMA1 Channel 4, 5, 6, 7 Interrupts                                */
  ADC_COMP_IRQn               = 12,     /*!< ADC&COMP Interrupts                                               */
  TIM1_BRK_UP_TRG_COM_IRQn    = 13,     /*!< TIM1 Break, Update, Trigger and Commutation Interrupts            */
  TIM1_CC_IRQn                = 14,     /*!< TIM1 Capture Compare Interrupt                                    */
  TIM2_IRQn                   = 15,     /*!< TIM2 global Interrupt                                             */
  TIM3_IRQn                   = 16,     /*!< TIM3 global Interrupt                                             */
  TIM6_LPTIM1_DAC_IRQn        = 17,     /*!< TIM6, LPTIM1, DAC global Interrupts                               */
  TIM7_IRQn                   = 18,     /*!< TIM7 global Interrupt                                             */
  TIM14_IRQn                  = 19,     /*!< TIM14 global Interrupt                                            */
  TIM15_IRQn                  = 20,     /*!< TIM15 global Interrupt                                            */
  TIM16_IRQn                  = 21,     /*!< TIM16 global Interrupt                                            */
  TIM17_IRQn                  = 22,     /*!< TIM17 global Interrupt                                            */
  I2C1_IRQn                   = 23,     /*!< I2C1 global Interrupt                                             */
  I2C2_IRQn                   = 24,     /*!< I2C2 global Interrupt                                             */
  SPI1_IRQn                   = 25,     /*!< SPI1 global Interrupt                                             */
  SPI2_IRQn                   = 26,     /*!< SPI2 global Interrupt                                             */
  USART1_IRQn                 = 27,     /*!< USART1 global Interrupt                                           */
  USART2_IRQn                 = 28,     /*!< USART2 global Interrupt                                           */
  USART3_4_IRQn               = 29,     /*!< USART3, 4 global Interrupts                                       */
  CAN_IRQn                    = 30,     /*!< CAN global Interrupt                                              */
  USB_IRQn                    = 31,     /*!< USB global Interrupt                                              */
} IRQn_Type;

/**
  * @}
  */

#include "core_cm0plus.h"               /* Cortex-M0+ processor and core peripherals */
#include "system_py32f0xx.h"            /* PY32F0xx System Header */
#include <stdint.h>

/** @addtogroup Peripheral_registers_structures
  * @{
  */

/**
* @brief ADC Registers
*/
typedef struct
{
  __IO uint32_t SR;          /*!< ADC desc SR,      Address offset: 0x00 */
  __IO uint32_t CR1;         /*!< ADC desc CR1,     Address offset: 0x04 */
  __IO uint32_t CR2;         /*!< ADC desc CR2,     Address offset: 0x08 */
  __IO uint32_t SMPR1;       /*!< ADC desc SMPR1,   Address offset: 0x0C */
  __IO uint32_t SMPR2;       /*!< ADC desc SMPR2,   Address offset: 0x10 */
  __IO uint32_t SMPR3;       /*!< ADC desc SMPR3,   Address offset: 0x14 */
  __IO uint32_t JOFR1;       /*!< ADC desc JOFR1,   Address offset: 0x18 */
  __IO uint32_t JOFR2;       /*!< ADC desc JOFR2,   Address offset: 0x1C */
  __IO uint32_t JOFR3;       /*!< ADC desc JOFR3,   Address offset: 0x20 */
  __IO uint32_t JOFR4;       /*!< ADC desc JOFR4,   Address offset: 0x24 */
  __IO uint32_t HTR;         /*!< ADC desc HTR,     Address offset: 0x28 */
  __IO uint32_t LTR;         /*!< ADC desc LTR,     Address offset: 0x2C */
  __IO uint32_t SQR1;        /*!< ADC desc SQR1,    Address offset: 0x30 */
  __IO uint32_t SQR2;        /*!< ADC desc SQR2,    Address offset: 0x34 */
  __IO uint32_t SQR3;        /*!< ADC desc SQR3,    Address offset: 0x38 */
  __IO uint32_t JSQR;        /*!< ADC desc JSQR,    Address offset: 0x3C */
  __IO uint32_t JDR1;        /*!< ADC desc JDR1,    Address offset: 0x40 */
  __IO uint32_t JDR2;        /*!< ADC desc JDR2,    Address offset: 0x44 */
  __IO uint32_t JDR3;        /*!< ADC desc JDR3,    Address offset: 0x48 */
  __IO uint32_t JDR4;        /*!< ADC desc JDR4,    Address offset: 0x4C */
  __IO uint32_t DR;          /*!< ADC desc DR,      Address offset: 0x50 */
  __IO uint32_t CCSR;        /*!< ADC desc CCSR,    Address offset: 0x54 */
} ADC_TypeDef;

/**
* @brief CAN LLC Acceptance filters Registers
*/
typedef struct
{
  __IO uint32_t ID;
  __IO uint32_t FORMAT;
  __IO uint32_t TYPE;
  __IO uint32_t RESERVED1;
} CAN_LLC_AC_TypeDef;

/**
* @brief CAN LLC Acceptance filters Registers
*/
typedef struct
{
  __IO uint32_t ID;
  __IO uint32_t FORMAT;
  __IO uint32_t TYPE;
  __IO uint32_t RESERVED1;
  __IO uint32_t RESERVED2;
  __IO uint32_t RESERVED3;
  __IO uint32_t TTCAN;
  __IO uint32_t DATA[2];
} CAN_LLC_TypeDef;

/**
* @brief CAN Registers
*/
typedef struct
{
  __IO uint32_t           TSNCR;       /*!< CAN desc TSNCR,   Address offset: 0x00 */
  __IO uint32_t           ACBTR;       /*!< CAN desc ACBTR,   Address offset: 0x04 */
  __IO uint32_t           RESERVED1;       /*!< CAN desc FDBTR,   Address offset: 0x08 */
  __IO uint32_t           RESERVED2;       /*!< CAN desc XLBTR,   Address offset: 0x0C */
  __IO uint32_t           RLSSP;       /*!< CAN desc RLSSP,   Address offset: 0x10 */
  __IO uint32_t           IFR;         /*!< CAN desc IFR,     Address offset: 0x14 */
  __IO uint32_t           IER;         /*!< CAN desc IER,     Address offset: 0x18 */
  __IO uint32_t           TSR;         /*!< CAN desc TSR,     Address offset: 0x1C */
  __IO uint32_t           RESERVED3;        /*!< CAN desc TTSL,    Address offset: 0x20 */
  __IO uint32_t           RESERVED4;        /*!< CAN desc TTSH,    Address offset: 0x24 */
  __IO uint32_t           MCR;         /*!< CAN desc MCR,     Address offset: 0x28 */
  __IO uint32_t           WECR;        /*!< CAN desc WECR,    Address offset: 0x2C */
  __IO uint32_t           REFMSG;      /*!< CAN desc REFMSG,  Address offset: 0x30 */
  __IO uint32_t           TTCR;        /*!< CAN desc TTCR,    Address offset: 0x34 */
  __IO uint32_t           TTTR;        /*!< CAN desc TTTR,    Address offset: 0x38 */
  __IO uint32_t           SCMS;        /*!< CAN desc SCMS,    Address offset: 0x3C */
  __IO uint32_t           RESERVED5;        /*!< CAN desc MESR,    Address offset: 0x40 */
  __IO uint32_t           ACFCR;       /*!< CAN desc ACFCR,   Address offset: 0x44 */
  CAN_LLC_AC_TypeDef    ACFC;          /*!< CAN desc ACFC,    Address offset: 0x48 - 0x57 */
  CAN_LLC_AC_TypeDef    ACFM;          /*!< CAN desc ACFM,    Address offset: 0x58 - 0x67 */
  uint8_t        RESERVED18[8];        /*!< Reserved,         Address offset: 0x68 - 0x6F */
  CAN_LLC_TypeDef       RBUF;          /*!< CAN desc RBUF,    Address offset: 0x70 - 0x93 */
  CAN_LLC_TypeDef       TBUF;          /*!< CAN desc TBUF,    Address offset: 0x94 - 0xB7 */
  __IO uint32_t           RESERVED6;       /*!< CAN desc PWMCR,   Address offset: 0xB8 */
} CAN_TypeDef;

/**
  * @brief CRC calculation unit
  */
typedef struct
{
  __IO uint32_t DR;             /*!< CRC Data register,                         Address offset: 0x00 */
  __IO uint32_t IDR;            /*!< CRC Independent data register,             Address offset: 0x04 */
  __IO uint32_t CR;             /*!< CRC Control register,                      Address offset: 0x08 */
} CRC_TypeDef;

/**
  * @brief Comparator
  */
typedef struct
{
  __IO uint32_t CSR;           /*!< COMP control and status register,           Address offset: 0x00 */
  __IO uint32_t FR;            /*!< COMP filter register,                       Address offset: 0x04 */
} COMP_TypeDef;

typedef struct
{
  __IO uint32_t CSR_ODD;       /*!< COMP control and status register located in register of comparator instance odd, used for bits common to several COMP instances,  Address offset: 0x00 */
  __IO uint32_t FR_ODD;
  __IO uint32_t CSR_EVEN;      /*!< COMP control and status register located in register of comparator instance even, used for bits common to several COMP instances, Address offset: 0x04 */
  __IO uint32_t FR_EVEN;
} COMP_Common_TypeDef;

/**
* @brief CTC Registers
*/
typedef struct
{
  __IO uint32_t CTL0;        /*!< CTC desc CTL0, Address offset: 0x00 */
  __IO uint32_t CTL1;        /*!< CTC desc CTL1, Address offset: 0x04 */
  __IO uint32_t SR;          /*!< CTC desc SR,   Address offset: 0x08 */
  __IO uint32_t INTC;        /*!< CTC desc INTC, Address offset: 0x0C */
} CTC_TypeDef;

/**
* @brief DAC Registers
*/
typedef struct
{
  __IO uint32_t CR;          /*!< DAC desc CR,      Address offset: 0x00 */
  __IO uint32_t SWTRIGR;     /*!< DAC desc SWTRIGR, Address offset: 0x04 */
  __IO uint32_t DHR12R1;     /*!< DAC desc DHR12R1, Address offset: 0x08 */
  __IO uint32_t DHR12L1;     /*!< DAC desc DHR12L1, Address offset: 0x0C */
  __IO uint32_t DHR8R1;      /*!< DAC desc DHR8R1,  Address offset: 0x10 */
  __IO uint32_t DHR12R2;     /*!< DAC desc DHR12R2, Address offset: 0x14 */
  __IO uint32_t DHR12L2;     /*!< DAC desc DHR12L2, Address offset: 0x18 */
  __IO uint32_t DHR8R2;      /*!< DAC desc DHR8R2,  Address offset: 0x1C */
  __IO uint32_t DHR12RD;     /*!< DAC desc DHR12RD, Address offset: 0x20 */
  __IO uint32_t DHR12LD;     /*!< DAC desc DHR12LD, Address offset: 0x24 */
  __IO uint32_t DHR8RD;      /*!< DAC desc DHR8RD,  Address offset: 0x28 */
  __IO uint32_t DOR1;        /*!< DAC desc DOR1,    Address offset: 0x2C */
  __IO uint32_t DOR2;        /*!< DAC desc DOR2,    Address offset: 0x30 */
  __IO uint32_t SR;          /*!< DAC desc SR,      Address offset: 0x34 */
} DAC_TypeDef;

/**
  * @brief Debug MCU
  */
typedef struct
{
  __IO uint32_t IDCODE;      /*!< MCU device ID code,              Address offset: 0x00 */
  __IO uint32_t CR;          /*!< Debug configuration register,    Address offset: 0x04 */
  __IO uint32_t APBFZ1;      /*!< Debug APB freeze register 1,     Address offset: 0x08 */
  __IO uint32_t APBFZ2;      /*!< Debug APB freeze register 2,     Address offset: 0x0C */
} DBGMCU_TypeDef;

/**
* @brief DIV Registers
*/
typedef struct
{
  __IO uint32_t DEND;       /*!< DIV desc DEND, Address offset: 0x00 */
  __IO uint32_t SOR;        /*!< DIV desc SOR,  Address offset: 0x04 */
  __IO uint32_t QUOT;       /*!< DIV desc REMA, Address offset: 0x08 */
  __IO uint32_t REMA;       /*!< DIV desc QUOT, Address offset: 0x0C */
  __IO uint32_t SIGN;       /*!< DIV desc SIGN, Address offset: 0x10 */
  __IO uint32_t STAT;       /*!< DIV desc STAT, Address offset: 0x1C */
} DIV_TypeDef;

/**
  * @brief DMA Controller
  */
typedef struct
{
  __IO uint32_t ISR;         /*!< DMA interrupt status register,     Address offset: 0x00 */
  __IO uint32_t IFCR;        /*!< DMA interrupt flag clear register, Address offset: 0x04 */
} DMA_TypeDef;

typedef struct
{
  __IO uint32_t CCR;         /*!< DMA channel x configuration register        */
  __IO uint32_t CNDTR;       /*!< DMA channel x number of data register       */
  __IO uint32_t CPAR;        /*!< DMA channel x peripheral address register   */
  __IO uint32_t CMAR;        /*!< DMA channel x memory address register       */
} DMA_Channel_TypeDef;

/**
  * @brief Asynch Interrupt/Event Controller (EXTI)
  */
typedef struct
{
  __IO uint32_t RTSR;          /*!< EXTI Rising Trigger Selection Register 1,        Address offset:   0x00 */
  __IO uint32_t FTSR;          /*!< EXTI Falling Trigger Selection Register 1,       Address offset:   0x04 */
  __IO uint32_t SWIER;         /*!< EXTI Software Interrupt event Register 1,        Address offset:   0x08 */
  __IO uint32_t PR;            /*!< EXTI Pending Register 1                          Address offset:   0x0C */
  uint32_t RESERVED1[20];      /*!< Reserved 1,                                      Address offset:   0x10 -- 0x5C */
  __IO uint32_t EXTICR[4];     /*!< EXTI External Interrupt Configuration Register,  Address offset:   0x60 -- 0x6C */
  uint32_t RESERVED2[4];       /*!< Reserved 2,                                      Address offset:   0x70 -- 0x7C */
  __IO uint32_t IMR;           /*!< EXTI Interrupt Mask Register,                    Address offset:   0x80 */
  __IO uint32_t EMR;           /*!< EXTI Event Mask Register,                        Address offset:   0x84 */
} EXTI_TypeDef;

/**
  * @brief FLASH Registers
  */
typedef struct
{
  __IO uint32_t ACR;                               /*!< FLASH Access Control register,     Address offset: 0x00 */
  uint32_t RESERVED1;                              /*!< Reserved1,                         Address offset: 0x04 */
  __IO uint32_t KEYR;                              /*!< FLASH Key register,                Address offset: 0x08 */
  __IO uint32_t OPTKEYR;                           /*!< FLASH Option Key register,         Address offset: 0x0C */
  __IO uint32_t SR;                                /*!< FLASH Status register,             Address offset: 0x10 */
  __IO uint32_t CR;                                /*!< FLASH Control register,            Address offset: 0x14 */
  uint32_t RESERVED2[2];                           /*!< Reserved2,                         Address offset: 0x18-0x1C */
  __IO uint32_t OPTR;                              /*!< FLASH Option register,             Address offset: 0x20 */
  __IO uint32_t SDKR;                              /*!< FLASH SDK address register,        Address offset: 0x24 */
  __IO uint32_t RESERVED3;                         /*!< RESERVED3,                         Address offset: 0x28 */
  __IO uint32_t WRPR;                              /*!< FLASH WRP address register,        Address offset: 0x2C */
  uint32_t RESERVED4[(0x90 - 0x2C) / 4 - 1];       /*!< RESERVED4,                         Address offset: 0x30-0x8C */
  __IO uint32_t STCR;                              /*!< FLASH sleep time config register,  Address offset: 0x90 */
  uint32_t RESERVED5[(0x100 - 0x90) / 4 - 1];      /*!< RESERVED5,                         Address offset: 0x94-0xFC */
  __IO uint32_t TS0;                               /*!< FLASH TS0 register,                Address offset: 0x100 */
  __IO uint32_t TS1;                               /*!< FLASH TS1 register,                Address offset: 0x104 */
  __IO uint32_t TS2P;                              /*!< FLASH TS2P register,               Address offset: 0x108 */
  __IO uint32_t TPS3;                              /*!< FLASH TPS3 register,               Address offset: 0x10C */
  __IO uint32_t TS3;                               /*!< FLASH TS3 register,                Address offset: 0x110 */
  __IO uint32_t PERTPE;                            /*!< FLASH PERTPE register,             Address offset: 0x114 */
  __IO uint32_t SMERTPE;                           /*!< FLASH SMERTPE register,            Address offset: 0x118 */
  __IO uint32_t PRGTPE;                            /*!< FLASH PRGTPE register,             Address offset: 0x11C */
  __IO uint32_t PRETPE;                            /*!< FLASH PRETPE register,             Address offset: 0x120 */
} FLASH_TypeDef;

/**
  * @brief Option Bytes
  */
typedef struct
{
  __IO uint8_t RDP;          /*!< FLASH option byte Read protection,                  Address offset: 0x00 */
  __IO uint8_t USER;         /*!< FLASH option byte user options,                     Address offset: 0x01 */
  __IO uint8_t nRDP;         /*!< Complemented FLASH option byte Read protection,     Address offset: 0x02 */
  __IO uint8_t nUSER;        /*!< Complemented FLASH option byte user options,        Address offset: 0x03 */
  __IO uint8_t SDK_STRT;     /*!< SDK area start address(stored in SDK[4:0]),         Address offset: 0x04 */
  __IO uint8_t SDK_END;      /*!< SDK area end address(stored in SDK[12:8]),          Address offset: 0x05 */
  __IO uint8_t nSDK_STRT;    /*!< Complemented SDK area start address,                Address offset: 0x06 */
  __IO uint8_t nSDK_END;     /*!< Complemented SDK area end address,                  Address offset: 0x07 */
  uint32_t RESERVED1;        /*!< RESERVED1,                                          Address offset: 0x08 */
  __IO uint16_t WRP;         /*!< FLASH option byte write protection,                 Address offset: 0x0C */
  __IO uint16_t nWRP;        /*!< Complemented FLASH option byte write protection,    Address offset: 0x0E */
} OB_TypeDef;

/**
  * @brief General Purpose I/O
  */
typedef struct
{
  __IO uint32_t MODER;       /*!< GPIO port mode register,               Address offset: 0x00      */
  __IO uint32_t OTYPER;      /*!< GPIO port output type register,        Address offset: 0x04      */
  __IO uint32_t OSPEEDR;     /*!< GPIO port output speed register,       Address offset: 0x08      */
  __IO uint32_t PUPDR;       /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C      */
  __IO uint32_t IDR;         /*!< GPIO port input data register,         Address offset: 0x10      */
  __IO uint32_t ODR;         /*!< GPIO port output data register,        Address offset: 0x14      */
  __IO uint32_t BSRR;        /*!< GPIO port bit set/reset  register,     Address offset: 0x18      */
  __IO uint32_t LCKR;        /*!< GPIO port configuration lock register, Address offset: 0x1C      */
  __IO uint32_t AFR[2];      /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
  __IO uint32_t BRR;         /*!< GPIO Bit Reset register,               Address offset: 0x28      */
} GPIO_TypeDef;

/**
* @brief I2C Registers
*/
typedef struct
{
  __IO uint32_t CR1;         /*!< I2C desc CR1,   Address offset: 0x00 */
  __IO uint32_t CR2;         /*!< I2C desc CR2,   Address offset: 0x04 */
  __IO uint32_t OAR1;        /*!< I2C desc OAR1,  Address offset: 0x08 */
  __IO uint32_t OAR2;        /*!< I2C desc OAR2,  Address offset: 0x0C */
  __IO uint32_t DR;          /*!< I2C desc DR,    Address offset: 0x10 */
  __IO uint32_t SR1;         /*!< I2C desc SR1,   Address offset: 0x14 */
  __IO uint32_t SR2;         /*!< I2C desc SR2,   Address offset: 0x18 */
  __IO uint32_t CCR;         /*!< I2C desc CCR,   Address offset: 0x1C */
  __IO uint32_t TRISE;       /*!< I2C desc TRISE, Address offset: 0x20 */
} I2C_TypeDef;

/**
  * @brief Independent WATCHDOG
  */
typedef struct
{
  __IO uint32_t KR;          /*!< IWDG Key register,       Address offset: 0x00 */
  __IO uint32_t PR;          /*!< IWDG Prescaler register, Address offset: 0x04 */
  __IO uint32_t RLR;         /*!< IWDG Reload register,    Address offset: 0x08 */
  __IO uint32_t SR;          /*!< IWDG Status register,    Address offset: 0x0C */
} IWDG_TypeDef;

/**
  * @brief LCD
  */
typedef struct
{
  __IO uint32_t CR0;         /*!< LCD desc CR0,       Address offset: 0x00 */
  __IO uint32_t CR1;         /*!< LCD desc CR1,       Address offset: 0x04 */
  __IO uint32_t INTCLR;      /*!< LCD desc INTCLR,    Address offset: 0x08 */
  __IO uint32_t POEN0;       /*!< LCD desc POEN0,     Address offset: 0x0C */
  __IO uint32_t POEN1;       /*!< LCD desc POEN1,     Address offset: 0x10 */
  __IO uint32_t RAM[16];     /*!< LCD desc RAM0~F,    Address offset: 0x14-0x53 */
} LCD_TypeDef;

/**
  * @brief LPTIMER
  */
typedef struct
{
  __IO uint32_t ISR;         /*!< LPTIM Interrupt and Status register, Address offset: 0x00 */
  __IO uint32_t ICR;         /*!< LPTIM Interrupt Clear register,      Address offset: 0x04 */
  __IO uint32_t IER;         /*!< LPTIM Interrupt Enable register,     Address offset: 0x08 */
  __IO uint32_t CFGR;        /*!< LPTIM Configuration register,        Address offset: 0x0C */
  __IO uint32_t CR;          /*!< LPTIM Control register,              Address offset: 0x10 */
  __IO uint32_t RESERVED1;   /*!< RESERVED1,                           Address offset: 0x14 */
  __IO uint32_t ARR;         /*!< LPTIM Autoreload register,           Address offset: 0x18 */
  __IO uint32_t CNT;         /*!< LPTIM Counter register,              Address offset: 0x1C */
} LPTIM_TypeDef;

/**
  * @brief OPA Registers
  */
typedef struct
{
  __IO uint32_t RESERVED1[0x30 / 4];       /*!< RESERVED1,    Address offset: 0x0-0x2C */
  __IO uint32_t CR0;                       /*!< OPA desc CR0, Address offset: 0x30 */
  __IO uint32_t CR1;                       /*!< OPA desc CR1, Address offset: 0x34 */
  __IO uint32_t CR2;                       /*!< OPA desc CR2, Address offset: 0x38 */
  __IO uint32_t CR;                        /*!< OPA desc CR,  Address offset: 0x3C */
} OPA_TypeDef;

/**
  * @brief Power Control
  */
typedef struct
{
  __IO uint32_t CR1;          /*!< PWR Power Control Register 1, Address offset: 0x00 */
  __IO uint32_t CR2;          /*!< PWR Power Control Register 2, Address offset: 0x04 */
  uint32_t RESERVED1[3];      /*!< Reserved1,                    Address offset: 0x08-0x10 */
  __IO uint32_t SR;           /*!< PWR Power Status Register,    Address offset: 0x14 */
} PWR_TypeDef;

/**
  * @brief Reset and Clock Control
  */
typedef struct
{
  __IO uint32_t CR;          /*!< RCC Clock Sources Control Register,                        Address offset: 0x00 */
  __IO uint32_t ICSCR;       /*!< RCC Internal Clock Sources Calibration Register,           Address offset: 0x04 */
  __IO uint32_t CFGR;        /*!< RCC Regulated Domain Clocks Configuration Register,        Address offset: 0x08 */
  __IO uint32_t PLLCFGR;     /*!< RCC System PLL configuration Register,                     Address offset: 0x0C */
  __IO uint32_t ECSCR;       /*!< RCC External clock source control register,                Address offset: 0x10 */
  __IO uint32_t RESERVED1;   /*!< Reserved,                                                  Address offset: 0x14 */
  __IO uint32_t CIER;        /*!< RCC Clock Interrupt Enable Register,                       Address offset: 0x18 */
  __IO uint32_t CIFR;        /*!< RCC Clock Interrupt Flag Register,                         Address offset: 0x1C */
  __IO uint32_t CICR;        /*!< RCC Clock Interrupt Clear Register,                        Address offset: 0x20 */
  __IO uint32_t IOPRSTR;     /*!< RCC IO port reset register,                                Address offset: 0x24 */
  __IO uint32_t AHBRSTR;     /*!< RCC AHB peripherals reset register,                        Address offset: 0x28 */
  __IO uint32_t APBRSTR1;    /*!< RCC APB peripherals reset register 1,                      Address offset: 0x2C */
  __IO uint32_t APBRSTR2;    /*!< RCC APB peripherals reset register 2,                      Address offset: 0x30 */
  __IO uint32_t IOPENR;      /*!< RCC IO port enable register,                               Address offset: 0x34 */
  __IO uint32_t AHBENR;      /*!< RCC AHB peripherals clock enable register,                 Address offset: 0x38 */
  __IO uint32_t APBENR1;     /*!< RCC APB peripherals clock enable register1,                Address offset: 0x3C */
  __IO uint32_t APBENR2;     /*!< RCC APB peripherals clock enable register2,                Address offset: 0x40 */
  uint32_t RESERVED2[4];     /*!< Reserved,                                                  Address offset: 0x44-0x50 */
  __IO uint32_t CCIPR;       /*!< RCC Peripherals Independent Clocks Configuration Register, Address offset: 0x54 */
  __IO uint32_t RESERVED3;   /*!< Reserved,                                                  Address offset: 0x58 */
  __IO uint32_t BDCR;        /*!< RCC Backup Domain Control Register,                        Address offset: 0x5C */
  __IO uint32_t CSR;         /*!< RCC Unregulated Domain Clock Control and Status Register,  Address offset: 0x60 */
} RCC_TypeDef;

/**
* @brief RTC Registers
*/
typedef struct
{
  __IO uint32_t CRH;         /*!< RTC desc CRH,  Address offset: 0x00 */
  __IO uint32_t CRL;         /*!< RTC desc CRL,  Address offset: 0x04 */
  __IO uint32_t PRLH;        /*!< RTC desc PRLH, Address offset: 0x08 */
  __IO uint32_t PRLL;        /*!< RTC desc PRLL, Address offset: 0x0C */
  __IO uint32_t DIVH;        /*!< RTC desc DIVH, Address offset: 0x10 */
  __IO uint32_t DIVL;        /*!< RTC desc DIVL, Address offset: 0x14 */
  __IO uint32_t CNTH;        /*!< RTC desc CNTH, Address offset: 0x18 */
  __IO uint32_t CNTL;        /*!< RTC desc CNTL, Address offset: 0x1C */
  __IO uint32_t ALRH;        /*!< RTC desc ALRH, Address offset: 0x20 */
  __IO uint32_t ALRL;        /*!< RTC desc ALRL, Address offset: 0x24 */
  uint32_t RESERVED1;
  __IO uint32_t BKP_RTCCR;   /*!< RTC desc BKP_RTCCR, Address offset: 0x2C */
} RTC_TypeDef;

/**
* @brief SPI Registers
*/
typedef struct
{
  __IO uint32_t CR1;         /*!< SPI desc CR1,     Address offset: 0x00 */
  __IO uint32_t CR2;         /*!< SPI desc CR2,     Address offset: 0x04 */
  __IO uint32_t SR;          /*!< SPI desc SR,      Address offset: 0x08 */
  __IO uint32_t DR;          /*!< SPI desc DR,      Address offset: 0x0C */
  __IO uint32_t CRCPR;       /*!< SPI desc CRCPR,   Address offset: 0x10 */
  __IO uint32_t RXCRCR;      /*!< SPI desc RXCRCR,  Address offset: 0x14 */
  __IO uint32_t TXCRCR;      /*!< SPI desc TXCRCR,  Address offset: 0x18 */
  __IO uint32_t I2SCFGR;     /*!< SPI desc I2SCFGR, Address offset: 0x1C */
  __IO uint32_t I2SPR;       /*!< SPI desc I2SPR,   Address offset: 0x20 */
} SPI_TypeDef;

/**
  * @brief System configuration controller
  */
typedef struct
{
  __IO uint32_t CFGR1;       /*!< SYSCFG desc CFGR1,   Address offset: 0x00 */
  __IO uint32_t CFGR2;       /*!< SYSCFG desc CFGR2,   Address offset: 0x04 */
  __IO uint32_t CFGR3;       /*!< SYSCFG desc CFGR3,   Address offset: 0x08 */
  __IO uint32_t CFGR4;       /*!< SYSCFG desc CFGR4,   Address offset: 0x0C */
  __IO uint32_t PAENS;       /*!< SYSCFG desc PAENS,   Address offset: 0x10 */
  __IO uint32_t PBENS;       /*!< SYSCFG desc PBENS,   Address offset: 0x14 */
  __IO uint32_t PCENS;       /*!< SYSCFG desc PCENS,   Address offset: 0x18 */
  __IO uint32_t PFENS;       /*!< SYSCFG desc PFENS,   Address offset: 0x1C */
  __IO uint32_t EIIC;        /*!< SYSCFG desc PEENS,   Address offset: 0x20 */
} SYSCFG_TypeDef;

/**
  * @brief TIM
  */
typedef struct
{
  __IO uint32_t CR1;         /*!< TIM control register 1,                Address offset: 0x00 */
  __IO uint32_t CR2;         /*!< TIM control register 2,                Address offset: 0x04 */
  __IO uint32_t SMCR;        /*!< TIM slave mode control register,       Address offset: 0x08 */
  __IO uint32_t DIER;        /*!< TIM DMA/interrupt enable register,     Address offset: 0x0C */
  __IO uint32_t SR;          /*!< TIM status register,                   Address offset: 0x10 */
  __IO uint32_t EGR;         /*!< TIM event generation register,         Address offset: 0x14 */
  __IO uint32_t CCMR1;       /*!< TIM capture/compare mode register 1,   Address offset: 0x18 */
  __IO uint32_t CCMR2;       /*!< TIM capture/compare mode register 2,   Address offset: 0x1C */
  __IO uint32_t CCER;        /*!< TIM capture/compare enable register,   Address offset: 0x20 */
  __IO uint32_t CNT;         /*!< TIM counter register,                  Address offset: 0x24 */
  __IO uint32_t PSC;         /*!< TIM prescaler register,                Address offset: 0x28 */
  __IO uint32_t ARR;         /*!< TIM auto-reload register,              Address offset: 0x2C */
  __IO uint32_t RCR;         /*!< TIM repetition counter register,       Address offset: 0x30 */
  __IO uint32_t CCR1;        /*!< TIM capture/compare register 1,        Address offset: 0x34 */
  __IO uint32_t CCR2;        /*!< TIM capture/compare register 2,        Address offset: 0x38 */
  __IO uint32_t CCR3;        /*!< TIM capture/compare register 3,        Address offset: 0x3C */
  __IO uint32_t CCR4;        /*!< TIM capture/compare register 4,        Address offset: 0x40 */
  __IO uint32_t BDTR;        /*!< TIM break and dead-time register,      Address offset: 0x44 */
  __IO uint32_t DCR;         /*!< TIM DMA control register,              Address offset: 0x48 */
  __IO uint32_t DMAR;        /*!< TIM DMA address for full transfer,     Address offset: 0x4C */
  __IO uint32_t OR;          /*!< TIM option register,                   Address offset: 0x50 */
} TIM_TypeDef;

/**
  * @brief Universal Synchronous Asynchronous Receiver Transmitter
  */
typedef struct
{
  __IO uint32_t SR;          /*!< USART Status  register ,                 Address offset: 0x00  */
  __IO uint32_t DR;          /*!< USART Data register,                     Address offset: 0x04  */
  __IO uint32_t BRR;         /*!< USART Baud rate register,                Address offset: 0x08  */
  __IO uint32_t CR1;         /*!< USART Control  register 1,               Address offset: 0x0C  */
  __IO uint32_t CR2;         /*!< USART Control  register 2,               Address offset: 0x10  */
  __IO uint32_t CR3;         /*!< USART Control  register 3,               Address offset: 0x14  */
  __IO uint32_t GTPR;        /*!< USART Guard time and prescaler register, Address offset: 0x18  */
} USART_TypeDef;

/**
* @brief USBD Registers
*/
typedef struct
{
  __IO uint32_t CR;           /*!< USBD desc CR,         Address offset: 0x00 */
  __IO uint32_t INTR;         /*!< USBD desc INTR,       Address offset: 0x04 */
  __IO uint32_t INTRE;        /*!< USBD desc INTRE,      Address offset: 0x08 */
  __IO uint32_t FRAME;        /*!< USBD desc FRAME,      Address offset: 0x0C */
  __IO uint32_t EP0CSR;       /*!< USBD desc EP0CSR,     Address offset: 0x10 */
  __IO uint32_t INEPxCSR;     /*!< USBD desc INEPxCSR,   Address offset: 0x14 */
  __IO uint32_t OUTEPxCSR;    /*!< USBD desc OUTEPxCSR,  Address offset: 0x18 */
  __IO uint32_t OUTCOUNT;     /*!< USBD desc OUTCOUNT,   Address offset: 0x1C */
  __IO uint32_t FIFODATA[16]; /*!< USBD desc FIFODATA,   Address offset: 0x20 - 0x3F */
} USBD_TypeDef;

/**
  * @brief Window WATCHDOG
  */
typedef struct
{
  __IO uint32_t CR;          /*!< WWDG Control register,       Address offset: 0x00 */
  __IO uint32_t CFR;         /*!< WWDG Configuration register, Address offset: 0x04 */
  __IO uint32_t SR;          /*!< WWDG Status register,        Address offset: 0x08 */
} WWDG_TypeDef;

/** @addtogroup Peripheral_memory_map
  * @{
  */
#define FLASH_BASE            (0x08000000UL)                      /*!< FLASH base address */
#define FLASH_END             (0x0801FFFFUL)                      /*!< FLASH end address */
#define FLASH_SIZE            (FLASH_END - FLASH_BASE + 1)
#define FLASH_PAGE_SIZE       0x00000100U                         /*!< FLASH Page Size, 256 Bytes */
#define FLASH_PAGE_NB         (FLASH_SIZE / FLASH_PAGE_SIZE)
#define FLASH_SECTOR_SIZE     0x00002000U                         /*!< FLASH Sector Size, 8192 Bytes */
#define FLASH_SECTOR_NB       (FLASH_SIZE / FLASH_SECTOR_SIZE)
#define SRAM_BASE             (0x20000000UL)                      /*!< SRAM base address */
#define SRAM_END              (0x20003FFFUL)                      /*!< SRAM end address */
#define PERIPH_BASE           (0x40000000UL)                      /*!< Peripheral base address */
#define IOPORT_BASE           (0x50000000UL)                      /*!< IOPORT base address */

/*!< Peripheral memory map */
#define APBPERIPH_BASE        (PERIPH_BASE)
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x00020000UL)

/*!< APB peripherals */
#define TIM2_BASE             (APBPERIPH_BASE + 0x00000000UL)
#define TIM3_BASE             (APBPERIPH_BASE + 0x00000400UL)
#define TIM6_BASE             (APBPERIPH_BASE + 0x00001000UL)
#define TIM7_BASE             (APBPERIPH_BASE + 0x00001400UL)
#define TIM14_BASE            (APBPERIPH_BASE + 0x00002000UL)
#define LCD_BASE              (APBPERIPH_BASE + 0x00002400UL)
#define RTC_BASE              (APBPERIPH_BASE + 0x00002800UL)
#define WWDG_BASE             (APBPERIPH_BASE + 0x00002C00UL)
#define IWDG_BASE             (APBPERIPH_BASE + 0x00003000UL)
#define SPI2_BASE             (APBPERIPH_BASE + 0x00003800UL)
#define USART2_BASE           (APBPERIPH_BASE + 0x00004400UL)
#define USART3_BASE           (APBPERIPH_BASE + 0x00004800UL)
#define USART4_BASE           (APBPERIPH_BASE + 0x00004C00UL)
#define I2C1_BASE             (APBPERIPH_BASE + 0x00005400UL)
#define I2C2_BASE             (APBPERIPH_BASE + 0x00005800UL)
#define USBD_BASE             (APBPERIPH_BASE + 0x00005C00UL)
#define USBD_SRAM_BASE        (APBPERIPH_BASE + 0x00006000UL)
#define CAN1_BASE             (APBPERIPH_BASE + 0x00006400UL)
#define CTC_BASE              (APBPERIPH_BASE + 0x00006C00UL)
#define PWR_BASE              (APBPERIPH_BASE + 0x00007000UL)
#define DAC1_BASE             (APBPERIPH_BASE + 0x00007400UL)
#define LPTIM1_BASE           (APBPERIPH_BASE + 0x00007C00UL)
#define SYSCFG_BASE           (APBPERIPH_BASE + 0x00010000UL)
#define COMP1_BASE            (APBPERIPH_BASE + 0x00010200UL)
#define COMP2_BASE            (APBPERIPH_BASE + 0x00010210UL)
#define COMP3_BASE            (APBPERIPH_BASE + 0x00010220UL)
#define ADC1_BASE             (APBPERIPH_BASE + 0x00012400UL)
#define OPA_BASE              (APBPERIPH_BASE + 0x00010300UL)
#define TIM1_BASE             (APBPERIPH_BASE + 0x00012C00UL)
#define SPI1_BASE             (APBPERIPH_BASE + 0x00013000UL)
#define USART1_BASE           (APBPERIPH_BASE + 0x00013800UL)
#define TIM15_BASE            (APBPERIPH_BASE + 0x00014000UL)
#define TIM16_BASE            (APBPERIPH_BASE + 0x00014400UL)
#define TIM17_BASE            (APBPERIPH_BASE + 0x00014800UL)
#define DBGMCU_BASE           (APBPERIPH_BASE + 0x00015800UL)

/*!< AHB peripherals */
#define DMA1_BASE             (AHBPERIPH_BASE + 0x00000000UL)
#define DMA1_Channel1_BASE    (DMA1_BASE + 0x00000008UL)
#define DMA1_Channel2_BASE    (DMA1_BASE + 0x0000001CUL)
#define DMA1_Channel3_BASE    (DMA1_BASE + 0x00000030UL)
#define DMA1_Channel4_BASE    (DMA1_BASE + 0x00000044UL)
#define DMA1_Channel5_BASE    (DMA1_BASE + 0x00000058UL)
#define DMA1_Channel6_BASE    (DMA1_BASE + 0x0000006CUL)
#define DMA1_Channel7_BASE    (DMA1_BASE + 0x00000080UL)
#define RCC_BASE              (AHBPERIPH_BASE + 0x00001000UL)
#define EXTI_BASE             (AHBPERIPH_BASE + 0x00001800UL)
#define FLASH_R_BASE          (AHBPERIPH_BASE + 0x00002000UL)   /*!< FLASH registers base address */
#define OB_BASE               0x1FFF3100UL                      /*!< FLASH Option Bytes base address */
#define FLASHSIZE_BASE        0x1FFF31FCUL                      /*!< FLASH Size register base address */
#define UID_BASE              0x1FFF3000UL                      /*!< Unique device ID register base address */
#define CRC_BASE              (AHBPERIPH_BASE + 0x00003000UL)
#define DIV_BASE              (AHBPERIPH_BASE + 0x00003800UL)

/*!< IOPORT */
#define GPIOA_BASE            (IOPORT_BASE + 0x00000000UL)
#define GPIOB_BASE            (IOPORT_BASE + 0x00000400UL)
#define GPIOC_BASE            (IOPORT_BASE + 0x00000800UL)
#define GPIOF_BASE            (IOPORT_BASE + 0x00001400UL)

/**
  * @}
  */

/** @addtogroup Peripheral_declaration
  * @{
  */
#define TIM2                ((TIM_TypeDef *) TIM2_BASE)
#define TIM3                ((TIM_TypeDef *) TIM3_BASE)
#define TIM6                ((TIM_TypeDef *) TIM6_BASE)
#define TIM7                ((TIM_TypeDef *) TIM7_BASE)
#define TIM14               ((TIM_TypeDef *) TIM14_BASE)
#define LCD                 ((LCD_TypeDef *) LCD_BASE)
#define RTC                 ((RTC_TypeDef *) RTC_BASE)
#define WWDG                ((WWDG_TypeDef *) WWDG_BASE)
#define IWDG                ((IWDG_TypeDef *) IWDG_BASE)
#define SPI2                ((SPI_TypeDef *) SPI2_BASE)
#define USART2              ((USART_TypeDef *) USART2_BASE)
#define USART3              ((USART_TypeDef *) USART3_BASE)
#define USART4              ((USART_TypeDef *) USART4_BASE)
#define I2C1                ((I2C_TypeDef *) I2C1_BASE)
#define I2C2                ((I2C_TypeDef *) I2C2_BASE)
#define USBD                ((USBD_TypeDef *) USBD_BASE)
#define USBD_SRAM           ((USBD_SRAM_TypeDef *) USBD_SRAM_BASE)
#define CAN1                ((CAN_TypeDef *) CAN1_BASE)
#define CTC                 ((CTC_TypeDef *) CTC_BASE)
#define PWR                 ((PWR_TypeDef *) PWR_BASE)
#define DAC1                ((DAC_TypeDef *) DAC1_BASE)
#define LPTIM               ((LPTIM_TypeDef *) LPTIM1_BASE)
#define LPTIM1              ((LPTIM_TypeDef *) LPTIM1_BASE)
#define SYSCFG              ((SYSCFG_TypeDef *) SYSCFG_BASE)
#define COMP1               ((COMP_TypeDef *) COMP1_BASE)
#define COMP2               ((COMP_TypeDef *) COMP2_BASE)
#define COMP3               ((COMP_TypeDef *) COMP3_BASE)
#define COMP12_COMMON       ((COMP_Common_TypeDef *) COMP1_BASE)
#define OPA                 ((OPA_TypeDef *) OPA_BASE)
#define ADC1                ((ADC_TypeDef *) ADC1_BASE)
#define TIM1                ((TIM_TypeDef *) TIM1_BASE)
#define SPI1                ((SPI_TypeDef *) SPI1_BASE)
#define USART1              ((USART_TypeDef *) USART1_BASE)
#define TIM15               ((TIM_TypeDef *) TIM15_BASE)
#define TIM16               ((TIM_TypeDef *) TIM16_BASE)
#define TIM17               ((TIM_TypeDef *) TIM17_BASE)
#define DBGMCU              ((DBGMCU_TypeDef *) DBGMCU_BASE)
#define DMA1                ((DMA_TypeDef *) DMA1_BASE)
#define DMA1_Channel1       ((DMA_Channel_TypeDef *) DMA1_Channel1_BASE)
#define DMA1_Channel2       ((DMA_Channel_TypeDef *) DMA1_Channel2_BASE)
#define DMA1_Channel3       ((DMA_Channel_TypeDef *) DMA1_Channel3_BASE)
#define DMA1_Channel4       ((DMA_Channel_TypeDef *) DMA1_Channel4_BASE)
#define DMA1_Channel5       ((DMA_Channel_TypeDef *) DMA1_Channel5_BASE)
#define DMA1_Channel6       ((DMA_Channel_TypeDef *) DMA1_Channel6_BASE)
#define DMA1_Channel7       ((DMA_Channel_TypeDef *) DMA1_Channel7_BASE)
#define RCC                 ((RCC_TypeDef *) RCC_BASE)
#define EXTI                ((EXTI_TypeDef *) EXTI_BASE)
#define FLASH               ((FLASH_TypeDef *) FLASH_R_BASE)
#define OB                  ((OB_TypeDef *) OB_BASE)
#define CRC                 ((CRC_TypeDef *) CRC_BASE)
#define DIV                 ((DIV_TypeDef *) DIV_BASE)
#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOF               ((GPIO_TypeDef *) GPIOF_BASE)

/**
  * @}
  */

/** @addtogroup Exported_constants
  * @{
  */

/** @addtogroup Peripheral_Registers_Bits_Definition
* @{
*/

/******************************************************************************/
/*                         Peripheral Registers Bits Definition               */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                      Analog to Digital Converter (ADC)                     */
/*                                                                            */
/******************************************************************************/
/********************  Bits definition for ADC_SR register  *******************/
#define ADC_SR_AWD_Pos           (0U)
#define ADC_SR_AWD_Msk           (0x1UL << ADC_SR_AWD_Pos)                     /*!< 0x00000080 */
#define ADC_SR_AWD                ADC_SR_AWD_Msk
#define ADC_SR_EOC_Pos           (1U)
#define ADC_SR_EOC_Msk           (0x1UL << ADC_SR_EOC_Pos)                     /*!< 0x00000004 */
#define ADC_SR_EOC                ADC_SR_EOC_Msk
#define ADC_SR_JEOC_Pos          (2U)
#define ADC_SR_JEOC_Msk          (0x1UL << ADC_SR_JEOC_Pos)                    /*!< 0x00000004 */
#define ADC_SR_JEOC               ADC_SR_JEOC_Msk
#define ADC_SR_JSTRT_Pos         (3U)
#define ADC_SR_JSTRT_Msk         (0x1UL << ADC_SR_JSTRT_Pos)                   /*!< 0x00000004 */
#define ADC_SR_JSTRT              ADC_SR_JSTRT_Msk
#define ADC_SR_STRT_Pos          (4U)
#define ADC_SR_STRT_Msk          (0x1UL << ADC_SR_STRT_Pos)                    /*!< 0x00000004 */
#define ADC_SR_STRT               ADC_SR_STRT_Msk
#define ADC_SR_OVER_Pos          (5U)
#define ADC_SR_OVER_Msk          (0x1UL << ADC_SR_OVER_Pos)                    /*!< 0x00000004 */
#define ADC_SR_OVER               ADC_SR_OVER_Msk

/*!< ADC_CR1 */
#define ADC_CR1_AWDCH_Pos                         (0U)
#define ADC_CR1_AWDCH_Msk                         (0x1FUL << ADC_CR1_AWDCH_Pos)                     /*!< 0x0000001F */
#define ADC_CR1_AWDCH                             ADC_CR1_AWDCH_Msk                                 /*!< AWDCH[4:0] bits (desc AWDCH) */
#define ADC_CR1_AWDCH_0                           (0x1UL << ADC_CR1_AWDCH_Pos)                      /*!< 0x00000001 */
#define ADC_CR1_AWDCH_1                           (0x2UL << ADC_CR1_AWDCH_Pos)                      /*!< 0x00000002 */
#define ADC_CR1_AWDCH_2                           (0x4UL << ADC_CR1_AWDCH_Pos)                      /*!< 0x00000004 */
#define ADC_CR1_AWDCH_3                           (0x8UL << ADC_CR1_AWDCH_Pos)                      /*!< 0x00000008 */
#define ADC_CR1_AWDCH_4                           (0x10UL << ADC_CR1_AWDCH_Pos)                     /*!< 0x00000010 */
#define ADC_CR1_EOCIE_Pos                         (5U)
#define ADC_CR1_EOCIE_Msk                         (0x1UL << ADC_CR1_EOCIE_Pos)                      /*!< 0x00000020 */
#define ADC_CR1_EOCIE                             ADC_CR1_EOCIE_Msk
#define ADC_CR1_AWDIE_Pos                         (6U)
#define ADC_CR1_AWDIE_Msk                         (0x1UL << ADC_CR1_AWDIE_Pos)                      /*!< 0x00000040 */
#define ADC_CR1_AWDIE                             ADC_CR1_AWDIE_Msk                                 /*!< desc AWDIE */
#define ADC_CR1_JEOCIE_Pos                        (7U)
#define ADC_CR1_JEOCIE_Msk                        (0x1UL << ADC_CR1_JEOCIE_Pos)                     /*!< 0x00000080 */
#define ADC_CR1_JEOCIE                            ADC_CR1_JEOCIE_Msk                                /*!< desc JEOCIE */
#define ADC_CR1_SCAN_Pos                          (8U)
#define ADC_CR1_SCAN_Msk                          (0x1UL << ADC_CR1_SCAN_Pos)                       /*!< 0x00000100 */
#define ADC_CR1_SCAN                              ADC_CR1_SCAN_Msk                                  /*!< desc SCAN */
#define ADC_CR1_AWDSGL_Pos                        (9U)
#define ADC_CR1_AWDSGL_Msk                        (0x1UL << ADC_CR1_AWDSGL_Pos)                     /*!< 0x00000200 */
#define ADC_CR1_AWDSGL                            ADC_CR1_AWDSGL_Msk                                /*!< desc AWDSGL */
#define ADC_CR1_JAUTO_Pos                         (10U)
#define ADC_CR1_JAUTO_Msk                         (0x1UL << ADC_CR1_JAUTO_Pos)                      /*!< 0x00000400 */
#define ADC_CR1_JAUTO                             ADC_CR1_JAUTO_Msk                                 /*!< desc JAUTO */
#define ADC_CR1_DISCEN_Pos                        (11U)
#define ADC_CR1_DISCEN_Msk                        (0x1UL << ADC_CR1_DISCEN_Pos)                     /*!< 0x00000800 */
#define ADC_CR1_DISCEN                            ADC_CR1_DISCEN_Msk                                /*!< desc DISCEN */
#define ADC_CR1_JDISCEN_Pos                       (12U)
#define ADC_CR1_JDISCEN_Msk                       (0x1UL << ADC_CR1_JDISCEN_Pos)                    /*!< 0x00001000 */
#define ADC_CR1_JDISCEN                           ADC_CR1_JDISCEN_Msk                               /*!< desc JDISCEN */
#define ADC_CR1_DISCNUM_Pos                       (13U)
#define ADC_CR1_DISCNUM_Msk                       (0x7UL << ADC_CR1_DISCNUM_Pos)                    /*!< 0x0000E000 */
#define ADC_CR1_DISCNUM                           ADC_CR1_DISCNUM_Msk                               /*!< DISCNUM[15:13] bits (desc DISCNUM) */
#define ADC_CR1_DISCNUM_0                         (0x1UL << ADC_CR1_DISCNUM_Pos)                    /*!< 0x00002000 */
#define ADC_CR1_DISCNUM_1                         (0x2UL << ADC_CR1_DISCNUM_Pos)                    /*!< 0x00004000 */
#define ADC_CR1_DISCNUM_2                         (0x4UL << ADC_CR1_DISCNUM_Pos)                    /*!< 0x00008000 */
#define ADC_CR1_JAWDEN_Pos                        (22U)
#define ADC_CR1_JAWDEN_Msk                        (0x1UL << ADC_CR1_JAWDEN_Pos)                     /*!< 0x00400000 */
#define ADC_CR1_JAWDEN                            ADC_CR1_JAWDEN_Msk
#define ADC_CR1_AWDEN_Pos                         (23U)
#define ADC_CR1_AWDEN_Msk                         (0x1UL << ADC_CR1_AWDEN_Pos)                      /*!< 0x00800000 */
#define ADC_CR1_AWDEN                             ADC_CR1_AWDEN_Msk
#define ADC_CR1_RESSEL_Pos                        (24U)
#define ADC_CR1_RESSEL_Msk                        (0x3UL << ADC_CR1_RESSEL_Pos)                     /*!< 0x03000000 */
#define ADC_CR1_RESSEL                            ADC_CR1_RESSEL_Msk
#define ADC_CR1_RESSEL_0                          (0x1UL << ADC_CR1_RESSEL_Pos)                     /*!< 0x01000000 */
#define ADC_CR1_RESSEL_1                          (0x2UL << ADC_CR1_RESSEL_Pos)
#define ADC_CR1_ADSTP_Pos                         (27U)
#define ADC_CR1_ADSTP_Msk                         (0x1UL << ADC_CR1_ADSTP_Pos)                      /*!< 0x08000000 */
#define ADC_CR1_ADSTP                             ADC_CR1_ADSTP_Msk
#define ADC_CR1_OVRIE_Pos                         (29U)
#define ADC_CR1_OVRIE_Msk                         (0x1UL << ADC_CR1_OVRIE_Pos)                      /*!< 0x20000000 */
#define ADC_CR1_OVRIE                             ADC_CR1_OVRIE_Msk

/*!< ADC_CR2 */
#define ADC_CR2_ADON_Pos                          (0U)
#define ADC_CR2_ADON_Msk                          (0x1UL << ADC_CR2_ADON_Pos)                       /*!< 0x00000001 */
#define ADC_CR2_ADON                              ADC_CR2_ADON_Msk                                  /*!< desc ADON */
#define ADC_CR2_CONT_Pos                          (1U)
#define ADC_CR2_CONT_Msk                          (0x1UL << ADC_CR2_CONT_Pos)                       /*!< 0x00000002 */
#define ADC_CR2_CONT                              ADC_CR2_CONT_Msk                                  /*!< desc CONT */
#define ADC_CR2_CAL_Pos                           (2U)
#define ADC_CR2_CAL_Msk                           (0x1UL << ADC_CR2_CAL_Pos)                        /*!< 0x00000004 */
#define ADC_CR2_CAL                               ADC_CR2_CAL_Msk                                   /*!< desc CAL */
#define ADC_CR2_RSTCAL_Pos                        (3U)
#define ADC_CR2_RSTCAL_Msk                        (0x1UL << ADC_CR2_RSTCAL_Pos)                     /*!< 0x00000008 */
#define ADC_CR2_RSTCAL                            ADC_CR2_RSTCAL_Msk
#define ADC_CR2_DMA_Pos                           (8U)
#define ADC_CR2_DMA_Msk                           (0x1UL << ADC_CR2_DMA_Pos)                        /*!< 0x00000100 */
#define ADC_CR2_DMA                               ADC_CR2_DMA_Msk
#define ADC_CR2_ALIGN_Pos                         (11U)
#define ADC_CR2_ALIGN_Msk                         (0x1UL << ADC_CR2_ALIGN_Pos)                      /*!< 0x00000800 */
#define ADC_CR2_ALIGN                             ADC_CR2_ALIGN_Msk                                 /*!< desc ALIGN */
#define ADC_CR2_JEXTSEL_Pos                       (12U)
#define ADC_CR2_JEXTSEL_Msk                       (0x7UL << ADC_CR2_JEXTSEL_Pos)                    /*!< 0x00007000 */
#define ADC_CR2_JEXTSEL                           ADC_CR2_JEXTSEL_Msk                               /*!< JEXTSEL[14:12] bits (desc JEXTSEL) */
#define ADC_CR2_JEXTSEL_0                         (0x1UL << ADC_CR2_JEXTSEL_Pos)                    /*!< 0x00001000 */
#define ADC_CR2_JEXTSEL_1                         (0x2UL << ADC_CR2_JEXTSEL_Pos)                    /*!< 0x00002000 */
#define ADC_CR2_JEXTSEL_2                         (0x4UL << ADC_CR2_JEXTSEL_Pos)                    /*!< 0x00004000 */
#define ADC_CR2_JEXTTRIG_Pos                      (15U)
#define ADC_CR2_JEXTTRIG_Msk                      (0x1UL << ADC_CR2_JEXTTRIG_Pos)                   /*!< 0x00008000 */
#define ADC_CR2_JEXTTRIG                          ADC_CR2_JEXTTRIG_Msk                              /*!< desc JEXTTRIG */
#define ADC_CR2_EXTSEL_Pos                        (17U)
#define ADC_CR2_EXTSEL_Msk                        (0x7UL << ADC_CR2_EXTSEL_Pos)                     /*!< 0x000E0000 */
#define ADC_CR2_EXTSEL                            ADC_CR2_EXTSEL_Msk                                /*!< EXTSEL[19:17] bits (desc EXTSEL) */
#define ADC_CR2_EXTSEL_0                          (0x1UL << ADC_CR2_EXTSEL_Pos)                     /*!< 0x00020000 */
#define ADC_CR2_EXTSEL_1                          (0x2UL << ADC_CR2_EXTSEL_Pos)                     /*!< 0x00040000 */
#define ADC_CR2_EXTSEL_2                          (0x4UL << ADC_CR2_EXTSEL_Pos)                     /*!< 0x00080000 */
#define ADC_CR2_EXTTRIG_Pos                       (20U)
#define ADC_CR2_EXTTRIG_Msk                       (0x1UL << ADC_CR2_EXTTRIG_Pos)                    /*!< 0x00100000 */
#define ADC_CR2_EXTTRIG                           ADC_CR2_EXTTRIG_Msk                               /*!< desc EXTTRIG */
#define ADC_CR2_JSWSTART_Pos                      (21U)
#define ADC_CR2_JSWSTART_Msk                      (0x1UL << ADC_CR2_JSWSTART_Pos)                   /*!< 0x00200000 */
#define ADC_CR2_JSWSTART                          ADC_CR2_JSWSTART_Msk                              /*!< desc JSWSTART */
#define ADC_CR2_SWSTART_Pos                       (22U)
#define ADC_CR2_SWSTART_Msk                       (0x1UL << ADC_CR2_SWSTART_Pos)                    /*!< 0x00400000 */
#define ADC_CR2_SWSTART                           ADC_CR2_SWSTART_Msk                               /*!< desc SWSTART */
#define ADC_CR2_TSVREFE_Pos                       (23U)
#define ADC_CR2_TSVREFE_Msk                       (0x1UL << ADC_CR2_TSVREFE_Pos)                    /*!< 0x00800000 */
#define ADC_CR2_TSVREFE                           ADC_CR2_TSVREFE_Msk
#define ADC_CR2_VREFBUFFERE_Pos                  (25U)
#define ADC_CR2_VREFBUFFERE_Msk                  (0x1UL << ADC_CR2_VREFBUFFERE_Pos)               /*!< 0x00800000 */
#define ADC_CR2_VREFBUFFERE                      ADC_CR2_VREFBUFFERE_Msk
#define ADC_CR2_VREFBUFFERSEL_Pos                 (26U)
#define ADC_CR2_VREFBUFFERSEL_Msk                 (0x3UL << ADC_CR2_VREFBUFFERSEL_Pos)              /*!< 0x0C000000 */
#define ADC_CR2_VREFBUFFERSEL                     ADC_CR2_VREFBUFFERSEL_Msk
#define ADC_CR2_VREFBUFFERSEL_0                   (0x1UL << ADC_CR2_VREFBUFFERSEL_Pos)              /*!< 0x04000000 */
#define ADC_CR2_VREFBUFFERSEL_1                   (0x2UL << ADC_CR2_VREFBUFFERSEL_Pos)              /*!< 0x08000000 */

/*!< ADC_SMPR1 */
#define ADC_SMPR1_SMP20_Pos                       (0U)
#define ADC_SMPR1_SMP20_Msk                       (0x7UL << ADC_SMPR1_SMP20_Pos)                    /*!< 0x00000007 */
#define ADC_SMPR1_SMP20                           ADC_SMPR1_SMP20_Msk                               /*!< SMP10[2:0] bits (desc SMP10) */
#define ADC_SMPR1_SMP20_0                         (0x1UL << ADC_SMPR1_SMP20_Pos)                    /*!< 0x00000001 */
#define ADC_SMPR1_SMP20_1                         (0x2UL << ADC_SMPR1_SMP20_Pos)                    /*!< 0x00000002 */
#define ADC_SMPR1_SMP20_2                         (0x4UL << ADC_SMPR1_SMP20_Pos)                    /*!< 0x00000004 */
#define ADC_SMPR1_SMP21_Pos                       (3U)
#define ADC_SMPR1_SMP21_Msk                       (0x7UL << ADC_SMPR1_SMP21_Pos)                    /*!< 0x00000038 */
#define ADC_SMPR1_SMP21                           ADC_SMPR1_SMP21_Msk                               /*!< SMP11[5:3] bits (desc SMP11) */
#define ADC_SMPR1_SMP21_0                         (0x1UL << ADC_SMPR1_SMP21_Pos)                    /*!< 0x00000008 */
#define ADC_SMPR1_SMP21_1                         (0x2UL << ADC_SMPR1_SMP21_Pos)                    /*!< 0x00000010 */
#define ADC_SMPR1_SMP21_2                         (0x4UL << ADC_SMPR1_SMP21_Pos)                    /*!< 0x00000020 */
#define ADC_SMPR1_SMP22_Pos                       (6U)
#define ADC_SMPR1_SMP22_Msk                       (0x7UL << ADC_SMPR1_SMP22_Pos)                    /*!< 0x000001C0 */
#define ADC_SMPR1_SMP22                           ADC_SMPR1_SMP22_Msk                               /*!< SMP12[8:6] bits (desc SMP12) */
#define ADC_SMPR1_SMP22_0                         (0x1UL << ADC_SMPR1_SMP22_Pos)                    /*!< 0x00000040 */
#define ADC_SMPR1_SMP22_1                         (0x2UL << ADC_SMPR1_SMP22_Pos)                    /*!< 0x00000080 */
#define ADC_SMPR1_SMP22_2                         (0x4UL << ADC_SMPR1_SMP22_Pos)                    /*!< 0x00000100 */
#define ADC_SMPR1_SMP23_Pos                       (9U)
#define ADC_SMPR1_SMP23_Msk                       (0x7UL << ADC_SMPR1_SMP23_Pos)                    /*!< 0x00000E00 */
#define ADC_SMPR1_SMP23                           ADC_SMPR1_SMP23_Msk                               /*!< SMP13[11:9] bits (desc SMP13) */
#define ADC_SMPR1_SMP23_0                         (0x1UL << ADC_SMPR1_SMP23_Pos)                    /*!< 0x00000200 */
#define ADC_SMPR1_SMP23_1                         (0x2UL << ADC_SMPR1_SMP23_Pos)                    /*!< 0x00000400 */
#define ADC_SMPR1_SMP23_2                         (0x4UL << ADC_SMPR1_SMP23_Pos)                    /*!< 0x00000800 */

/*!< ADC_SMPR2 */
#define ADC_SMPR2_SMP10_Pos                        (0U)
#define ADC_SMPR2_SMP10_Msk                        (0x7UL << ADC_SMPR2_SMP10_Pos)                   /*!< 0x00000007 */
#define ADC_SMPR2_SMP10                            ADC_SMPR2_SMP10_Msk                              /*!< SMP0[2:0] bits (desc SMP0) */
#define ADC_SMPR2_SMP10_0                          (0x1UL << ADC_SMPR2_SMP10_Pos)                   /*!< 0x00000001 */
#define ADC_SMPR2_SMP10_1                          (0x2UL << ADC_SMPR2_SMP10_Pos)                   /*!< 0x00000002 */
#define ADC_SMPR2_SMP10_2                          (0x4UL << ADC_SMPR2_SMP10_Pos)
#define ADC_SMPR2_SMP11_Pos                        (3U)
#define ADC_SMPR2_SMP11_Msk                        (0x7UL << ADC_SMPR2_SMP11_Pos)                   /*!< 0x00000007 */
#define ADC_SMPR2_SMP11                            ADC_SMPR2_SMP11_Msk                              /*!< SMP0[2:0] bits (desc SMP0) */
#define ADC_SMPR2_SMP11_0                          (0x1UL << ADC_SMPR2_SMP11_Pos)                   /*!< 0x00000001 */
#define ADC_SMPR2_SMP11_1                          (0x2UL << ADC_SMPR2_SMP11_Pos)                   /*!< 0x00000002 */
#define ADC_SMPR2_SMP11_2                          (0x4UL << ADC_SMPR2_SMP11_Pos)
#define ADC_SMPR2_SMP12_Pos                        (6U)
#define ADC_SMPR2_SMP12_Msk                        (0x7UL << ADC_SMPR2_SMP12_Pos)                   /*!< 0x00000007 */
#define ADC_SMPR2_SMP12                            ADC_SMPR2_SMP12_Msk                              /*!< SMP0[2:0] bits (desc SMP0) */
#define ADC_SMPR2_SMP12_0                          (0x1UL << ADC_SMPR2_SMP12_Pos)                   /*!< 0x00000001 */
#define ADC_SMPR2_SMP12_1                          (0x2UL << ADC_SMPR2_SMP12_Pos)                   /*!< 0x00000002 */
#define ADC_SMPR2_SMP12_2                          (0x4UL << ADC_SMPR2_SMP12_Pos)
#define ADC_SMPR2_SMP13_Pos                        (9U)
#define ADC_SMPR2_SMP13_Msk                        (0x7UL << ADC_SMPR2_SMP13_Pos)                   /*!< 0x00000007 */
#define ADC_SMPR2_SMP13                            ADC_SMPR2_SMP13_Msk                              /*!< SMP0[2:0] bits (desc SMP0) */
#define ADC_SMPR2_SMP13_0                          (0x1UL << ADC_SMPR2_SMP13_Pos)                   /*!< 0x00000001 */
#define ADC_SMPR2_SMP13_1                          (0x2UL << ADC_SMPR2_SMP13_Pos)                   /*!< 0x00000002 */
#define ADC_SMPR2_SMP13_2                          (0x4UL << ADC_SMPR2_SMP13_Pos)
#define ADC_SMPR2_SMP14_Pos                        (12U)
#define ADC_SMPR2_SMP14_Msk                        (0x7UL << ADC_SMPR2_SMP14_Pos)                   /*!< 0x00000007 */
#define ADC_SMPR2_SMP14                            ADC_SMPR2_SMP14_Msk                              /*!< SMP0[2:0] bits (desc SMP0) */
#define ADC_SMPR2_SMP14_0                          (0x1UL << ADC_SMPR2_SMP14_Pos)                   /*!< 0x00000001 */
#define ADC_SMPR2_SMP14_1                          (0x2UL << ADC_SMPR2_SMP14_Pos)                   /*!< 0x00000002 */
#define ADC_SMPR2_SMP14_2                          (0x4UL << ADC_SMPR2_SMP14_Pos)
#define ADC_SMPR2_SMP15_Pos                       (15U)
#define ADC_SMPR2_SMP15_Msk                       (0x7UL << ADC_SMPR2_SMP15_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR2_SMP15                           ADC_SMPR2_SMP15_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR2_SMP15_0                         (0x1UL << ADC_SMPR2_SMP15_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR2_SMP15_1                         (0x2UL << ADC_SMPR2_SMP15_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR2_SMP15_2                         (0x4UL << ADC_SMPR2_SMP15_Pos)                    /*!< 0x00020000 */
#define ADC_SMPR2_SMP16_Pos                       (18U)
#define ADC_SMPR2_SMP16_Msk                       (0x7UL << ADC_SMPR2_SMP16_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR2_SMP16                           ADC_SMPR2_SMP16_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR2_SMP16_0                         (0x1UL << ADC_SMPR2_SMP16_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR2_SMP16_1                         (0x2UL << ADC_SMPR2_SMP16_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR2_SMP16_2                         (0x4UL << ADC_SMPR2_SMP16_Pos)
#define ADC_SMPR2_SMP17_Pos                       (21U)
#define ADC_SMPR2_SMP17_Msk                       (0x7UL << ADC_SMPR2_SMP17_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR2_SMP17                           ADC_SMPR2_SMP17_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR2_SMP17_0                         (0x1UL << ADC_SMPR2_SMP17_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR2_SMP17_1                         (0x2UL << ADC_SMPR2_SMP17_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR2_SMP17_2                         (0x4UL << ADC_SMPR2_SMP17_Pos)
#define ADC_SMPR2_SMP18_Pos                       (24U)
#define ADC_SMPR2_SMP18_Msk                       (0x7UL << ADC_SMPR2_SMP18_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR2_SMP18                           ADC_SMPR2_SMP18_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR2_SMP18_0                         (0x1UL << ADC_SMPR2_SMP18_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR2_SMP18_1                         (0x2UL << ADC_SMPR2_SMP18_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR2_SMP18_2                         (0x4UL << ADC_SMPR2_SMP18_Pos)
#define ADC_SMPR2_SMP19_Pos                       (27U)
#define ADC_SMPR2_SMP19_Msk                       (0x7UL << ADC_SMPR2_SMP19_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR2_SMP19                           ADC_SMPR2_SMP19_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR2_SMP19_0                         (0x1UL << ADC_SMPR2_SMP19_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR2_SMP19_1                         (0x2UL << ADC_SMPR2_SMP19_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR2_SMP19_2                         (0x4UL << ADC_SMPR2_SMP19_Pos)

/*!< ADC_SMPR3 */
#define ADC_SMPR3_SMP0_Pos                        (0U)
#define ADC_SMPR3_SMP0_Msk                        (0x7UL << ADC_SMPR3_SMP0_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR3_SMP0                            ADC_SMPR3_SMP0_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR3_SMP0_0                          (0x1UL << ADC_SMPR3_SMP0_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR3_SMP0_1                          (0x2UL << ADC_SMPR3_SMP0_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR3_SMP0_2                          (0x4UL << ADC_SMPR3_SMP0_Pos)
#define ADC_SMPR3_SMP1_Pos                        (3U)
#define ADC_SMPR3_SMP1_Msk                        (0x7UL << ADC_SMPR3_SMP1_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR3_SMP1                            ADC_SMPR3_SMP1_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR3_SMP1_0                          (0x1UL << ADC_SMPR3_SMP1_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR3_SMP1_1                          (0x2UL << ADC_SMPR3_SMP1_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR3_SMP1_2                          (0x4UL << ADC_SMPR3_SMP1_Pos)
#define ADC_SMPR3_SMP2_Pos                        (6U)
#define ADC_SMPR3_SMP2_Msk                        (0x7UL << ADC_SMPR3_SMP2_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR3_SMP2                            ADC_SMPR3_SMP2_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR3_SMP2_0                          (0x1UL << ADC_SMPR3_SMP2_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR3_SMP2_1                          (0x2UL << ADC_SMPR3_SMP2_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR3_SMP2_2                          (0x4UL << ADC_SMPR3_SMP2_Pos)
#define ADC_SMPR3_SMP3_Pos                        (9U)
#define ADC_SMPR3_SMP3_Msk                        (0x7UL << ADC_SMPR3_SMP3_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR3_SMP3                            ADC_SMPR3_SMP3_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR3_SMP3_0                          (0x1UL << ADC_SMPR3_SMP3_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR3_SMP3_1                          (0x2UL << ADC_SMPR3_SMP3_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR3_SMP3_2                          (0x4UL << ADC_SMPR3_SMP3_Pos)
#define ADC_SMPR3_SMP4_Pos                        (12U)
#define ADC_SMPR3_SMP4_Msk                        (0x7UL << ADC_SMPR3_SMP4_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR3_SMP4                            ADC_SMPR3_SMP4_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR3_SMP4_0                          (0x1UL << ADC_SMPR3_SMP4_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR3_SMP4_1                          (0x2UL << ADC_SMPR3_SMP4_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR3_SMP4_2                          (0x4UL << ADC_SMPR3_SMP4_Pos)
#define ADC_SMPR3_SMP5_Pos                        (15U)
#define ADC_SMPR3_SMP5_Msk                        (0x7UL << ADC_SMPR3_SMP5_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR3_SMP5                            ADC_SMPR3_SMP5_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR3_SMP5_0                          (0x1UL << ADC_SMPR3_SMP5_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR3_SMP5_1                          (0x2UL << ADC_SMPR3_SMP5_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR3_SMP5_2                          (0x4UL << ADC_SMPR3_SMP5_Pos)
#define ADC_SMPR3_SMP6_Pos                        (18U)
#define ADC_SMPR3_SMP6_Msk                        (0x7UL << ADC_SMPR3_SMP6_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR3_SMP6                            ADC_SMPR3_SMP6_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR3_SMP6_0                          (0x1UL << ADC_SMPR3_SMP6_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR3_SMP6_1                          (0x2UL << ADC_SMPR3_SMP6_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR3_SMP6_2                          (0x4UL << ADC_SMPR3_SMP6_Pos)
#define ADC_SMPR3_SMP7_Pos                        (21U)
#define ADC_SMPR3_SMP7_Msk                        (0x7UL << ADC_SMPR3_SMP7_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR3_SMP7                            ADC_SMPR3_SMP7_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR3_SMP7_0                          (0x1UL << ADC_SMPR3_SMP7_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR3_SMP7_1                          (0x2UL << ADC_SMPR3_SMP7_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR3_SMP7_2                          (0x4UL << ADC_SMPR3_SMP7_Pos)
#define ADC_SMPR3_SMP8_Pos                        (24U)
#define ADC_SMPR3_SMP8_Msk                        (0x7UL << ADC_SMPR3_SMP8_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR3_SMP8                            ADC_SMPR3_SMP8_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR3_SMP8_0                          (0x1UL << ADC_SMPR3_SMP8_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR3_SMP8_1                          (0x2UL << ADC_SMPR3_SMP8_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR3_SMP8_2                          (0x4UL << ADC_SMPR3_SMP8_Pos)
#define ADC_SMPR3_SMP9_Pos                        (27U)
#define ADC_SMPR3_SMP9_Msk                        (0x7UL << ADC_SMPR3_SMP9_Pos)                    /*!< 0x00038000 */
#define ADC_SMPR3_SMP9                            ADC_SMPR3_SMP9_Msk                               /*!< SMP15[17:15] bits (desc SMP15) */
#define ADC_SMPR3_SMP9_0                          (0x1UL << ADC_SMPR3_SMP9_Pos)                    /*!< 0x00008000 */
#define ADC_SMPR3_SMP9_1                          (0x2UL << ADC_SMPR3_SMP9_Pos)                    /*!< 0x00010000 */
#define ADC_SMPR3_SMP9_2                          (0x4UL << ADC_SMPR3_SMP9_Pos)

/*!< ADC_JOFR1 */
#define ADC_JOFR1_JOFFSET1_Pos                    (0U)
#define ADC_JOFR1_JOFFSET1_Msk                    (0xFFFUL << ADC_JOFR1_JOFFSET1_Pos)               /*!< 0x00000FFF */
#define ADC_JOFR1_JOFFSET1                        ADC_JOFR1_JOFFSET1_Msk                            /*!< JOFFSET1[11:0] bits (desc JOFFSET1) */

/*!< ADC_JOFR2 */
#define ADC_JOFR2_JOFFSET2_Pos                    (0U)
#define ADC_JOFR2_JOFFSET2_Msk                    (0xFFFUL << ADC_JOFR2_JOFFSET2_Pos)               /*!< 0x00000FFF */
#define ADC_JOFR2_JOFFSET2                        ADC_JOFR2_JOFFSET2_Msk                            /*!< JOFFSET2[11:0] bits (desc JOFFSET2) */

/*!< ADC_JOFR3 */
#define ADC_JOFR3_JOFFSET3_Pos                    (0U)
#define ADC_JOFR3_JOFFSET3_Msk                    (0xFFFUL << ADC_JOFR3_JOFFSET3_Pos)               /*!< 0x00000FFF */
#define ADC_JOFR3_JOFFSET3                        ADC_JOFR3_JOFFSET3_Msk                            /*!< JOFFSET3[11:0] bits (desc JOFFSET3) */

/*!< ADC_JOFR4 */
#define ADC_JOFR4_JOFFSET4_Pos                    (0U)
#define ADC_JOFR4_JOFFSET4_Msk                    (0xFFFUL << ADC_JOFR4_JOFFSET4_Pos)               /*!< 0x00000FFF */
#define ADC_JOFR4_JOFFSET4                        ADC_JOFR4_JOFFSET4_Msk                            /*!< JOFFSET4[11:0] bits (desc JOFFSET4) */

/*!< ADC_HTR */
#define ADC_HTR_HT_Pos                            (0U)
#define ADC_HTR_HT_Msk                            (0xFFFUL << ADC_HTR_HT_Pos)                       /*!< 0x00000FFF */
#define ADC_HTR_HT                                ADC_HTR_HT_Msk                                    /*!< HT[11:0] bits (desc HT) */

/*!< ADC_LTR */
#define ADC_LTR_LT_Pos                            (0U)
#define ADC_LTR_LT_Msk                            (0xFFFUL << ADC_LTR_LT_Pos)                       /*!< 0x00000FFF */
#define ADC_LTR_LT                                ADC_LTR_LT_Msk

/*!< ADC_SQR1 */
#define ADC_SQR1_SQ13_Pos                         (0U)
#define ADC_SQR1_SQ13_Msk                         (0x1FUL << ADC_SQR1_SQ13_Pos)                     /*!< 0x0000001F */
#define ADC_SQR1_SQ13                             ADC_SQR1_SQ13_Msk                                 /*!< SQ13[4:0] bits (desc SQ13) */
#define ADC_SQR1_SQ13_0                           (0x1UL << ADC_SQR1_SQ13_Pos)                      /*!< 0x00000001 */
#define ADC_SQR1_SQ13_1                           (0x2UL << ADC_SQR1_SQ13_Pos)                      /*!< 0x00000002 */
#define ADC_SQR1_SQ13_2                           (0x4UL << ADC_SQR1_SQ13_Pos)                      /*!< 0x00000004 */
#define ADC_SQR1_SQ13_3                           (0x8UL << ADC_SQR1_SQ13_Pos)                      /*!< 0x00000008 */
#define ADC_SQR1_SQ13_4                           (0x10UL << ADC_SQR1_SQ13_Pos)                     /*!< 0x00000010 */
#define ADC_SQR1_SQ14_Pos                         (5U)
#define ADC_SQR1_SQ14_Msk                         (0x1FUL << ADC_SQR1_SQ14_Pos)                     /*!< 0x000003E0 */
#define ADC_SQR1_SQ14                             ADC_SQR1_SQ14_Msk                                 /*!< SQ14[9:5] bits (desc SQ14) */
#define ADC_SQR1_SQ14_0                           (0x1UL << ADC_SQR1_SQ14_Pos)                      /*!< 0x00000020 */
#define ADC_SQR1_SQ14_1                           (0x2UL << ADC_SQR1_SQ14_Pos)                      /*!< 0x00000040 */
#define ADC_SQR1_SQ14_2                           (0x4UL << ADC_SQR1_SQ14_Pos)                      /*!< 0x00000080 */
#define ADC_SQR1_SQ14_3                           (0x8UL << ADC_SQR1_SQ14_Pos)                      /*!< 0x00000100 */
#define ADC_SQR1_SQ14_4                           (0x10UL << ADC_SQR1_SQ14_Pos)                     /*!< 0x00000200 */
#define ADC_SQR1_SQ15_Pos                         (10U)
#define ADC_SQR1_SQ15_Msk                         (0x1FUL << ADC_SQR1_SQ15_Pos)                     /*!< 0x00007C00 */
#define ADC_SQR1_SQ15                             ADC_SQR1_SQ15_Msk                                 /*!< SQ15[14:10] bits (desc SQ15) */
#define ADC_SQR1_SQ15_0                           (0x1UL << ADC_SQR1_SQ15_Pos)                      /*!< 0x00000400 */
#define ADC_SQR1_SQ15_1                           (0x2UL << ADC_SQR1_SQ15_Pos)                      /*!< 0x00000800 */
#define ADC_SQR1_SQ15_2                           (0x4UL << ADC_SQR1_SQ15_Pos)                      /*!< 0x00001000 */
#define ADC_SQR1_SQ15_3                           (0x8UL << ADC_SQR1_SQ15_Pos)                      /*!< 0x00002000 */
#define ADC_SQR1_SQ15_4                           (0x10UL << ADC_SQR1_SQ15_Pos)                     /*!< 0x00004000 */
#define ADC_SQR1_SQ16_Pos                         (15U)
#define ADC_SQR1_SQ16_Msk                         (0x1FUL << ADC_SQR1_SQ16_Pos)                     /*!< 0x000F8000 */
#define ADC_SQR1_SQ16                             ADC_SQR1_SQ16_Msk                                 /*!< SQ16[19:15] bits (desc SQ16) */
#define ADC_SQR1_SQ16_0                           (0x1UL << ADC_SQR1_SQ16_Pos)                      /*!< 0x00008000 */
#define ADC_SQR1_SQ16_1                           (0x2UL << ADC_SQR1_SQ16_Pos)                      /*!< 0x00010000 */
#define ADC_SQR1_SQ16_2                           (0x4UL << ADC_SQR1_SQ16_Pos)                      /*!< 0x00020000 */
#define ADC_SQR1_SQ16_3                           (0x8UL << ADC_SQR1_SQ16_Pos)                      /*!< 0x00040000 */
#define ADC_SQR1_SQ16_4                           (0x10UL << ADC_SQR1_SQ16_Pos)                     /*!< 0x00080000 */
#define ADC_SQR1_L_Pos                            (20U)
#define ADC_SQR1_L_Msk                            (0xFUL << ADC_SQR1_L_Pos)                         /*!< 0x00F00000 */
#define ADC_SQR1_L                                ADC_SQR1_L_Msk                                    /*!< L[23:20] bits (desc L) */
#define ADC_SQR1_L_0                              (0x1UL << ADC_SQR1_L_Pos)                         /*!< 0x00100000 */
#define ADC_SQR1_L_1                              (0x2UL << ADC_SQR1_L_Pos)                         /*!< 0x00200000 */
#define ADC_SQR1_L_2                              (0x4UL << ADC_SQR1_L_Pos)                         /*!< 0x00400000 */
#define ADC_SQR1_L_3                              (0x8UL << ADC_SQR1_L_Pos)

/*!< ADC_SQR2 */
#define ADC_SQR2_SQ7_Pos                          (0U)
#define ADC_SQR2_SQ7_Msk                          (0x1FUL << ADC_SQR2_SQ7_Pos)                      /*!< 0x0000001F */
#define ADC_SQR2_SQ7                              ADC_SQR2_SQ7_Msk                                  /*!< SQ7[4:0] bits (desc SQ7) */
#define ADC_SQR2_SQ7_0                            (0x1UL << ADC_SQR2_SQ7_Pos)                       /*!< 0x00000001 */
#define ADC_SQR2_SQ7_1                            (0x2UL << ADC_SQR2_SQ7_Pos)                       /*!< 0x00000002 */
#define ADC_SQR2_SQ7_2                            (0x4UL << ADC_SQR2_SQ7_Pos)                       /*!< 0x00000004 */
#define ADC_SQR2_SQ7_3                            (0x8UL << ADC_SQR2_SQ7_Pos)                       /*!< 0x00000008 */
#define ADC_SQR2_SQ7_4                            (0x10UL << ADC_SQR2_SQ7_Pos)                      /*!< 0x00000010 */
#define ADC_SQR2_SQ8_Pos                          (5U)
#define ADC_SQR2_SQ8_Msk                          (0x1FUL << ADC_SQR2_SQ8_Pos)                      /*!< 0x000003E0 */
#define ADC_SQR2_SQ8                              ADC_SQR2_SQ8_Msk                                  /*!< SQ8[9:5] bits (desc SQ8) */
#define ADC_SQR2_SQ8_0                            (0x1UL << ADC_SQR2_SQ8_Pos)                       /*!< 0x00000020 */
#define ADC_SQR2_SQ8_1                            (0x2UL << ADC_SQR2_SQ8_Pos)                       /*!< 0x00000040 */
#define ADC_SQR2_SQ8_2                            (0x4UL << ADC_SQR2_SQ8_Pos)                       /*!< 0x00000080 */
#define ADC_SQR2_SQ8_3                            (0x8UL << ADC_SQR2_SQ8_Pos)                       /*!< 0x00000100 */
#define ADC_SQR2_SQ8_4                            (0x10UL << ADC_SQR2_SQ8_Pos)                      /*!< 0x00000200 */
#define ADC_SQR2_SQ9_Pos                          (10U)
#define ADC_SQR2_SQ9_Msk                          (0x1FUL << ADC_SQR2_SQ9_Pos)                      /*!< 0x00007C00 */
#define ADC_SQR2_SQ9                              ADC_SQR2_SQ9_Msk                                  /*!< SQ9[14:10] bits (desc SQ9) */
#define ADC_SQR2_SQ9_0                            (0x1UL << ADC_SQR2_SQ9_Pos)                       /*!< 0x00000400 */
#define ADC_SQR2_SQ9_1                            (0x2UL << ADC_SQR2_SQ9_Pos)                       /*!< 0x00000800 */
#define ADC_SQR2_SQ9_2                            (0x4UL << ADC_SQR2_SQ9_Pos)                       /*!< 0x00001000 */
#define ADC_SQR2_SQ9_3                            (0x8UL << ADC_SQR2_SQ9_Pos)                       /*!< 0x00002000 */
#define ADC_SQR2_SQ9_4                            (0x10UL << ADC_SQR2_SQ9_Pos)                      /*!< 0x00004000 */
#define ADC_SQR2_SQ10_Pos                         (15U)
#define ADC_SQR2_SQ10_Msk                         (0x1FUL << ADC_SQR2_SQ10_Pos)                     /*!< 0x000F8000 */
#define ADC_SQR2_SQ10                             ADC_SQR2_SQ10_Msk                                 /*!< SQ10[19:15] bits (desc SQ10) */
#define ADC_SQR2_SQ10_0                           (0x1UL << ADC_SQR2_SQ10_Pos)                      /*!< 0x00008000 */
#define ADC_SQR2_SQ10_1                           (0x2UL << ADC_SQR2_SQ10_Pos)                      /*!< 0x00010000 */
#define ADC_SQR2_SQ10_2                           (0x4UL << ADC_SQR2_SQ10_Pos)                      /*!< 0x00020000 */
#define ADC_SQR2_SQ10_3                           (0x8UL << ADC_SQR2_SQ10_Pos)                      /*!< 0x00040000 */
#define ADC_SQR2_SQ10_4                           (0x10UL << ADC_SQR2_SQ10_Pos)                     /*!< 0x00080000 */
#define ADC_SQR2_SQ11_Pos                         (20U)
#define ADC_SQR2_SQ11_Msk                         (0x1FUL << ADC_SQR2_SQ11_Pos)                     /*!< 0x01F00000 */
#define ADC_SQR2_SQ11                             ADC_SQR2_SQ11_Msk                                 /*!< SQ11[24:20] bits (desc SQ11) */
#define ADC_SQR2_SQ11_0                           (0x1UL << ADC_SQR2_SQ11_Pos)                      /*!< 0x00100000 */
#define ADC_SQR2_SQ11_1                           (0x2UL << ADC_SQR2_SQ11_Pos)                      /*!< 0x00200000 */
#define ADC_SQR2_SQ11_2                           (0x4UL << ADC_SQR2_SQ11_Pos)                      /*!< 0x00400000 */
#define ADC_SQR2_SQ11_3                           (0x8UL << ADC_SQR2_SQ11_Pos)                      /*!< 0x00800000 */
#define ADC_SQR2_SQ11_4                           (0x10UL << ADC_SQR2_SQ11_Pos)                     /*!< 0x01000000 */
#define ADC_SQR2_SQ12_Pos                         (25U)
#define ADC_SQR2_SQ12_Msk                         (0x1FUL << ADC_SQR2_SQ12_Pos)                     /*!< 0x3E000000 */
#define ADC_SQR2_SQ12                             ADC_SQR2_SQ12_Msk                                 /*!< SQ12[29:25] bits (desc SQ12) */
#define ADC_SQR2_SQ12_0                           (0x1UL << ADC_SQR2_SQ12_Pos)                      /*!< 0x02000000 */
#define ADC_SQR2_SQ12_1                           (0x2UL << ADC_SQR2_SQ12_Pos)                      /*!< 0x04000000 */
#define ADC_SQR2_SQ12_2                           (0x4UL << ADC_SQR2_SQ12_Pos)                      /*!< 0x08000000 */
#define ADC_SQR2_SQ12_3                           (0x8UL << ADC_SQR2_SQ12_Pos)                      /*!< 0x10000000 */
#define ADC_SQR2_SQ12_4                           (0x10UL << ADC_SQR2_SQ12_Pos)                     /*!< 0x20000000 */

/*!< ADC_SQR3 */
#define ADC_SQR3_SQ1_Pos                          (0U)
#define ADC_SQR3_SQ1_Msk                          (0x1FUL << ADC_SQR3_SQ1_Pos)                      /*!< 0x0000001F */
#define ADC_SQR3_SQ1                              ADC_SQR3_SQ1_Msk                                  /*!< SQ1[4:0] bits (desc SQ1) */
#define ADC_SQR3_SQ1_0                            (0x1UL << ADC_SQR3_SQ1_Pos)                       /*!< 0x00000001 */
#define ADC_SQR3_SQ1_1                            (0x2UL << ADC_SQR3_SQ1_Pos)                       /*!< 0x00000002 */
#define ADC_SQR3_SQ1_2                            (0x4UL << ADC_SQR3_SQ1_Pos)                       /*!< 0x00000004 */
#define ADC_SQR3_SQ1_3                            (0x8UL << ADC_SQR3_SQ1_Pos)                       /*!< 0x00000008 */
#define ADC_SQR3_SQ1_4                            (0x10UL << ADC_SQR3_SQ1_Pos)                      /*!< 0x00000010 */
#define ADC_SQR3_SQ2_Pos                          (5U)
#define ADC_SQR3_SQ2_Msk                          (0x1FUL << ADC_SQR3_SQ2_Pos)                      /*!< 0x000003E0 */
#define ADC_SQR3_SQ2                              ADC_SQR3_SQ2_Msk                                  /*!< SQ2[9:5] bits (desc SQ2) */
#define ADC_SQR3_SQ2_0                            (0x1UL << ADC_SQR3_SQ2_Pos)                       /*!< 0x00000020 */
#define ADC_SQR3_SQ2_1                            (0x2UL << ADC_SQR3_SQ2_Pos)                       /*!< 0x00000040 */
#define ADC_SQR3_SQ2_2                            (0x4UL << ADC_SQR3_SQ2_Pos)                       /*!< 0x00000080 */
#define ADC_SQR3_SQ2_3                            (0x8UL << ADC_SQR3_SQ2_Pos)                       /*!< 0x00000100 */
#define ADC_SQR3_SQ2_4                            (0x10UL << ADC_SQR3_SQ2_Pos)                      /*!< 0x00000200 */
#define ADC_SQR3_SQ3_Pos                          (10U)
#define ADC_SQR3_SQ3_Msk                          (0x1FUL << ADC_SQR3_SQ3_Pos)                      /*!< 0x00007C00 */
#define ADC_SQR3_SQ3                              ADC_SQR3_SQ3_Msk                                  /*!< SQ3[14:10] bits (desc SQ3) */
#define ADC_SQR3_SQ3_0                            (0x1UL << ADC_SQR3_SQ3_Pos)                       /*!< 0x00000400 */
#define ADC_SQR3_SQ3_1                            (0x2UL << ADC_SQR3_SQ3_Pos)                       /*!< 0x00000800 */
#define ADC_SQR3_SQ3_2                            (0x4UL << ADC_SQR3_SQ3_Pos)                       /*!< 0x00001000 */
#define ADC_SQR3_SQ3_3                            (0x8UL << ADC_SQR3_SQ3_Pos)                       /*!< 0x00002000 */
#define ADC_SQR3_SQ3_4                            (0x10UL << ADC_SQR3_SQ3_Pos)                      /*!< 0x00004000 */
#define ADC_SQR3_SQ4_Pos                          (15U)
#define ADC_SQR3_SQ4_Msk                          (0x1FUL << ADC_SQR3_SQ4_Pos)                      /*!< 0x000F8000 */
#define ADC_SQR3_SQ4                              ADC_SQR3_SQ4_Msk                                  /*!< SQ4[19:15] bits (desc SQ4) */
#define ADC_SQR3_SQ4_0                            (0x1UL << ADC_SQR3_SQ4_Pos)                       /*!< 0x00008000 */
#define ADC_SQR3_SQ4_1                            (0x2UL << ADC_SQR3_SQ4_Pos)                       /*!< 0x00010000 */
#define ADC_SQR3_SQ4_2                            (0x4UL << ADC_SQR3_SQ4_Pos)                       /*!< 0x00020000 */
#define ADC_SQR3_SQ4_3                            (0x8UL << ADC_SQR3_SQ4_Pos)                       /*!< 0x00040000 */
#define ADC_SQR3_SQ4_4                            (0x10UL << ADC_SQR3_SQ4_Pos)
#define ADC_SQR3_SQ5_Pos                          (20U)
#define ADC_SQR3_SQ5_Msk                          (0x1UL << ADC_SQR3_SQ5_Pos)                       /*!< 0x01000000 */
#define ADC_SQR3_SQ5                              ADC_SQR3_SQ5_Msk
#define ADC_SQR3_SQ5_0                            (0x1UL << ADC_SQR3_SQ5_Pos)                       /*!< 0x00008000 */
#define ADC_SQR3_SQ5_1                            (0x2UL << ADC_SQR3_SQ5_Pos)                       /*!< 0x00010000 */
#define ADC_SQR3_SQ5_2                            (0x4UL << ADC_SQR3_SQ5_Pos)                       /*!< 0x00020000 */
#define ADC_SQR3_SQ5_3                            (0x8UL << ADC_SQR3_SQ5_Pos)                       /*!< 0x00040000 */
#define ADC_SQR3_SQ5_4                            (0x10UL << ADC_SQR3_SQ5_Pos)
#define ADC_SQR3_SQ6_Pos                          (25U)
#define ADC_SQR3_SQ6_Msk                          (0x1FUL << ADC_SQR3_SQ6_Pos)                      /*!< 0x3E000000 */
#define ADC_SQR3_SQ6                              ADC_SQR3_SQ6_Msk                                  /*!< SQ6[29:25] bits (desc SQ6) */
#define ADC_SQR3_SQ6_0                            (0x1UL << ADC_SQR3_SQ6_Pos)                       /*!< 0x02000000 */
#define ADC_SQR3_SQ6_1                            (0x2UL << ADC_SQR3_SQ6_Pos)                       /*!< 0x04000000 */
#define ADC_SQR3_SQ6_2                            (0x4UL << ADC_SQR3_SQ6_Pos)                       /*!< 0x08000000 */
#define ADC_SQR3_SQ6_3                            (0x8UL << ADC_SQR3_SQ6_Pos)                       /*!< 0x10000000 */
#define ADC_SQR3_SQ6_4                            (0x10UL << ADC_SQR3_SQ6_Pos)

/*!< ADC_JSQR */
#define ADC_JSQR_JSQ1_Pos                         (0U)
#define ADC_JSQR_JSQ1_Msk                         (0x1FUL << ADC_JSQR_JSQ1_Pos)                     /*!< 0x0000001F */
#define ADC_JSQR_JSQ1                             ADC_JSQR_JSQ1_Msk                                 /*!< JSQ1[4:0] bits (desc JSQ1) */
#define ADC_JSQR_JSQ1_0                           (0x1UL << ADC_JSQR_JSQ1_Pos)                      /*!< 0x00000001 */
#define ADC_JSQR_JSQ1_1                           (0x2UL << ADC_JSQR_JSQ1_Pos)                      /*!< 0x00000002 */
#define ADC_JSQR_JSQ1_2                           (0x4UL << ADC_JSQR_JSQ1_Pos)                      /*!< 0x00000004 */
#define ADC_JSQR_JSQ1_3                           (0x8UL << ADC_JSQR_JSQ1_Pos)                      /*!< 0x00000008 */
#define ADC_JSQR_JSQ1_4                           (0x10UL << ADC_JSQR_JSQ1_Pos)                     /*!< 0x00000010 */
#define ADC_JSQR_JSQ2_Pos                         (5U)
#define ADC_JSQR_JSQ2_Msk                         (0x1FUL << ADC_JSQR_JSQ2_Pos)                     /*!< 0x000003E0 */
#define ADC_JSQR_JSQ2                             ADC_JSQR_JSQ2_Msk                                 /*!< JSQ2[9:5] bits (desc JSQ2) */
#define ADC_JSQR_JSQ2_0                           (0x1UL << ADC_JSQR_JSQ2_Pos)                      /*!< 0x00000020 */
#define ADC_JSQR_JSQ2_1                           (0x2UL << ADC_JSQR_JSQ2_Pos)                      /*!< 0x00000040 */
#define ADC_JSQR_JSQ2_2                           (0x4UL << ADC_JSQR_JSQ2_Pos)                      /*!< 0x00000080 */
#define ADC_JSQR_JSQ2_3                           (0x8UL << ADC_JSQR_JSQ2_Pos)                      /*!< 0x00000100 */
#define ADC_JSQR_JSQ2_4                           (0x10UL << ADC_JSQR_JSQ2_Pos)                     /*!< 0x00000200 */
#define ADC_JSQR_JSQ3_Pos                         (10U)
#define ADC_JSQR_JSQ3_Msk                         (0x1FUL << ADC_JSQR_JSQ3_Pos)                     /*!< 0x00007C00 */
#define ADC_JSQR_JSQ3                             ADC_JSQR_JSQ3_Msk                                 /*!< JSQ3[14:10] bits (desc JSQ3) */
#define ADC_JSQR_JSQ3_0                           (0x1UL << ADC_JSQR_JSQ3_Pos)                      /*!< 0x00000400 */
#define ADC_JSQR_JSQ3_1                           (0x2UL << ADC_JSQR_JSQ3_Pos)                      /*!< 0x00000800 */
#define ADC_JSQR_JSQ3_2                           (0x4UL << ADC_JSQR_JSQ3_Pos)                      /*!< 0x00001000 */
#define ADC_JSQR_JSQ3_3                           (0x8UL << ADC_JSQR_JSQ3_Pos)                      /*!< 0x00002000 */
#define ADC_JSQR_JSQ3_4                           (0x10UL << ADC_JSQR_JSQ3_Pos)                     /*!< 0x00004000 */
#define ADC_JSQR_JSQ4_Pos                         (15U)
#define ADC_JSQR_JSQ4_Msk                         (0x1FUL << ADC_JSQR_JSQ4_Pos)                     /*!< 0x000F8000 */
#define ADC_JSQR_JSQ4                             ADC_JSQR_JSQ4_Msk                                 /*!< JSQ4[19:15] bits (desc JSQ4) */
#define ADC_JSQR_JSQ4_0                           (0x1UL << ADC_JSQR_JSQ4_Pos)                      /*!< 0x00008000 */
#define ADC_JSQR_JSQ4_1                           (0x2UL << ADC_JSQR_JSQ4_Pos)                      /*!< 0x00010000 */
#define ADC_JSQR_JSQ4_2                           (0x4UL << ADC_JSQR_JSQ4_Pos)                      /*!< 0x00020000 */
#define ADC_JSQR_JSQ4_3                           (0x8UL << ADC_JSQR_JSQ4_Pos)                      /*!< 0x00040000 */
#define ADC_JSQR_JSQ4_4                           (0x10UL << ADC_JSQR_JSQ4_Pos)                     /*!< 0x00080000 */
#define ADC_JSQR_JL_Pos                           (20U)
#define ADC_JSQR_JL_Msk                           (0xFUL << ADC_JSQR_JL_Pos)                        /*!< 0x00300000 */
#define ADC_JSQR_JL                               ADC_JSQR_JL_Msk                                   /*!< JL[21:20] bits (desc JL) */
#define ADC_JSQR_JL_0                             (0x1UL << ADC_JSQR_JL_Pos)                        /*!< 0x00100000 */
#define ADC_JSQR_JL_1                             (0x2UL << ADC_JSQR_JL_Pos)                        /*!< 0x00200000 */
#define ADC_JSQR_JL_2                             (0x4UL << ADC_JSQR_JL_Pos)
#define ADC_JSQR_JL_3                             (0x8UL << ADC_JSQR_JL_Pos)

/*!< ADC_JDR1 */
#define ADC_JDR1_JDR1_Pos                         (0U)
#define ADC_JDR1_JDR1_Msk                         (0xFFFFUL << ADC_JDR1_JDR1_Pos)                   /*!< 0x0000FFFF */
#define ADC_JDR1_JDR1                             ADC_JDR1_JDR1_Msk                                 /*!< JDR1[15:0] bits (desc JDR1) */

/*!< ADC_JDR2 */
#define ADC_JDR2_JDR2_Pos                         (0U)
#define ADC_JDR2_JDR2_Msk                         (0xFFFFUL << ADC_JDR2_JDR2_Pos)                   /*!< 0x0000FFFF */
#define ADC_JDR2_JDR2                             ADC_JDR2_JDR2_Msk                                 /*!< JDR2[15:0] bits (desc JDR2) */

/*!< ADC_JDR3 */
#define ADC_JDR3_JDR3_Pos                         (0U)
#define ADC_JDR3_JDR3_Msk                         (0xFFFFUL << ADC_JDR3_JDR3_Pos)                   /*!< 0x0000FFFF */
#define ADC_JDR3_JDR3                             ADC_JDR3_JDR3_Msk                                 /*!< JDR3[15:0] bits (desc JDR3) */

/*!< ADC_JDR4 */
#define ADC_JDR4_JDR4_Pos                         (0U)
#define ADC_JDR4_JDR4_Msk                         (0xFFFFUL << ADC_JDR4_JDR4_Pos)                   /*!< 0x0000FFFF */
#define ADC_JDR4_JDR4                             ADC_JDR4_JDR4_Msk

/*!< ADC_DR */
#define ADC_DR_DATA_Pos                           (0U)
#define ADC_DR_DATA_Msk                           (0xFFFFUL << ADC_DR_DATA_Pos)                     /*!< 0x0000FFFF */
#define ADC_DR_DATA                               ADC_DR_DATA_Msk

/*!< ADC_CCSR */
#define ADC_CCSR_CALSEL_Pos                       (11U)
#define ADC_CCSR_CALSEL_Msk                       (0x1UL << ADC_CCSR_CALSEL_Pos)                    /*!< 0x00000800 */
#define ADC_CCSR_CALSEL                           ADC_CCSR_CALSEL_Msk                               /*!< desc CALSEL */
#define ADC_CCSR_CALSMP_Pos                       (12U)
#define ADC_CCSR_CALSMP_Msk                       (0x3UL << ADC_CCSR_CALSMP_Pos)                    /*!< 0x00003000 */
#define ADC_CCSR_CALSMP                           ADC_CCSR_CALSMP_Msk                               /*!< CALSMP[13:12] bits (desc CALSMP) */
#define ADC_CCSR_CALSMP_0                         (0x1UL << ADC_CCSR_CALSMP_Pos)                    /*!< 0x00001000 */
#define ADC_CCSR_CALSMP_1                         (0x2UL << ADC_CCSR_CALSMP_Pos)                    /*!< 0x00002000 */
#define ADC_CCSR_CALBYP_Pos                       (14U)
#define ADC_CCSR_CALBYP_Msk                       (0x1UL << ADC_CCSR_CALBYP_Pos)                    /*!< 0x00004000 */
#define ADC_CCSR_CALBYP                           ADC_CCSR_CALBYP_Msk                               /*!< desc CALBYP */
#define ADC_CCSR_CALSET_Pos                       (15U)
#define ADC_CCSR_CALSET_Msk                       (0x1UL << ADC_CCSR_CALSET_Pos)                    /*!< 0x00008000 */
#define ADC_CCSR_CALSET                           ADC_CCSR_CALSET_Msk                               /*!< desc CALSET */
#define ADC_CCSR_OFFSUC_Pos                       (29U)
#define ADC_CCSR_OFFSUC_Msk                       (0x1UL << ADC_CCSR_OFFSUC_Pos)                    /*!< 0x20000000 */
#define ADC_CCSR_OFFSUC                           ADC_CCSR_OFFSUC_Msk                               /*!< desc OFFSUC */
#define ADC_CCSR_CAPSUC_Pos                       (30U)
#define ADC_CCSR_CAPSUC_Msk                       (0x1UL << ADC_CCSR_CAPSUC_Pos)                    /*!< 0x40000000 */
#define ADC_CCSR_CAPSUC                           ADC_CCSR_CAPSUC_Msk                               /*!< desc CAPSUC */
#define ADC_CCSR_CALON_Pos                        (31U)
#define ADC_CCSR_CALON_Msk                        (0x1UL << ADC_CCSR_CALON_Pos)                     /*!< 0x80000000 */
#define ADC_CCSR_CALON                            ADC_CCSR_CALON_Msk

/****************************************************************************/
/*                                                                          */
/*                     Controller Area Network(CAN)                         */
/*                                                                          */
/****************************************************************************/
/*********************  Bits Define For Peripheral CAN  *********************/
/*!< CAN_TSNCR */
#define CAN_TSNCR_VERSION_Pos                   (0U)
#define CAN_TSNCR_VERSION_Msk                   (0xFFFFUL << CAN_TSNCR_VERSION_Pos)             /*!< 0x0000FFFF */
#define CAN_TSNCR_VERSION                       CAN_TSNCR_VERSION_Msk                           /*!< VERSION[15:0] bits (desc VERSION) */
#define CAN_TSNCR_CES_Pos                       (16U)
#define CAN_TSNCR_CES_Msk                       (0x1UL << CAN_TSNCR_CES_Pos)                    /*!< 0x00010000 */
#define CAN_TSNCR_CES                           CAN_TSNCR_CES_Msk                               /*!< desc CES */
#define CAN_TSNCR_ROP_Pos                       (17U)
#define CAN_TSNCR_ROP_Msk                       (0x1UL << CAN_TSNCR_ROP_Pos)                    /*!< 0x00020000 */
#define CAN_TSNCR_ROP                           CAN_TSNCR_ROP_Msk                               /*!< desc ROP */

/*!< CAN_ACBTR */
#define CAN_ACBTR_AC_SEG_1_Pos                  (0U)
#define CAN_ACBTR_AC_SEG_1_Msk                  (0x1FFUL << CAN_ACBTR_AC_SEG_1_Pos)             /*!< 0x000001FF */
#define CAN_ACBTR_AC_SEG_1                      CAN_ACBTR_AC_SEG_1_Msk                          /*!< AC_SEG_1[8:0] bits (desc AC_SEG_1) */
#define CAN_ACBTR_AC_SEG_2_Pos                  (16U)
#define CAN_ACBTR_AC_SEG_2_Msk                  (0x7FUL << CAN_ACBTR_AC_SEG_2_Pos)              /*!< 0x007F0000 */
#define CAN_ACBTR_AC_SEG_2                      CAN_ACBTR_AC_SEG_2_Msk                          /*!< AC_SEG_2[22:16] bits (desc AC_SEG_2) */
#define CAN_ACBTR_AC_SEG_2_0                    (0x1UL << CAN_ACBTR_AC_SEG_2_Pos)               /*!< 0x00010000 */
#define CAN_ACBTR_AC_SEG_2_1                    (0x2UL << CAN_ACBTR_AC_SEG_2_Pos)               /*!< 0x00020000 */
#define CAN_ACBTR_AC_SEG_2_2                    (0x4UL << CAN_ACBTR_AC_SEG_2_Pos)               /*!< 0x00040000 */
#define CAN_ACBTR_AC_SEG_2_3                    (0x8UL << CAN_ACBTR_AC_SEG_2_Pos)               /*!< 0x00080000 */
#define CAN_ACBTR_AC_SEG_2_4                    (0x10UL << CAN_ACBTR_AC_SEG_2_Pos)              /*!< 0x00100000 */
#define CAN_ACBTR_AC_SEG_2_5                    (0x20UL << CAN_ACBTR_AC_SEG_2_Pos)              /*!< 0x00200000 */
#define CAN_ACBTR_AC_SEG_2_6                    (0x40UL << CAN_ACBTR_AC_SEG_2_Pos)              /*!< 0x00400000 */
#define CAN_ACBTR_AC_SJW_Pos                    (24U)
#define CAN_ACBTR_AC_SJW_Msk                    (0x7FUL << CAN_ACBTR_AC_SJW_Pos)                /*!< 0x7F000000 */
#define CAN_ACBTR_AC_SJW                        CAN_ACBTR_AC_SJW_Msk                            /*!< AC_SJW[30:24] bits (desc AC_SJW) */
#define CAN_ACBTR_AC_SJW_0                      (0x1UL << CAN_ACBTR_AC_SJW_Pos)                 /*!< 0x01000000 */
#define CAN_ACBTR_AC_SJW_1                      (0x2UL << CAN_ACBTR_AC_SJW_Pos)                 /*!< 0x02000000 */
#define CAN_ACBTR_AC_SJW_2                      (0x4UL << CAN_ACBTR_AC_SJW_Pos)                 /*!< 0x04000000 */
#define CAN_ACBTR_AC_SJW_3                      (0x8UL << CAN_ACBTR_AC_SJW_Pos)                 /*!< 0x08000000 */
#define CAN_ACBTR_AC_SJW_4                      (0x10UL << CAN_ACBTR_AC_SJW_Pos)                /*!< 0x10000000 */
#define CAN_ACBTR_AC_SJW_5                      (0x20UL << CAN_ACBTR_AC_SJW_Pos)                /*!< 0x20000000 */
#define CAN_ACBTR_AC_SJW_6                      (0x40UL << CAN_ACBTR_AC_SJW_Pos)                /*!< 0x40000000 */

/*!< CAN_RLSSP */
#define CAN_RLSSP_PRESC_Pos                     (0U)
#define CAN_RLSSP_PRESC_Msk                     (0x1FUL << CAN_RLSSP_PRESC_Pos)                 /*!< 0x0000001F */
#define CAN_RLSSP_PRESC                         CAN_RLSSP_PRESC_Msk                             /*!< PRESC[4:0] bits (desc PRESC) */
#define CAN_RLSSP_PRESC_0                       (0x1UL << CAN_RLSSP_PRESC_Pos)                  /*!< 0x00000001 */
#define CAN_RLSSP_PRESC_1                       (0x2UL << CAN_RLSSP_PRESC_Pos)                  /*!< 0x00000002 */
#define CAN_RLSSP_PRESC_2                       (0x4UL << CAN_RLSSP_PRESC_Pos)                  /*!< 0x00000004 */
#define CAN_RLSSP_PRESC_3                       (0x8UL << CAN_RLSSP_PRESC_Pos)                  /*!< 0x00000008 */
#define CAN_RLSSP_PRESC_4                       (0x10UL << CAN_RLSSP_PRESC_Pos)                 /*!< 0x00000010 */
#define CAN_RLSSP_REALIM_Pos                    (24U)
#define CAN_RLSSP_REALIM_Msk                    (0x7UL << CAN_RLSSP_REALIM_Pos)                 /*!< 0x07000000 */
#define CAN_RLSSP_REALIM                        CAN_RLSSP_REALIM_Msk                            /*!< REALIM[26:24] bits (desc REALIM) */
#define CAN_RLSSP_REALIM_0                      (0x1UL << CAN_RLSSP_REALIM_Pos)                 /*!< 0x01000000 */
#define CAN_RLSSP_REALIM_1                      (0x2UL << CAN_RLSSP_REALIM_Pos)                 /*!< 0x02000000 */
#define CAN_RLSSP_REALIM_2                      (0x4UL << CAN_RLSSP_REALIM_Pos)                 /*!< 0x04000000 */
#define CAN_RLSSP_RETLIM_Pos                    (28U)
#define CAN_RLSSP_RETLIM_Msk                    (0x7UL << CAN_RLSSP_RETLIM_Pos)                 /*!< 0x70000000 */
#define CAN_RLSSP_RETLIM                        CAN_RLSSP_RETLIM_Msk                            /*!< RETLIM[30:28] bits (desc RETLIM) */
#define CAN_RLSSP_RETLIM_0                      (0x1UL << CAN_RLSSP_RETLIM_Pos)                 /*!< 0x10000000 */
#define CAN_RLSSP_RETLIM_1                      (0x2UL << CAN_RLSSP_RETLIM_Pos)                 /*!< 0x20000000 */
#define CAN_RLSSP_RETLIM_2                      (0x4UL << CAN_RLSSP_RETLIM_Pos)                 /*!< 0x40000000 */

/*!< CAN_IFR */
#define CAN_IFR_AIF_Pos                         (0U)
#define CAN_IFR_AIF_Msk                         (0x1UL << CAN_IFR_AIF_Pos)                      /*!< 0x00000001 */
#define CAN_IFR_AIF                             CAN_IFR_AIF_Msk                                 /*!< desc AIF */
#define CAN_IFR_EIF_Pos                         (1U)
#define CAN_IFR_EIF_Msk                         (0x1UL << CAN_IFR_EIF_Pos)                      /*!< 0x00000002 */
#define CAN_IFR_EIF                             CAN_IFR_EIF_Msk                                 /*!< desc EIF */
#define CAN_IFR_TSIF_Pos                        (2U)
#define CAN_IFR_TSIF_Msk                        (0x1UL << CAN_IFR_TSIF_Pos)                     /*!< 0x00000004 */
#define CAN_IFR_TSIF                            CAN_IFR_TSIF_Msk                                /*!< desc TSIF */
#define CAN_IFR_TPIF_Pos                        (3U)
#define CAN_IFR_TPIF_Msk                        (0x1UL << CAN_IFR_TPIF_Pos)                     /*!< 0x00000008 */
#define CAN_IFR_TPIF                            CAN_IFR_TPIF_Msk                                /*!< desc TPIF */
#define CAN_IFR_RAFIF_Pos                       (4U)
#define CAN_IFR_RAFIF_Msk                       (0x1UL << CAN_IFR_RAFIF_Pos)                    /*!< 0x00000010 */
#define CAN_IFR_RAFIF                           CAN_IFR_RAFIF_Msk                               /*!< desc RAFIF */
#define CAN_IFR_RFIF_Pos                        (5U)
#define CAN_IFR_RFIF_Msk                        (0x1UL << CAN_IFR_RFIF_Pos)                     /*!< 0x00000020 */
#define CAN_IFR_RFIF                            CAN_IFR_RFIF_Msk                                /*!< desc RFIF */
#define CAN_IFR_ROIF_Pos                        (6U)
#define CAN_IFR_ROIF_Msk                        (0x1UL << CAN_IFR_ROIF_Pos)                     /*!< 0x00000040 */
#define CAN_IFR_ROIF                            CAN_IFR_ROIF_Msk                                /*!< desc ROIF */
#define CAN_IFR_RIF_Pos                         (7U)
#define CAN_IFR_RIF_Msk                         (0x1UL << CAN_IFR_RIF_Pos)                      /*!< 0x00000080 */
#define CAN_IFR_RIF                             CAN_IFR_RIF_Msk                                 /*!< desc RIF */
#define CAN_IFR_BEIF_Pos                        (8U)
#define CAN_IFR_BEIF_Msk                        (0x1UL << CAN_IFR_BEIF_Pos)                     /*!< 0x00000100 */
#define CAN_IFR_BEIF                            CAN_IFR_BEIF_Msk                                /*!< desc BEIF */
#define CAN_IFR_ALIF_Pos                        (9U)
#define CAN_IFR_ALIF_Msk                        (0x1UL << CAN_IFR_ALIF_Pos)                     /*!< 0x00000200 */
#define CAN_IFR_ALIF                            CAN_IFR_ALIF_Msk                                /*!< desc ALIF */
#define CAN_IFR_EPIF_Pos                        (10U)
#define CAN_IFR_EPIF_Msk                        (0x1UL << CAN_IFR_EPIF_Pos)                     /*!< 0x00000400 */
#define CAN_IFR_EPIF                            CAN_IFR_EPIF_Msk                                /*!< desc EPIF */
#define CAN_IFR_TTIF_Pos                        (11U)
#define CAN_IFR_TTIF_Msk                        (0x1UL << CAN_IFR_TTIF_Pos)                     /*!< 0x00000800 */
#define CAN_IFR_TTIF                            CAN_IFR_TTIF_Msk                                /*!< desc TTIF */
#define CAN_IFR_TEIF_Pos                        (12U)
#define CAN_IFR_TEIF_Msk                        (0x1UL << CAN_IFR_TEIF_Pos)                     /*!< 0x00001000 */
#define CAN_IFR_TEIF                            CAN_IFR_TEIF_Msk                                /*!< desc TEIF */
#define CAN_IFR_WTIF_Pos                        (13U)
#define CAN_IFR_WTIF_Msk                        (0x1UL << CAN_IFR_WTIF_Pos)                     /*!< 0x00002000 */
#define CAN_IFR_WTIF                            CAN_IFR_WTIF_Msk                                /*!< desc WTIF */
#define CAN_IFR_EPASS_Pos                       (30U)
#define CAN_IFR_EPASS_Msk                       (0x1UL << CAN_IFR_EPASS_Pos)                    /*!< 0x40000000 */
#define CAN_IFR_EPASS                           CAN_IFR_EPASS_Msk                               /*!< desc EPASS */
#define CAN_IFR_EWARN_Pos                       (31U)
#define CAN_IFR_EWARN_Msk                       (0x1UL << CAN_IFR_EWARN_Pos)                    /*!< 0x80000000 */
#define CAN_IFR_EWARN                           CAN_IFR_EWARN_Msk                               /*!< desc EWARN */

/*!< CAN_IER */
#define CAN_IER_EIE_Pos                         (1U)
#define CAN_IER_EIE_Msk                         (0x1UL << CAN_IER_EIE_Pos)                      /*!< 0x00000002 */
#define CAN_IER_EIE                             CAN_IER_EIE_Msk                                 /*!< desc EIE */
#define CAN_IER_TSIE_Pos                        (2U)
#define CAN_IER_TSIE_Msk                        (0x1UL << CAN_IER_TSIE_Pos)                     /*!< 0x00000004 */
#define CAN_IER_TSIE                            CAN_IER_TSIE_Msk                                /*!< desc TSIE */
#define CAN_IER_TPIE_Pos                        (3U)
#define CAN_IER_TPIE_Msk                        (0x1UL << CAN_IER_TPIE_Pos)                     /*!< 0x00000008 */
#define CAN_IER_TPIE                            CAN_IER_TPIE_Msk                                /*!< desc TPIE */
#define CAN_IER_RAFIE_Pos                       (4U)
#define CAN_IER_RAFIE_Msk                       (0x1UL << CAN_IER_RAFIE_Pos)                    /*!< 0x00000010 */
#define CAN_IER_RAFIE                           CAN_IER_RAFIE_Msk                               /*!< desc RAFIE */
#define CAN_IER_RFIE_Pos                        (5U)
#define CAN_IER_RFIE_Msk                        (0x1UL << CAN_IER_RFIE_Pos)                     /*!< 0x00000020 */
#define CAN_IER_RFIE                            CAN_IER_RFIE_Msk                                /*!< desc RFIE */
#define CAN_IER_ROIE_Pos                        (6U)
#define CAN_IER_ROIE_Msk                        (0x1UL << CAN_IER_ROIE_Pos)                     /*!< 0x00000040 */
#define CAN_IER_ROIE                            CAN_IER_ROIE_Msk                                /*!< desc ROIE */
#define CAN_IER_RIE_Pos                         (7U)
#define CAN_IER_RIE_Msk                         (0x1UL << CAN_IER_RIE_Pos)                      /*!< 0x00000080 */
#define CAN_IER_RIE                             CAN_IER_RIE_Msk                                 /*!< desc RIE */
#define CAN_IER_BEIE_Pos                        (8U)
#define CAN_IER_BEIE_Msk                        (0x1UL << CAN_IER_BEIE_Pos)                     /*!< 0x00000100 */
#define CAN_IER_BEIE                            CAN_IER_BEIE_Msk                                /*!< desc BEIE */
#define CAN_IER_ALIE_Pos                        (9U)
#define CAN_IER_ALIE_Msk                        (0x1UL << CAN_IER_ALIE_Pos)                     /*!< 0x00000200 */
#define CAN_IER_ALIE                            CAN_IER_ALIE_Msk                                /*!< desc ALIE */
#define CAN_IER_EPIE_Pos                        (10U)
#define CAN_IER_EPIE_Msk                        (0x1UL << CAN_IER_EPIE_Pos)                     /*!< 0x00000400 */
#define CAN_IER_EPIE                            CAN_IER_EPIE_Msk                                /*!< desc EPIE */
#define CAN_IER_TTIE_Pos                        (11U)
#define CAN_IER_TTIE_Msk                        (0x1UL << CAN_IER_TTIE_Pos)                     /*!< 0x00000800 */
#define CAN_IER_TTIE                            CAN_IER_TTIE_Msk                                /*!< desc TTIE */
#define CAN_IER_WTIE_Pos                        (13U)
#define CAN_IER_WTIE_Msk                        (0x1UL << CAN_IER_WTIE_Pos)                     /*!< 0x00002000 */
#define CAN_IER_WTIE                            CAN_IER_WTIE_Msk                                /*!< desc WTIE */

/*!< CAN_TSR */
#define CAN_TSR_HANDLE_L_Pos                    (0U)
#define CAN_TSR_HANDLE_L_Msk                    (0xFFUL << CAN_TSR_HANDLE_L_Pos)                /*!< 0x000000FF */
#define CAN_TSR_HANDLE_L                        CAN_TSR_HANDLE_L_Msk                            /*!< HANDLE_L[7:0] bits (desc HANDLE_L) */
#define CAN_TSR_TSTAT_L_Pos                     (8U)
#define CAN_TSR_TSTAT_L_Msk                     (0x7UL << CAN_TSR_TSTAT_L_Pos)                  /*!< 0x00000700 */
#define CAN_TSR_TSTAT_L                         CAN_TSR_TSTAT_L_Msk                             /*!< TSTAT_L[10:8] bits (desc TSTAT_L) */
#define CAN_TSR_TSTAT_L_0                       (0x1UL << CAN_TSR_TSTAT_L_Pos)                  /*!< 0x00000100 */
#define CAN_TSR_TSTAT_L_1                       (0x2UL << CAN_TSR_TSTAT_L_Pos)                  /*!< 0x00000200 */
#define CAN_TSR_TSTAT_L_2                       (0x4UL << CAN_TSR_TSTAT_L_Pos)                  /*!< 0x00000400 */
#define CAN_TSR_HANDLE_H_Pos                    (16U)
#define CAN_TSR_HANDLE_H_Msk                    (0xFFUL << CAN_TSR_HANDLE_H_Pos)                /*!< 0x00FF0000 */
#define CAN_TSR_HANDLE_H                        CAN_TSR_HANDLE_H_Msk                            /*!< HANDLE_H[23:16] bits (desc HANDLE_H) */
#define CAN_TSR_TSTAT_H_Pos                     (24U)
#define CAN_TSR_TSTAT_H_Msk                     (0x7UL << CAN_TSR_TSTAT_H_Pos)                  /*!< 0x07000000 */
#define CAN_TSR_TSTAT_H                         CAN_TSR_TSTAT_H_Msk                             /*!< TSTAT_H[26:24] bits (desc TSTAT_H) */
#define CAN_TSR_TSTAT_H_0                       (0x1UL << CAN_TSR_TSTAT_H_Pos)                  /*!< 0x01000000 */
#define CAN_TSR_TSTAT_H_1                       (0x2UL << CAN_TSR_TSTAT_H_Pos)                  /*!< 0x02000000 */
#define CAN_TSR_TSTAT_H_2                       (0x4UL << CAN_TSR_TSTAT_H_Pos)                  /*!< 0x04000000 */

/*!< CAN_MCR */
#define CAN_MCR_BUSOFF_Pos                      (0U)
#define CAN_MCR_BUSOFF_Msk                      (0x1UL << CAN_MCR_BUSOFF_Pos)                   /*!< 0x00000001 */
#define CAN_MCR_BUSOFF                          CAN_MCR_BUSOFF_Msk                              /*!< desc BUSOFF */
#define CAN_MCR_LBMI_Pos                        (5U)
#define CAN_MCR_LBMI_Msk                        (0x1UL << CAN_MCR_LBMI_Pos)                     /*!< 0x00000020 */
#define CAN_MCR_LBMI                            CAN_MCR_LBMI_Msk                                /*!< desc LBMI */
#define CAN_MCR_LBME_Pos                        (6U)
#define CAN_MCR_LBME_Msk                        (0x1UL << CAN_MCR_LBME_Pos)                     /*!< 0x00000040 */
#define CAN_MCR_LBME                            CAN_MCR_LBME_Msk                                /*!< desc LBME */
#define CAN_MCR_RESET_Pos                       (7U)
#define CAN_MCR_RESET_Msk                       (0x1UL << CAN_MCR_RESET_Pos)                    /*!< 0x00000080 */
#define CAN_MCR_RESET                           CAN_MCR_RESET_Msk                               /*!< desc RESET */
#define CAN_MCR_TSA_Pos                         (8U)
#define CAN_MCR_TSA_Msk                         (0x1UL << CAN_MCR_TSA_Pos)                      /*!< 0x00000100 */
#define CAN_MCR_TSA                             CAN_MCR_TSA_Msk                                 /*!< desc TSA */
#define CAN_MCR_TSALL_Pos                       (9U)
#define CAN_MCR_TSALL_Msk                       (0x1UL << CAN_MCR_TSALL_Pos)                    /*!< 0x00000200 */
#define CAN_MCR_TSALL                           CAN_MCR_TSALL_Msk                               /*!< desc TSALL */
#define CAN_MCR_TSONE_Pos                       (10U)
#define CAN_MCR_TSONE_Msk                       (0x1UL << CAN_MCR_TSONE_Pos)                    /*!< 0x00000400 */
#define CAN_MCR_TSONE                           CAN_MCR_TSONE_Msk                               /*!< desc TSONE */
#define CAN_MCR_TPA_Pos                         (11U)
#define CAN_MCR_TPA_Msk                         (0x1UL << CAN_MCR_TPA_Pos)                      /*!< 0x00000800 */
#define CAN_MCR_TPA                             CAN_MCR_TPA_Msk                                 /*!< desc TPA */
#define CAN_MCR_TPE_Pos                         (12U)
#define CAN_MCR_TPE_Msk                         (0x1UL << CAN_MCR_TPE_Pos)                      /*!< 0x00001000 */
#define CAN_MCR_TPE                             CAN_MCR_TPE_Msk                                 /*!< desc TPE */
#define CAN_MCR_STBY_Pos                        (13U)
#define CAN_MCR_STBY_Msk                        (0x1UL << CAN_MCR_STBY_Pos)                     /*!< 0x00002000 */
#define CAN_MCR_STBY                            CAN_MCR_STBY_Msk                                /*!< desc STBY */
#define CAN_MCR_LOM_Pos                         (14U)
#define CAN_MCR_LOM_Msk                         (0x1UL << CAN_MCR_LOM_Pos)                      /*!< 0x00004000 */
#define CAN_MCR_LOM                             CAN_MCR_LOM_Msk                                 /*!< desc LOM */
#define CAN_MCR_TBSEL_Pos                       (15U)
#define CAN_MCR_TBSEL_Msk                       (0x1UL << CAN_MCR_TBSEL_Pos)                    /*!< 0x00008000 */
#define CAN_MCR_TBSEL                           CAN_MCR_TBSEL_Msk                               /*!< desc TBSEL */
#define CAN_MCR_TSSTAT_Pos                      (16U)
#define CAN_MCR_TSSTAT_Msk                      (0x3UL << CAN_MCR_TSSTAT_Pos)                   /*!< 0x00030000 */
#define CAN_MCR_TSSTAT                          CAN_MCR_TSSTAT_Msk                              /*!< TSSTAT[17:16] bits (desc TSSTAT) */
#define CAN_MCR_TSSTAT_0                        (0x1UL << CAN_MCR_TSSTAT_Pos)                   /*!< 0x00010000 */
#define CAN_MCR_TSSTAT_1                        (0x2UL << CAN_MCR_TSSTAT_Pos)                   /*!< 0x00020000 */
#define CAN_MCR_TSFF_Pos                        (18U)
#define CAN_MCR_TSFF_Msk                        (0x1UL << CAN_MCR_TSFF_Pos)                     /*!< 0x00040000 */
#define CAN_MCR_TSFF                            CAN_MCR_TSFF_Msk                                /*!< desc TSFF */
#define CAN_MCR_TTTBM_Pos                       (20U)
#define CAN_MCR_TTTBM_Msk                       (0x1UL << CAN_MCR_TTTBM_Pos)                    /*!< 0x00100000 */
#define CAN_MCR_TTTBM                           CAN_MCR_TTTBM_Msk                               /*!< desc TTTBM */
#define CAN_MCR_TSMODE_Pos                      (21U)
#define CAN_MCR_TSMODE_Msk                      (0x1UL << CAN_MCR_TSMODE_Pos)                   /*!< 0x00200000 */
#define CAN_MCR_TSMODE                          CAN_MCR_TSMODE_Msk                              /*!< desc TSMODE */
#define CAN_MCR_TSNEXT_Pos                      (22U)
#define CAN_MCR_TSNEXT_Msk                      (0x1UL << CAN_MCR_TSNEXT_Pos)                   /*!< 0x00400000 */
#define CAN_MCR_TSNEXT                          CAN_MCR_TSNEXT_Msk                              /*!< desc TSNEXT */
#define CAN_MCR_RSTAT_Pos                       (24U)
#define CAN_MCR_RSTAT_Msk                       (0x3UL << CAN_MCR_RSTAT_Pos)                    /*!< 0x03000000 */
#define CAN_MCR_RSTAT                           CAN_MCR_RSTAT_Msk                               /*!< RSTAT[25:24] bits (desc RSTAT) */
#define CAN_MCR_RSTAT_0                         (0x1UL << CAN_MCR_RSTAT_Pos)                    /*!< 0x01000000 */
#define CAN_MCR_RSTAT_1                         (0x2UL << CAN_MCR_RSTAT_Pos)                    /*!< 0x02000000 */
#define CAN_MCR_RBALL_Pos                       (27U)
#define CAN_MCR_RBALL_Msk                       (0x1UL << CAN_MCR_RBALL_Pos)                    /*!< 0x08000000 */
#define CAN_MCR_RBALL                           CAN_MCR_RBALL_Msk                               /*!< desc RBALL */
#define CAN_MCR_RREL_Pos                        (28U)
#define CAN_MCR_RREL_Msk                        (0x1UL << CAN_MCR_RREL_Pos)                     /*!< 0x10000000 */
#define CAN_MCR_RREL                            CAN_MCR_RREL_Msk                                /*!< desc RREL */
#define CAN_MCR_ROV_Pos                         (29U)
#define CAN_MCR_ROV_Msk                         (0x1UL << CAN_MCR_ROV_Pos)                      /*!< 0x20000000 */
#define CAN_MCR_ROV                             CAN_MCR_ROV_Msk                                 /*!< desc ROV */
#define CAN_MCR_ROM_Pos                         (30U)
#define CAN_MCR_ROM_Msk                         (0x1UL << CAN_MCR_ROM_Pos)                      /*!< 0x40000000 */
#define CAN_MCR_ROM                             CAN_MCR_ROM_Msk                                 /*!< desc ROM */
#define CAN_MCR_SACK_Pos                        (31U)
#define CAN_MCR_SACK_Msk                        (0x1UL << CAN_MCR_SACK_Pos)                     /*!< 0x80000000 */
#define CAN_MCR_SACK                            CAN_MCR_SACK_Msk                                /*!< desc SACK */

/*!< CAN_WECR */
#define CAN_WECR_EWL_Pos                        (0U)
#define CAN_WECR_EWL_Msk                        (0xFUL << CAN_WECR_EWL_Pos)                     /*!< 0x0000000F */
#define CAN_WECR_EWL                            CAN_WECR_EWL_Msk                                /*!< EWL[3:0] bits (desc EWL) */
#define CAN_WECR_EWL_0                          (0x1UL << CAN_WECR_EWL_Pos)                     /*!< 0x00000001 */
#define CAN_WECR_EWL_1                          (0x2UL << CAN_WECR_EWL_Pos)                     /*!< 0x00000002 */
#define CAN_WECR_EWL_2                          (0x4UL << CAN_WECR_EWL_Pos)                     /*!< 0x00000004 */
#define CAN_WECR_EWL_3                          (0x8UL << CAN_WECR_EWL_Pos)                     /*!< 0x00000008 */
#define CAN_WECR_AFWL_Pos                       (4U)
#define CAN_WECR_AFWL_Msk                       (0xFUL << CAN_WECR_AFWL_Pos)                    /*!< 0x000000F0 */
#define CAN_WECR_AFWL                           CAN_WECR_AFWL_Msk                               /*!< AFWL[7:4] bits (desc AFWL) */
#define CAN_WECR_AFWL_0                         (0x1UL << CAN_WECR_AFWL_Pos)                    /*!< 0x00000010 */
#define CAN_WECR_AFWL_1                         (0x2UL << CAN_WECR_AFWL_Pos)                    /*!< 0x00000020 */
#define CAN_WECR_AFWL_2                         (0x4UL << CAN_WECR_AFWL_Pos)                    /*!< 0x00000040 */
#define CAN_WECR_AFWL_3                         (0x8UL << CAN_WECR_AFWL_Pos)                    /*!< 0x00000080 */
#define CAN_WECR_ALC_Pos                        (8U)
#define CAN_WECR_ALC_Msk                        (0x1FUL << CAN_WECR_ALC_Pos)                    /*!< 0x00001F00 */
#define CAN_WECR_ALC                            CAN_WECR_ALC_Msk                                /*!< ALC[12:8] bits (desc ALC) */
#define CAN_WECR_ALC_0                          (0x1UL << CAN_WECR_ALC_Pos)                     /*!< 0x00000100 */
#define CAN_WECR_ALC_1                          (0x2UL << CAN_WECR_ALC_Pos)                     /*!< 0x00000200 */
#define CAN_WECR_ALC_2                          (0x4UL << CAN_WECR_ALC_Pos)                     /*!< 0x00000400 */
#define CAN_WECR_ALC_3                          (0x8UL << CAN_WECR_ALC_Pos)                     /*!< 0x00000800 */
#define CAN_WECR_ALC_4                          (0x10UL << CAN_WECR_ALC_Pos)                    /*!< 0x00001000 */
#define CAN_WECR_KOER_Pos                       (13U)
#define CAN_WECR_KOER_Msk                       (0x7UL << CAN_WECR_KOER_Pos)                    /*!< 0x0000E000 */
#define CAN_WECR_KOER                           CAN_WECR_KOER_Msk                               /*!< KOER[15:13] bits (desc KOER) */
#define CAN_WECR_KOER_0                         (0x1UL << CAN_WECR_KOER_Pos)                    /*!< 0x00002000 */
#define CAN_WECR_KOER_1                         (0x2UL << CAN_WECR_KOER_Pos)                    /*!< 0x00004000 */
#define CAN_WECR_KOER_2                         (0x4UL << CAN_WECR_KOER_Pos)                    /*!< 0x00008000 */
#define CAN_WECR_RECNT_Pos                      (16U)
#define CAN_WECR_RECNT_Msk                      (0xFFUL << CAN_WECR_RECNT_Pos)                  /*!< 0x00FF0000 */
#define CAN_WECR_RECNT                          CAN_WECR_RECNT_Msk                              /*!< RECNT[23:16] bits (desc RECNT) */
#define CAN_WECR_TECNT_Pos                      (24U)
#define CAN_WECR_TECNT_Msk                      (0xFFUL << CAN_WECR_TECNT_Pos)                  /*!< 0xFF000000 */
#define CAN_WECR_TECNT                          CAN_WECR_TECNT_Msk                              /*!< TECNT[31:24] bits (desc TECNT) */

/*!< CAN_REFMSG */
#define CAN_REFMSG_REF_ID_Pos                   (0U)
#define CAN_REFMSG_REF_ID_Msk                   (0x1FFFFFFFUL << CAN_REFMSG_REF_ID_Pos)         /*!< 0x1FFFFFFF */
#define CAN_REFMSG_REF_ID                       CAN_REFMSG_REF_ID_Msk                           /*!< REF_ID[28:0] bits (desc REF_ID) */
#define CAN_REFMSG_REF_IDE_Pos                  (31U)
#define CAN_REFMSG_REF_IDE_Msk                  (0x1UL << CAN_REFMSG_REF_IDE_Pos)               /*!< 0x80000000 */
#define CAN_REFMSG_REF_IDE                      CAN_REFMSG_REF_IDE_Msk                          /*!< desc REF_IDE */

/*!< CAN_TTCR */
#define CAN_TTCR_TTPTR_Pos                      (0U)
#define CAN_TTCR_TTPTR_Msk                      (0x3FUL << CAN_TTCR_TTPTR_Pos)                  /*!< 0x0000003F */
#define CAN_TTCR_TTPTR                          CAN_TTCR_TTPTR_Msk                              /*!< TTPTR[5:0] bits (desc TTPTR) */
#define CAN_TTCR_TTPTR_0                        (0x1UL << CAN_TTCR_TTPTR_Pos)                   /*!< 0x00000001 */
#define CAN_TTCR_TTPTR_1                        (0x2UL << CAN_TTCR_TTPTR_Pos)                   /*!< 0x00000002 */
#define CAN_TTCR_TTPTR_2                        (0x4UL << CAN_TTCR_TTPTR_Pos)                   /*!< 0x00000004 */
#define CAN_TTCR_TTPTR_3                        (0x8UL << CAN_TTCR_TTPTR_Pos)                   /*!< 0x00000008 */
#define CAN_TTCR_TTPTR_4                        (0x10UL << CAN_TTCR_TTPTR_Pos)                  /*!< 0x00000010 */
#define CAN_TTCR_TTPTR_5                        (0x20UL << CAN_TTCR_TTPTR_Pos)                  /*!< 0x00000020 */
#define CAN_TTCR_TTYPE_Pos                      (8U)
#define CAN_TTCR_TTYPE_Msk                      (0x7UL << CAN_TTCR_TTYPE_Pos)                   /*!< 0x00000700 */
#define CAN_TTCR_TTYPE                          CAN_TTCR_TTYPE_Msk                              /*!< TTYPE[10:8] bits (desc TTYPE) */
#define CAN_TTCR_TTYPE_0                        (0x1UL << CAN_TTCR_TTYPE_Pos)                   /*!< 0x00000100 */
#define CAN_TTCR_TTYPE_1                        (0x2UL << CAN_TTCR_TTYPE_Pos)                   /*!< 0x00000200 */
#define CAN_TTCR_TTYPE_2                        (0x4UL << CAN_TTCR_TTYPE_Pos)                   /*!< 0x00000400 */
#define CAN_TTCR_TEW_Pos                        (12U)
#define CAN_TTCR_TEW_Msk                        (0xFUL << CAN_TTCR_TEW_Pos)                     /*!< 0x0000F000 */
#define CAN_TTCR_TEW                            CAN_TTCR_TEW_Msk                                /*!< TEW[15:12] bits (desc TEW) */
#define CAN_TTCR_TEW_0                          (0x1UL << CAN_TTCR_TEW_Pos)                     /*!< 0x00001000 */
#define CAN_TTCR_TEW_1                          (0x2UL << CAN_TTCR_TEW_Pos)                     /*!< 0x00002000 */
#define CAN_TTCR_TEW_2                          (0x4UL << CAN_TTCR_TEW_Pos)                     /*!< 0x00004000 */
#define CAN_TTCR_TEW_3                          (0x8UL << CAN_TTCR_TEW_Pos)                     /*!< 0x00008000 */
#define CAN_TTCR_TBPTR_Pos                      (16U)
#define CAN_TTCR_TBPTR_Msk                      (0x3FUL << CAN_TTCR_TBPTR_Pos)                  /*!< 0x003F0000 */
#define CAN_TTCR_TBPTR                          CAN_TTCR_TBPTR_Msk                              /*!< TBPTR[21:16] bits (desc TBPTR) */
#define CAN_TTCR_TBPTR_0                        (0x1UL << CAN_TTCR_TBPTR_Pos)                   /*!< 0x00010000 */
#define CAN_TTCR_TBPTR_1                        (0x2UL << CAN_TTCR_TBPTR_Pos)                   /*!< 0x00020000 */
#define CAN_TTCR_TBPTR_2                        (0x4UL << CAN_TTCR_TBPTR_Pos)                   /*!< 0x00040000 */
#define CAN_TTCR_TBPTR_3                        (0x8UL << CAN_TTCR_TBPTR_Pos)                   /*!< 0x00080000 */
#define CAN_TTCR_TBPTR_4                        (0x10UL << CAN_TTCR_TBPTR_Pos)                  /*!< 0x00100000 */
#define CAN_TTCR_TBPTR_5                        (0x20UL << CAN_TTCR_TBPTR_Pos)                  /*!< 0x00200000 */
#define CAN_TTCR_TBF_Pos                        (22U)
#define CAN_TTCR_TBF_Msk                        (0x1UL << CAN_TTCR_TBF_Pos)                     /*!< 0x00400000 */
#define CAN_TTCR_TBF                            CAN_TTCR_TBF_Msk                                /*!< desc TBF */
#define CAN_TTCR_TBE_Pos                        (23U)
#define CAN_TTCR_TBE_Msk                        (0x1UL << CAN_TTCR_TBE_Pos)                     /*!< 0x00800000 */
#define CAN_TTCR_TBE                            CAN_TTCR_TBE_Msk                                /*!< desc TBE */
#define CAN_TTCR_TTEN_Pos                       (24U)
#define CAN_TTCR_TTEN_Msk                       (0x1UL << CAN_TTCR_TTEN_Pos)                    /*!< 0x01000000 */
#define CAN_TTCR_TTEN                           CAN_TTCR_TTEN_Msk                               /*!< desc TTEN */
#define CAN_TTCR_T_PRESC_Pos                    (25U)
#define CAN_TTCR_T_PRESC_Msk                    (0x3UL << CAN_TTCR_T_PRESC_Pos)                 /*!< 0x06000000 */
#define CAN_TTCR_T_PRESC                        CAN_TTCR_T_PRESC_Msk                            /*!< T_PRESC[26:25] bits (desc T_PRESC) */
#define CAN_TTCR_T_PRESC_0                      (0x1UL << CAN_TTCR_T_PRESC_Pos)                 /*!< 0x02000000 */
#define CAN_TTCR_T_PRESC_1                      (0x2UL << CAN_TTCR_T_PRESC_Pos)                 /*!< 0x04000000 */

/*!< CAN_TTTR */
#define CAN_TTTR_TT_TRIG_Pos                    (0U)
#define CAN_TTTR_TT_TRIG_Msk                    (0xFFFFUL << CAN_TTTR_TT_TRIG_Pos)              /*!< 0x0000FFFF */
#define CAN_TTTR_TT_TRIG                        CAN_TTTR_TT_TRIG_Msk                            /*!< TT_TRIG[15:0] bits (desc TT_TRIG) */
#define CAN_TTTR_TT_WTRIG_Pos                   (16U)
#define CAN_TTTR_TT_WTRIG_Msk                   (0xFFFFUL << CAN_TTTR_TT_WTRIG_Pos)             /*!< 0xFFFF0000 */
#define CAN_TTTR_TT_WTRIG                       CAN_TTTR_TT_WTRIG_Msk                           /*!< TT_WTRIG[31:16] bits (desc TT_WTRIG) */

/*!< CAN_SCMS */
#define CAN_SCMS_FSTIM_Pos                      (1U)
#define CAN_SCMS_FSTIM_Msk                      (0x7UL << CAN_SCMS_FSTIM_Pos)                   /*!< 0x0000000E */
#define CAN_SCMS_FSTIM                          CAN_SCMS_FSTIM_Msk                              /*!< FSTIM[3:1] bits (desc FSTIM) */
#define CAN_SCMS_FSTIM_0                        (0x1UL << CAN_SCMS_FSTIM_Pos)                   /*!< 0x00000002 */
#define CAN_SCMS_FSTIM_1                        (0x2UL << CAN_SCMS_FSTIM_Pos)                   /*!< 0x00000004 */
#define CAN_SCMS_FSTIM_2                        (0x4UL << CAN_SCMS_FSTIM_Pos)                   /*!< 0x00000008 */
#define CAN_SCMS_ACFA_Pos                       (24U)
#define CAN_SCMS_ACFA_Msk                       (0x1UL << CAN_SCMS_ACFA_Pos)                    /*!< 0x01000000 */
#define CAN_SCMS_ACFA                           CAN_SCMS_ACFA_Msk                               /*!< desc ACFA */
#define CAN_SCMS_TXS_Pos                        (25U)
#define CAN_SCMS_TXS_Msk                        (0x1UL << CAN_SCMS_TXS_Pos)                     /*!< 0x02000000 */
#define CAN_SCMS_TXS                            CAN_SCMS_TXS_Msk                                /*!< desc TXS */
#define CAN_SCMS_TXB_Pos                        (26U)
#define CAN_SCMS_TXB_Msk                        (0x1UL << CAN_SCMS_TXB_Pos)                     /*!< 0x04000000 */
#define CAN_SCMS_TXB                            CAN_SCMS_TXB_Msk                                /*!< desc TXB */
#define CAN_SCMS_HELOC_Pos                      (27U)
#define CAN_SCMS_HELOC_Msk                      (0x3UL << CAN_SCMS_HELOC_Pos)                   /*!< 0x18000000 */
#define CAN_SCMS_HELOC                          CAN_SCMS_HELOC_Msk                              /*!< HELOC[28:27] bits (desc HELOC) */
#define CAN_SCMS_HELOC_0                        (0x1UL << CAN_SCMS_HELOC_Pos)                   /*!< 0x08000000 */
#define CAN_SCMS_HELOC_1                        (0x2UL << CAN_SCMS_HELOC_Pos)                   /*!< 0x10000000 */

/*!< CAN_ACFCR */
#define CAN_ACFCR_ACFADR_Pos                    (0U)
#define CAN_ACFCR_ACFADR_Msk                    (0xFUL << CAN_ACFCR_ACFADR_Pos)                 /*!< 0x0000000F */
#define CAN_ACFCR_ACFADR                        CAN_ACFCR_ACFADR_Msk                            /*!< ACFADR[3:0] bits (desc ACFADR) */
#define CAN_ACFCR_ACFADR_0                      (0x1UL << CAN_ACFCR_ACFADR_Pos)                 /*!< 0x00000001 */
#define CAN_ACFCR_ACFADR_1                      (0x2UL << CAN_ACFCR_ACFADR_Pos)                 /*!< 0x00000002 */
#define CAN_ACFCR_ACFADR_2                      (0x4UL << CAN_ACFCR_ACFADR_Pos)                 /*!< 0x00000004 */
#define CAN_ACFCR_ACFADR_3                      (0x8UL << CAN_ACFCR_ACFADR_Pos)                 /*!< 0x00000008 */
#define CAN_ACFCR_AE_0_Pos                      (16U)
#define CAN_ACFCR_AE_0_Msk                      (0x1UL << CAN_ACFCR_AE_0_Pos)                   /*!< 0x00010000 */
#define CAN_ACFCR_AE_0                          CAN_ACFCR_AE_0_Msk                              /*!< desc AE_0 */
#define CAN_ACFCR_AE_1_Pos                      (17U)
#define CAN_ACFCR_AE_1_Msk                      (0x1UL << CAN_ACFCR_AE_1_Pos)                   /*!< 0x00020000 */
#define CAN_ACFCR_AE_1                          CAN_ACFCR_AE_1_Msk                              /*!< desc AE_1 */
#define CAN_ACFCR_AE_2_Pos                      (18U)
#define CAN_ACFCR_AE_2_Msk                      (0x1UL << CAN_ACFCR_AE_2_Pos)                   /*!< 0x00040000 */
#define CAN_ACFCR_AE_2                          CAN_ACFCR_AE_2_Msk                              /*!< desc AE_2 */
#define CAN_ACFCR_AE_3_Pos                      (19U)
#define CAN_ACFCR_AE_3_Msk                      (0x1UL << CAN_ACFCR_AE_3_Pos)                   /*!< 0x00080000 */
#define CAN_ACFCR_AE_3                          CAN_ACFCR_AE_3_Msk                              /*!< desc AE_3 */
#define CAN_ACFCR_AE_4_Pos                      (20U)
#define CAN_ACFCR_AE_4_Msk                      (0x1UL << CAN_ACFCR_AE_4_Pos)                   /*!< 0x00100000 */
#define CAN_ACFCR_AE_4                          CAN_ACFCR_AE_4_Msk                              /*!< desc AE_4 */
#define CAN_ACFCR_AE_5_Pos                      (21U)
#define CAN_ACFCR_AE_5_Msk                      (0x1UL << CAN_ACFCR_AE_5_Pos)                   /*!< 0x00200000 */
#define CAN_ACFCR_AE_5                          CAN_ACFCR_AE_5_Msk                              /*!< desc AE_5 */
#define CAN_ACFCR_AE_6_Pos                      (22U)
#define CAN_ACFCR_AE_6_Msk                      (0x1UL << CAN_ACFCR_AE_6_Pos)                   /*!< 0x00400000 */
#define CAN_ACFCR_AE_6                          CAN_ACFCR_AE_6_Msk                              /*!< desc AE_6 */
#define CAN_ACFCR_AE_7_Pos                      (23U)
#define CAN_ACFCR_AE_7_Msk                      (0x1UL << CAN_ACFCR_AE_7_Pos)                   /*!< 0x00800000 */
#define CAN_ACFCR_AE_7                          CAN_ACFCR_AE_7_Msk                              /*!< desc AE_7 */
#define CAN_ACFCR_AE_8_Pos                      (24U)
#define CAN_ACFCR_AE_8_Msk                      (0x1UL << CAN_ACFCR_AE_8_Pos)                   /*!< 0x01000000 */
#define CAN_ACFCR_AE_8                          CAN_ACFCR_AE_8_Msk                              /*!< desc AE_8 */
#define CAN_ACFCR_AE_9_Pos                      (25U)
#define CAN_ACFCR_AE_9_Msk                      (0x1UL << CAN_ACFCR_AE_9_Pos)                   /*!< 0x02000000 */
#define CAN_ACFCR_AE_9                          CAN_ACFCR_AE_9_Msk                              /*!< desc AE_9 */
#define CAN_ACFCR_AE_10_Pos                     (26U)
#define CAN_ACFCR_AE_10_Msk                     (0x1UL << CAN_ACFCR_AE_10_Pos)                  /*!< 0x04000000 */
#define CAN_ACFCR_AE_10                         CAN_ACFCR_AE_10_Msk                             /*!< desc AE_10 */
#define CAN_ACFCR_AE_11_Pos                     (27U)
#define CAN_ACFCR_AE_11_Msk                     (0x1UL << CAN_ACFCR_AE_11_Pos)                  /*!< 0x08000000 */
#define CAN_ACFCR_AE_11                         CAN_ACFCR_AE_11_Msk                             /*!< desc AE_11 */

/****************************************************************************/
/*                                                                          */
/*                      Analog Comparators (COMP)                           */
/*                                                                          */
/****************************************************************************/
/**********************  Bit definition for COMP_CSR register  **************/
#define COMP_CSR_EN_Pos            (0U)
#define COMP_CSR_EN_Msk            (0x1UL << COMP_CSR_EN_Pos)                  /*!< 0x00000001 */
#define COMP_CSR_EN                COMP_CSR_EN_Msk                             /*!< Comparator enable */
#define COMP_CSR_COMP1_EN          COMP_CSR_EN
#define COMP_CSR_COMP2_EN          COMP_CSR_EN

#define COMP_CSR_INNSEL_Pos        (2U)
#define COMP_CSR_INNSEL_Msk        (0xFUL << COMP_CSR_INNSEL_Pos)              /*!< 0x0000003C */
#define COMP_CSR_INNSEL            COMP_CSR_INNSEL_Msk                         /*!< desc INNSEL */
#define COMP_CSR_INNSEL_0          (0x1UL << COMP_CSR_INNSEL_Pos)              /*!< 0x00000004 */
#define COMP_CSR_INNSEL_1          (0x2UL << COMP_CSR_INNSEL_Pos)              /*!< 0x00000008 */
#define COMP_CSR_INNSEL_2          (0x4UL << COMP_CSR_INNSEL_Pos)              /*!< 0x00000010 */
#define COMP_CSR_INNSEL_3          (0x8UL << COMP_CSR_INNSEL_Pos)              /*!< 0x00000020 */

#define COMP_CSR_INPSEL_Pos        (6U)
#define COMP_CSR_INPSEL_Msk        (0xFUL << COMP_CSR_INPSEL_Pos)              /*!< 0x000003C0 */
#define COMP_CSR_INPSEL            COMP_CSR_INPSEL_Msk                         /*!< Comparator plus minus selection */
#define COMP_CSR_INPSEL_0          (0x1UL << COMP_CSR_INPSEL_Pos)              /*!< 0x00000040 */
#define COMP_CSR_INPSEL_1          (0x2UL << COMP_CSR_INPSEL_Pos)              /*!< 0x00000080 */
#define COMP_CSR_INPSEL_2          (0x4UL << COMP_CSR_INPSEL_Pos)              /*!< 0x00000100 */
#define COMP_CSR_INPSEL_3          (0x8UL << COMP_CSR_INPSEL_Pos)              /*!< 0x00000200 */

#define COMP_CSR_WINMODE_Pos       (11U)
#define COMP_CSR_WINMODE_Msk       (0x1UL << COMP_CSR_WINMODE_Pos)             /*!< 0x00000800 */
#define COMP_CSR_WINMODE           COMP_CSR_WINMODE_Msk                        /*!< Pair of comparators window mode. Bit intended to be used with COMP common instance (COMP_Common_TypeDef) */

#define COMP_CSR_POLARITY_Pos      (15U)
#define COMP_CSR_POLARITY_Msk      (0x1UL << COMP_CSR_POLARITY_Pos)            /*!< 0x00008000 */
#define COMP_CSR_POLARITY          COMP_CSR_POLARITY_Msk                       /*!< Comparator output polarity */

#define COMP_CSR_HYST_Pos          (16U)
#define COMP_CSR_HYST_Msk          (0x1UL << COMP_CSR_HYST_Pos)                /*!< 0x00010000 */
#define COMP_CSR_HYST              COMP_CSR_HYST_Msk                           /*!< Comparator hysteresis enable */
/*DEFINE FOR COMP1 CSR_HYST*/
#define COMP1_CSR_HYST_Pos         (16U)
#define COMP1_CSR_HYST_Msk         (0x1UL << COMP1_CSR_HYST_Pos)                /*!< 0x00010000 */
#define COMP1_CSR_HYST             COMP1_CSR_HYST_Msk                           /*!< Comparator1 hysteresis enable */
/*DEFINE FOR COMP2 CSR_HYST*/
#define COMP2_CSR_HYST_Pos         (1U)
#define COMP2_CSR_HYST_Msk         (0x1UL << COMP2_CSR_HYST_Pos)                /*!< 0x00000002 */
#define COMP2_CSR_HYST             COMP2_CSR_HYST_Msk                           /*!< Comparator2 hysteresis enable */
/*DEFINE FOR COMP3 CSR_HYST*/
#define COMP3_CSR_HYST_Pos         (1U)
#define COMP3_CSR_HYST_Msk         (0x1UL << COMP3_CSR_HYST_Pos)                /*!< 0x00000002 */
#define COMP3_CSR_HYST             COMP3_CSR_HYST_Msk                           /*!< Comparator3 hysteresis enable */

#define COMP_CSR_PWRMODE_Pos       (18U)
#define COMP_CSR_PWRMODE_Msk       (0x3UL << COMP_CSR_PWRMODE_Pos)             /*!< 0x000C0000 */
#define COMP_CSR_PWRMODE           COMP_CSR_PWRMODE_Msk                        /*!< Comparator power mode */
#define COMP_CSR_PWRMODE_0         (0x1UL << COMP_CSR_PWRMODE_Pos)             /*!< 0x00040000 */
#define COMP_CSR_PWRMODE_1         (0x2UL << COMP_CSR_PWRMODE_Pos)             /*!< 0x00080000 */

#define COMP_CSR_VCDIV_Pos         (20U)
#define COMP_CSR_VCDIV_Msk         (0x3FUL << COMP_CSR_VCDIV_Pos)              /*!< 0x03F00000 */
#define COMP_CSR_VCDIV             COMP_CSR_VCDIV_Msk                          /*!< VCDIV[25:20] bits (desc VCDIV) */
#define COMP_CSR_VCDIV_0           (0x1UL << COMP_CSR_VCDIV_Pos)               /*!< 0x00100000 */
#define COMP_CSR_VCDIV_1           (0x2UL << COMP_CSR_VCDIV_Pos)               /*!< 0x00200000 */
#define COMP_CSR_VCDIV_2           (0x4UL << COMP_CSR_VCDIV_Pos)               /*!< 0x00400000 */
#define COMP_CSR_VCDIV_3           (0x8UL << COMP_CSR_VCDIV_Pos)               /*!< 0x00800000 */
#define COMP_CSR_VCDIV_4           (0x10UL << COMP_CSR_VCDIV_Pos)              /*!< 0x01000000 */
#define COMP_CSR_VCDIV_5           (0x20UL << COMP_CSR_VCDIV_Pos)              /*!< 0x02000000 */

#define COMP_CSR_VCDIV_EN_Pos      (26U)
#define COMP_CSR_VCDIV_EN_Msk      (0x1UL << COMP_CSR_VCDIV_EN_Pos)            /*!< 0x04000000 */
#define COMP_CSR_VCDIV_EN          COMP_CSR_VCDIV_EN_Msk                       /*!< desc VCDIV_EN */

#define COMP_CSR_VCSEL_Pos         (27U)
#define COMP_CSR_VCSEL_Msk         (0x1UL << COMP_CSR_VCSEL_Pos)               /*!< 0x08000000 */
#define COMP_CSR_VCSEL             COMP_CSR_VCSEL_Msk                          /*!< desc VCSEL */

#define COMP_CSR_COMP_OUT_Pos      (30U)
#define COMP_CSR_COMP_OUT_Msk      (0x1UL << COMP_CSR_COMP_OUT_Pos)            /*!< 0x40000000 */
#define COMP_CSR_COMP_OUT          COMP_CSR_COMP_OUT_Msk                       /*!< desc COMP_OUT */

/**********************  Bit definition for COMP_FR register  ***************/
#define COMP_FR_FLTEN_Pos          (0U)
#define COMP_FR_FLTEN_Msk          (0x1UL << COMP_FR_FLTEN_Pos)                /*!< 0x00000001 */
#define COMP_FR_FLTEN              COMP_FR_FLTEN_Msk                           /*!< Comparator filter enable */
#define COMP_FR_FLTCNT_Pos         (16U)
#define COMP_FR_FLTCNT_Msk         (0xFFFFUL << COMP_FR_FLTCNT_Pos)            /*!< 0xFFFF0000 */
#define COMP_FR_FLTCNT             COMP_FR_FLTCNT_Msk                          /*!< Comparator filter counter */

/****************************************************************************/
/*                                                                          */
/*                          CRC calculation unit (CRC)                      */
/*                                                                          */
/****************************************************************************/
/*******************  Bit definition for CRC_DR register  *******************/
#define CRC_DR_DR_Pos            (0U)
#define CRC_DR_DR_Msk            (0xFFFFFFFFUL << CRC_DR_DR_Pos)                /*!< 0xFFFFFFFF */
#define CRC_DR_DR                CRC_DR_DR_Msk                                  /*!< Data register bits */

/*******************  Bit definition for CRC_IDR register  ******************/
#define CRC_IDR_IDR_Pos          (0U)
#define CRC_IDR_IDR_Msk          (0xFFUL << CRC_IDR_IDR_Pos)                    /*!< 0xFFFFFFFF */
#define CRC_IDR_IDR              CRC_IDR_IDR_Msk                                /*!< General-purpose 8-bit data register bits */

/********************  Bit definition for CRC_CR register  ******************/
#define CRC_CR_RESET_Pos         (0U)
#define CRC_CR_RESET_Msk         (0x1UL << CRC_CR_RESET_Pos)                    /*!< 0x00000001 */
#define CRC_CR_RESET             CRC_CR_RESET_Msk                               /*!< RESET the CRC computation unit bit */

/****************************************************************************/
/*                                                                          */
/*                       Clock Triming Controller(CTC)                      */
/*                                                                          */
/****************************************************************************/
/*********************  Bits Define For Peripheral CTC  *********************/
/*!< CTC_CTL0 */
#define CTC_CTL0_CKOKIE_Pos                       (0U)
#define CTC_CTL0_CKOKIE_Msk                       (0x1UL << CTC_CTL0_CKOKIE_Pos)                    /*!< 0x00000001 */
#define CTC_CTL0_CKOKIE                           CTC_CTL0_CKOKIE_Msk                               /*!< desc CKOKIE */
#define CTC_CTL0_CKWARNIE_Pos                     (1U)
#define CTC_CTL0_CKWARNIE_Msk                     (0x1UL << CTC_CTL0_CKWARNIE_Pos)                  /*!< 0x00000002 */
#define CTC_CTL0_CKWARNIE                         CTC_CTL0_CKWARNIE_Msk                             /*!< desc CKWARNIE */
#define CTC_CTL0_ERRIE_Pos                        (2U)
#define CTC_CTL0_ERRIE_Msk                        (0x1UL << CTC_CTL0_ERRIE_Pos)                     /*!< 0x00000004 */
#define CTC_CTL0_ERRIE                            CTC_CTL0_ERRIE_Msk                                /*!< desc ERRIE */
#define CTC_CTL0_EREFIE_Pos                       (3U)
#define CTC_CTL0_EREFIE_Msk                       (0x1UL << CTC_CTL0_EREFIE_Pos)                    /*!< 0x00000008 */
#define CTC_CTL0_EREFIE                           CTC_CTL0_EREFIE_Msk                               /*!< desc EREFIE */
#define CTC_CTL0_CNTEN_Pos                        (5U)
#define CTC_CTL0_CNTEN_Msk                        (0x1UL << CTC_CTL0_CNTEN_Pos)                     /*!< 0x00000020 */
#define CTC_CTL0_CNTEN                            CTC_CTL0_CNTEN_Msk                                /*!< desc CNTEN */
#define CTC_CTL0_AUTOTRIM_Pos                     (6U)
#define CTC_CTL0_AUTOTRIM_Msk                     (0x1UL << CTC_CTL0_AUTOTRIM_Pos)                  /*!< 0x00000040 */
#define CTC_CTL0_AUTOTRIM                         CTC_CTL0_AUTOTRIM_Msk                             /*!< desc AUTOTRIM */
#define CTC_CTL0_SWREFPUL_Pos                     (7U)
#define CTC_CTL0_SWREFPUL_Msk                     (0x1UL << CTC_CTL0_SWREFPUL_Pos)                  /*!< 0x00000080 */
#define CTC_CTL0_SWREFPUL                         CTC_CTL0_SWREFPUL_Msk                             /*!< desc SWREFPUL */
#define CTC_CTL0_TRIMVALUE_Pos                    (8U)
#define CTC_CTL0_TRIMVALUE_Msk                    (0x3FUL << CTC_CTL0_TRIMVALUE_Pos)                /*!< 0x00003F00 */
#define CTC_CTL0_TRIMVALUE                        CTC_CTL0_TRIMVALUE_Msk                            /*!< TRIMVALUE[13:8] bits (desc TRIMVALUE) */
#define CTC_CTL0_TRIMVALUE_0                      (0x1UL << CTC_CTL0_TRIMVALUE_Pos)                 /*!< 0x00000100 */
#define CTC_CTL0_TRIMVALUE_1                      (0x2UL << CTC_CTL0_TRIMVALUE_Pos)                 /*!< 0x00000200 */
#define CTC_CTL0_TRIMVALUE_2                      (0x4UL << CTC_CTL0_TRIMVALUE_Pos)                 /*!< 0x00000400 */
#define CTC_CTL0_TRIMVALUE_3                      (0x8UL << CTC_CTL0_TRIMVALUE_Pos)                 /*!< 0x00000800 */
#define CTC_CTL0_TRIMVALUE_4                      (0x10UL << CTC_CTL0_TRIMVALUE_Pos)                /*!< 0x00001000 */
#define CTC_CTL0_TRIMVALUE_5                      (0x20UL << CTC_CTL0_TRIMVALUE_Pos)                /*!< 0x00002000 */
#define CTC_CTL0_TRIMVALUE_6                      (0x40UL << CTC_CTL0_TRIMVALUE_Pos)                /*!< 0x00004000 */

/*!< CTC_CTL1 */
#define CTC_CTL1_RLVALUE_Pos                      (0U)
#define CTC_CTL1_RLVALUE_Msk                      (0xFFFFUL << CTC_CTL1_RLVALUE_Pos)                /*!< 0x0000FFFF */
#define CTC_CTL1_RLVALUE                          CTC_CTL1_RLVALUE_Msk                              /*!< RLVALUE[15:0] bits (desc RLVALUE) */
#define CTC_CTL1_CKLIM_Pos                        (16U)
#define CTC_CTL1_CKLIM_Msk                        (0xFFUL << CTC_CTL1_CKLIM_Pos)                    /*!< 0x00FF0000 */
#define CTC_CTL1_CKLIM                            CTC_CTL1_CKLIM_Msk                                /*!< CKLIM[23:16] bits (desc CKLIM) */
#define CTC_CTL1_REFPSC_Pos                       (24U)
#define CTC_CTL1_REFPSC_Msk                       (0x7UL << CTC_CTL1_REFPSC_Pos)                    /*!< 0x07000000 */
#define CTC_CTL1_REFPSC                           CTC_CTL1_REFPSC_Msk                               /*!< REFPSC[26:24] bits (desc REFPSC) */
#define CTC_CTL1_REFPSC_0                         (0x1UL << CTC_CTL1_REFPSC_Pos)                    /*!< 0x01000000 */
#define CTC_CTL1_REFPSC_1                         (0x2UL << CTC_CTL1_REFPSC_Pos)                    /*!< 0x02000000 */
#define CTC_CTL1_REFPSC_2                         (0x4UL << CTC_CTL1_REFPSC_Pos)                    /*!< 0x04000000 */
#define CTC_CTL1_REFSEL_Pos                       (28U)
#define CTC_CTL1_REFSEL_Msk                       (0x3UL << CTC_CTL1_REFSEL_Pos)                    /*!< 0x30000000 */
#define CTC_CTL1_REFSEL                           CTC_CTL1_REFSEL_Msk                               /*!< REFSEL[29:28] bits (desc REFSEL) */
#define CTC_CTL1_REFSEL_0                         (0x1UL << CTC_CTL1_REFSEL_Pos)                    /*!< 0x10000000 */
#define CTC_CTL1_REFSEL_1                         (0x2UL << CTC_CTL1_REFSEL_Pos)                    /*!< 0x20000000 */
#define CTC_CTL1_REFPOL_Pos                       (31U)
#define CTC_CTL1_REFPOL_Msk                       (0x1UL << CTC_CTL1_REFPOL_Pos)                    /*!< 0x80000000 */
#define CTC_CTL1_REFPOL                           CTC_CTL1_REFPOL_Msk                               /*!< desc REFPOL */

/*!< CTC_SR */
#define CTC_SR_CKOKIF_Pos                         (0U)
#define CTC_SR_CKOKIF_Msk                         (0x1UL << CTC_SR_CKOKIF_Pos)                      /*!< 0x00000001 */
#define CTC_SR_CKOKIF                             CTC_SR_CKOKIF_Msk                                 /*!< desc CKOKIF */
#define CTC_SR_CKWARNIF_Pos                       (1U)
#define CTC_SR_CKWARNIF_Msk                       (0x1UL << CTC_SR_CKWARNIF_Pos)                    /*!< 0x00000002 */
#define CTC_SR_CKWARNIF                           CTC_SR_CKWARNIF_Msk                               /*!< desc CKWARNIF */
#define CTC_SR_ERRIF_Pos                          (2U)
#define CTC_SR_ERRIF_Msk                          (0x1UL << CTC_SR_ERRIF_Pos)                       /*!< 0x00000004 */
#define CTC_SR_ERRIF                              CTC_SR_ERRIF_Msk                                  /*!< desc ERRIF */
#define CTC_SR_EREFIF_Pos                         (3U)
#define CTC_SR_EREFIF_Msk                         (0x1UL << CTC_SR_EREFIF_Pos)                      /*!< 0x00000008 */
#define CTC_SR_EREFIF                             CTC_SR_EREFIF_Msk                                 /*!< desc EREFIF */
#define CTC_SR_CKERR_Pos                          (8U)
#define CTC_SR_CKERR_Msk                          (0x1UL << CTC_SR_CKERR_Pos)                       /*!< 0x00000100 */
#define CTC_SR_CKERR                              CTC_SR_CKERR_Msk                                  /*!< desc CKERR */
#define CTC_SR_REFMISS_Pos                        (9U)
#define CTC_SR_REFMISS_Msk                        (0x1UL << CTC_SR_REFMISS_Pos)                     /*!< 0x00000200 */
#define CTC_SR_REFMISS                            CTC_SR_REFMISS_Msk                                /*!< desc REFMISS */
#define CTC_SR_TRIMERR_Pos                        (10U)
#define CTC_SR_TRIMERR_Msk                        (0x1UL << CTC_SR_TRIMERR_Pos)                     /*!< 0x00000400 */
#define CTC_SR_TRIMERR                            CTC_SR_TRIMERR_Msk                                /*!< desc TRIMERR */
#define CTC_SR_REFDIR_Pos                         (15U)
#define CTC_SR_REFDIR_Msk                         (0x1UL << CTC_SR_REFDIR_Pos)                      /*!< 0x00008000 */
#define CTC_SR_REFDIR                             CTC_SR_REFDIR_Msk                                 /*!< desc REFDIR */
#define CTC_SR_REFCAP_Pos                         (16U)
#define CTC_SR_REFCAP_Msk                         (0xFFFFUL << CTC_SR_REFCAP_Pos)                   /*!< 0xFFFF0000 */
#define CTC_SR_REFCAP                             CTC_SR_REFCAP_Msk                                 /*!< REFCAP[31:16] bits (desc REFCAP) */

/*!< CTC_INTC */
#define CTC_INTC_CKOKIC_Pos                       (0U)
#define CTC_INTC_CKOKIC_Msk                       (0x1UL << CTC_INTC_CKOKIC_Pos)                    /*!< 0x00000001 */
#define CTC_INTC_CKOKIC                           CTC_INTC_CKOKIC_Msk                               /*!< desc CKOKIC */
#define CTC_INTC_CKWARNIC_Pos                     (1U)
#define CTC_INTC_CKWARNIC_Msk                     (0x1UL << CTC_INTC_CKWARNIC_Pos)                  /*!< 0x00000002 */
#define CTC_INTC_CKWARNIC                         CTC_INTC_CKWARNIC_Msk                             /*!< desc CKWARNIC */
#define CTC_INTC_ERRIC_Pos                        (2U)
#define CTC_INTC_ERRIC_Msk                        (0x1UL << CTC_INTC_ERRIC_Pos)                     /*!< 0x00000004 */
#define CTC_INTC_ERRIC                            CTC_INTC_ERRIC_Msk                                /*!< desc ERRIC */
#define CTC_INTC_EREFIC_Pos                       (3U)
#define CTC_INTC_EREFIC_Msk                       (0x1UL << CTC_INTC_EREFIC_Pos)                    /*!< 0x00000008 */
#define CTC_INTC_EREFIC                           CTC_INTC_EREFIC_Msk                               /*!< desc EREFIC */

/****************************************************************************/
/*                                                                          */
/*                    Digital to Analog Converter(DAC)                      */
/*                                                                          */
/****************************************************************************/
/*********************  Bits Define For Peripheral DAC  *********************/
/*!< DAC_CR */
#define DAC_CR_EN1_Pos                            (0U)
#define DAC_CR_EN1_Msk                            (0x1UL << DAC_CR_EN1_Pos)                         /*!< 0x00000001 */
#define DAC_CR_EN1                                DAC_CR_EN1_Msk                                    /*!< desc EN1 */
#define DAC_CR_BOFF1_Pos                          (1U)
#define DAC_CR_BOFF1_Msk                          (0x1UL << DAC_CR_BOFF1_Pos)                       /*!< 0x00000002 */
#define DAC_CR_BOFF1                              DAC_CR_BOFF1_Msk                                  /*!< desc BOFF1 */
#define DAC_CR_TEN1_Pos                           (2U)
#define DAC_CR_TEN1_Msk                           (0x1UL << DAC_CR_TEN1_Pos)                        /*!< 0x00000004 */
#define DAC_CR_TEN1                               DAC_CR_TEN1_Msk                                   /*!< desc TEN1 */
#define DAC_CR_TSEL1_Pos                          (3U)
#define DAC_CR_TSEL1_Msk                          (0x7UL << DAC_CR_TSEL1_Pos)                       /*!< 0x00000038 */
#define DAC_CR_TSEL1                              DAC_CR_TSEL1_Msk                                  /*!< TSEL1[5:3] bits (desc TSEL1) */
#define DAC_CR_TSEL1_0                            (0x1UL << DAC_CR_TSEL1_Pos)                       /*!< 0x00000008 */
#define DAC_CR_TSEL1_1                            (0x2UL << DAC_CR_TSEL1_Pos)                       /*!< 0x00000010 */
#define DAC_CR_TSEL1_2                            (0x4UL << DAC_CR_TSEL1_Pos)                       /*!< 0x00000020 */
#define DAC_CR_WAVE1_Pos                          (6U)
#define DAC_CR_WAVE1_Msk                          (0x3UL << DAC_CR_WAVE1_Pos)                       /*!< 0x000000C0 */
#define DAC_CR_WAVE1                              DAC_CR_WAVE1_Msk                                  /*!< WAVE1[7:6] bits (desc WAVE1) */
#define DAC_CR_WAVE1_0                            (0x1UL << DAC_CR_WAVE1_Pos)                       /*!< 0x00000040 */
#define DAC_CR_WAVE1_1                            (0x2UL << DAC_CR_WAVE1_Pos)                       /*!< 0x00000080 */
#define DAC_CR_MAMP1_Pos                          (8U)
#define DAC_CR_MAMP1_Msk                          (0xFUL << DAC_CR_MAMP1_Pos)                       /*!< 0x00000F00 */
#define DAC_CR_MAMP1                              DAC_CR_MAMP1_Msk                                  /*!< MAMP1[11:8] bits (desc MAMP1) */
#define DAC_CR_MAMP1_0                            (0x1UL << DAC_CR_MAMP1_Pos)                       /*!< 0x00000100 */
#define DAC_CR_MAMP1_1                            (0x2UL << DAC_CR_MAMP1_Pos)                       /*!< 0x00000200 */
#define DAC_CR_MAMP1_2                            (0x4UL << DAC_CR_MAMP1_Pos)                       /*!< 0x00000400 */
#define DAC_CR_MAMP1_3                            (0x8UL << DAC_CR_MAMP1_Pos)                       /*!< 0x00000800 */
#define DAC_CR_DMAEN1_Pos                         (12U)
#define DAC_CR_DMAEN1_Msk                         (0x1UL << DAC_CR_DMAEN1_Pos)                      /*!< 0x00001000 */
#define DAC_CR_DMAEN1                             DAC_CR_DMAEN1_Msk                                 /*!< desc DMAEN1 */
#define DAC_CR_DMAUDRIE1_Pos                      (13U)
#define DAC_CR_DMAUDRIE1_Msk                      (0x1UL << DAC_CR_DMAUDRIE1_Pos)                   /*!< 0x00002000 */
#define DAC_CR_DMAUDRIE1                          DAC_CR_DMAUDRIE1_Msk                              /*!< desc DMAUDRIE1 */
#define DAC_CR_EN2_Pos                            (16U)
#define DAC_CR_EN2_Msk                            (0x1UL << DAC_CR_EN2_Pos)                         /*!< 0x00010000 */
#define DAC_CR_EN2                                DAC_CR_EN2_Msk                                    /*!< desc EN2 */
#define DAC_CR_BOFF2_Pos                          (17U)
#define DAC_CR_BOFF2_Msk                          (0x1UL << DAC_CR_BOFF2_Pos)                       /*!< 0x00020000 */
#define DAC_CR_BOFF2                              DAC_CR_BOFF2_Msk                                  /*!< desc BOFF2 */
#define DAC_CR_TEN2_Pos                           (18U)
#define DAC_CR_TEN2_Msk                           (0x1UL << DAC_CR_TEN2_Pos)                        /*!< 0x00040000 */
#define DAC_CR_TEN2                               DAC_CR_TEN2_Msk                                   /*!< desc TEN2 */
#define DAC_CR_TSEL2_Pos                          (19U)
#define DAC_CR_TSEL2_Msk                          (0x7UL << DAC_CR_TSEL2_Pos)                       /*!< 0x00380000 */
#define DAC_CR_TSEL2                              DAC_CR_TSEL2_Msk                                  /*!< TSEL2[21:19] bits (desc TSEL2) */
#define DAC_CR_TSEL2_0                            (0x1UL << DAC_CR_TSEL2_Pos)                       /*!< 0x00080000 */
#define DAC_CR_TSEL2_1                            (0x2UL << DAC_CR_TSEL2_Pos)                       /*!< 0x00100000 */
#define DAC_CR_TSEL2_2                            (0x4UL << DAC_CR_TSEL2_Pos)                       /*!< 0x00200000 */
#define DAC_CR_WAVE2_Pos                          (22U)
#define DAC_CR_WAVE2_Msk                          (0x3UL << DAC_CR_WAVE2_Pos)                       /*!< 0x00C00000 */
#define DAC_CR_WAVE2                              DAC_CR_WAVE2_Msk                                  /*!< WAVE2[23:22] bits (desc WAVE2) */
#define DAC_CR_WAVE2_0                            (0x1UL << DAC_CR_WAVE2_Pos)                       /*!< 0x00400000 */
#define DAC_CR_WAVE2_1                            (0x2UL << DAC_CR_WAVE2_Pos)                       /*!< 0x00800000 */
#define DAC_CR_MAMP2_Pos                          (24U)
#define DAC_CR_MAMP2_Msk                          (0xFUL << DAC_CR_MAMP2_Pos)                       /*!< 0x0F000000 */
#define DAC_CR_MAMP2                              DAC_CR_MAMP2_Msk                                  /*!< MAMP2[27:24] bits (desc MAMP2) */
#define DAC_CR_MAMP2_0                            (0x1UL << DAC_CR_MAMP2_Pos)                       /*!< 0x01000000 */
#define DAC_CR_MAMP2_1                            (0x2UL << DAC_CR_MAMP2_Pos)                       /*!< 0x02000000 */
#define DAC_CR_MAMP2_2                            (0x4UL << DAC_CR_MAMP2_Pos)                       /*!< 0x04000000 */
#define DAC_CR_MAMP2_3                            (0x8UL << DAC_CR_MAMP2_Pos)                       /*!< 0x08000000 */
#define DAC_CR_DMAEN2_Pos                         (28U)
#define DAC_CR_DMAEN2_Msk                         (0x1UL << DAC_CR_DMAEN2_Pos)                      /*!< 0x10000000 */
#define DAC_CR_DMAEN2                             DAC_CR_DMAEN2_Msk                                 /*!< desc DMAEN2 */
#define DAC_CR_DMAUDRIE2_Pos                      (29U)
#define DAC_CR_DMAUDRIE2_Msk                      (0x1UL << DAC_CR_DMAUDRIE2_Pos)                   /*!< 0x20000000 */
#define DAC_CR_DMAUDRIE2                          DAC_CR_DMAUDRIE2_Msk                              /*!< desc DMAUDRIE2 */
#define DAC_CR_DAC2CEN_Pos                        (30U)
#define DAC_CR_DAC2CEN_Msk                        (0x1UL << DAC_CR_DAC2CEN_Pos)                     /*!< 0x40000000 */
#define DAC_CR_DAC2CEN                            DAC_CR_DAC2CEN_Msk                                /*!< desc DAC2CEN */

/*!< DAC_SWTRIGR */
#define DAC_SWTRIGR_SWTRIG1_Pos                   (0U)
#define DAC_SWTRIGR_SWTRIG1_Msk                   (0x1UL << DAC_SWTRIGR_SWTRIG1_Pos)                /*!< 0x00000001 */
#define DAC_SWTRIGR_SWTRIG1                       DAC_SWTRIGR_SWTRIG1_Msk                           /*!< desc SWTRIG1 */
#define DAC_SWTRIGR_SWTRIG2_Pos                   (1U)
#define DAC_SWTRIGR_SWTRIG2_Msk                   (0x1UL << DAC_SWTRIGR_SWTRIG2_Pos)                /*!< 0x00000002 */
#define DAC_SWTRIGR_SWTRIG2                       DAC_SWTRIGR_SWTRIG2_Msk                           /*!< desc SWTRIG2 */

/*!< DAC_DHR12R1 */
#define DAC_DHR12R1_DACC1DHR_Pos                  (0U)
#define DAC_DHR12R1_DACC1DHR_Msk                  (0xFFFUL << DAC_DHR12R1_DACC1DHR_Pos)             /*!< 0x00000FFF */
#define DAC_DHR12R1_DACC1DHR                      DAC_DHR12R1_DACC1DHR_Msk                          /*!< DACC1DHR[11:0] bits (desc DACC1DHR) */

/*!< DAC_DHR12L1 */
#define DAC_DHR12L1_DACC1DHR_Pos                  (4U)
#define DAC_DHR12L1_DACC1DHR_Msk                  (0xFFFUL << DAC_DHR12L1_DACC1DHR_Pos)             /*!< 0x00007FF8 */
#define DAC_DHR12L1_DACC1DHR                      DAC_DHR12L1_DACC1DHR_Msk                          /*!< DACC1DHR[14:3] bits (desc DACC1DHR) */

/*!< DAC_DHR8R1 */
#define DAC_DHR8R1_DACC1DHR_Pos                   (0U)
#define DAC_DHR8R1_DACC1DHR_Msk                   (0xFFUL << DAC_DHR8R1_DACC1DHR_Pos)               /*!< 0x000000FF */
#define DAC_DHR8R1_DACC1DHR                       DAC_DHR8R1_DACC1DHR_Msk                           /*!< DACC1DHR[7:0] bits (desc DACC1DHR) */

/*!< DAC_DHR12R2 */
#define DAC_DHR12R2_DACC2DHR_Pos                  (0U)
#define DAC_DHR12R2_DACC2DHR_Msk                  (0xFFFUL << DAC_DHR12R2_DACC2DHR_Pos)             /*!< 0x00000FFF */
#define DAC_DHR12R2_DACC2DHR                      DAC_DHR12R2_DACC2DHR_Msk                          /*!< DACC2DHR[11:0] bits (desc DACC2DHR) */

/*!< DAC_DHR12L2 */
#define DAC_DHR12L2_DACC2DHR_Pos                  (4U)
#define DAC_DHR12L2_DACC2DHR_Msk                  (0xFFFUL << DAC_DHR12L2_DACC2DHR_Pos)             /*!< 0x0000FFF0 */
#define DAC_DHR12L2_DACC2DHR                      DAC_DHR12L2_DACC2DHR_Msk                          /*!< DACC2DHR[15:4] bits (desc DACC2DHR) */

/*!< DAC_DHR8R2 */
#define DAC_DHR8R2_DACC2DHR_Pos                   (0U)
#define DAC_DHR8R2_DACC2DHR_Msk                   (0xFFUL << DAC_DHR8R2_DACC2DHR_Pos)               /*!< 0x000000FF */
#define DAC_DHR8R2_DACC2DHR                       DAC_DHR8R2_DACC2DHR_Msk                           /*!< DACC2DHR[7:0] bits (desc DACC2DHR) */

/*!< DAC_DHR12RD */
#define DAC_DHR12RD_DACC1DHR_Pos                  (0U)
#define DAC_DHR12RD_DACC1DHR_Msk                  (0xFFFUL << DAC_DHR12RD_DACC1DHR_Pos)             /*!< 0x00000FFF */
#define DAC_DHR12RD_DACC1DHR                      DAC_DHR12RD_DACC1DHR_Msk                          /*!< DACC1DHR[11:0] bits (desc DACC1DHR) */
#define DAC_DHR12RD_DACC2DHR_Pos                  (16U)
#define DAC_DHR12RD_DACC2DHR_Msk                  (0xFFFUL << DAC_DHR12RD_DACC2DHR_Pos)             /*!< 0x0FFF0000 */
#define DAC_DHR12RD_DACC2DHR                      DAC_DHR12RD_DACC2DHR_Msk                          /*!< DACC2DHR[27:16] bits (desc DACC2DHR) */

/*!< DAC_DHR12LD */
#define DAC_DHR12LD_DACC1DHR_Pos                  (4U)
#define DAC_DHR12LD_DACC1DHR_Msk                  (0xFFFUL << DAC_DHR12LD_DACC1DHR_Pos)             /*!< 0x0000FFF0 */
#define DAC_DHR12LD_DACC1DHR                      DAC_DHR12LD_DACC1DHR_Msk                          /*!< DACC1DHR[15:4] bits (desc DACC1DHR) */
#define DAC_DHR12LD_DACC2DHR_Pos                  (20U)
#define DAC_DHR12LD_DACC2DHR_Msk                  (0xFFFUL << DAC_DHR12LD_DACC2DHR_Pos)             /*!< 0xFFF00000 */
#define DAC_DHR12LD_DACC2DHR                      DAC_DHR12LD_DACC2DHR_Msk                          /*!< DACC2DHR[31:20] bits (desc DACC2DHR) */

/*!< DAC_DHR8RD */
#define DAC_DHR8RD_DACC1DHR_Pos                   (0U)
#define DAC_DHR8RD_DACC1DHR_Msk                   (0xFFUL << DAC_DHR8RD_DACC1DHR_Pos)               /*!< 0x000000FF */
#define DAC_DHR8RD_DACC1DHR                       DAC_DHR8RD_DACC1DHR_Msk                           /*!< DACC1DHR[7:0] bits (desc DACC1DHR) */
#define DAC_DHR8RD_DACC2DHR_Pos                   (8U)
#define DAC_DHR8RD_DACC2DHR_Msk                   (0xFFUL << DAC_DHR8RD_DACC2DHR_Pos)               /*!< 0x0000FF00 */
#define DAC_DHR8RD_DACC2DHR                       DAC_DHR8RD_DACC2DHR_Msk                           /*!< DACC2DHR[15:8] bits (desc DACC2DHR) */

/*!< DAC_DOR1 */
#define DAC_DOR1_DACC1DOR_Pos                     (0U)
#define DAC_DOR1_DACC1DOR_Msk                     (0xFFFUL << DAC_DOR1_DACC1DOR_Pos)                /*!< 0x00000FFF */
#define DAC_DOR1_DACC1DOR                         DAC_DOR1_DACC1DOR_Msk                             /*!< DACC1DOR[11:0] bits (desc DACC1DOR) */

/*!< DAC_DOR2 */
#define DAC_DOR2_DACC2DOR_Pos                     (0U)
#define DAC_DOR2_DACC2DOR_Msk                     (0xFFFUL << DAC_DOR2_DACC2DOR_Pos)                /*!< 0x00000FFF */
#define DAC_DOR2_DACC2DOR                         DAC_DOR2_DACC2DOR_Msk                             /*!< DACC2DOR[11:0] bits (desc DACC2DOR) */

/*!< DAC_SR */
#define DAC_SR_DMAUDR1_Pos                        (13U)
#define DAC_SR_DMAUDR1_Msk                        (0x1UL << DAC_SR_DMAUDR1_Pos)                     /*!< 0x00002000 */
#define DAC_SR_DMAUDR1                            DAC_SR_DMAUDR1_Msk                                /*!< desc DMAUDR1 */
#define DAC_SR_DMAUDR2_Pos                        (29U)
#define DAC_SR_DMAUDR2_Msk                        (0x1UL << DAC_SR_DMAUDR2_Pos)                     /*!< 0x20000000 */
#define DAC_SR_DMAUDR2                            DAC_SR_DMAUDR2_Msk                                /*!< desc DMAUDR2 */

/****************************************************************************/
/*                                                                          */
/*                                Debug MCU (DBGMCU)                        */
/*                                                                          */
/****************************************************************************/
/********************  Bit definition for DBG_IDCODE register  **************/
#define DBGMCU_IDCODE_REV_ID_Pos                          (0U)
#define DBGMCU_IDCODE_REV_ID_Msk                          (0xFFFFFFFFUL << DBGMCU_IDCODE_REV_ID_Pos) /*!< 0xFFFFFFFF */
#define DBGMCU_IDCODE_REV_ID                              DBGMCU_IDCODE_REV_ID_Msk

/********************  Bit definition for DBGMCU_CR register  ***************/
#define DBGMCU_CR_DBG_SLEEP_Pos                           (0U)
#define DBGMCU_CR_DBG_SLEEP_Msk                           (0x1UL << DBGMCU_CR_DBG_SLEEP_Pos)     /*!< 0x00000001 */
#define DBGMCU_CR_DBG_SLEEP                               DBGMCU_CR_DBG_SLEEP_Msk
#define DBGMCU_CR_DBG_STOP_Pos                            (1U)
#define DBGMCU_CR_DBG_STOP_Msk                            (0x1UL << DBGMCU_CR_DBG_STOP_Pos)      /*!< 0x00000002 */
#define DBGMCU_CR_DBG_STOP                                DBGMCU_CR_DBG_STOP_Msk

/********************  Bit definition for DBGMCU_APB_FZ1 register  **********/
#define DBGMCU_APB_FZ1_DBG_TIM2_STOP_Pos                  (0U)
#define DBGMCU_APB_FZ1_DBG_TIM2_STOP_Msk                  (0x1UL << DBGMCU_APB_FZ1_DBG_TIM2_STOP_Pos) /*!< 0x00000001 */
#define DBGMCU_APB_FZ1_DBG_TIM2_STOP                      DBGMCU_APB_FZ1_DBG_TIM2_STOP_Msk
#define DBGMCU_APB_FZ1_DBG_TIM3_STOP_Pos                  (1U)
#define DBGMCU_APB_FZ1_DBG_TIM3_STOP_Msk                  (0x1UL << DBGMCU_APB_FZ1_DBG_TIM3_STOP_Pos) /*!< 0x00000002 */
#define DBGMCU_APB_FZ1_DBG_TIM3_STOP                      DBGMCU_APB_FZ1_DBG_TIM3_STOP_Msk
#define DBGMCU_APB_FZ1_DBG_TIM6_STOP_Pos                  (4U)
#define DBGMCU_APB_FZ1_DBG_TIM6_STOP_Msk                  (0x1UL << DBGMCU_APB_FZ1_DBG_TIM6_STOP_Pos) /*!< 0x00000010 */
#define DBGMCU_APB_FZ1_DBG_TIM6_STOP                      DBGMCU_APB_FZ1_DBG_TIM6_STOP_Msk
#define DBGMCU_APB_FZ1_DBG_TIM7_STOP_Pos                  (5U)
#define DBGMCU_APB_FZ1_DBG_TIM7_STOP_Msk                  (0x1UL << DBGMCU_APB_FZ1_DBG_TIM7_STOP_Pos)           /*!< 0x00000020 */
#define DBGMCU_APB_FZ1_DBG_TIM7_STOP                      DBGMCU_APB_FZ1_DBG_TIM7_STOP_Msk
#define DBGMCU_APB_FZ1_DBG_RTC_STOP_Pos                   (10U)
#define DBGMCU_APB_FZ1_DBG_RTC_STOP_Msk                   (0x1UL << DBGMCU_APB_FZ1_DBG_RTC_STOP_Pos)            /*!< 0x00000400 */
#define DBGMCU_APB_FZ1_DBG_RTC_STOP                       DBGMCU_APB_FZ1_DBG_RTC_STOP_Msk
#define DBGMCU_APB_FZ1_DBG_WWDG_STOP_Pos                  (11U)
#define DBGMCU_APB_FZ1_DBG_WWDG_STOP_Msk                  (0x1UL << DBGMCU_APB_FZ1_DBG_WWDG_STOP_Pos)           /*!< 0x00000800 */
#define DBGMCU_APB_FZ1_DBG_WWDG_STOP                      DBGMCU_APB_FZ1_DBG_WWDG_STOP_Msk
#define DBGMCU_APB_FZ1_DBG_IWDG_STOP_Pos                  (12U)
#define DBGMCU_APB_FZ1_DBG_IWDG_STOP_Msk                  (0x1UL << DBGMCU_APB_FZ1_DBG_IWDG_STOP_Pos)           /*!< 0x00001000 */
#define DBGMCU_APB_FZ1_DBG_IWDG_STOP                      DBGMCU_APB_FZ1_DBG_IWDG_STOP_Msk
#define DBGMCU_APB_FZ1_DBG_CAN_STOP_Pos                   (19U)
#define DBGMCU_APB_FZ1_DBG_CAN_STOP_Msk                   (0x1UL << DBGMCU_APB_FZ1_DBG_CAN_STOP_Pos)            /*!< 0x00080000 */
#define DBGMCU_APB_FZ1_DBG_CAN_STOP                       DBGMCU_APB_FZ1_DBG_CAN_STOP_Msk
#define DBGMCU_APB_FZ1_DBG_CAN1_STOP                      DBGMCU_APB_FZ1_DBG_CAN_STOP_Msk
#define DBGMCU_APB_FZ1_DBG_I2C1_SMBUS_TIMEOUT_Pos         (21U)
#define DBGMCU_APB_FZ1_DBG_I2C1_SMBUS_TIMEOUT_Msk         (0x1UL << DBGMCU_APB_FZ1_DBG_I2C1_SMBUS_TIMEOUT_Pos)  /*!< 0x00200000 */
#define DBGMCU_APB_FZ1_DBG_I2C1_SMBUS_TIMEOUT             DBGMCU_APB_FZ1_DBG_I2C1_SMBUS_TIMEOUT_Msk
#define DBGMCU_APB_FZ1_DBG_I2C2_SMBUS_TIMEOUT_Pos         (22U)
#define DBGMCU_APB_FZ1_DBG_I2C2_SMBUS_TIMEOUT_Msk         (0x1UL << DBGMCU_APB_FZ1_DBG_I2C2_SMBUS_TIMEOUT_Pos)  /*!< 0x00400000 */
#define DBGMCU_APB_FZ1_DBG_I2C2_SMBUS_TIMEOUT             DBGMCU_APB_FZ1_DBG_I2C2_SMBUS_TIMEOUT_Msk
#define DBGMCU_APB_FZ1_DBG_LPTIM_STOP_Pos                 (31U)
#define DBGMCU_APB_FZ1_DBG_LPTIM_STOP_Msk                 (0x1UL << DBGMCU_APB_FZ1_DBG_LPTIM_STOP_Pos)          /*!< 0x00001000 */
#define DBGMCU_APB_FZ1_DBG_LPTIM_STOP                     DBGMCU_APB_FZ1_DBG_LPTIM_STOP_Msk
#define DBGMCU_APB_FZ1_DBG_LPTIM1_STOP                    DBGMCU_APB_FZ1_DBG_LPTIM_STOP_Msk

/********************  Bit definition for DBGMCU_APB_FZ2 register  **********/
#define DBGMCU_APB_FZ2_DBG_TIM1_STOP_Pos                  (11U)
#define DBGMCU_APB_FZ2_DBG_TIM1_STOP_Msk                  (0x1UL << DBGMCU_APB_FZ2_DBG_TIM1_STOP_Pos)           /*!< 0x00000800 */
#define DBGMCU_APB_FZ2_DBG_TIM1_STOP                      DBGMCU_APB_FZ2_DBG_TIM1_STOP_Msk
#define DBGMCU_APB_FZ2_DBG_TIM14_STOP_Pos                 (15U)
#define DBGMCU_APB_FZ2_DBG_TIM14_STOP_Msk                 (0x1UL << DBGMCU_APB_FZ2_DBG_TIM14_STOP_Pos)          /*!< 0x00008000 */
#define DBGMCU_APB_FZ2_DBG_TIM14_STOP                     DBGMCU_APB_FZ2_DBG_TIM14_STOP_Msk
#define DBGMCU_APB_FZ2_DBG_TIM15_STOP_Pos                 (16U)
#define DBGMCU_APB_FZ2_DBG_TIM15_STOP_Msk                 (0x1UL << DBGMCU_APB_FZ2_DBG_TIM15_STOP_Pos)          /*!< 0x00010000 */
#define DBGMCU_APB_FZ2_DBG_TIM15_STOP                     DBGMCU_APB_FZ2_DBG_TIM15_STOP_Msk
#define DBGMCU_APB_FZ2_DBG_TIM16_STOP_Pos                 (17U)
#define DBGMCU_APB_FZ2_DBG_TIM16_STOP_Msk                 (0x1UL << DBGMCU_APB_FZ2_DBG_TIM16_STOP_Pos)          /*!< 0x00020000 */
#define DBGMCU_APB_FZ2_DBG_TIM16_STOP                     DBGMCU_APB_FZ2_DBG_TIM16_STOP_Msk
#define DBGMCU_APB_FZ2_DBG_TIM17_STOP_Pos                 (18U)
#define DBGMCU_APB_FZ2_DBG_TIM17_STOP_Msk                 (0x1UL << DBGMCU_APB_FZ2_DBG_TIM17_STOP_Pos)          /*!< 0x00040000 */
#define DBGMCU_APB_FZ2_DBG_TIM17_STOP                     DBGMCU_APB_FZ2_DBG_TIM17_STOP_Msk

/****************************************************************************/
/*                                                                          */
/*                           DMA Controller (DMA)                           */
/*                                                                          */
/****************************************************************************/
/*******************  Bit definition for DMA_ISR register  ******************/
#define DMA_ISR_GIF1_Pos                          (0U)
#define DMA_ISR_GIF1_Msk                          (0x1UL << DMA_ISR_GIF1_Pos)                       /*!< 0x00000001 */
#define DMA_ISR_GIF1                              DMA_ISR_GIF1_Msk                                  /*!< Channel 1 Global interrupt flag */
#define DMA_ISR_TCIF1_Pos                         (1U)
#define DMA_ISR_TCIF1_Msk                         (0x1UL << DMA_ISR_TCIF1_Pos)                      /*!< 0x00000002 */
#define DMA_ISR_TCIF1                             DMA_ISR_TCIF1_Msk                                 /*!< Channel 1 Transfer Complete flag */
#define DMA_ISR_HTIF1_Pos                         (2U)
#define DMA_ISR_HTIF1_Msk                         (0x1UL << DMA_ISR_HTIF1_Pos)                      /*!< 0x00000004 */
#define DMA_ISR_HTIF1                             DMA_ISR_HTIF1_Msk                                 /*!< Channel 1 Half Transfer flag */
#define DMA_ISR_TEIF1_Pos                         (3U)
#define DMA_ISR_TEIF1_Msk                         (0x1UL << DMA_ISR_TEIF1_Pos)                      /*!< 0x00000008 */
#define DMA_ISR_TEIF1                             DMA_ISR_TEIF1_Msk                                 /*!< Channel 1 Transfer Error flag */
#define DMA_ISR_GIF2_Pos                          (4U)
#define DMA_ISR_GIF2_Msk                          (0x1UL << DMA_ISR_GIF2_Pos)                       /*!< 0x00000010 */
#define DMA_ISR_GIF2                              DMA_ISR_GIF2_Msk                                  /*!< Channel 2 Global interrupt flag */
#define DMA_ISR_TCIF2_Pos                         (5U)
#define DMA_ISR_TCIF2_Msk                         (0x1UL << DMA_ISR_TCIF2_Pos)                      /*!< 0x00000020 */
#define DMA_ISR_TCIF2                             DMA_ISR_TCIF2_Msk                                 /*!< Channel 2 Transfer Complete flag */
#define DMA_ISR_HTIF2_Pos                         (6U)
#define DMA_ISR_HTIF2_Msk                         (0x1UL << DMA_ISR_HTIF2_Pos)                      /*!< 0x00000040 */
#define DMA_ISR_HTIF2                             DMA_ISR_HTIF2_Msk                                 /*!< Channel 2 Half Transfer flag */
#define DMA_ISR_TEIF2_Pos                         (7U)
#define DMA_ISR_TEIF2_Msk                         (0x1UL << DMA_ISR_TEIF2_Pos)                      /*!< 0x00000080 */
#define DMA_ISR_TEIF2                             DMA_ISR_TEIF2_Msk                                 /*!< Channel 2 Transfer Error flag */
#define DMA_ISR_GIF3_Pos                          (8U)
#define DMA_ISR_GIF3_Msk                          (0x1UL << DMA_ISR_GIF3_Pos)                       /*!< 0x00000100 */
#define DMA_ISR_GIF3                              DMA_ISR_GIF3_Msk                                  /*!< Channel 3 Global interrupt flag */
#define DMA_ISR_TCIF3_Pos                         (9U)
#define DMA_ISR_TCIF3_Msk                         (0x1UL << DMA_ISR_TCIF3_Pos)                      /*!< 0x00000200 */
#define DMA_ISR_TCIF3                             DMA_ISR_TCIF3_Msk                                 /*!< Channel 3 Transfer Complete flag */
#define DMA_ISR_HTIF3_Pos                         (10U)
#define DMA_ISR_HTIF3_Msk                         (0x1UL << DMA_ISR_HTIF3_Pos)                      /*!< 0x00000400 */
#define DMA_ISR_HTIF3                             DMA_ISR_HTIF3_Msk                                 /*!< Channel 3 Half Transfer flag */
#define DMA_ISR_TEIF3_Pos                         (11U)
#define DMA_ISR_TEIF3_Msk                         (0x1UL << DMA_ISR_TEIF3_Pos)                      /*!< 0x00000800 */
#define DMA_ISR_TEIF3                             DMA_ISR_TEIF3_Msk                                 /*!< Channel 3 Transfer Error flag */
#define DMA_ISR_GIF4_Pos                          (12U)
#define DMA_ISR_GIF4_Msk                          (0x1UL << DMA_ISR_GIF4_Pos)                       /*!< 0x00001000 */
#define DMA_ISR_GIF4                              DMA_ISR_GIF4_Msk                                  /*!< Channel 4 Global interrupt flag */
#define DMA_ISR_TCIF4_Pos                         (13U)
#define DMA_ISR_TCIF4_Msk                         (0x1UL << DMA_ISR_TCIF4_Pos)                      /*!< 0x00002000 */
#define DMA_ISR_TCIF4                             DMA_ISR_TCIF4_Msk                                 /*!< Channel 4 Transfer Complete flag */
#define DMA_ISR_HTIF4_Pos                         (14U)
#define DMA_ISR_HTIF4_Msk                         (0x1UL << DMA_ISR_HTIF4_Pos)                      /*!< 0x00004000 */
#define DMA_ISR_HTIF4                             DMA_ISR_HTIF4_Msk                                 /*!< Channel 4 Half Transfer flag */
#define DMA_ISR_TEIF4_Pos                         (15U)
#define DMA_ISR_TEIF4_Msk                         (0x1UL << DMA_ISR_TEIF4_Pos)                      /*!< 0x00008000 */
#define DMA_ISR_TEIF4                             DMA_ISR_TEIF4_Msk                                 /*!< Channel 4 Transfer Error flag */
#define DMA_ISR_GIF5_Pos                          (16U)
#define DMA_ISR_GIF5_Msk                          (0x1UL << DMA_ISR_GIF5_Pos)                       /*!< 0x00010000 */
#define DMA_ISR_GIF5                              DMA_ISR_GIF5_Msk                                  /*!< Channel 5 Global interrupt flag */
#define DMA_ISR_TCIF5_Pos                         (17U)
#define DMA_ISR_TCIF5_Msk                         (0x1UL << DMA_ISR_TCIF5_Pos)                      /*!< 0x00020000 */
#define DMA_ISR_TCIF5                             DMA_ISR_TCIF5_Msk                                 /*!< Channel 5 Transfer Complete flag */
#define DMA_ISR_HTIF5_Pos                         (18U)
#define DMA_ISR_HTIF5_Msk                         (0x1UL << DMA_ISR_HTIF5_Pos)                      /*!< 0x00040000 */
#define DMA_ISR_HTIF5                             DMA_ISR_HTIF5_Msk                                 /*!< Channel 5 Half Transfer flag */
#define DMA_ISR_TEIF5_Pos                         (19U)
#define DMA_ISR_TEIF5_Msk                         (0x1UL << DMA_ISR_TEIF5_Pos)                      /*!< 0x00080000 */
#define DMA_ISR_TEIF5                             DMA_ISR_TEIF5_Msk                                 /*!< Channel 5 Transfer Error flag */
#define DMA_ISR_GIF6_Pos                          (20U)
#define DMA_ISR_GIF6_Msk                          (0x1UL << DMA_ISR_GIF6_Pos)                       /*!< 0x00100000 */
#define DMA_ISR_GIF6                              DMA_ISR_GIF6_Msk                                  /*!< Channel 6 Global interrupt flag */
#define DMA_ISR_TCIF6_Pos                         (21U)
#define DMA_ISR_TCIF6_Msk                         (0x1UL << DMA_ISR_TCIF6_Pos)                      /*!< 0x00200000 */
#define DMA_ISR_TCIF6                             DMA_ISR_TCIF6_Msk                                 /*!< Channel 6 Transfer Complete flag */
#define DMA_ISR_HTIF6_Pos                         (22U)
#define DMA_ISR_HTIF6_Msk                         (0x1UL << DMA_ISR_HTIF6_Pos)                      /*!< 0x00400000 */
#define DMA_ISR_HTIF6                             DMA_ISR_HTIF6_Msk                                 /*!< Channel 6 Half Transfer flag */
#define DMA_ISR_TEIF6_Pos                         (23U)
#define DMA_ISR_TEIF6_Msk                         (0x1UL << DMA_ISR_TEIF6_Pos)                      /*!< 0x00800000 */
#define DMA_ISR_TEIF6                             DMA_ISR_TEIF6_Msk                                 /*!< Channel 6 Transfer Error flag */
#define DMA_ISR_GIF7_Pos                          (24U)
#define DMA_ISR_GIF7_Msk                          (0x1UL << DMA_ISR_GIF7_Pos)                       /*!< 0x01000000 */
#define DMA_ISR_GIF7                              DMA_ISR_GIF7_Msk                                  /*!< Channel 7 Global interrupt flag */
#define DMA_ISR_TCIF7_Pos                         (25U)
#define DMA_ISR_TCIF7_Msk                         (0x1UL << DMA_ISR_TCIF7_Pos)                      /*!< 0x02000000 */
#define DMA_ISR_TCIF7                             DMA_ISR_TCIF7_Msk                                 /*!< Channel 7 Transfer Complete flag */
#define DMA_ISR_HTIF7_Pos                         (26U)
#define DMA_ISR_HTIF7_Msk                         (0x1UL << DMA_ISR_HTIF7_Pos)                      /*!< 0x04000000 */
#define DMA_ISR_HTIF7                             DMA_ISR_HTIF7_Msk                                 /*!< Channel 7 Half Transfer flag */
#define DMA_ISR_TEIF7_Pos                         (27U)
#define DMA_ISR_TEIF7_Msk                         (0x1UL << DMA_ISR_TEIF7_Pos)                      /*!< 0x08000000 */
#define DMA_ISR_TEIF7                             DMA_ISR_TEIF7_Msk                                 /*!< Channel 7 Transfer Error flag */

/*******************  Bit definition for DMA_IFCR register  *****************/
#define DMA_IFCR_CGIF1_Pos                        (0U)
#define DMA_IFCR_CGIF1_Msk                        (0x1UL << DMA_IFCR_CGIF1_Pos)                     /*!< 0x00000001 */
#define DMA_IFCR_CGIF1                            DMA_IFCR_CGIF1_Msk                                /*!< Channel 1 Global interrupt clear */
#define DMA_IFCR_CTCIF1_Pos                       (1U)
#define DMA_IFCR_CTCIF1_Msk                       (0x1UL << DMA_IFCR_CTCIF1_Pos)                    /*!< 0x00000002 */
#define DMA_IFCR_CTCIF1                           DMA_IFCR_CTCIF1_Msk                               /*!< Channel 1 Transfer Complete clear */
#define DMA_IFCR_CHTIF1_Pos                       (2U)
#define DMA_IFCR_CHTIF1_Msk                       (0x1UL << DMA_IFCR_CHTIF1_Pos)                    /*!< 0x00000004 */
#define DMA_IFCR_CHTIF1                           DMA_IFCR_CHTIF1_Msk                               /*!< Channel 1 Half Transfer clear */
#define DMA_IFCR_CTEIF1_Pos                       (3U)
#define DMA_IFCR_CTEIF1_Msk                       (0x1UL << DMA_IFCR_CTEIF1_Pos)                    /*!< 0x00000008 */
#define DMA_IFCR_CTEIF1                           DMA_IFCR_CTEIF1_Msk                               /*!< Channel 1 Transfer Error clear */
#define DMA_IFCR_CGIF2_Pos                        (4U)
#define DMA_IFCR_CGIF2_Msk                        (0x1UL << DMA_IFCR_CGIF2_Pos)                     /*!< 0x00000010 */
#define DMA_IFCR_CGIF2                            DMA_IFCR_CGIF2_Msk                                /*!< Channel 2 Global interrupt clear */
#define DMA_IFCR_CTCIF2_Pos                       (5U)
#define DMA_IFCR_CTCIF2_Msk                       (0x1UL << DMA_IFCR_CTCIF2_Pos)                    /*!< 0x00000020 */
#define DMA_IFCR_CTCIF2                           DMA_IFCR_CTCIF2_Msk                               /*!< Channel 2 Transfer Complete clear */
#define DMA_IFCR_CHTIF2_Pos                       (6U)
#define DMA_IFCR_CHTIF2_Msk                       (0x1UL << DMA_IFCR_CHTIF2_Pos)                    /*!< 0x00000040 */
#define DMA_IFCR_CHTIF2                           DMA_IFCR_CHTIF2_Msk                               /*!< Channel 2 Half Transfer clear */
#define DMA_IFCR_CTEIF2_Pos                       (7U)
#define DMA_IFCR_CTEIF2_Msk                       (0x1UL << DMA_IFCR_CTEIF2_Pos)                    /*!< 0x00000080 */
#define DMA_IFCR_CTEIF2                           DMA_IFCR_CTEIF2_Msk                               /*!< Channel 2 Transfer Error clear */
#define DMA_IFCR_CGIF3_Pos                        (8U)
#define DMA_IFCR_CGIF3_Msk                        (0x1UL << DMA_IFCR_CGIF3_Pos)                     /*!< 0x00000100 */
#define DMA_IFCR_CGIF3                            DMA_IFCR_CGIF3_Msk                                /*!< Channel 3 Global interrupt clear */
#define DMA_IFCR_CTCIF3_Pos                       (9U)
#define DMA_IFCR_CTCIF3_Msk                       (0x1UL << DMA_IFCR_CTCIF3_Pos)                    /*!< 0x00000200 */
#define DMA_IFCR_CTCIF3                           DMA_IFCR_CTCIF3_Msk                               /*!< Channel 3 Transfer Complete clear */
#define DMA_IFCR_CHTIF3_Pos                       (10U)
#define DMA_IFCR_CHTIF3_Msk                       (0x1UL << DMA_IFCR_CHTIF3_Pos)                    /*!< 0x00000400 */
#define DMA_IFCR_CHTIF3                           DMA_IFCR_CHTIF3_Msk                               /*!< Channel 3 Half Transfer clear */
#define DMA_IFCR_CTEIF3_Pos                       (11U)
#define DMA_IFCR_CTEIF3_Msk                       (0x1UL << DMA_IFCR_CTEIF3_Pos)                    /*!< 0x00000800 */
#define DMA_IFCR_CTEIF3                           DMA_IFCR_CTEIF3_Msk                               /*!< Channel 3 Transfer Error clear */
#define DMA_IFCR_CGIF4_Pos                        (12U)
#define DMA_IFCR_CGIF4_Msk                        (0x1UL << DMA_IFCR_CGIF4_Pos)                     /*!< 0x00001000 */
#define DMA_IFCR_CGIF4                            DMA_IFCR_CGIF4_Msk                                /*!< Channel 4 Global interrupt clear */
#define DMA_IFCR_CTCIF4_Pos                       (13U)
#define DMA_IFCR_CTCIF4_Msk                       (0x1UL << DMA_IFCR_CTCIF4_Pos)                    /*!< 0x00002000 */
#define DMA_IFCR_CTCIF4                           DMA_IFCR_CTCIF4_Msk                               /*!< Channel 4 Transfer Complete clear */
#define DMA_IFCR_CHTIF4_Pos                       (14U)
#define DMA_IFCR_CHTIF4_Msk                       (0x1UL << DMA_IFCR_CHTIF4_Pos)                    /*!< 0x00004000 */
#define DMA_IFCR_CHTIF4                           DMA_IFCR_CHTIF4_Msk                               /*!< Channel 4 Half Transfer clear */
#define DMA_IFCR_CTEIF4_Pos                       (15U)
#define DMA_IFCR_CTEIF4_Msk                       (0x1UL << DMA_IFCR_CTEIF4_Pos)                    /*!< 0x00008000 */
#define DMA_IFCR_CTEIF4                           DMA_IFCR_CTEIF4_Msk                               /*!< Channel 4 Transfer Error clear */
#define DMA_IFCR_CGIF5_Pos                        (16U)
#define DMA_IFCR_CGIF5_Msk                        (0x1UL << DMA_IFCR_CGIF5_Pos)                     /*!< 0x00010000 */
#define DMA_IFCR_CGIF5                            DMA_IFCR_CGIF5_Msk                                /*!< Channel 5 Global interrupt clear */
#define DMA_IFCR_CTCIF5_Pos                       (17U)
#define DMA_IFCR_CTCIF5_Msk                       (0x1UL << DMA_IFCR_CTCIF5_Pos)                    /*!< 0x00020000 */
#define DMA_IFCR_CTCIF5                           DMA_IFCR_CTCIF5_Msk                               /*!< Channel 5 Transfer Complete clear */
#define DMA_IFCR_CHTIF5_Pos                       (18U)
#define DMA_IFCR_CHTIF5_Msk                       (0x1UL << DMA_IFCR_CHTIF5_Pos)                    /*!< 0x00040000 */
#define DMA_IFCR_CHTIF5                           DMA_IFCR_CHTIF5_Msk                               /*!< Channel 5 Half Transfer clear */
#define DMA_IFCR_CTEIF5_Pos                       (19U)
#define DMA_IFCR_CTEIF5_Msk                       (0x1UL << DMA_IFCR_CTEIF5_Pos)                    /*!< 0x00080000 */
#define DMA_IFCR_CTEIF5                           DMA_IFCR_CTEIF5_Msk                               /*!< Channel 5 Transfer Error clear */
#define DMA_IFCR_CGIF6_Pos                        (20U)
#define DMA_IFCR_CGIF6_Msk                        (0x1UL << DMA_IFCR_CGIF6_Pos)                     /*!< 0x00100000 */
#define DMA_IFCR_CGIF6                            DMA_IFCR_CGIF6_Msk                                /*!< Channel 6 Global interrupt clear */
#define DMA_IFCR_CTCIF6_Pos                       (21U)
#define DMA_IFCR_CTCIF6_Msk                       (0x1UL << DMA_IFCR_CTCIF6_Pos)                    /*!< 0x00200000 */
#define DMA_IFCR_CTCIF6                           DMA_IFCR_CTCIF6_Msk                               /*!< Channel 6 Transfer Complete clear */
#define DMA_IFCR_CHTIF6_Pos                       (22U)
#define DMA_IFCR_CHTIF6_Msk                       (0x1UL << DMA_IFCR_CHTIF6_Pos)                    /*!< 0x00400000 */
#define DMA_IFCR_CHTIF6                           DMA_IFCR_CHTIF6_Msk                               /*!< Channel 6 Half Transfer clear */
#define DMA_IFCR_CTEIF6_Pos                       (23U)
#define DMA_IFCR_CTEIF6_Msk                       (0x1UL << DMA_IFCR_CTEIF6_Pos)                    /*!< 0x00800000 */
#define DMA_IFCR_CTEIF6                           DMA_IFCR_CTEIF6_Msk                               /*!< Channel 6 Transfer Error clear */
#define DMA_IFCR_CGIF7_Pos                        (24U)
#define DMA_IFCR_CGIF7_Msk                        (0x1UL << DMA_IFCR_CGIF7_Pos)                     /*!< 0x01000000 */
#define DMA_IFCR_CGIF7                            DMA_IFCR_CGIF7_Msk                                /*!< Channel 7 Global interrupt clear */
#define DMA_IFCR_CTCIF7_Pos                       (25U)
#define DMA_IFCR_CTCIF7_Msk                       (0x1UL << DMA_IFCR_CTCIF7_Pos)                    /*!< 0x02000000 */
#define DMA_IFCR_CTCIF7                           DMA_IFCR_CTCIF7_Msk                               /*!< Channel 7 Transfer Complete clear */
#define DMA_IFCR_CHTIF7_Pos                       (26U)
#define DMA_IFCR_CHTIF7_Msk                       (0x1UL << DMA_IFCR_CHTIF7_Pos)                    /*!< 0x04000000 */
#define DMA_IFCR_CHTIF7                           DMA_IFCR_CHTIF7_Msk                               /*!< Channel 7 Half Transfer clear */
#define DMA_IFCR_CTEIF7_Pos                       (27U)
#define DMA_IFCR_CTEIF7_Msk                       (0x1UL << DMA_IFCR_CTEIF7_Pos)                    /*!< 0x08000000 */
#define DMA_IFCR_CTEIF7                           DMA_IFCR_CTEIF7_Msk                               /*!< Channel 7 Transfer Error clear */

/*******************  Bit definition for DMA_CCR register  ******************/
#define DMA_CCR_EN_Pos         (0U)
#define DMA_CCR_EN_Msk         (0x1UL << DMA_CCR_EN_Pos)                       /*!< 0x00000001 */
#define DMA_CCR_EN             DMA_CCR_EN_Msk                                  /*!< Channel enable                      */
#define DMA_CCR_TCIE_Pos       (1U)
#define DMA_CCR_TCIE_Msk       (0x1UL << DMA_CCR_TCIE_Pos)                     /*!< 0x00000002 */
#define DMA_CCR_TCIE           DMA_CCR_TCIE_Msk                                /*!< Transfer complete interrupt enable  */
#define DMA_CCR_HTIE_Pos       (2U)
#define DMA_CCR_HTIE_Msk       (0x1UL << DMA_CCR_HTIE_Pos)                     /*!< 0x00000004 */
#define DMA_CCR_HTIE           DMA_CCR_HTIE_Msk                                /*!< Half Transfer interrupt enable      */
#define DMA_CCR_TEIE_Pos       (3U)
#define DMA_CCR_TEIE_Msk       (0x1UL << DMA_CCR_TEIE_Pos)                     /*!< 0x00000008 */
#define DMA_CCR_TEIE           DMA_CCR_TEIE_Msk                                /*!< Transfer error interrupt enable     */
#define DMA_CCR_DIR_Pos        (4U)
#define DMA_CCR_DIR_Msk        (0x1UL << DMA_CCR_DIR_Pos)                      /*!< 0x00000010 */
#define DMA_CCR_DIR            DMA_CCR_DIR_Msk                                 /*!< Data transfer direction             */
#define DMA_CCR_CIRC_Pos       (5U)
#define DMA_CCR_CIRC_Msk       (0x1UL << DMA_CCR_CIRC_Pos)                     /*!< 0x00000020 */
#define DMA_CCR_CIRC           DMA_CCR_CIRC_Msk                                /*!< Circular mode                       */
#define DMA_CCR_PINC_Pos       (6U)
#define DMA_CCR_PINC_Msk       (0x1UL << DMA_CCR_PINC_Pos)                     /*!< 0x00000040 */
#define DMA_CCR_PINC           DMA_CCR_PINC_Msk                                /*!< Peripheral increment mode           */
#define DMA_CCR_MINC_Pos       (7U)
#define DMA_CCR_MINC_Msk       (0x1UL << DMA_CCR_MINC_Pos)                     /*!< 0x00000080 */
#define DMA_CCR_MINC           DMA_CCR_MINC_Msk                                /*!< Memory increment mode               */
#define DMA_CCR_PSIZE_Pos      (8U)
#define DMA_CCR_PSIZE_Msk      (0x3UL << DMA_CCR_PSIZE_Pos)                    /*!< 0x00000300 */
#define DMA_CCR_PSIZE          DMA_CCR_PSIZE_Msk                               /*!< PSIZE[1:0] bits (Peripheral size)   */
#define DMA_CCR_PSIZE_0        (0x1UL << DMA_CCR_PSIZE_Pos)                    /*!< 0x00000100 */
#define DMA_CCR_PSIZE_1        (0x2UL << DMA_CCR_PSIZE_Pos)                    /*!< 0x00000200 */
#define DMA_CCR_MSIZE_Pos      (10U)
#define DMA_CCR_MSIZE_Msk      (0x3UL << DMA_CCR_MSIZE_Pos)                    /*!< 0x00000C00 */
#define DMA_CCR_MSIZE          DMA_CCR_MSIZE_Msk                               /*!< MSIZE[1:0] bits (Memory size)       */
#define DMA_CCR_MSIZE_0        (0x1UL << DMA_CCR_MSIZE_Pos)                    /*!< 0x00000400 */
#define DMA_CCR_MSIZE_1        (0x2UL << DMA_CCR_MSIZE_Pos)                    /*!< 0x00000800 */
#define DMA_CCR_PL_Pos         (12U)
#define DMA_CCR_PL_Msk         (0x3UL << DMA_CCR_PL_Pos)                       /*!< 0x00003000 */
#define DMA_CCR_PL             DMA_CCR_PL_Msk                                  /*!< PL[1:0] bits(Channel Priority level)*/
#define DMA_CCR_PL_0           (0x1UL << DMA_CCR_PL_Pos)                       /*!< 0x00001000 */
#define DMA_CCR_PL_1           (0x2UL << DMA_CCR_PL_Pos)                       /*!< 0x00002000 */
#define DMA_CCR_MEM2MEM_Pos    (14U)
#define DMA_CCR_MEM2MEM_Msk    (0x1UL << DMA_CCR_MEM2MEM_Pos)                  /*!< 0x00004000 */
#define DMA_CCR_MEM2MEM        DMA_CCR_MEM2MEM_Msk                             /*!< Memory to memory mode               */

/******************  Bit definition for DMA_CNDTR register  *****************/
#define DMA_CNDTR_NDT_Pos      (0U)
#define DMA_CNDTR_NDT_Msk      (0xFFFFUL << DMA_CNDTR_NDT_Pos)                 /*!< 0x0000FFFF */
#define DMA_CNDTR_NDT          DMA_CNDTR_NDT_Msk                               /*!< Number of data to Transfer          */

/******************  Bit definition for DMA_CPAR register  ******************/
#define DMA_CPAR_PA_Pos        (0U)
#define DMA_CPAR_PA_Msk        (0xFFFFFFFFUL << DMA_CPAR_PA_Pos)               /*!< 0xFFFFFFFF */
#define DMA_CPAR_PA            DMA_CPAR_PA_Msk                                 /*!< Peripheral Address                  */

/******************  Bit definition for DMA_CMAR register  ******************/
#define DMA_CMAR_MA_Pos        (0U)
#define DMA_CMAR_MA_Msk        (0xFFFFFFFFUL << DMA_CMAR_MA_Pos)               /*!< 0xFFFFFFFF */
#define DMA_CMAR_MA            DMA_CMAR_MA_Msk                                 /*!< Memory Address                      */

/****************************************************************************/
/*                                                                          */
/*                    External Interrupt/Event Controller (EXTI)            */
/*                                                                          */
/****************************************************************************/
/******************  Bit definition for EXTI_RTSR register  *****************/
#define EXTI_RTSR_RT0_Pos           (0U)
#define EXTI_RTSR_RT0_Msk           (0x1UL << EXTI_RTSR_RT0_Pos)             /*!< 0x00000001 */
#define EXTI_RTSR_RT0               EXTI_RTSR_RT0_Msk                        /*!< Rising trigger configuration for input line 0 */
#define EXTI_RTSR_RT1_Pos           (1U)
#define EXTI_RTSR_RT1_Msk           (0x1UL << EXTI_RTSR_RT1_Pos)             /*!< 0x00000002 */
#define EXTI_RTSR_RT1               EXTI_RTSR_RT1_Msk                        /*!< Rising trigger configuration for input line 1 */
#define EXTI_RTSR_RT2_Pos           (2U)
#define EXTI_RTSR_RT2_Msk           (0x1UL << EXTI_RTSR_RT2_Pos)             /*!< 0x00000004 */
#define EXTI_RTSR_RT2               EXTI_RTSR_RT2_Msk                        /*!< Rising trigger configuration for input line 2 */
#define EXTI_RTSR_RT3_Pos           (3U)
#define EXTI_RTSR_RT3_Msk           (0x1UL << EXTI_RTSR_RT3_Pos)             /*!< 0x00000008 */
#define EXTI_RTSR_RT3               EXTI_RTSR_RT3_Msk                        /*!< Rising trigger configuration for input line 3 */
#define EXTI_RTSR_RT4_Pos           (4U)
#define EXTI_RTSR_RT4_Msk           (0x1UL << EXTI_RTSR_RT4_Pos)             /*!< 0x00000010 */
#define EXTI_RTSR_RT4               EXTI_RTSR_RT4_Msk                        /*!< Rising trigger configuration for input line 4 */
#define EXTI_RTSR_RT5_Pos           (5U)
#define EXTI_RTSR_RT5_Msk           (0x1UL << EXTI_RTSR_RT5_Pos)             /*!< 0x00000020 */
#define EXTI_RTSR_RT5               EXTI_RTSR_RT5_Msk                        /*!< Rising trigger configuration for input line 5 */
#define EXTI_RTSR_RT6_Pos           (6U)
#define EXTI_RTSR_RT6_Msk           (0x1UL << EXTI_RTSR_RT6_Pos)             /*!< 0x00000040 */
#define EXTI_RTSR_RT6               EXTI_RTSR_RT6_Msk                        /*!< Rising trigger configuration for input line 6 */
#define EXTI_RTSR_RT7_Pos           (7U)
#define EXTI_RTSR_RT7_Msk           (0x1UL << EXTI_RTSR_RT7_Pos)             /*!< 0x00000080 */
#define EXTI_RTSR_RT7               EXTI_RTSR_RT7_Msk                        /*!< Rising trigger configuration for input line 7 */
#define EXTI_RTSR_RT8_Pos           (8U)
#define EXTI_RTSR_RT8_Msk           (0x1UL << EXTI_RTSR_RT8_Pos)             /*!< 0x00000100 */
#define EXTI_RTSR_RT8               EXTI_RTSR_RT8_Msk                        /*!< Rising trigger configuration for input line 8 */
#define EXTI_RTSR_RT9_Pos           (9U)
#define EXTI_RTSR_RT9_Msk           (0x1UL << EXTI_RTSR_RT9_Pos)             /*!< 0x00000200 */
#define EXTI_RTSR_RT9               EXTI_RTSR_RT9_Msk                        /*!< Rising trigger configuration for input line 9 */
#define EXTI_RTSR_RT10_Pos          (10U)
#define EXTI_RTSR_RT10_Msk          (0x1UL << EXTI_RTSR_RT10_Pos)            /*!< 0x00000400 */
#define EXTI_RTSR_RT10              EXTI_RTSR_RT10_Msk                       /*!< Rising trigger configuration for input line 10 */
#define EXTI_RTSR_RT11_Pos          (11U)
#define EXTI_RTSR_RT11_Msk          (0x1UL << EXTI_RTSR_RT11_Pos)            /*!< 0x00000800 */
#define EXTI_RTSR_RT11              EXTI_RTSR_RT11_Msk                       /*!< Rising trigger configuration for input line 11 */
#define EXTI_RTSR_RT12_Pos          (12U)
#define EXTI_RTSR_RT12_Msk          (0x1UL << EXTI_RTSR_RT12_Pos)            /*!< 0x00001000 */
#define EXTI_RTSR_RT12              EXTI_RTSR_RT12_Msk                       /*!< Rising trigger configuration for input line 12 */
#define EXTI_RTSR_RT13_Pos          (13U)
#define EXTI_RTSR_RT13_Msk          (0x1UL << EXTI_RTSR_RT13_Pos)            /*!< 0x00002000 */
#define EXTI_RTSR_RT13              EXTI_RTSR_RT13_Msk                       /*!< Rising trigger configuration for input line 13 */
#define EXTI_RTSR_RT14_Pos          (14U)
#define EXTI_RTSR_RT14_Msk          (0x1UL << EXTI_RTSR_RT14_Pos)            /*!< 0x00004000 */
#define EXTI_RTSR_RT14              EXTI_RTSR_RT14_Msk                       /*!< Rising trigger configuration for input line 14 */
#define EXTI_RTSR_RT15_Pos          (15U)
#define EXTI_RTSR_RT15_Msk          (0x1UL << EXTI_RTSR_RT15_Pos)            /*!< 0x00008000 */
#define EXTI_RTSR_RT15              EXTI_RTSR_RT15_Msk                       /*!< Rising trigger configuration for input line 15 */
#define EXTI_RTSR_RT16_Pos          (16U)
#define EXTI_RTSR_RT16_Msk          (0x1UL << EXTI_RTSR_RT16_Pos)            /*!< 0x00010000 */
#define EXTI_RTSR_RT16              EXTI_RTSR_RT16_Msk                       /*!< Rising trigger configuration for input line 16 */
#define EXTI_RTSR_RT17_Pos          (17U)
#define EXTI_RTSR_RT17_Msk          (0x1UL << EXTI_RTSR_RT16_Pos)            /*!< 0x00020000 */
#define EXTI_RTSR_RT17              EXTI_RTSR_RT17_Msk                       /*!< Rising trigger configuration for input line 17 */
#define EXTI_RTSR_RT18_Pos          (18U)
#define EXTI_RTSR_RT18_Msk          (0x1UL << EXTI_RTSR_RT18_Pos)            /*!< 0x00040000 */
#define EXTI_RTSR_RT18              EXTI_RTSR_RT18_Msk                       /*!< Rising trigger configuration for input line 18 */
#define EXTI_RTSR_RT20_Pos          (20U)
#define EXTI_RTSR_RT20_Msk          (0x1UL << EXTI_RTSR_RT20_Pos)            /*!< 0x00040000 */
#define EXTI_RTSR_RT20              EXTI_RTSR_RT20_Msk                       /*!< Rising trigger configuration for input line 20 */

/******************  Bit definition for EXTI_FTSR register  *****************/
#define EXTI_FTSR_FT0_Pos           (0U)
#define EXTI_FTSR_FT0_Msk           (0x1UL << EXTI_FTSR_FT0_Pos)             /*!< 0x00000001 */
#define EXTI_FTSR_FT0               EXTI_FTSR_FT0_Msk                        /*!< Falling trigger configuration for input line 0 */
#define EXTI_FTSR_FT1_Pos           (1U)
#define EXTI_FTSR_FT1_Msk           (0x1UL << EXTI_FTSR_FT1_Pos)             /*!< 0x00000002 */
#define EXTI_FTSR_FT1               EXTI_FTSR_FT1_Msk                        /*!< Falling trigger configuration for input line 1 */
#define EXTI_FTSR_FT2_Pos           (2U)
#define EXTI_FTSR_FT2_Msk           (0x1UL << EXTI_FTSR_FT2_Pos)             /*!< 0x00000004 */
#define EXTI_FTSR_FT2               EXTI_FTSR_FT2_Msk                        /*!< Falling trigger configuration for input line 2 */
#define EXTI_FTSR_FT3_Pos           (3U)
#define EXTI_FTSR_FT3_Msk           (0x1UL << EXTI_FTSR_FT3_Pos)             /*!< 0x00000008 */
#define EXTI_FTSR_FT3               EXTI_FTSR_FT3_Msk                        /*!< Falling trigger configuration for input line 3 */
#define EXTI_FTSR_FT4_Pos           (4U)
#define EXTI_FTSR_FT4_Msk           (0x1UL << EXTI_FTSR_FT4_Pos)             /*!< 0x00000010 */
#define EXTI_FTSR_FT4               EXTI_FTSR_FT4_Msk                        /*!< Falling trigger configuration for input line 4 */
#define EXTI_FTSR_FT5_Pos           (5U)
#define EXTI_FTSR_FT5_Msk           (0x1UL << EXTI_FTSR_FT5_Pos)             /*!< 0x00000020 */
#define EXTI_FTSR_FT5               EXTI_FTSR_FT5_Msk                        /*!< Falling trigger configuration for input line 5 */
#define EXTI_FTSR_FT6_Pos           (6U)
#define EXTI_FTSR_FT6_Msk           (0x1UL << EXTI_FTSR_FT6_Pos)             /*!< 0x00000040 */
#define EXTI_FTSR_FT6               EXTI_FTSR_FT6_Msk                        /*!< Falling trigger configuration for input line 6 */
#define EXTI_FTSR_FT7_Pos           (7U)
#define EXTI_FTSR_FT7_Msk           (0x1UL << EXTI_FTSR_FT7_Pos)             /*!< 0x00000080 */
#define EXTI_FTSR_FT7               EXTI_FTSR_FT7_Msk                        /*!< Falling trigger configuration for input line 7 */
#define EXTI_FTSR_FT8_Pos           (8U)
#define EXTI_FTSR_FT8_Msk           (0x1UL << EXTI_FTSR_FT8_Pos)             /*!< 0x00000100 */
#define EXTI_FTSR_FT8               EXTI_FTSR_FT8_Msk                        /*!< Falling trigger configuration for input line 8 */
#define EXTI_FTSR_FT9_Pos           (9U)
#define EXTI_FTSR_FT9_Msk           (0x1UL << EXTI_FTSR_FT9_Pos)             /*!< 0x00000200 */
#define EXTI_FTSR_FT9               EXTI_FTSR_FT9_Msk                        /*!< Falling trigger configuration for input line 9 */
#define EXTI_FTSR_FT10_Pos          (10U)
#define EXTI_FTSR_FT10_Msk          (0x1UL << EXTI_FTSR_FT10_Pos)            /*!< 0x00000400 */
#define EXTI_FTSR_FT10              EXTI_FTSR_FT10_Msk                       /*!< Falling trigger configuration for input line 10 */
#define EXTI_FTSR_FT11_Pos          (11U)
#define EXTI_FTSR_FT11_Msk          (0x1UL << EXTI_FTSR_FT11_Pos)            /*!< 0x00000800 */
#define EXTI_FTSR_FT11              EXTI_FTSR_FT11_Msk                       /*!< Falling trigger configuration for input line 11 */
#define EXTI_FTSR_FT12_Pos          (12U)
#define EXTI_FTSR_FT12_Msk          (0x1UL << EXTI_FTSR_FT12_Pos)            /*!< 0x00001000 */
#define EXTI_FTSR_FT12              EXTI_FTSR_FT12_Msk                       /*!< Falling trigger configuration for input line 12 */
#define EXTI_FTSR_FT13_Pos          (13U)
#define EXTI_FTSR_FT13_Msk          (0x1UL << EXTI_FTSR_FT13_Pos)            /*!< 0x00002000 */
#define EXTI_FTSR_FT13              EXTI_FTSR_FT13_Msk                       /*!< Falling trigger configuration for input line 13 */
#define EXTI_FTSR_FT14_Pos          (14U)
#define EXTI_FTSR_FT14_Msk          (0x1UL << EXTI_FTSR_FT14_Pos)            /*!< 0x00004000 */
#define EXTI_FTSR_FT14              EXTI_FTSR_FT14_Msk                       /*!< Falling trigger configuration for input line 14 */
#define EXTI_FTSR_FT15_Pos          (15U)
#define EXTI_FTSR_FT15_Msk          (0x1UL << EXTI_FTSR_FT15_Pos)            /*!< 0x00008000 */
#define EXTI_FTSR_FT15              EXTI_FTSR_FT15_Msk                       /*!< Falling trigger configuration for input line 15 */
#define EXTI_FTSR_FT16_Pos          (16U)
#define EXTI_FTSR_FT16_Msk          (0x1UL << EXTI_FTSR_FT16_Pos)            /*!< 0x00010000 */
#define EXTI_FTSR_FT16              EXTI_FTSR_FT16_Msk                       /*!< Falling trigger configuration for input line 16 */
#define EXTI_FTSR_FT17_Pos          (17U)
#define EXTI_FTSR_FT17_Msk          (0x1UL << EXTI_FTSR_FT17_Pos)            /*!< 0x00020000 */
#define EXTI_FTSR_FT17              EXTI_FTSR_FT17_Msk                       /*!< Falling trigger configuration for input line 17 */
#define EXTI_FTSR_FT18_Pos          (18U)
#define EXTI_FTSR_FT18_Msk          (0x1UL << EXTI_FTSR_FT18_Pos)            /*!< 0x00040000 */
#define EXTI_FTSR_FT18              EXTI_FTSR_FT18_Msk                       /*!< Falling trigger configuration for input line 18 */
#define EXTI_FTSR_FT20_Pos          (20U)
#define EXTI_FTSR_FT20_Msk          (0x1UL << EXTI_FTSR_FT20_Pos)            /*!< 0x00040000 */
#define EXTI_FTSR_FT20              EXTI_FTSR_FT20_Msk                       /*!< Falling trigger configuration for input line 20 */

/******************  Bit definition for EXTI_SWIER register  ****************/
#define EXTI_SWIER_SWI0_Pos         (0U)
#define EXTI_SWIER_SWI0_Msk         (0x1UL << EXTI_SWIER_SWI0_Pos)           /*!< 0x00000001 */
#define EXTI_SWIER_SWI0             EXTI_SWIER_SWI0_Msk                      /*!< Software Interrupt on line 0 */
#define EXTI_SWIER_SWI1_Pos         (1U)
#define EXTI_SWIER_SWI1_Msk         (0x1UL << EXTI_SWIER_SWI1_Pos)           /*!< 0x00000002 */
#define EXTI_SWIER_SWI1             EXTI_SWIER_SWI1_Msk                      /*!< Software Interrupt on line 1 */
#define EXTI_SWIER_SWI2_Pos         (2U)
#define EXTI_SWIER_SWI2_Msk         (0x1UL << EXTI_SWIER_SWI2_Pos)           /*!< 0x00000004 */
#define EXTI_SWIER_SWI2             EXTI_SWIER_SWI2_Msk                      /*!< Software Interrupt on line 2 */
#define EXTI_SWIER_SWI3_Pos         (3U)
#define EXTI_SWIER_SWI3_Msk         (0x1UL << EXTI_SWIER_SWI3_Pos)           /*!< 0x00000008 */
#define EXTI_SWIER_SWI3             EXTI_SWIER_SWI3_Msk                      /*!< Software Interrupt on line 3 */
#define EXTI_SWIER_SWI4_Pos         (4U)
#define EXTI_SWIER_SWI4_Msk         (0x1UL << EXTI_SWIER_SWI4_Pos)           /*!< 0x00000010 */
#define EXTI_SWIER_SWI4             EXTI_SWIER_SWI4_Msk                      /*!< Software Interrupt on line 4 */
#define EXTI_SWIER_SWI5_Pos         (5U)
#define EXTI_SWIER_SWI5_Msk         (0x1UL << EXTI_SWIER_SWI5_Pos)           /*!< 0x00000020 */
#define EXTI_SWIER_SWI5             EXTI_SWIER_SWI5_Msk                      /*!< Software Interrupt on line 5 */
#define EXTI_SWIER_SWI6_Pos         (6U)
#define EXTI_SWIER_SWI6_Msk         (0x1UL << EXTI_SWIER_SWI6_Pos)           /*!< 0x00000040 */
#define EXTI_SWIER_SWI6             EXTI_SWIER_SWI6_Msk                      /*!< Software Interrupt on line 6 */
#define EXTI_SWIER_SWI7_Pos         (7U)
#define EXTI_SWIER_SWI7_Msk         (0x1UL << EXTI_SWIER_SWI7_Pos)           /*!< 0x00000080 */
#define EXTI_SWIER_SWI7             EXTI_SWIER_SWI7_Msk                      /*!< Software Interrupt on line 7 */
#define EXTI_SWIER_SWI8_Pos         (8U)
#define EXTI_SWIER_SWI8_Msk         (0x1UL << EXTI_SWIER_SWI8_Pos)           /*!< 0x00000100 */
#define EXTI_SWIER_SWI8             EXTI_SWIER_SWI8_Msk                      /*!< Software Interrupt on line 8 */
#define EXTI_SWIER_SWI9_Pos         (9U)
#define EXTI_SWIER_SWI9_Msk         (0x1UL << EXTI_SWIER_SWI9_Pos)           /*!< 0x00000200 */
#define EXTI_SWIER_SWI9             EXTI_SWIER_SWI9_Msk                      /*!< Software Interrupt on line 9 */
#define EXTI_SWIER_SWI10_Pos        (10U)
#define EXTI_SWIER_SWI10_Msk        (0x1UL << EXTI_SWIER_SWI10_Pos)          /*!< 0x00000400 */
#define EXTI_SWIER_SWI10            EXTI_SWIER_SWI10_Msk                     /*!< Software Interrupt on line 10 */
#define EXTI_SWIER_SWI11_Pos        (11U)
#define EXTI_SWIER_SWI11_Msk        (0x1UL << EXTI_SWIER_SWI11_Pos)          /*!< 0x00000800 */
#define EXTI_SWIER_SWI11            EXTI_SWIER_SWI11_Msk                     /*!< Software Interrupt on line 11 */
#define EXTI_SWIER_SWI12_Pos        (12U)
#define EXTI_SWIER_SWI12_Msk        (0x1UL << EXTI_SWIER_SWI12_Pos)          /*!< 0x00001000 */
#define EXTI_SWIER_SWI12            EXTI_SWIER_SWI12_Msk                     /*!< Software Interrupt on line 12 */
#define EXTI_SWIER_SWI13_Pos        (13U)
#define EXTI_SWIER_SWI13_Msk        (0x1UL << EXTI_SWIER_SWI13_Pos)          /*!< 0x00002000 */
#define EXTI_SWIER_SWI13            EXTI_SWIER_SWI13_Msk                     /*!< Software Interrupt on line 13 */
#define EXTI_SWIER_SWI14_Pos        (14U)
#define EXTI_SWIER_SWI14_Msk        (0x1UL << EXTI_SWIER_SWI14_Pos)          /*!< 0x00004000 */
#define EXTI_SWIER_SWI14            EXTI_SWIER_SWI14_Msk                     /*!< Software Interrupt on line 14 */
#define EXTI_SWIER_SWI15_Pos        (15U)
#define EXTI_SWIER_SWI15_Msk        (0x1UL << EXTI_SWIER_SWI15_Pos)          /*!< 0x00008000 */
#define EXTI_SWIER_SWI15            EXTI_SWIER_SWI15_Msk                     /*!< Software Interrupt on line 15 */
#define EXTI_SWIER_SWI16_Pos        (16U)
#define EXTI_SWIER_SWI16_Msk        (0x1UL << EXTI_SWIER_SWI16_Pos)          /*!< 0x00010000 */
#define EXTI_SWIER_SWI16            EXTI_SWIER_SWI16_Msk                     /*!< Software Interrupt on line 16 */
#define EXTI_SWIER_SWI17_Pos        (17U)
#define EXTI_SWIER_SWI17_Msk        (0x1UL << EXTI_SWIER_SWI17_Pos)          /*!< 0x00020000 */
#define EXTI_SWIER_SWI17            EXTI_SWIER_SWI17_Msk                     /*!< Software Interrupt on line 17 */
#define EXTI_SWIER_SWI18_Pos        (18U)
#define EXTI_SWIER_SWI18_Msk        (0x1UL << EXTI_SWIER_SWI18_Pos)          /*!< 0x00040000 */
#define EXTI_SWIER_SWI18            EXTI_SWIER_SWI18_Msk                     /*!< Software Interrupt on line 18*/
#define EXTI_SWIER_SWI20_Pos        (20U)
#define EXTI_SWIER_SWI20_Msk        (0x1UL << EXTI_SWIER_SWI20_Pos)          /*!< 0x00040000 */
#define EXTI_SWIER_SWI20            EXTI_SWIER_SWI20_Msk                     /*!< Software Interrupt on line 20*/

/*******************  Bit definition for EXTI_PR register  ******************/
#define EXTI_PR_PR0_Pos             (0U)
#define EXTI_PR_PR0_Msk             (0x1UL << EXTI_PR_PR0_Pos)            /*!< 0x00000001 */
#define EXTI_PR_PR0                  EXTI_PR_PR0_Msk                      /*!< Rising Pending Interrupt Flag on line 0 */
#define EXTI_PR_PR1_Pos             (1U)
#define EXTI_PR_PR1_Msk             (0x1UL << EXTI_PR_PR1_Pos)            /*!< 0x00000002 */
#define EXTI_PR_PR1                 EXTI_PR_PR1_Msk                       /*!< Rising Pending Interrupt Flag on line 1 */
#define EXTI_PR_PR2_Pos             (2U)
#define EXTI_PR_PR2_Msk             (0x1UL << EXTI_PR_PR2_Pos)            /*!< 0x00000004 */
#define EXTI_PR_PR2                 EXTI_PR_PR2_Msk                       /*!< Rising Pending Interrupt Flag on line 2 */
#define EXTI_PR_PR3_Pos             (3U)
#define EXTI_PR_PR3_Msk             (0x1UL << EXTI_PR_PR3_Pos)            /*!< 0x00000008 */
#define EXTI_PR_PR3                 EXTI_PR_PR3_Msk                       /*!< Rising Pending Interrupt Flag on line 3 */
#define EXTI_PR_PR_Pos              (4U)
#define EXTI_PR_PR_Msk              (0x1UL <<EXTI_PR_PR_Pos )             /*!< 0x00000010 */
#define EXTI_PR_PR4                 EXTI_PR_PR_Msk                        /*!< Rising Pending Interrupt Flag on line 4 */
#define EXTI_PR_PR5_Pos             (5U)
#define EXTI_PR_PR5_Msk             (0x1UL <<EXTI_PR_PR5_Pos )            /*!< 0x00000020 */
#define EXTI_PR_PR5                 EXTI_PR_PR5_Msk                       /*!< Rising Pending Interrupt Flag on line 5 */
#define EXTI_PR_PR6_Pos             (6U)
#define EXTI_PR_PR6_Msk             (0x1UL << EXTI_PR_PR6_Pos)            /*!< 0x00000040 */
#define EXTI_PR_PR6                 EXTI_PR_PR6_Msk                       /*!< Rising Pending Interrupt Flag on line 6 */
#define EXTI_PR_PR7_Pos             (7U)
#define EXTI_PR_PR7_Msk             (0x1UL <<EXTI_PR_PR7_Pos)             /*!< 0x00000080 */
#define EXTI_PR_PR7                 EXTI_PR_PR7_Msk                       /*!< Rising Pending Interrupt Flag on line 7 */
#define EXTI_PR_PR8_Pos             (8U)
#define EXTI_PR_PR8_Msk             (0x1UL << EXTI_PR_PR8_Pos )           /*!< 0x00000100 */
#define EXTI_PR_PR8                 EXTI_PR_PR8_Msk                       /*!< Rising Pending Interrupt Flag on line 8 */
#define EXTI_PR_PR9_Pos             (9U)
#define EXTI_PR_PR9_Msk             (0x1UL << EXTI_PR_PR9_Pos)            /*!< 0x00000200 */
#define EXTI_PR_PR9                 EXTI_PR_PR9_Msk                       /*!< Rising Pending Interrupt Flag on line 9 */
#define EXTI_PR_PR10_Pos            (10U)
#define EXTI_PR_PR10_Msk            (0x1UL << EXTI_PR_PR10_Pos)           /*!< 0x00000400 */
#define EXTI_PR_PR10                EXTI_PR_PR10_Msk                      /*!< Rising Pending Interrupt Flag on line 10 */
#define EXTI_PR_PR11_Pos            (11U)
#define EXTI_PR_PR11_Msk            (0x1UL << EXTI_PR_PR11_Pos)           /*!< 0x00000800 */
#define EXTI_PR_PR11                EXTI_PR_PR11_Msk                      /*!< Rising Pending Interrupt Flag on line 11 */
#define EXTI_PR_PR12_Pos            (12U)
#define EXTI_PR_PR12_Msk            (0x1UL << EXTI_PR_PR12_Pos)           /*!< 0x00001000 */
#define EXTI_PR_PR12                EXTI_PR_PR12_Msk                      /*!< Rising Pending Interrupt Flag on line 12 */
#define EXTI_PR_PR13_Pos            (13U)
#define EXTI_PR_PR13_Msk            (0x1UL << EXTI_PR_PR13_Pos)           /*!< 0x00002000 */
#define EXTI_PR_PR13                EXTI_PR_PR13_Msk                      /*!< Rising Pending Interrupt Flag on line 13 */
#define EXTI_PR_PR14_Pos            (14U)
#define EXTI_PR_PR14_Msk            (0x1UL << EXTI_PR_PR14_Pos)           /*!< 0x00004000 */
#define EXTI_PR_PR14                EXTI_PR_PR14_Msk                      /*!< Rising Pending Interrupt Flag on line 14 */
#define EXTI_PR_PR15_Pos            (15U)
#define EXTI_PR_PR15_Msk            (0x1UL << EXTI_PR_PR15_Pos)           /*!< 0x00008000 */
#define EXTI_PR_PR15                EXTI_PR_PR15_Msk                      /*!< Rising Pending Interrupt Flag on line 15 */
#define EXTI_PR_PR16_Pos            (16U)
#define EXTI_PR_PR16_Msk            (0x1UL << EXTI_PR_PR16_Pos)           /*!< 0x00010000 */
#define EXTI_PR_PR16                EXTI_PR_PR16_Msk                      /*!< Rising Pending Interrupt Flag on line 16 */
#define EXTI_PR_PR17_Pos            (17U)
#define EXTI_PR_PR17_Msk            (0x1UL << EXTI_PR_PR17_Pos)           /*!< 0x00020000 */
#define EXTI_PR_PR17                EXTI_PR_PR17_Msk                      /*!< Rising Pending Interrupt Flag on line 17 */
#define EXTI_PR_PR18_Pos            (18U)
#define EXTI_PR_PR18_Msk            (0x1UL << EXTI_PR_PR18_Pos)           /*!< 0x00080000 */
#define EXTI_PR_PR18                EXTI_PR_PR18_Msk                      /*!< Rising Pending Interrupt Flag on line 18 */
#define EXTI_PR_PR20_Pos            (20U)
#define EXTI_PR_PR20_Msk            (0x1UL << EXTI_PR_PR20_Pos)           /*!< 0x00080000 */
#define EXTI_PR_PR20                EXTI_PR_PR20_Msk                      /*!< Rising Pending Interrupt Flag on line 20 */

/*****************  Bit definition for EXTI_EXTICR1 register  ***************/
#define EXTI_EXTICR1_EXTI0_Pos       (0U)
#define EXTI_EXTICR1_EXTI0_Msk       (0x3UL << EXTI_EXTICR1_EXTI0_Pos)         /*!< 0x00000003 */
#define EXTI_EXTICR1_EXTI0           EXTI_EXTICR1_EXTI0_Msk                    /*!< EXTI 0 configuration */
#define EXTI_EXTICR1_EXTI0_0         (0x1UL << EXTI_EXTICR1_EXTI0_Pos)         /*!< 0x00000001 */
#define EXTI_EXTICR1_EXTI0_1         (0x2UL << EXTI_EXTICR1_EXTI0_Pos)         /*!< 0x00000002 */
#define EXTI_EXTICR1_EXTI1_Pos       (8U)
#define EXTI_EXTICR1_EXTI1_Msk       (0x3UL << EXTI_EXTICR1_EXTI1_Pos)         /*!< 0x00000300 */
#define EXTI_EXTICR1_EXTI1           EXTI_EXTICR1_EXTI1_Msk                    /*!< EXTI 1 configuration */
#define EXTI_EXTICR1_EXTI1_0         (0x1UL << EXTI_EXTICR1_EXTI1_Pos)         /*!< 0x00000100 */
#define EXTI_EXTICR1_EXTI1_1         (0x2UL << EXTI_EXTICR1_EXTI1_Pos)         /*!< 0x00000200 */
#define EXTI_EXTICR1_EXTI2_Pos       (16U)
#define EXTI_EXTICR1_EXTI2_Msk       (0x3UL << EXTI_EXTICR1_EXTI2_Pos)         /*!< 0x00030000 */
#define EXTI_EXTICR1_EXTI2           EXTI_EXTICR1_EXTI2_Msk                    /*!< EXTI 2 configuration */
#define EXTI_EXTICR1_EXTI2_0         (0x1UL << EXTI_EXTICR1_EXTI2_Pos)         /*!< 0x00010000 */
#define EXTI_EXTICR1_EXTI2_1         (0x2UL << EXTI_EXTICR1_EXTI2_Pos)         /*!< 0x00020000 */
#define EXTI_EXTICR1_EXTI3_Pos       (24U)
#define EXTI_EXTICR1_EXTI3_Msk       (0x3UL << EXTI_EXTICR1_EXTI3_Pos)         /*!< 0x03000000 */
#define EXTI_EXTICR1_EXTI3           EXTI_EXTICR1_EXTI3_Msk                    /*!< EXTI 3 configuration */
#define EXTI_EXTICR1_EXTI3_0         (0x1UL << EXTI_EXTICR1_EXTI3_Pos)         /*!< 0x01000000 */
#define EXTI_EXTICR1_EXTI3_1         (0x2UL << EXTI_EXTICR1_EXTI3_Pos)         /*!< 0x02000000 */

/*****************  Bit definition for EXTI_EXTICR2 register  ***************/
#define EXTI_EXTICR2_EXTI4_Pos       (0U)
#define EXTI_EXTICR2_EXTI4_Msk       (0x3UL << EXTI_EXTICR2_EXTI4_Pos)         /*!< 0x00000003 */
#define EXTI_EXTICR2_EXTI4           EXTI_EXTICR2_EXTI4_Msk                    /*!< EXTI 4 configuration */
#define EXTI_EXTICR2_EXTI4_0         (0x1UL << EXTI_EXTICR2_EXTI4_Pos)         /*!< 0x00000001 */
#define EXTI_EXTICR2_EXTI4_1         (0x2UL << EXTI_EXTICR2_EXTI4_Pos)         /*!< 0x00000002 */
#define EXTI_EXTICR2_EXTI5_Pos       (8U)
#define EXTI_EXTICR2_EXTI5_Msk       (0x1UL << EXTI_EXTICR2_EXTI5_Pos)         /*!< 0x00000100 */
#define EXTI_EXTICR2_EXTI5           EXTI_EXTICR2_EXTI5_Msk                    /*!< EXTI 5 configuration */
#define EXTI_EXTICR2_EXTI5_0         (0x1UL << EXTI_EXTICR2_EXTI5_Pos)         /*!< 0x00000001 */
#define EXTI_EXTICR2_EXTI5_1         (0x2UL << EXTI_EXTICR2_EXTI5_Pos)         /*!< 0x00000002 */
#define EXTI_EXTICR2_EXTI6_Pos       (16U)
#define EXTI_EXTICR2_EXTI6_Msk       (0x1UL << EXTI_EXTICR2_EXTI6_Pos)         /*!< 0x00010000 */
#define EXTI_EXTICR2_EXTI6           EXTI_EXTICR2_EXTI6_Msk                    /*!< EXTI 6 configuration */
#define EXTI_EXTICR2_EXTI6_0         (0x1UL << EXTI_EXTICR2_EXTI6_Pos)         /*!< 0x00000001 */
#define EXTI_EXTICR2_EXTI6_1         (0x2UL << EXTI_EXTICR2_EXTI6_Pos)         /*!< 0x00000002 */
#define EXTI_EXTICR2_EXTI7_Pos       (24U)
#define EXTI_EXTICR2_EXTI7_Msk       (0x1UL << EXTI_EXTICR2_EXTI7_Pos)         /*!< 0x01000000 */
#define EXTI_EXTICR2_EXTI7           EXTI_EXTICR2_EXTI7_Msk                    /*!< EXTI 7 configuration */
#define EXTI_EXTICR2_EXTI7_0         (0x1UL << EXTI_EXTICR2_EXTI7_Pos)         /*!< 0x00000001 */
#define EXTI_EXTICR2_EXTI7_1         (0x2UL << EXTI_EXTICR2_EXTI7_Pos)         /*!< 0x00000002 */

/*****************  Bit definition for EXTI_EXTICR3 register  ***************/
#define EXTI_EXTICR3_EXTI8_Pos       (0U)
#define EXTI_EXTICR3_EXTI8_Msk       (0x3UL << EXTI_EXTICR3_EXTI8_Pos)         /*!< 0x00000003 */
#define EXTI_EXTICR3_EXTI8           EXTI_EXTICR3_EXTI8_Msk                    /*!< EXTI 8 configuration */
#define EXTI_EXTICR3_EXTI8_0         (0x1UL << EXTI_EXTICR3_EXTI8_Pos)         /*!< 0x00000001 */
#define EXTI_EXTICR3_EXTI8_1         (0x2UL << EXTI_EXTICR3_EXTI8_Pos)         /*!< 0x00000002 */
#define EXTI_EXTICR3_EXTI9_Pos       (8U)
#define EXTI_EXTICR3_EXTI9_Msk       (0x3UL << EXTI_EXTICR3_EXTI9_Pos)         /*!< 0x00000300 */
#define EXTI_EXTICR3_EXTI9           EXTI_EXTICR3_EXTI9_Msk                    /*!< EXTI 9 configuration */
#define EXTI_EXTICR3_EXTI9_0         (0x1UL << EXTI_EXTICR3_EXTI9_Pos)         /*!< 0x00000100 */
#define EXTI_EXTICR3_EXTI9_1         (0x2UL << EXTI_EXTICR3_EXTI9_Pos)         /*!< 0x00000200 */
#define EXTI_EXTICR3_EXTI10_Pos      (16U)
#define EXTI_EXTICR3_EXTI10_Msk      (0x3UL << EXTI_EXTICR3_EXTI10_Pos)         /*!< 0x00030000 */
#define EXTI_EXTICR3_EXTI10          EXTI_EXTICR3_EXTI10_Msk                    /*!< EXTI10 configuration */
#define EXTI_EXTICR3_EXTI10_0        (0x1UL << EXTI_EXTICR3_EXTI10_Pos)         /*!< 0x00010000 */
#define EXTI_EXTICR3_EXTI10_1        (0x2UL << EXTI_EXTICR3_EXTI10_Pos)         /*!< 0x00020000 */
#define EXTI_EXTICR3_EXTI11_Pos      (24U)
#define EXTI_EXTICR3_EXTI11_Msk      (0x3UL << EXTI_EXTICR3_EXTI11_Pos)         /*!< 0x03000000 */
#define EXTI_EXTICR3_EXTI11          EXTI_EXTICR3_EXTI11_Msk                    /*!< EXTI 11 configuration */
#define EXTI_EXTICR3_EXTI11_0        (0x1UL << EXTI_EXTICR3_EXTI11_Pos)         /*!< 0x01000000 */
#define EXTI_EXTICR3_EXTI11_1        (0x2UL << EXTI_EXTICR3_EXTI11_Pos)         /*!< 0x02000000 */

/*****************  Bit definition for EXTI_EXTICR4 register  ***************/
#define EXTI_EXTICR4_EXTI12_Pos       (0U)
#define EXTI_EXTICR4_EXTI12_Msk       (0x3UL << EXTI_EXTICR4_EXTI12_Pos)         /*!< 0x00000003 */
#define EXTI_EXTICR4_EXTI12           EXTI_EXTICR4_EXTI12_Msk                    /*!< EXTI 12 configuration */
#define EXTI_EXTICR4_EXTI12_0         (0x1UL << EXTI_EXTICR4_EXTI12_Pos)         /*!< 0x00000001 */
#define EXTI_EXTICR4_EXTI12_1         (0x2UL << EXTI_EXTICR4_EXTI12_Pos)         /*!< 0x00000002 */
#define EXTI_EXTICR4_EXTI13_Pos       (8U)
#define EXTI_EXTICR4_EXTI13_Msk       (0x3UL << EXTI_EXTICR4_EXTI13_Pos)         /*!< 0x00000300 */
#define EXTI_EXTICR4_EXTI13           EXTI_EXTICR4_EXTI13_Msk                    /*!< EXTI 13 configuration */
#define EXTI_EXTICR4_EXTI13_0         (0x1UL << EXTI_EXTICR4_EXTI13_Pos)         /*!< 0x00000100 */
#define EXTI_EXTICR4_EXTI13_1         (0x2UL << EXTI_EXTICR4_EXTI13_Pos)         /*!< 0x00000200 */
#define EXTI_EXTICR4_EXTI14_Pos       (16U)
#define EXTI_EXTICR4_EXTI14_Msk       (0x3UL << EXTI_EXTICR4_EXTI14_Pos)         /*!< 0x00030000 */
#define EXTI_EXTICR4_EXTI14           EXTI_EXTICR4_EXTI14_Msk                    /*!< EXTI 14 configuration */
#define EXTI_EXTICR4_EXTI14_0         (0x1UL << EXTI_EXTICR4_EXTI14_Pos)         /*!< 0x00010000 */
#define EXTI_EXTICR4_EXTI14_1         (0x2UL << EXTI_EXTICR4_EXTI14_Pos)         /*!< 0x00020000 */
#define EXTI_EXTICR4_EXTI15_Pos       (24U)
#define EXTI_EXTICR4_EXTI15_Msk       (0x3UL << EXTI_EXTICR4_EXTI15_Pos)         /*!< 0x03000000 */
#define EXTI_EXTICR4_EXTI15           EXTI_EXTICR4_EXTI15_Msk                    /*!< EXTI 15 configuration */
#define EXTI_EXTICR4_EXTI15_0         (0x1UL << EXTI_EXTICR4_EXTI15_Pos)         /*!< 0x01000000 */
#define EXTI_EXTICR4_EXTI15_1         (0x2UL << EXTI_EXTICR4_EXTI15_Pos)         /*!< 0x02000000 */

/*******************  Bit definition for EXTI_IMR register  *****************/
#define EXTI_IMR_IM_Pos             (0U)
#define EXTI_IMR_IM_Msk             (0x201FFFFFUL << EXTI_IMR_IM_Pos)        /*!< 0x200FFFFF */
#define EXTI_IMR_IM                 EXTI_IMR_IM_Msk                          /*!< Interrupt Mask All */
#define EXTI_IMR_IM0_Pos            (0U)
#define EXTI_IMR_IM0_Msk            (0x1UL << EXTI_IMR_IM0_Pos)              /*!< 0x00000001 */
#define EXTI_IMR_IM0                EXTI_IMR_IM0_Msk                         /*!< Interrupt Mask on line 0 */
#define EXTI_IMR_IM1_Pos            (1U)
#define EXTI_IMR_IM1_Msk            (0x1UL << EXTI_IMR_IM1_Pos)              /*!< 0x00000002 */
#define EXTI_IMR_IM1                EXTI_IMR_IM1_Msk                         /*!< Interrupt Mask on line 1 */
#define EXTI_IMR_IM2_Pos            (2U)
#define EXTI_IMR_IM2_Msk            (0x1UL << EXTI_IMR_IM2_Pos)              /*!< 0x00000004 */
#define EXTI_IMR_IM2                EXTI_IMR_IM2_Msk                         /*!< Interrupt Mask on line 2 */
#define EXTI_IMR_IM3_Pos            (3U)
#define EXTI_IMR_IM3_Msk            (0x1UL << EXTI_IMR_IM3_Pos)              /*!< 0x00000008 */
#define EXTI_IMR_IM3                EXTI_IMR_IM3_Msk                         /*!< Interrupt Mask on line 3 */
#define EXTI_IMR_IM4_Pos            (4U)
#define EXTI_IMR_IM4_Msk            (0x1UL << EXTI_IMR_IM4_Pos)              /*!< 0x00000010 */
#define EXTI_IMR_IM4                EXTI_IMR_IM4_Msk                         /*!< Interrupt Mask on line 4 */
#define EXTI_IMR_IM5_Pos            (5U)
#define EXTI_IMR_IM5_Msk            (0x1UL << EXTI_IMR_IM5_Pos)              /*!< 0x00000020 */
#define EXTI_IMR_IM5                EXTI_IMR_IM5_Msk                         /*!< Interrupt Mask on line 5 */
#define EXTI_IMR_IM6_Pos            (6U)
#define EXTI_IMR_IM6_Msk            (0x1UL << EXTI_IMR_IM6_Pos)              /*!< 0x00000040 */
#define EXTI_IMR_IM6                EXTI_IMR_IM6_Msk                         /*!< Interrupt Mask on line 6 */
#define EXTI_IMR_IM7_Pos            (7U)
#define EXTI_IMR_IM7_Msk            (0x1UL << EXTI_IMR_IM7_Pos)              /*!< 0x00000080 */
#define EXTI_IMR_IM7                EXTI_IMR_IM7_Msk                         /*!< Interrupt Mask on line 7 */
#define EXTI_IMR_IM8_Pos            (8U)
#define EXTI_IMR_IM8_Msk            (0x1UL << EXTI_IMR_IM8_Pos)              /*!< 0x00000100 */
#define EXTI_IMR_IM8                EXTI_IMR_IM8_Msk                         /*!< Interrupt Mask on line 8 */
#define EXTI_IMR_IM9_Pos            (9U)
#define EXTI_IMR_IM9_Msk            (0x1UL << EXTI_IMR_IM9_Pos)              /*!< 0x00000200 */
#define EXTI_IMR_IM9                EXTI_IMR_IM9_Msk                         /*!< Interrupt Mask on line 9 */
#define EXTI_IMR_IM10_Pos           (10U)
#define EXTI_IMR_IM10_Msk           (0x1UL << EXTI_IMR_IM10_Pos)             /*!< 0x00000400 */
#define EXTI_IMR_IM10               EXTI_IMR_IM10_Msk                        /*!< Interrupt Mask on line 10 */
#define EXTI_IMR_IM11_Pos           (11U)
#define EXTI_IMR_IM11_Msk           (0x1UL << EXTI_IMR_IM11_Pos)             /*!< 0x00000800 */
#define EXTI_IMR_IM11               EXTI_IMR_IM11_Msk                        /*!< Interrupt Mask on line 11 */
#define EXTI_IMR_IM12_Pos           (12U)
#define EXTI_IMR_IM12_Msk           (0x1UL << EXTI_IMR_IM12_Pos)             /*!< 0x00001000 */
#define EXTI_IMR_IM12               EXTI_IMR_IM12_Msk                        /*!< Interrupt Mask on line 12 */
#define EXTI_IMR_IM13_Pos           (13U)
#define EXTI_IMR_IM13_Msk           (0x1UL << EXTI_IMR_IM13_Pos)             /*!< 0x00002000 */
#define EXTI_IMR_IM13               EXTI_IMR_IM13_Msk                        /*!< Interrupt Mask on line 13 */
#define EXTI_IMR_IM14_Pos           (14U)
#define EXTI_IMR_IM14_Msk           (0x1UL << EXTI_IMR_IM14_Pos)             /*!< 0x00004000 */
#define EXTI_IMR_IM14               EXTI_IMR_IM14_Msk                        /*!< Interrupt Mask on line 14 */
#define EXTI_IMR_IM15_Pos           (15U)
#define EXTI_IMR_IM15_Msk           (0x1UL << EXTI_IMR_IM15_Pos)             /*!< 0x00008000 */
#define EXTI_IMR_IM15               EXTI_IMR_IM15_Msk                        /*!< Interrupt Mask on line 15 */
#define EXTI_IMR_IM16_Pos           (16U)
#define EXTI_IMR_IM16_Msk           (0x1UL << EXTI_IMR_IM16_Pos)             /*!< 0x00010000 */
#define EXTI_IMR_IM16               EXTI_IMR_IM16_Msk                        /*!< Interrupt Mask on line 16 */
#define EXTI_IMR_IM17_Pos           (17U)
#define EXTI_IMR_IM17_Msk           (0x1UL << EXTI_IMR_IM17_Pos)             /*!< 0x00020000 */
#define EXTI_IMR_IM17               EXTI_IMR_IM17_Msk                        /*!< Interrupt Mask on line 17 */
#define EXTI_IMR_IM18_Pos           (18U)
#define EXTI_IMR_IM18_Msk           (0x1UL << EXTI_IMR_IM18_Pos)             /*!< 0x00040000 */
#define EXTI_IMR_IM18               EXTI_IMR_IM18_Msk                        /*!< Interrupt Mask on line 18 */
#define EXTI_IMR_IM19_Pos           (19U)
#define EXTI_IMR_IM19_Msk           (0x1UL << EXTI_IMR_IM19_Pos)             /*!< 0x00080000 */
#define EXTI_IMR_IM19               EXTI_IMR_IM19_Msk                        /*!< Interrupt Mask on line 19 */
#define EXTI_IMR_IM20_Pos           (20U)
#define EXTI_IMR_IM20_Msk           (0x1UL << EXTI_IMR_IM20_Pos)             /*!< 0x20000000 */
#define EXTI_IMR_IM20               EXTI_IMR_IM20_Msk                        /*!< Interrupt Mask on line 20 */
#define EXTI_IMR_IM29_Pos           (29U)
#define EXTI_IMR_IM29_Msk           (0x1UL << EXTI_IMR_IM29_Pos)             /*!< 0x20000000 */
#define EXTI_IMR_IM29               EXTI_IMR_IM29_Msk                        /*!< Interrupt Mask on line 29 */

/*******************  Bit definition for EXTI_EMR register  *****************/
#define EXTI_EMR_EM_Pos             (0U)
#define EXTI_EMR_EM_Msk             (0x201FFFFFUL << EXTI_EMR_EM_Pos)        /*!< 0x200FFFFF */
#define EXTI_EMR_EM                 EXTI_EMR_EM_Msk                          /*!< Event Mask All */
#define EXTI_EMR_EM0_Pos            (0U)
#define EXTI_EMR_EM0_Msk            (0x1UL << EXTI_EMR_EM0_Pos)              /*!< 0x00000001 */
#define EXTI_EMR_EM0                EXTI_EMR_EM0_Msk                         /*!< Event Mask on line 0 */
#define EXTI_EMR_EM1_Pos            (1U)
#define EXTI_EMR_EM1_Msk            (0x1UL << EXTI_EMR_EM1_Pos)              /*!< 0x00000002 */
#define EXTI_EMR_EM1                EXTI_EMR_EM1_Msk                         /*!< Event Mask on line 1 */
#define EXTI_EMR_EM2_Pos            (2U)
#define EXTI_EMR_EM2_Msk            (0x1UL << EXTI_EMR_EM2_Pos)              /*!< 0x00000004 */
#define EXTI_EMR_EM2                EXTI_EMR_EM2_Msk                         /*!< Event Mask on line 2 */
#define EXTI_EMR_EM3_Pos            (3U)
#define EXTI_EMR_EM3_Msk            (0x1UL << EXTI_EMR_EM3_Pos)              /*!< 0x00000008 */
#define EXTI_EMR_EM3                EXTI_EMR_EM3_Msk                         /*!< Event Mask on line 3 */
#define EXTI_EMR_EM4_Pos            (4U)
#define EXTI_EMR_EM4_Msk            (0x1UL << EXTI_EMR_EM4_Pos)              /*!< 0x00000010 */
#define EXTI_EMR_EM4                EXTI_EMR_EM4_Msk                         /*!< Event Mask on line 4 */
#define EXTI_EMR_EM5_Pos            (5U)
#define EXTI_EMR_EM5_Msk            (0x1UL << EXTI_EMR_EM5_Pos)              /*!< 0x00000020 */
#define EXTI_EMR_EM5                EXTI_EMR_EM5_Msk                         /*!< Event Mask on line 5 */
#define EXTI_EMR_EM6_Pos            (6U)
#define EXTI_EMR_EM6_Msk            (0x1UL << EXTI_EMR_EM6_Pos)              /*!< 0x00000040 */
#define EXTI_EMR_EM6                EXTI_EMR_EM6_Msk                         /*!< Event Mask on line 6 */
#define EXTI_EMR_EM7_Pos            (7U)
#define EXTI_EMR_EM7_Msk            (0x1UL << EXTI_EMR_EM7_Pos)              /*!< 0x00000080 */
#define EXTI_EMR_EM7                EXTI_EMR_EM7_Msk                         /*!< Event Mask on line 7 */
#define EXTI_EMR_EM8_Pos            (8U)
#define EXTI_EMR_EM8_Msk            (0x1UL << EXTI_EMR_EM8_Pos)              /*!< 0x00000100 */
#define EXTI_EMR_EM8                EXTI_EMR_EM8_Msk                         /*!< Event Mask on line 8 */
#define EXTI_EMR_EM9_Pos            (9U)
#define EXTI_EMR_EM9_Msk            (0x1UL << EXTI_EMR_EM9_Pos)              /*!< 0x00000200 */
#define EXTI_EMR_EM9                EXTI_EMR_EM9_Msk                         /*!< Event Mask on line 9 */
#define EXTI_EMR_EM10_Pos           (10U)
#define EXTI_EMR_EM10_Msk           (0x1UL << EXTI_EMR_EM10_Pos)             /*!< 0x00000400 */
#define EXTI_EMR_EM10               EXTI_EMR_EM10_Msk                        /*!< Event Mask on line 10 */
#define EXTI_EMR_EM11_Pos           (11U)
#define EXTI_EMR_EM11_Msk           (0x1UL << EXTI_EMR_EM11_Pos)             /*!< 0x00000800 */
#define EXTI_EMR_EM11               EXTI_EMR_EM11_Msk                        /*!< Event Mask on line 11 */
#define EXTI_EMR_EM12_Pos           (12U)
#define EXTI_EMR_EM12_Msk           (0x1UL << EXTI_EMR_EM12_Pos)             /*!< 0x00001000 */
#define EXTI_EMR_EM12               EXTI_EMR_EM12_Msk                        /*!< Event Mask on line 12 */
#define EXTI_EMR_EM13_Pos           (13U)
#define EXTI_EMR_EM13_Msk           (0x1UL << EXTI_EMR_EM13_Pos)             /*!< 0x00002000 */
#define EXTI_EMR_EM13               EXTI_EMR_EM13_Msk                        /*!< Event Mask on line 13 */
#define EXTI_EMR_EM14_Pos           (14U)
#define EXTI_EMR_EM14_Msk           (0x1UL << EXTI_EMR_EM14_Pos)             /*!< 0x00004000 */
#define EXTI_EMR_EM14               EXTI_EMR_EM14_Msk                        /*!< Event Mask on line 14 */
#define EXTI_EMR_EM15_Pos           (15U)
#define EXTI_EMR_EM15_Msk           (0x1UL << EXTI_EMR_EM15_Pos)             /*!< 0x00008000 */
#define EXTI_EMR_EM15               EXTI_EMR_EM15_Msk                        /*!< Event Mask on line 15 */
#define EXTI_EMR_EM16_Pos           (16U)
#define EXTI_EMR_EM16_Msk           (0x1UL << EXTI_EMR_EM16_Pos)             /*!< 0x00010000 */
#define EXTI_EMR_EM16               EXTI_EMR_EM16_Msk                        /*!< Event Mask on line 16 */
#define EXTI_EMR_EM17_Pos           (17U)
#define EXTI_EMR_EM17_Msk           (0x1UL << EXTI_EMR_EM17_Pos)             /*!< 0x00020000 */
#define EXTI_EMR_EM17               EXTI_EMR_EM17_Msk                        /*!< Event Mask on line 17 */
#define EXTI_EMR_EM18_Pos           (18U)
#define EXTI_EMR_EM18_Msk           (0x1UL << EXTI_EMR_EM18_Pos)             /*!< 0x00040000 */
#define EXTI_EMR_EM18               EXTI_EMR_EM18_Msk                        /*!< Event Mask on line 18 */
#define EXTI_EMR_EM19_Pos           (19U)
#define EXTI_EMR_EM19_Msk           (0x1UL << EXTI_EMR_EM19_Pos)             /*!< 0x00080000 */
#define EXTI_EMR_EM19               EXTI_EMR_EM19_Msk                        /*!< Event Mask on line 19 */
#define EXTI_EMR_EM20_Pos           (20U)
#define EXTI_EMR_EM20_Msk           (0x1UL << EXTI_EMR_EM20_Pos)             /*!< 0x20000000 */
#define EXTI_EMR_EM20               EXTI_EMR_EM20_Msk                        /*!< Event Mask on line 20 */
#define EXTI_EMR_EM29_Pos           (29U)
#define EXTI_EMR_EM29_Msk           (0x1UL << EXTI_EMR_EM29_Pos)             /*!< 0x20000000 */
#define EXTI_EMR_EM29               EXTI_EMR_EM29_Msk                        /*!< Event Mask on line 29 */

/****************************************************************************/
/*                                                                          */
/*                   FLASH and Option Bytes Registers                       */
/*                                                                          */
/****************************************************************************/
#define GPIO_NRST_CONFIG_SUPPORT         /*!< GPIO feature available only on specific devices: Configure NRST pin */
#define FLASH_SECURABLE_MEMORY_SUPPORT   /*!< Flash feature available only on specific devices: allow to secure memory */
#define FLASH_PCROP_SUPPORT              /*!< Flash feature available only on specific devices: proprietary code read protection areas selected by option */

/*******************  Bits definition for FLASH_ACR register  ***************/
#define FLASH_ACR_LATENCY_Pos             (0U)
#define FLASH_ACR_LATENCY_Msk             (0x3UL << FLASH_ACR_LATENCY_Pos)     /*!< 0x00000001 */
#define FLASH_ACR_LATENCY_0               (0x1UL << FLASH_ACR_LATENCY_Pos)     /*!< 0x00000001 */
#define FLASH_ACR_LATENCY_1               (0x2UL << FLASH_ACR_LATENCY_Pos)     /*!< 0x00000001 */
#define FLASH_ACR_LATENCY                 FLASH_ACR_LATENCY_Msk

/******************  Bit definition for FLASH_KEYR register  ****************/
#define FLASH_KEYR_KEY_Pos                (0U)
#define FLASH_KEYR_KEY_Msk                (0xFFFFFFFFUL << FLASH_KEYR_KEY_Pos) /*!< 0xFFFFFFFF */
#define FLASH_KEYR_KEY                    FLASH_KEYR_KEY_Msk                   /*!< FPEC Key */

/*****************  Bit definition for FLASH_OPTKEYR register  **************/
#define FLASH_OPTKEYR_OPTKEY_Pos          (0U)
#define FLASH_OPTKEYR_OPTKEY_Msk          (0xFFFFFFFFUL << FLASH_OPTKEYR_OPTKEY_Pos) /*!< 0xFFFFFFFF */
#define FLASH_OPTKEYR_OPTKEY              FLASH_OPTKEYR_OPTKEY_Msk                   /*!< Option Byte Key */

/******************  FLASH Keys  ********************************************/
#define FLASH_KEY1_Pos                    (0U)
#define FLASH_KEY1_Msk                    (0x45670123UL << FLASH_KEY1_Pos)           /*!< 0x45670123 */
#define FLASH_KEY1                        FLASH_KEY1_Msk                             /*!< Flash program erase key1 */
#define FLASH_KEY2_Pos                    (0U)
#define FLASH_KEY2_Msk                    (0xCDEF89ABUL << FLASH_KEY2_Pos)           /*!< 0xCDEF89AB */
#define FLASH_KEY2                        FLASH_KEY2_Msk                             /*!< Flash program erase key2: used with FLASH_PEKEY1
                                                                                      to unlock the write access to the FPEC. */

#define FLASH_OPTKEY1_Pos                 (0U)
#define FLASH_OPTKEY1_Msk                 (0x08192A3BUL << FLASH_OPTKEY1_Pos)        /*!< 0x08192A3B */
#define FLASH_OPTKEY1                     FLASH_OPTKEY1_Msk                          /*!< Flash option key1 */
#define FLASH_OPTKEY2_Pos                 (0U)
#define FLASH_OPTKEY2_Msk                 (0x4C5D6E7FUL << FLASH_OPTKEY2_Pos)        /*!< 0x4C5D6E7F */
#define FLASH_OPTKEY2                     FLASH_OPTKEY2_Msk                          /*!< Flash option key2: used with FLASH_OPTKEY1 to
                                                                                      unlock the write access to the option byte block */

/*******************  Bits definition for FLASH_SR register  ****************/
#define FLASH_SR_EOP_Pos                  (0U)
#define FLASH_SR_EOP_Msk                  (0x1UL << FLASH_SR_EOP_Pos)      /*!< 0x00000001 */
#define FLASH_SR_EOP                      FLASH_SR_EOP_Msk
#define FLASH_SR_WRPERR_Pos               (4U)
#define FLASH_SR_WRPERR_Msk               (0x1UL << FLASH_SR_WRPERR_Pos)   /*!< 0x00000010 */
#define FLASH_SR_WRPERR                   FLASH_SR_WRPERR_Msk
#define FLASH_SR_OPTVERR_Pos              (15U)
#define FLASH_SR_OPTVERR_Msk              (0x1UL << FLASH_SR_OPTVERR_Pos)  /*!< 0x00008000 */
#define FLASH_SR_OPTVERR                  FLASH_SR_OPTVERR_Msk
#define FLASH_SR_BSY_Pos                  (16U)
#define FLASH_SR_BSY_Msk                  (0x1UL << FLASH_SR_BSY_Pos)      /*!< 0x00010000 */
#define FLASH_SR_BSY                      FLASH_SR_BSY_Msk

/*******************  Bits definition for FLASH_CR register  ****************/
#define FLASH_CR_PG_Pos                 (0U)
#define FLASH_CR_PG_Msk                 (0x1UL << FLASH_CR_PG_Pos)          /*!< 0x00000001 */
#define FLASH_CR_PG                     FLASH_CR_PG_Msk
#define FLASH_CR_PER_Pos                (1U)
#define FLASH_CR_PER_Msk                (0x1UL << FLASH_CR_PER_Pos)         /*!< 0x00000002 */
#define FLASH_CR_PER                    FLASH_CR_PER_Msk
#define FLASH_CR_MER_Pos                (2U)
#define FLASH_CR_MER_Msk                (0x1UL << FLASH_CR_MER_Pos)         /*!< 0x00000004 */
#define FLASH_CR_MER                    FLASH_CR_MER_Msk
#define FLASH_CR_SER_Pos                (11U)
#define FLASH_CR_SER_Msk                (0x1UL << FLASH_CR_SER_Pos)         /*!< 0x00000800 */
#define FLASH_CR_SER                    FLASH_CR_SER_Msk
#define FLASH_CR_OPTSTRT_Pos            (17U)
#define FLASH_CR_OPTSTRT_Msk            (0x1UL << FLASH_CR_OPTSTRT_Pos)     /*!< 0x00020000 */
#define FLASH_CR_OPTSTRT                FLASH_CR_OPTSTRT_Msk
#define FLASH_CR_PGSTRT_Pos             (19U)
#define FLASH_CR_PGSTRT_Msk             (0x1UL << FLASH_CR_PGSTRT_Pos)      /*!< 0x00080000 */
#define FLASH_CR_PGSTRT                 FLASH_CR_PGSTRT_Msk
#define FLASH_CR_EOPIE_Pos              (24U)
#define FLASH_CR_EOPIE_Msk              (0x1UL << FLASH_CR_EOPIE_Pos)       /*!< 0x01000000 */
#define FLASH_CR_EOPIE                  FLASH_CR_EOPIE_Msk
#define FLASH_CR_ERRIE_Pos              (25U)
#define FLASH_CR_ERRIE_Msk              (0x1UL << FLASH_CR_ERRIE_Pos)       /*!< 0x02000000 */
#define FLASH_CR_ERRIE                  FLASH_CR_ERRIE_Msk
#define FLASH_CR_OBL_LAUNCH_Pos         (27U)
#define FLASH_CR_OBL_LAUNCH_Msk         (0x1UL << FLASH_CR_OBL_LAUNCH_Pos)  /*!< 0x08000000 */
#define FLASH_CR_OBL_LAUNCH             FLASH_CR_OBL_LAUNCH_Msk
#define FLASH_CR_OPTLOCK_Pos            (30U)
#define FLASH_CR_OPTLOCK_Msk            (0x1UL << FLASH_CR_OPTLOCK_Pos)     /*!< 0x40000000 */
#define FLASH_CR_OPTLOCK                FLASH_CR_OPTLOCK_Msk
#define FLASH_CR_LOCK_Pos               (31U)
#define FLASH_CR_LOCK_Msk               (0x1UL << FLASH_CR_LOCK_Pos)        /*!< 0x80000000 */
#define FLASH_CR_LOCK                   FLASH_CR_LOCK_Msk

/*******************  Bits definition for FLASH_OPTR register  **************/
#define FLASH_OPTR_RDP_Pos              (0U)
#define FLASH_OPTR_RDP_Msk              (0xFFUL << FLASH_OPTR_RDP_Pos)
#define FLASH_OPTR_RDP                  FLASH_OPTR_RDP_Msk
#define FLASH_OPTR_IWDG_SW_Pos          (11U)
#define FLASH_OPTR_IWDG_SW_Msk          (0x1UL << FLASH_OPTR_IWDG_SW_Pos)    /*!< 0x00000800 */
#define FLASH_OPTR_IWDG_SW              FLASH_OPTR_IWDG_SW_Msk
#define FLASH_OPTR_WWDG_SW_Pos          (12U)
#define FLASH_OPTR_WWDG_SW_Msk          (0x1UL << FLASH_OPTR_WWDG_SW_Pos)    /*!< 0x00001000 */
#define FLASH_OPTR_WWDG_SW              FLASH_OPTR_WWDG_SW_Msk
#define FLASH_OPTR_NRST_MODE_Pos        (13U)
#define FLASH_OPTR_NRST_MODE_Msk        (0x1UL << FLASH_OPTR_NRST_MODE_Pos)  /*!< 0x00002000 */
#define FLASH_OPTR_NRST_MODE            FLASH_OPTR_NRST_MODE_Msk
#define FLASH_OPTR_nBOOT1_Pos           (14U)
#define FLASH_OPTR_nBOOT1_Msk           (0x1UL << FLASH_OPTR_nBOOT1_Pos)     /*!< 0x00004000 */
#define FLASH_OPTR_nBOOT1               FLASH_OPTR_nBOOT1_Msk
#define FLASH_OPTR_IWDG_STOP_Pos        (15U)
#define FLASH_OPTR_IWDG_STOP_Msk        (0x1UL << FLASH_OPTR_IWDG_STOP_Pos)   /*!< 0x00008000 */
#define FLASH_OPTR_IWDG_STOP            FLASH_OPTR_IWDG_STOP_Msk

#define FLASH_OPTR_RDP_LEVEL_0          (0xAA)
#define FLASH_OPTR_RDP_LEVEL_1          (0x55)

/*******************  Bits definition for FLASH_SDKR register  **************/
#define FLASH_SDKR_SDK_STRT_Pos           (0U)
#define FLASH_SDKR_SDK_STRT_Msk           (0x1FUL << FLASH_SDKR_SDK_STRT_Pos)
#define FLASH_SDKR_SDK_STRT               FLASH_SDKR_SDK_STRT_Msk
#define FLASH_SDKR_SDK_STRT_0             (0x01UL << FLASH_SDKR_SDK_STRT_Pos)
#define FLASH_SDKR_SDK_STRT_1             (0x02UL << FLASH_SDKR_SDK_STRT_Pos)
#define FLASH_SDKR_SDK_STRT_2             (0x04UL << FLASH_SDKR_SDK_STRT_Pos)
#define FLASH_SDKR_SDK_STRT_3             (0x08UL << FLASH_SDKR_SDK_STRT_Pos)
#define FLASH_SDKR_SDK_STRT_4             (0x10UL << FLASH_SDKR_SDK_STRT_Pos)
#define FLASH_SDKR_SDK_END_Pos            (8U)
#define FLASH_SDKR_SDK_END_Msk            (0x1FUL << FLASH_SDKR_SDK_END_Pos)
#define FLASH_SDKR_SDK_END                FLASH_SDKR_SDK_END_Msk
#define FLASH_SDKR_SDK_END_0              (0x01UL << FLASH_SDKR_SDK_END_Pos)
#define FLASH_SDKR_SDK_END_1              (0x02UL << FLASH_SDKR_SDK_END_Pos)
#define FLASH_SDKR_SDK_END_2              (0x04UL << FLASH_SDKR_SDK_END_Pos)
#define FLASH_SDKR_SDK_END_3              (0x08UL << FLASH_SDKR_SDK_END_Pos)
#define FLASH_SDKR_SDK_END_4              (0x10UL << FLASH_SDKR_SDK_END_Pos)
#define FLASH_SDKR_BOR_EN_Pos             (5U)
#define FLASH_SDKR_BOR_EN_Msk             (0x1UL << FLASH_SDKR_BOR_EN_Pos)     /*!< 0x00000020 */
#define FLASH_SDKR_BOR_EN                 FLASH_SDKR_BOR_EN_Msk
#define FLASH_SDKR_BOR_LEV_Pos            (13U)
#define FLASH_SDKR_BOR_LEV_Msk            (0x7UL << FLASH_SDKR_BOR_LEV_Pos)    /*!< 0x0000E000 */
#define FLASH_SDKR_BOR_LEV                FLASH_SDKR_BOR_LEV_Msk
#define FLASH_SDKR_BOR_LEV_0              (0x1UL << FLASH_SDKR_BOR_LEV_Pos)    /*!< 0x00002000 */
#define FLASH_SDKR_BOR_LEV_1              (0x2UL << FLASH_SDKR_BOR_LEV_Pos)    /*!< 0x00004000 */
#define FLASH_SDKR_BOR_LEV_2              (0x4UL << FLASH_SDKR_BOR_LEV_Pos)    /*!< 0x00005000 */

/******************  Bits definition for FLASH_WRPR register  ***************/
#define FLASH_WRPR_WRP_Pos              (0U)
#define FLASH_WRPR_WRP_Msk              (0xFFFFUL << FLASH_WRPR_WRP_Pos) /*!< 0x0000FFFF */
#define FLASH_WRPR_WRP                  FLASH_WRPR_WRP_Msk
#define FLASH_WRPR_WRP_0                (0x0001UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_1                (0x0002UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_2                (0x0004UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_3                (0x0008UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_4                (0x0010UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_5                (0x0020UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_6                (0x0040UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_7                (0x0080UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_8                (0x0100UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_9                (0x0200UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_10               (0x0400UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_11               (0x0800UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_12               (0x1000UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_13               (0x2000UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_14               (0x4000UL << FLASH_WRPR_WRP_Pos)
#define FLASH_WRPR_WRP_15               (0x8000UL << FLASH_WRPR_WRP_Pos)

/******************  Bits definition for FLASH_STCR register  ***************/
#define FLASH_STCR_SLEEP_EN_Pos         (0U)
#define FLASH_STCR_SLEEP_EN_Msk         (0x1U << FLASH_STCR_SLEEP_EN_Pos)
#define FLASH_STCR_SLEEP_EN             FLASH_STCR_SLEEP_EN_Msk
#define FLASH_STCR_SLEEP_TIME_Pos       (8U)
#define FLASH_STCR_SLEEP_TIME_Msk       (0xFFU << FLASH_STCR_SLEEP_TIME_Pos)
#define FLASH_STCR_SLEEP_TIME           FLASH_STCR_SLEEP_TIME_Msk

/******************  Bits definition for FLASH_TS0 register  ****************/
#define FLASH_TS0_TS0_Pos              (0U)
#define FLASH_TS0_TS0_Msk                (0xFFUL << FLASH_TS0_TS0_Pos) /*!< 0x000000FF */
#define FLASH_TS0_TS0                    FLASH_TS0_TS0_Msk
#define FLASH_TS0_TS0_0                (0x0001UL << FLASH_TS0_TS0_Pos)
#define FLASH_TS0_TS0_1                (0x0002UL << FLASH_TS0_TS0_Pos)
#define FLASH_TS0_TS0_2                (0x0004UL << FLASH_TS0_TS0_Pos)
#define FLASH_TS0_TS0_3                (0x0008UL << FLASH_TS0_TS0_Pos)
#define FLASH_TS0_TS0_4                (0x0010UL << FLASH_TS0_TS0_Pos)
#define FLASH_TS0_TS0_5                (0x0020UL << FLASH_TS0_TS0_Pos)
#define FLASH_TS0_TS0_6                (0x0040UL << FLASH_TS0_TS0_Pos)
#define FLASH_TS0_TS0_7                (0x0080UL << FLASH_TS0_TS0_Pos)

/******************  Bits definition for FLASH_TS1 register  ****************/
#define FLASH_TS1_TS1_Pos              (0U)
#define FLASH_TS1_TS1_Msk              (0x1FFUL << FLASH_TS1_TS1_Pos) /*!< 0x000000FF */
#define FLASH_TS1_TS1                  FLASH_TS1_TS1_Msk
#define FLASH_TS1_TS1_0                (0x0001UL << FLASH_TS1_TS1_Pos)
#define FLASH_TS1_TS1_1                (0x0002UL << FLASH_TS1_TS1_Pos)
#define FLASH_TS1_TS1_2                (0x0004UL << FLASH_TS1_TS1_Pos)
#define FLASH_TS1_TS1_3                (0x0008UL << FLASH_TS1_TS1_Pos)
#define FLASH_TS1_TS1_4                (0x0010UL << FLASH_TS1_TS1_Pos)
#define FLASH_TS1_TS1_5                (0x0020UL << FLASH_TS1_TS1_Pos)
#define FLASH_TS1_TS1_6                (0x0040UL << FLASH_TS1_TS1_Pos)
#define FLASH_TS1_TS1_7                (0x0080UL << FLASH_TS1_TS1_Pos)
#define FLASH_TS1_TS1_8                (0x0100UL << FLASH_TS1_TS1_Pos)

/******************  Bits definition for FLASH_TS2P register  ***************/
#define FLASH_TS2P_TS2P_Pos              (0U)
#define FLASH_TS2P_TS2P_Msk              (0xFFUL << FLASH_TS2P_TS2P_Pos) /*!< 0x000000FF */
#define FLASH_TS2P_TS2P                  FLASH_TS2P_TS2P_Msk
#define FLASH_TS2P_TS2P_0                (0x0001UL << FLASH_TS2P_TS2P_Pos)
#define FLASH_TS2P_TS2P_1                (0x0002UL << FLASH_TS2P_TS2P_Pos)
#define FLASH_TS2P_TS2P_2                (0x0004UL << FLASH_TS2P_TS2P_Pos)
#define FLASH_TS2P_TS2P_3                (0x0008UL << FLASH_TS2P_TS2P_Pos)
#define FLASH_TS2P_TS2P_4                (0x0010UL << FLASH_TS2P_TS2P_Pos)
#define FLASH_TS2P_TS2P_5                (0x0020UL << FLASH_TS2P_TS2P_Pos)
#define FLASH_TS2P_TS2P_6                (0x0040UL << FLASH_TS2P_TS2P_Pos)
#define FLASH_TS2P_TS2P_7                (0x0080UL << FLASH_TS2P_TS2P_Pos)

/******************  Bits definition for FLASH_TPS3 register  ***************/
#define FLASH_TPS3_TPS3_Pos              (0U)
#define FLASH_TPS3_TPS3_Msk              (0x7FFUL << FLASH_TPS3_TPS3_Pos) /*!< 0x000007FF */
#define FLASH_TPS3_TPS3                  FLASH_TPS3_TPS3_Msk
#define FLASH_TPS3_TPS3_0                (0x0001UL << FLASH_TPS3_TPS3_Pos)
#define FLASH_TPS3_TPS3_1                (0x0002UL << FLASH_TPS3_TPS3_Pos)
#define FLASH_TPS3_TPS3_2                (0x0004UL << FLASH_TPS3_TPS3_Pos)
#define FLASH_TPS3_TPS3_3                (0x0008UL << FLASH_TPS3_TPS3_Pos)
#define FLASH_TPS3_TPS3_4                (0x0010UL << FLASH_TPS3_TPS3_Pos)
#define FLASH_TPS3_TPS3_5                (0x0020UL << FLASH_TPS3_TPS3_Pos)
#define FLASH_TPS3_TPS3_6                (0x0040UL << FLASH_TPS3_TPS3_Pos)
#define FLASH_TPS3_TPS3_7                (0x0080UL << FLASH_TPS3_TPS3_Pos)
#define FLASH_TPS3_TPS3_8                (0x0100UL << FLASH_TPS3_TPS3_Pos)
#define FLASH_TPS3_TPS3_9                (0x0200UL << FLASH_TPS3_TPS3_Pos)
#define FLASH_TPS3_TPS3_10               (0x0400UL << FLASH_TPS3_TPS3_Pos)

/******************  Bits definition for FLASH_TS3 register  ****************/
#define FLASH_TS3_TS3_Pos              (0U)
#define FLASH_TS3_TS3_Msk                (0xFFUL << FLASH_TS3_TS3_Pos) /*!< 0x000000FF */
#define FLASH_TS3_TS3                    FLASH_TS3_TS3_Msk
#define FLASH_TS3_TS3_0                (0x0001UL << FLASH_TS3_TS3_Pos)
#define FLASH_TS3_TS3_1                (0x0002UL << FLASH_TS3_TS3_Pos)
#define FLASH_TS3_TS3_2                (0x0004UL << FLASH_TS3_TS3_Pos)
#define FLASH_TS3_TS3_3                (0x0008UL << FLASH_TS3_TS3_Pos)
#define FLASH_TS3_TS3_4                (0x0010UL << FLASH_TS3_TS3_Pos)
#define FLASH_TS3_TS3_5                (0x0020UL << FLASH_TS3_TS3_Pos)
#define FLASH_TS3_TS3_6                (0x0040UL << FLASH_TS3_TS3_Pos)
#define FLASH_TS3_TS3_7                (0x0080UL << FLASH_TS3_TS3_Pos)

/******************  Bits definition for FLASH_PERTPE register  *************/
#define FLASH_PERTPE_PERTPE_Pos              (0U)
#define FLASH_PERTPE_PERTPE_Msk              (0x1FFFFUL << FLASH_PERTPE_PERTPE_Pos) /*!< 0x0001FFFF */
#define FLASH_PERTPE_PERTPE                  FLASH_PERTPE_PERTPE_Msk
#define FLASH_PERTPE_PERTPE_0                (0x00001UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_1                (0x00002UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_2                (0x00004UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_3                (0x00008UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_4                (0x00010UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_5                (0x00020UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_6                (0x00040UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_7                (0x00080UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_8                (0x00100UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_9                (0x00200UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_10               (0x00400UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_11               (0x00800UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_12               (0x01000UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_13               (0x02000UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_14               (0x04000UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_15               (0x08000UL << FLASH_PERTPE_PERTPE_Pos)
#define FLASH_PERTPE_PERTPE_16               (0x10000UL << FLASH_PERTPE_PERTPE_Pos)

/******************  Bits definition for FLASH_SMERTPE register  ************/
#define FLASH_SMERTPE_SMERTPE_Pos              (0U)
#define FLASH_SMERTPE_SMERTPE_Msk                (0x1FFFFUL << FLASH_SMERTPE_SMERTPE_Pos) /*!< 0x0001FFFF */
#define FLASH_SMERTPE_SMERTPE                    FLASH_SMERTPE_SMERTPE_Msk
#define FLASH_SMERTPE_SMERTPE_0                (0x00001UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_1                (0x00002UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_2                (0x00004UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_3                (0x00008UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_4                (0x00010UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_5                (0x00020UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_6                (0x00040UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_7                (0x00080UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_8                (0x00100UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_9                (0x00200UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_10               (0x00400UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_11               (0x00800UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_12               (0x01000UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_13               (0x02000UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_14               (0x04000UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_15               (0x08000UL << FLASH_SMERTPE_SMERTPE_Pos)
#define FLASH_SMERTPE_SMERTPE_16               (0x10000UL << FLASH_SMERTPE_SMERTPE_Pos)

/******************  Bits definition for FLASH_PRGTPE register  *************/
#define FLASH_PRGTPE_PRGTPE_Pos              (0U)
#define FLASH_PRGTPE_PRGTPE_Msk              (0xFFFFUL << FLASH_PRGTPE_PRGTPE_Pos) /*!< 0x0000FFFF */
#define FLASH_PRGTPE_PRGTPE                  FLASH_PRGTPE_PRGTPE_Msk
#define FLASH_PRGTPE_PRGTPE_0                (0x0001UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_1                (0x0002UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_2                (0x0004UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_3                (0x0008UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_4                (0x0010UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_5                (0x0020UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_6                (0x0040UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_7                (0x0080UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_8                (0x0100UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_9                (0x0200UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_10               (0x0400UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_11               (0x0800UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_12               (0x1000UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_13               (0x2000UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_14               (0x4000UL << FLASH_PRGTPE_PRGTPE_Pos)
#define FLASH_PRGTPE_PRGTPE_15               (0x8000UL << FLASH_PRGTPE_PRGTPE_Pos)

/******************  Bits definition for FLASH_PRETPE register  *************/
#define FLASH_PRETPE_PRETPE_Pos              (0U)
#define FLASH_PRETPE_PRETPE_Msk              (0x3FFFUL << FLASH_PRETPE_PRETPE_Pos) /*!< 0x00003FFF */
#define FLASH_PRETPE_PRETPE                  FLASH_PRETPE_PRETPE_Msk
#define FLASH_PRETPE_PRETPE_0                (0x0001UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_1                (0x0002UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_2                (0x0004UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_3                (0x0008UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_4                (0x0010UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_5                (0x0020UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_6                (0x0040UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_7                (0x0080UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_8                (0x0100UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_9                (0x0200UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_10               (0x0400UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_11               (0x0800UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_12               (0x1000UL << FLASH_PRETPE_PRETPE_Pos)
#define FLASH_PRETPE_PRETPE_13               (0x2000UL << FLASH_PRETPE_PRETPE_Pos)

/****************************************************************************/
/*                                                                          */
/*                            General Purpose I/O (GPIO)                    */
/*                                                                          */
/****************************************************************************/
/******************  Bits definition for GPIO_MODER register  ***************/
#define GPIO_MODER_MODE0_Pos           (0U)
#define GPIO_MODER_MODE0_Msk           (0x3UL << GPIO_MODER_MODE0_Pos)          /*!< 0x00000003 */
#define GPIO_MODER_MODE0               GPIO_MODER_MODE0_Msk
#define GPIO_MODER_MODE0_0             (0x1UL << GPIO_MODER_MODE0_Pos)          /*!< 0x00000001 */
#define GPIO_MODER_MODE0_1             (0x2UL << GPIO_MODER_MODE0_Pos)          /*!< 0x00000002 */
#define GPIO_MODER_MODE1_Pos           (2U)
#define GPIO_MODER_MODE1_Msk           (0x3UL << GPIO_MODER_MODE1_Pos)          /*!< 0x0000000C */
#define GPIO_MODER_MODE1               GPIO_MODER_MODE1_Msk
#define GPIO_MODER_MODE1_0             (0x1UL << GPIO_MODER_MODE1_Pos)          /*!< 0x00000004 */
#define GPIO_MODER_MODE1_1             (0x2UL << GPIO_MODER_MODE1_Pos)          /*!< 0x00000008 */
#define GPIO_MODER_MODE2_Pos           (4U)
#define GPIO_MODER_MODE2_Msk           (0x3UL << GPIO_MODER_MODE2_Pos)          /*!< 0x00000030 */
#define GPIO_MODER_MODE2               GPIO_MODER_MODE2_Msk
#define GPIO_MODER_MODE2_0             (0x1UL << GPIO_MODER_MODE2_Pos)          /*!< 0x00000010 */
#define GPIO_MODER_MODE2_1             (0x2UL << GPIO_MODER_MODE2_Pos)          /*!< 0x00000020 */
#define GPIO_MODER_MODE3_Pos           (6U)
#define GPIO_MODER_MODE3_Msk           (0x3UL << GPIO_MODER_MODE3_Pos)          /*!< 0x000000C0 */
#define GPIO_MODER_MODE3               GPIO_MODER_MODE3_Msk
#define GPIO_MODER_MODE3_0             (0x1UL << GPIO_MODER_MODE3_Pos)          /*!< 0x00000040 */
#define GPIO_MODER_MODE3_1             (0x2UL << GPIO_MODER_MODE3_Pos)          /*!< 0x00000080 */
#define GPIO_MODER_MODE4_Pos           (8U)
#define GPIO_MODER_MODE4_Msk           (0x3UL << GPIO_MODER_MODE4_Pos)          /*!< 0x00000300 */
#define GPIO_MODER_MODE4               GPIO_MODER_MODE4_Msk
#define GPIO_MODER_MODE4_0             (0x1UL << GPIO_MODER_MODE4_Pos)          /*!< 0x00000100 */
#define GPIO_MODER_MODE4_1             (0x2UL << GPIO_MODER_MODE4_Pos)          /*!< 0x00000200 */
#define GPIO_MODER_MODE5_Pos           (10U)
#define GPIO_MODER_MODE5_Msk           (0x3UL << GPIO_MODER_MODE5_Pos)          /*!< 0x00000C00 */
#define GPIO_MODER_MODE5               GPIO_MODER_MODE5_Msk
#define GPIO_MODER_MODE5_0             (0x1UL << GPIO_MODER_MODE5_Pos)          /*!< 0x00000400 */
#define GPIO_MODER_MODE5_1             (0x2UL << GPIO_MODER_MODE5_Pos)          /*!< 0x00000800 */
#define GPIO_MODER_MODE6_Pos           (12U)
#define GPIO_MODER_MODE6_Msk           (0x3UL << GPIO_MODER_MODE6_Pos)          /*!< 0x00003000 */
#define GPIO_MODER_MODE6               GPIO_MODER_MODE6_Msk
#define GPIO_MODER_MODE6_0             (0x1UL << GPIO_MODER_MODE6_Pos)          /*!< 0x00001000 */
#define GPIO_MODER_MODE6_1             (0x2UL << GPIO_MODER_MODE6_Pos)          /*!< 0x00002000 */
#define GPIO_MODER_MODE7_Pos           (14U)
#define GPIO_MODER_MODE7_Msk           (0x3UL << GPIO_MODER_MODE7_Pos)          /*!< 0x0000C000 */
#define GPIO_MODER_MODE7               GPIO_MODER_MODE7_Msk
#define GPIO_MODER_MODE7_0             (0x1UL << GPIO_MODER_MODE7_Pos)          /*!< 0x00004000 */
#define GPIO_MODER_MODE7_1             (0x2UL << GPIO_MODER_MODE7_Pos)          /*!< 0x00008000 */
#define GPIO_MODER_MODE8_Pos           (16U)
#define GPIO_MODER_MODE8_Msk           (0x3UL << GPIO_MODER_MODE8_Pos)          /*!< 0x00030000 */
#define GPIO_MODER_MODE8               GPIO_MODER_MODE8_Msk
#define GPIO_MODER_MODE8_0             (0x1UL << GPIO_MODER_MODE8_Pos)          /*!< 0x00010000 */
#define GPIO_MODER_MODE8_1             (0x2UL << GPIO_MODER_MODE8_Pos)          /*!< 0x00020000 */
#define GPIO_MODER_MODE9_Pos           (18U)
#define GPIO_MODER_MODE9_Msk           (0x3UL << GPIO_MODER_MODE9_Pos)          /*!< 0x000C0000 */
#define GPIO_MODER_MODE9               GPIO_MODER_MODE9_Msk
#define GPIO_MODER_MODE9_0             (0x1UL << GPIO_MODER_MODE9_Pos)          /*!< 0x00040000 */
#define GPIO_MODER_MODE9_1             (0x2UL << GPIO_MODER_MODE9_Pos)          /*!< 0x00080000 */
#define GPIO_MODER_MODE10_Pos          (20U)
#define GPIO_MODER_MODE10_Msk          (0x3UL << GPIO_MODER_MODE10_Pos)         /*!< 0x00300000 */
#define GPIO_MODER_MODE10              GPIO_MODER_MODE10_Msk
#define GPIO_MODER_MODE10_0            (0x1UL << GPIO_MODER_MODE10_Pos)         /*!< 0x00100000 */
#define GPIO_MODER_MODE10_1            (0x2UL << GPIO_MODER_MODE10_Pos)         /*!< 0x00200000 */
#define GPIO_MODER_MODE11_Pos          (22U)
#define GPIO_MODER_MODE11_Msk          (0x3UL << GPIO_MODER_MODE11_Pos)         /*!< 0x00C00000 */
#define GPIO_MODER_MODE11              GPIO_MODER_MODE11_Msk
#define GPIO_MODER_MODE11_0            (0x1UL << GPIO_MODER_MODE11_Pos)         /*!< 0x00400000 */
#define GPIO_MODER_MODE11_1            (0x2UL << GPIO_MODER_MODE11_Pos)         /*!< 0x00800000 */
#define GPIO_MODER_MODE12_Pos          (24U)
#define GPIO_MODER_MODE12_Msk          (0x3UL << GPIO_MODER_MODE12_Pos)         /*!< 0x03000000 */
#define GPIO_MODER_MODE12              GPIO_MODER_MODE12_Msk
#define GPIO_MODER_MODE12_0            (0x1UL << GPIO_MODER_MODE12_Pos)         /*!< 0x01000000 */
#define GPIO_MODER_MODE12_1            (0x2UL << GPIO_MODER_MODE12_Pos)         /*!< 0x02000000 */
#define GPIO_MODER_MODE13_Pos          (26U)
#define GPIO_MODER_MODE13_Msk          (0x3UL << GPIO_MODER_MODE13_Pos)         /*!< 0x0C000000 */
#define GPIO_MODER_MODE13              GPIO_MODER_MODE13_Msk
#define GPIO_MODER_MODE13_0            (0x1UL << GPIO_MODER_MODE13_Pos)         /*!< 0x04000000 */
#define GPIO_MODER_MODE13_1            (0x2UL << GPIO_MODER_MODE13_Pos)         /*!< 0x08000000 */
#define GPIO_MODER_MODE14_Pos          (28U)
#define GPIO_MODER_MODE14_Msk          (0x3UL << GPIO_MODER_MODE14_Pos)         /*!< 0x30000000 */
#define GPIO_MODER_MODE14              GPIO_MODER_MODE14_Msk
#define GPIO_MODER_MODE14_0            (0x1UL << GPIO_MODER_MODE14_Pos)         /*!< 0x10000000 */
#define GPIO_MODER_MODE14_1            (0x2UL << GPIO_MODER_MODE14_Pos)         /*!< 0x20000000 */
#define GPIO_MODER_MODE15_Pos          (30U)
#define GPIO_MODER_MODE15_Msk          (0x3UL << GPIO_MODER_MODE15_Pos)         /*!< 0xC0000000 */
#define GPIO_MODER_MODE15              GPIO_MODER_MODE15_Msk
#define GPIO_MODER_MODE15_0            (0x1UL << GPIO_MODER_MODE15_Pos)         /*!< 0x40000000 */
#define GPIO_MODER_MODE15_1            (0x2UL << GPIO_MODER_MODE15_Pos)         /*!< 0x80000000 */

/******************  Bits definition for GPIO_OTYPER register  **************/
#define GPIO_OTYPER_OT0_Pos            (0U)
#define GPIO_OTYPER_OT0_Msk            (0x1UL << GPIO_OTYPER_OT0_Pos)           /*!< 0x00000001 */
#define GPIO_OTYPER_OT0                GPIO_OTYPER_OT0_Msk
#define GPIO_OTYPER_OT1_Pos            (1U)
#define GPIO_OTYPER_OT1_Msk            (0x1UL << GPIO_OTYPER_OT1_Pos)           /*!< 0x00000002 */
#define GPIO_OTYPER_OT1                GPIO_OTYPER_OT1_Msk
#define GPIO_OTYPER_OT2_Pos            (2U)
#define GPIO_OTYPER_OT2_Msk            (0x1UL << GPIO_OTYPER_OT2_Pos)           /*!< 0x00000004 */
#define GPIO_OTYPER_OT2                GPIO_OTYPER_OT2_Msk
#define GPIO_OTYPER_OT3_Pos            (3U)
#define GPIO_OTYPER_OT3_Msk            (0x1UL << GPIO_OTYPER_OT3_Pos)           /*!< 0x00000008 */
#define GPIO_OTYPER_OT3                GPIO_OTYPER_OT3_Msk
#define GPIO_OTYPER_OT4_Pos            (4U)
#define GPIO_OTYPER_OT4_Msk            (0x1UL << GPIO_OTYPER_OT4_Pos)           /*!< 0x00000010 */
#define GPIO_OTYPER_OT4                GPIO_OTYPER_OT4_Msk
#define GPIO_OTYPER_OT5_Pos            (5U)
#define GPIO_OTYPER_OT5_Msk            (0x1UL << GPIO_OTYPER_OT5_Pos)           /*!< 0x00000020 */
#define GPIO_OTYPER_OT5                GPIO_OTYPER_OT5_Msk
#define GPIO_OTYPER_OT6_Pos            (6U)
#define GPIO_OTYPER_OT6_Msk            (0x1UL << GPIO_OTYPER_OT6_Pos)           /*!< 0x00000040 */
#define GPIO_OTYPER_OT6                GPIO_OTYPER_OT6_Msk
#define GPIO_OTYPER_OT7_Pos            (7U)
#define GPIO_OTYPER_OT7_Msk            (0x1UL << GPIO_OTYPER_OT7_Pos)           /*!< 0x00000080 */
#define GPIO_OTYPER_OT7                GPIO_OTYPER_OT7_Msk
#define GPIO_OTYPER_OT8_Pos            (8U)
#define GPIO_OTYPER_OT8_Msk            (0x1UL << GPIO_OTYPER_OT8_Pos)           /*!< 0x00000100 */
#define GPIO_OTYPER_OT8                GPIO_OTYPER_OT8_Msk
#define GPIO_OTYPER_OT9_Pos            (9U)
#define GPIO_OTYPER_OT9_Msk            (0x1UL << GPIO_OTYPER_OT9_Pos)           /*!< 0x00000200 */
#define GPIO_OTYPER_OT9                GPIO_OTYPER_OT9_Msk
#define GPIO_OTYPER_OT10_Pos           (10U)
#define GPIO_OTYPER_OT10_Msk           (0x1UL << GPIO_OTYPER_OT10_Pos)          /*!< 0x00000400 */
#define GPIO_OTYPER_OT10               GPIO_OTYPER_OT10_Msk
#define GPIO_OTYPER_OT11_Pos           (11U)
#define GPIO_OTYPER_OT11_Msk           (0x1UL << GPIO_OTYPER_OT11_Pos)          /*!< 0x00000800 */
#define GPIO_OTYPER_OT11               GPIO_OTYPER_OT11_Msk
#define GPIO_OTYPER_OT12_Pos           (12U)
#define GPIO_OTYPER_OT12_Msk           (0x1UL << GPIO_OTYPER_OT12_Pos)          /*!< 0x00001000 */
#define GPIO_OTYPER_OT12               GPIO_OTYPER_OT12_Msk
#define GPIO_OTYPER_OT13_Pos           (13U)
#define GPIO_OTYPER_OT13_Msk           (0x1UL << GPIO_OTYPER_OT13_Pos)          /*!< 0x00002000 */
#define GPIO_OTYPER_OT13               GPIO_OTYPER_OT13_Msk
#define GPIO_OTYPER_OT14_Pos           (14U)
#define GPIO_OTYPER_OT14_Msk           (0x1UL << GPIO_OTYPER_OT14_Pos)          /*!< 0x00004000 */
#define GPIO_OTYPER_OT14               GPIO_OTYPER_OT14_Msk
#define GPIO_OTYPER_OT15_Pos           (15U)
#define GPIO_OTYPER_OT15_Msk           (0x1UL << GPIO_OTYPER_OT15_Pos)          /*!< 0x00008000 */
#define GPIO_OTYPER_OT15               GPIO_OTYPER_OT15_Msk

/******************  Bits definition for GPIO_OSPEEDR register  *************/
#define GPIO_OSPEEDR_OSPEED0_Pos       (0U)
#define GPIO_OSPEEDR_OSPEED0_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED0_Pos)      /*!< 0x00000003 */
#define GPIO_OSPEEDR_OSPEED0           GPIO_OSPEEDR_OSPEED0_Msk
#define GPIO_OSPEEDR_OSPEED0_0         (0x1UL << GPIO_OSPEEDR_OSPEED0_Pos)      /*!< 0x00000001 */
#define GPIO_OSPEEDR_OSPEED0_1         (0x2UL << GPIO_OSPEEDR_OSPEED0_Pos)      /*!< 0x00000002 */
#define GPIO_OSPEEDR_OSPEED1_Pos       (2U)
#define GPIO_OSPEEDR_OSPEED1_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED1_Pos)      /*!< 0x0000000C */
#define GPIO_OSPEEDR_OSPEED1           GPIO_OSPEEDR_OSPEED1_Msk
#define GPIO_OSPEEDR_OSPEED1_0         (0x1UL << GPIO_OSPEEDR_OSPEED1_Pos)      /*!< 0x00000004 */
#define GPIO_OSPEEDR_OSPEED1_1         (0x2UL << GPIO_OSPEEDR_OSPEED1_Pos)      /*!< 0x00000008 */
#define GPIO_OSPEEDR_OSPEED2_Pos       (4U)
#define GPIO_OSPEEDR_OSPEED2_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED2_Pos)      /*!< 0x00000030 */
#define GPIO_OSPEEDR_OSPEED2           GPIO_OSPEEDR_OSPEED2_Msk
#define GPIO_OSPEEDR_OSPEED2_0         (0x1UL << GPIO_OSPEEDR_OSPEED2_Pos)      /*!< 0x00000010 */
#define GPIO_OSPEEDR_OSPEED2_1         (0x2UL << GPIO_OSPEEDR_OSPEED2_Pos)      /*!< 0x00000020 */
#define GPIO_OSPEEDR_OSPEED3_Pos       (6U)
#define GPIO_OSPEEDR_OSPEED3_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED3_Pos)      /*!< 0x000000C0 */
#define GPIO_OSPEEDR_OSPEED3           GPIO_OSPEEDR_OSPEED3_Msk
#define GPIO_OSPEEDR_OSPEED3_0         (0x1UL << GPIO_OSPEEDR_OSPEED3_Pos)      /*!< 0x00000040 */
#define GPIO_OSPEEDR_OSPEED3_1         (0x2UL << GPIO_OSPEEDR_OSPEED3_Pos)      /*!< 0x00000080 */
#define GPIO_OSPEEDR_OSPEED4_Pos       (8U)
#define GPIO_OSPEEDR_OSPEED4_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED4_Pos)      /*!< 0x00000300 */
#define GPIO_OSPEEDR_OSPEED4           GPIO_OSPEEDR_OSPEED4_Msk
#define GPIO_OSPEEDR_OSPEED4_0         (0x1UL << GPIO_OSPEEDR_OSPEED4_Pos)      /*!< 0x00000100 */
#define GPIO_OSPEEDR_OSPEED4_1         (0x2UL << GPIO_OSPEEDR_OSPEED4_Pos)      /*!< 0x00000200 */
#define GPIO_OSPEEDR_OSPEED5_Pos       (10U)
#define GPIO_OSPEEDR_OSPEED5_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED5_Pos)      /*!< 0x00000C00 */
#define GPIO_OSPEEDR_OSPEED5           GPIO_OSPEEDR_OSPEED5_Msk
#define GPIO_OSPEEDR_OSPEED5_0         (0x1UL << GPIO_OSPEEDR_OSPEED5_Pos)      /*!< 0x00000400 */
#define GPIO_OSPEEDR_OSPEED5_1         (0x2UL << GPIO_OSPEEDR_OSPEED5_Pos)      /*!< 0x00000800 */
#define GPIO_OSPEEDR_OSPEED6_Pos       (12U)
#define GPIO_OSPEEDR_OSPEED6_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED6_Pos)      /*!< 0x00003000 */
#define GPIO_OSPEEDR_OSPEED6           GPIO_OSPEEDR_OSPEED6_Msk
#define GPIO_OSPEEDR_OSPEED6_0         (0x1UL << GPIO_OSPEEDR_OSPEED6_Pos)      /*!< 0x00001000 */
#define GPIO_OSPEEDR_OSPEED6_1         (0x2UL << GPIO_OSPEEDR_OSPEED6_Pos)      /*!< 0x00002000 */
#define GPIO_OSPEEDR_OSPEED7_Pos       (14U)
#define GPIO_OSPEEDR_OSPEED7_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED7_Pos)      /*!< 0x0000C000 */
#define GPIO_OSPEEDR_OSPEED7           GPIO_OSPEEDR_OSPEED7_Msk
#define GPIO_OSPEEDR_OSPEED7_0         (0x1UL << GPIO_OSPEEDR_OSPEED7_Pos)      /*!< 0x00004000 */
#define GPIO_OSPEEDR_OSPEED7_1         (0x2UL << GPIO_OSPEEDR_OSPEED7_Pos)      /*!< 0x00008000 */
#define GPIO_OSPEEDR_OSPEED8_Pos       (16U)
#define GPIO_OSPEEDR_OSPEED8_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED8_Pos)      /*!< 0x00030000 */
#define GPIO_OSPEEDR_OSPEED8           GPIO_OSPEEDR_OSPEED8_Msk
#define GPIO_OSPEEDR_OSPEED8_0         (0x1UL << GPIO_OSPEEDR_OSPEED8_Pos)      /*!< 0x00010000 */
#define GPIO_OSPEEDR_OSPEED8_1         (0x2UL << GPIO_OSPEEDR_OSPEED8_Pos)      /*!< 0x00020000 */
#define GPIO_OSPEEDR_OSPEED9_Pos       (18U)
#define GPIO_OSPEEDR_OSPEED9_Msk       (0x3UL << GPIO_OSPEEDR_OSPEED9_Pos)      /*!< 0x000C0000 */
#define GPIO_OSPEEDR_OSPEED9           GPIO_OSPEEDR_OSPEED9_Msk
#define GPIO_OSPEEDR_OSPEED9_0         (0x1UL << GPIO_OSPEEDR_OSPEED9_Pos)      /*!< 0x00040000 */
#define GPIO_OSPEEDR_OSPEED9_1         (0x2UL << GPIO_OSPEEDR_OSPEED9_Pos)      /*!< 0x00080000 */
#define GPIO_OSPEEDR_OSPEED10_Pos      (20U)
#define GPIO_OSPEEDR_OSPEED10_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED10_Pos)     /*!< 0x00300000 */
#define GPIO_OSPEEDR_OSPEED10          GPIO_OSPEEDR_OSPEED10_Msk
#define GPIO_OSPEEDR_OSPEED10_0        (0x1UL << GPIO_OSPEEDR_OSPEED10_Pos)     /*!< 0x00100000 */
#define GPIO_OSPEEDR_OSPEED10_1        (0x2UL << GPIO_OSPEEDR_OSPEED10_Pos)     /*!< 0x00200000 */
#define GPIO_OSPEEDR_OSPEED11_Pos      (22U)
#define GPIO_OSPEEDR_OSPEED11_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED11_Pos)     /*!< 0x00C00000 */
#define GPIO_OSPEEDR_OSPEED11          GPIO_OSPEEDR_OSPEED11_Msk
#define GPIO_OSPEEDR_OSPEED11_0        (0x1UL << GPIO_OSPEEDR_OSPEED11_Pos)     /*!< 0x00400000 */
#define GPIO_OSPEEDR_OSPEED11_1        (0x2UL << GPIO_OSPEEDR_OSPEED11_Pos)     /*!< 0x00800000 */
#define GPIO_OSPEEDR_OSPEED12_Pos      (24U)
#define GPIO_OSPEEDR_OSPEED12_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED12_Pos)     /*!< 0x03000000 */
#define GPIO_OSPEEDR_OSPEED12          GPIO_OSPEEDR_OSPEED12_Msk
#define GPIO_OSPEEDR_OSPEED12_0        (0x1UL << GPIO_OSPEEDR_OSPEED12_Pos)     /*!< 0x01000000 */
#define GPIO_OSPEEDR_OSPEED12_1        (0x2UL << GPIO_OSPEEDR_OSPEED12_Pos)     /*!< 0x02000000 */
#define GPIO_OSPEEDR_OSPEED13_Pos      (26U)
#define GPIO_OSPEEDR_OSPEED13_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED13_Pos)     /*!< 0x0C000000 */
#define GPIO_OSPEEDR_OSPEED13          GPIO_OSPEEDR_OSPEED13_Msk
#define GPIO_OSPEEDR_OSPEED13_0        (0x1UL << GPIO_OSPEEDR_OSPEED13_Pos)     /*!< 0x04000000 */
#define GPIO_OSPEEDR_OSPEED13_1        (0x2UL << GPIO_OSPEEDR_OSPEED13_Pos)     /*!< 0x08000000 */
#define GPIO_OSPEEDR_OSPEED14_Pos      (28U)
#define GPIO_OSPEEDR_OSPEED14_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED14_Pos)     /*!< 0x30000000 */
#define GPIO_OSPEEDR_OSPEED14          GPIO_OSPEEDR_OSPEED14_Msk
#define GPIO_OSPEEDR_OSPEED14_0        (0x1UL << GPIO_OSPEEDR_OSPEED14_Pos)     /*!< 0x10000000 */
#define GPIO_OSPEEDR_OSPEED14_1        (0x2UL << GPIO_OSPEEDR_OSPEED14_Pos)     /*!< 0x20000000 */
#define GPIO_OSPEEDR_OSPEED15_Pos      (30U)
#define GPIO_OSPEEDR_OSPEED15_Msk      (0x3UL << GPIO_OSPEEDR_OSPEED15_Pos)     /*!< 0xC0000000 */
#define GPIO_OSPEEDR_OSPEED15          GPIO_OSPEEDR_OSPEED15_Msk
#define GPIO_OSPEEDR_OSPEED15_0        (0x1UL << GPIO_OSPEEDR_OSPEED15_Pos)     /*!< 0x40000000 */
#define GPIO_OSPEEDR_OSPEED15_1        (0x2UL << GPIO_OSPEEDR_OSPEED15_Pos)     /*!< 0x80000000 */

/******************  Bits definition for GPIO_PUPDR register  ***************/
#define GPIO_PUPDR_PUPD0_Pos           (0U)
#define GPIO_PUPDR_PUPD0_Msk           (0x3UL << GPIO_PUPDR_PUPD0_Pos)          /*!< 0x00000003 */
#define GPIO_PUPDR_PUPD0               GPIO_PUPDR_PUPD0_Msk
#define GPIO_PUPDR_PUPD0_0             (0x1UL << GPIO_PUPDR_PUPD0_Pos)          /*!< 0x00000001 */
#define GPIO_PUPDR_PUPD0_1             (0x2UL << GPIO_PUPDR_PUPD0_Pos)          /*!< 0x00000002 */
#define GPIO_PUPDR_PUPD1_Pos           (2U)
#define GPIO_PUPDR_PUPD1_Msk           (0x3UL << GPIO_PUPDR_PUPD1_Pos)          /*!< 0x0000000C */
#define GPIO_PUPDR_PUPD1               GPIO_PUPDR_PUPD1_Msk
#define GPIO_PUPDR_PUPD1_0             (0x1UL << GPIO_PUPDR_PUPD1_Pos)          /*!< 0x00000004 */
#define GPIO_PUPDR_PUPD1_1             (0x2UL << GPIO_PUPDR_PUPD1_Pos)          /*!< 0x00000008 */
#define GPIO_PUPDR_PUPD2_Pos           (4U)
#define GPIO_PUPDR_PUPD2_Msk           (0x3UL << GPIO_PUPDR_PUPD2_Pos)          /*!< 0x00000030 */
#define GPIO_PUPDR_PUPD2               GPIO_PUPDR_PUPD2_Msk
#define GPIO_PUPDR_PUPD2_0             (0x1UL << GPIO_PUPDR_PUPD2_Pos)          /*!< 0x00000010 */
#define GPIO_PUPDR_PUPD2_1             (0x2UL << GPIO_PUPDR_PUPD2_Pos)          /*!< 0x00000020 */
#define GPIO_PUPDR_PUPD3_Pos           (6U)
#define GPIO_PUPDR_PUPD3_Msk           (0x3UL << GPIO_PUPDR_PUPD3_Pos)          /*!< 0x000000C0 */
#define GPIO_PUPDR_PUPD3               GPIO_PUPDR_PUPD3_Msk
#define GPIO_PUPDR_PUPD3_0             (0x1UL << GPIO_PUPDR_PUPD3_Pos)          /*!< 0x00000040 */
#define GPIO_PUPDR_PUPD3_1             (0x2UL << GPIO_PUPDR_PUPD3_Pos)          /*!< 0x00000080 */
#define GPIO_PUPDR_PUPD4_Pos           (8U)
#define GPIO_PUPDR_PUPD4_Msk           (0x3UL << GPIO_PUPDR_PUPD4_Pos)          /*!< 0x00000300 */
#define GPIO_PUPDR_PUPD4               GPIO_PUPDR_PUPD4_Msk
#define GPIO_PUPDR_PUPD4_0             (0x1UL << GPIO_PUPDR_PUPD4_Pos)          /*!< 0x00000100 */
#define GPIO_PUPDR_PUPD4_1             (0x2UL << GPIO_PUPDR_PUPD4_Pos)          /*!< 0x00000200 */
#define GPIO_PUPDR_PUPD5_Pos           (10U)
#define GPIO_PUPDR_PUPD5_Msk           (0x3UL << GPIO_PUPDR_PUPD5_Pos)          /*!< 0x00000C00 */
#define GPIO_PUPDR_PUPD5               GPIO_PUPDR_PUPD5_Msk
#define GPIO_PUPDR_PUPD5_0             (0x1UL << GPIO_PUPDR_PUPD5_Pos)          /*!< 0x00000400 */
#define GPIO_PUPDR_PUPD5_1             (0x2UL << GPIO_PUPDR_PUPD5_Pos)          /*!< 0x00000800 */
#define GPIO_PUPDR_PUPD6_Pos           (12U)
#define GPIO_PUPDR_PUPD6_Msk           (0x3UL << GPIO_PUPDR_PUPD6_Pos)          /*!< 0x00003000 */
#define GPIO_PUPDR_PUPD6               GPIO_PUPDR_PUPD6_Msk
#define GPIO_PUPDR_PUPD6_0             (0x1UL << GPIO_PUPDR_PUPD6_Pos)          /*!< 0x00001000 */
#define GPIO_PUPDR_PUPD6_1             (0x2UL << GPIO_PUPDR_PUPD6_Pos)          /*!< 0x00002000 */
#define GPIO_PUPDR_PUPD7_Pos           (14U)
#define GPIO_PUPDR_PUPD7_Msk           (0x3UL << GPIO_PUPDR_PUPD7_Pos)          /*!< 0x0000C000 */
#define GPIO_PUPDR_PUPD7               GPIO_PUPDR_PUPD7_Msk
#define GPIO_PUPDR_PUPD7_0             (0x1UL << GPIO_PUPDR_PUPD7_Pos)          /*!< 0x00004000 */
#define GPIO_PUPDR_PUPD7_1             (0x2UL << GPIO_PUPDR_PUPD7_Pos)          /*!< 0x00008000 */
#define GPIO_PUPDR_PUPD8_Pos           (16U)
#define GPIO_PUPDR_PUPD8_Msk           (0x3UL << GPIO_PUPDR_PUPD8_Pos)          /*!< 0x00030000 */
#define GPIO_PUPDR_PUPD8               GPIO_PUPDR_PUPD8_Msk
#define GPIO_PUPDR_PUPD8_0             (0x1UL << GPIO_PUPDR_PUPD8_Pos)          /*!< 0x00010000 */
#define GPIO_PUPDR_PUPD8_1             (0x2UL << GPIO_PUPDR_PUPD8_Pos)          /*!< 0x00020000 */
#define GPIO_PUPDR_PUPD9_Pos           (18U)
#define GPIO_PUPDR_PUPD9_Msk           (0x3UL << GPIO_PUPDR_PUPD9_Pos)          /*!< 0x000C0000 */
#define GPIO_PUPDR_PUPD9               GPIO_PUPDR_PUPD9_Msk
#define GPIO_PUPDR_PUPD9_0             (0x1UL << GPIO_PUPDR_PUPD9_Pos)          /*!< 0x00040000 */
#define GPIO_PUPDR_PUPD9_1             (0x2UL << GPIO_PUPDR_PUPD9_Pos)          /*!< 0x00080000 */
#define GPIO_PUPDR_PUPD10_Pos          (20U)
#define GPIO_PUPDR_PUPD10_Msk          (0x3UL << GPIO_PUPDR_PUPD10_Pos)         /*!< 0x00300000 */
#define GPIO_PUPDR_PUPD10              GPIO_PUPDR_PUPD10_Msk
#define GPIO_PUPDR_PUPD10_0            (0x1UL << GPIO_PUPDR_PUPD10_Pos)         /*!< 0x00100000 */
#define GPIO_PUPDR_PUPD10_1            (0x2UL << GPIO_PUPDR_PUPD10_Pos)         /*!< 0x00200000 */
#define GPIO_PUPDR_PUPD11_Pos          (22U)
#define GPIO_PUPDR_PUPD11_Msk          (0x3UL << GPIO_PUPDR_PUPD11_Pos)         /*!< 0x00C00000 */
#define GPIO_PUPDR_PUPD11              GPIO_PUPDR_PUPD11_Msk
#define GPIO_PUPDR_PUPD11_0            (0x1UL << GPIO_PUPDR_PUPD11_Pos)         /*!< 0x00400000 */
#define GPIO_PUPDR_PUPD11_1            (0x2UL << GPIO_PUPDR_PUPD11_Pos)         /*!< 0x00800000 */
#define GPIO_PUPDR_PUPD12_Pos          (24U)
#define GPIO_PUPDR_PUPD12_Msk          (0x3UL << GPIO_PUPDR_PUPD12_Pos)         /*!< 0x03000000 */
#define GPIO_PUPDR_PUPD12              GPIO_PUPDR_PUPD12_Msk
#define GPIO_PUPDR_PUPD12_0            (0x1UL << GPIO_PUPDR_PUPD12_Pos)         /*!< 0x01000000 */
#define GPIO_PUPDR_PUPD12_1            (0x2UL << GPIO_PUPDR_PUPD12_Pos)         /*!< 0x02000000 */
#define GPIO_PUPDR_PUPD13_Pos          (26U)
#define GPIO_PUPDR_PUPD13_Msk          (0x3UL << GPIO_PUPDR_PUPD13_Pos)         /*!< 0x0C000000 */
#define GPIO_PUPDR_PUPD13              GPIO_PUPDR_PUPD13_Msk
#define GPIO_PUPDR_PUPD13_0            (0x1UL << GPIO_PUPDR_PUPD13_Pos)         /*!< 0x04000000 */
#define GPIO_PUPDR_PUPD13_1            (0x2UL << GPIO_PUPDR_PUPD13_Pos)         /*!< 0x08000000 */
#define GPIO_PUPDR_PUPD14_Pos          (28U)
#define GPIO_PUPDR_PUPD14_Msk          (0x3UL << GPIO_PUPDR_PUPD14_Pos)         /*!< 0x30000000 */
#define GPIO_PUPDR_PUPD14              GPIO_PUPDR_PUPD14_Msk
#define GPIO_PUPDR_PUPD14_0            (0x1UL << GPIO_PUPDR_PUPD14_Pos)         /*!< 0x10000000 */
#define GPIO_PUPDR_PUPD14_1            (0x2UL << GPIO_PUPDR_PUPD14_Pos)         /*!< 0x20000000 */
#define GPIO_PUPDR_PUPD15_Pos          (30U)
#define GPIO_PUPDR_PUPD15_Msk          (0x3UL << GPIO_PUPDR_PUPD15_Pos)         /*!< 0xC0000000 */
#define GPIO_PUPDR_PUPD15              GPIO_PUPDR_PUPD15_Msk
#define GPIO_PUPDR_PUPD15_0            (0x1UL << GPIO_PUPDR_PUPD15_Pos)         /*!< 0x40000000 */
#define GPIO_PUPDR_PUPD15_1            (0x2UL << GPIO_PUPDR_PUPD15_Pos)         /*!< 0x80000000 */

/******************  Bits definition for GPIO_IDR register  *****************/
#define GPIO_IDR_ID0_Pos               (0U)
#define GPIO_IDR_ID0_Msk               (0x1UL << GPIO_IDR_ID0_Pos)              /*!< 0x00000001 */
#define GPIO_IDR_ID0                   GPIO_IDR_ID0_Msk
#define GPIO_IDR_ID1_Pos               (1U)
#define GPIO_IDR_ID1_Msk               (0x1UL << GPIO_IDR_ID1_Pos)              /*!< 0x00000002 */
#define GPIO_IDR_ID1                   GPIO_IDR_ID1_Msk
#define GPIO_IDR_ID2_Pos               (2U)
#define GPIO_IDR_ID2_Msk               (0x1UL << GPIO_IDR_ID2_Pos)              /*!< 0x00000004 */
#define GPIO_IDR_ID2                   GPIO_IDR_ID2_Msk
#define GPIO_IDR_ID3_Pos               (3U)
#define GPIO_IDR_ID3_Msk               (0x1UL << GPIO_IDR_ID3_Pos)              /*!< 0x00000008 */
#define GPIO_IDR_ID3                   GPIO_IDR_ID3_Msk
#define GPIO_IDR_ID4_Pos               (4U)
#define GPIO_IDR_ID4_Msk               (0x1UL << GPIO_IDR_ID4_Pos)              /*!< 0x00000010 */
#define GPIO_IDR_ID4                   GPIO_IDR_ID4_Msk
#define GPIO_IDR_ID5_Pos               (5U)
#define GPIO_IDR_ID5_Msk               (0x1UL << GPIO_IDR_ID5_Pos)              /*!< 0x00000020 */
#define GPIO_IDR_ID5                   GPIO_IDR_ID5_Msk
#define GPIO_IDR_ID6_Pos               (6U)
#define GPIO_IDR_ID6_Msk               (0x1UL << GPIO_IDR_ID6_Pos)              /*!< 0x00000040 */
#define GPIO_IDR_ID6                   GPIO_IDR_ID6_Msk
#define GPIO_IDR_ID7_Pos               (7U)
#define GPIO_IDR_ID7_Msk               (0x1UL << GPIO_IDR_ID7_Pos)              /*!< 0x00000080 */
#define GPIO_IDR_ID7                   GPIO_IDR_ID7_Msk
#define GPIO_IDR_ID8_Pos               (8U)
#define GPIO_IDR_ID8_Msk               (0x1UL << GPIO_IDR_ID8_Pos)              /*!< 0x00000100 */
#define GPIO_IDR_ID8                   GPIO_IDR_ID8_Msk
#define GPIO_IDR_ID9_Pos               (9U)
#define GPIO_IDR_ID9_Msk               (0x1UL << GPIO_IDR_ID9_Pos)              /*!< 0x00000200 */
#define GPIO_IDR_ID9                   GPIO_IDR_ID9_Msk
#define GPIO_IDR_ID10_Pos              (10U)
#define GPIO_IDR_ID10_Msk              (0x1UL << GPIO_IDR_ID10_Pos)             /*!< 0x00000400 */
#define GPIO_IDR_ID10                  GPIO_IDR_ID10_Msk
#define GPIO_IDR_ID11_Pos              (11U)
#define GPIO_IDR_ID11_Msk              (0x1UL << GPIO_IDR_ID11_Pos)             /*!< 0x00000800 */
#define GPIO_IDR_ID11                  GPIO_IDR_ID11_Msk
#define GPIO_IDR_ID12_Pos              (12U)
#define GPIO_IDR_ID12_Msk              (0x1UL << GPIO_IDR_ID12_Pos)             /*!< 0x00001000 */
#define GPIO_IDR_ID12                  GPIO_IDR_ID12_Msk
#define GPIO_IDR_ID13_Pos              (13U)
#define GPIO_IDR_ID13_Msk              (0x1UL << GPIO_IDR_ID13_Pos)             /*!< 0x00002000 */
#define GPIO_IDR_ID13                  GPIO_IDR_ID13_Msk
#define GPIO_IDR_ID14_Pos              (14U)
#define GPIO_IDR_ID14_Msk              (0x1UL << GPIO_IDR_ID14_Pos)             /*!< 0x00004000 */
#define GPIO_IDR_ID14                  GPIO_IDR_ID14_Msk
#define GPIO_IDR_ID15_Pos              (15U)
#define GPIO_IDR_ID15_Msk              (0x1UL << GPIO_IDR_ID15_Pos)             /*!< 0x00008000 */
#define GPIO_IDR_ID15                  GPIO_IDR_ID15_Msk

/******************  Bits definition for GPIO_ODR register  *****************/
#define GPIO_ODR_OD0_Pos               (0U)
#define GPIO_ODR_OD0_Msk               (0x1UL << GPIO_ODR_OD0_Pos)              /*!< 0x00000001 */
#define GPIO_ODR_OD0                   GPIO_ODR_OD0_Msk
#define GPIO_ODR_OD1_Pos               (1U)
#define GPIO_ODR_OD1_Msk               (0x1UL << GPIO_ODR_OD1_Pos)              /*!< 0x00000002 */
#define GPIO_ODR_OD1                   GPIO_ODR_OD1_Msk
#define GPIO_ODR_OD2_Pos               (2U)
#define GPIO_ODR_OD2_Msk               (0x1UL << GPIO_ODR_OD2_Pos)              /*!< 0x00000004 */
#define GPIO_ODR_OD2                   GPIO_ODR_OD2_Msk
#define GPIO_ODR_OD3_Pos               (3U)
#define GPIO_ODR_OD3_Msk               (0x1UL << GPIO_ODR_OD3_Pos)              /*!< 0x00000008 */
#define GPIO_ODR_OD3                   GPIO_ODR_OD3_Msk
#define GPIO_ODR_OD4_Pos               (4U)
#define GPIO_ODR_OD4_Msk               (0x1UL << GPIO_ODR_OD4_Pos)              /*!< 0x00000010 */
#define GPIO_ODR_OD4                   GPIO_ODR_OD4_Msk
#define GPIO_ODR_OD5_Pos               (5U)
#define GPIO_ODR_OD5_Msk               (0x1UL << GPIO_ODR_OD5_Pos)              /*!< 0x00000020 */
#define GPIO_ODR_OD5                   GPIO_ODR_OD5_Msk
#define GPIO_ODR_OD6_Pos               (6U)
#define GPIO_ODR_OD6_Msk               (0x1UL << GPIO_ODR_OD6_Pos)              /*!< 0x00000040 */
#define GPIO_ODR_OD6                   GPIO_ODR_OD6_Msk
#define GPIO_ODR_OD7_Pos               (7U)
#define GPIO_ODR_OD7_Msk               (0x1UL << GPIO_ODR_OD7_Pos)              /*!< 0x00000080 */
#define GPIO_ODR_OD7                   GPIO_ODR_OD7_Msk
#define GPIO_ODR_OD8_Pos               (8U)
#define GPIO_ODR_OD8_Msk               (0x1UL << GPIO_ODR_OD8_Pos)              /*!< 0x00000100 */
#define GPIO_ODR_OD8                   GPIO_ODR_OD8_Msk
#define GPIO_ODR_OD9_Pos               (9U)
#define GPIO_ODR_OD9_Msk               (0x1UL << GPIO_ODR_OD9_Pos)              /*!< 0x00000200 */
#define GPIO_ODR_OD9                   GPIO_ODR_OD9_Msk
#define GPIO_ODR_OD10_Pos              (10U)
#define GPIO_ODR_OD10_Msk              (0x1UL << GPIO_ODR_OD10_Pos)             /*!< 0x00000400 */
#define GPIO_ODR_OD10                  GPIO_ODR_OD10_Msk
#define GPIO_ODR_OD11_Pos              (11U)
#define GPIO_ODR_OD11_Msk              (0x1UL << GPIO_ODR_OD11_Pos)             /*!< 0x00000800 */
#define GPIO_ODR_OD11                  GPIO_ODR_OD11_Msk
#define GPIO_ODR_OD12_Pos              (12U)
#define GPIO_ODR_OD12_Msk              (0x1UL << GPIO_ODR_OD12_Pos)             /*!< 0x00001000 */
#define GPIO_ODR_OD12                  GPIO_ODR_OD12_Msk
#define GPIO_ODR_OD13_Pos              (13U)
#define GPIO_ODR_OD13_Msk              (0x1UL << GPIO_ODR_OD13_Pos)             /*!< 0x00002000 */
#define GPIO_ODR_OD13                  GPIO_ODR_OD13_Msk
#define GPIO_ODR_OD14_Pos              (14U)
#define GPIO_ODR_OD14_Msk              (0x1UL << GPIO_ODR_OD14_Pos)             /*!< 0x00004000 */
#define GPIO_ODR_OD14                  GPIO_ODR_OD14_Msk
#define GPIO_ODR_OD15_Pos              (15U)
#define GPIO_ODR_OD15_Msk              (0x1UL << GPIO_ODR_OD15_Pos)             /*!< 0x00008000 */
#define GPIO_ODR_OD15                  GPIO_ODR_OD15_Msk

/******************  Bits definition for GPIO_BSRR register  ****************/
#define GPIO_BSRR_BS0_Pos              (0U)
#define GPIO_BSRR_BS0_Msk              (0x1UL << GPIO_BSRR_BS0_Pos)             /*!< 0x00000001 */
#define GPIO_BSRR_BS0                  GPIO_BSRR_BS0_Msk
#define GPIO_BSRR_BS1_Pos              (1U)
#define GPIO_BSRR_BS1_Msk              (0x1UL << GPIO_BSRR_BS1_Pos)             /*!< 0x00000002 */
#define GPIO_BSRR_BS1                  GPIO_BSRR_BS1_Msk
#define GPIO_BSRR_BS2_Pos              (2U)
#define GPIO_BSRR_BS2_Msk              (0x1UL << GPIO_BSRR_BS2_Pos)             /*!< 0x00000004 */
#define GPIO_BSRR_BS2                  GPIO_BSRR_BS2_Msk
#define GPIO_BSRR_BS3_Pos              (3U)
#define GPIO_BSRR_BS3_Msk              (0x1UL << GPIO_BSRR_BS3_Pos)             /*!< 0x00000008 */
#define GPIO_BSRR_BS3                  GPIO_BSRR_BS3_Msk
#define GPIO_BSRR_BS4_Pos              (4U)
#define GPIO_BSRR_BS4_Msk              (0x1UL << GPIO_BSRR_BS4_Pos)             /*!< 0x00000010 */
#define GPIO_BSRR_BS4                  GPIO_BSRR_BS4_Msk
#define GPIO_BSRR_BS5_Pos              (5U)
#define GPIO_BSRR_BS5_Msk              (0x1UL << GPIO_BSRR_BS5_Pos)             /*!< 0x00000020 */
#define GPIO_BSRR_BS5                  GPIO_BSRR_BS5_Msk
#define GPIO_BSRR_BS6_Pos              (6U)
#define GPIO_BSRR_BS6_Msk              (0x1UL << GPIO_BSRR_BS6_Pos)             /*!< 0x00000040 */
#define GPIO_BSRR_BS6                  GPIO_BSRR_BS6_Msk
#define GPIO_BSRR_BS7_Pos              (7U)
#define GPIO_BSRR_BS7_Msk              (0x1UL << GPIO_BSRR_BS7_Pos)             /*!< 0x00000080 */
#define GPIO_BSRR_BS7                  GPIO_BSRR_BS7_Msk
#define GPIO_BSRR_BS8_Pos              (8U)
#define GPIO_BSRR_BS8_Msk              (0x1UL << GPIO_BSRR_BS8_Pos)             /*!< 0x00000100 */
#define GPIO_BSRR_BS8                  GPIO_BSRR_BS8_Msk
#define GPIO_BSRR_BS9_Pos              (9U)
#define GPIO_BSRR_BS9_Msk              (0x1UL << GPIO_BSRR_BS9_Pos)             /*!< 0x00000200 */
#define GPIO_BSRR_BS9                  GPIO_BSRR_BS9_Msk
#define GPIO_BSRR_BS10_Pos             (10U)
#define GPIO_BSRR_BS10_Msk             (0x1UL << GPIO_BSRR_BS10_Pos)            /*!< 0x00000400 */
#define GPIO_BSRR_BS10                 GPIO_BSRR_BS10_Msk
#define GPIO_BSRR_BS11_Pos             (11U)
#define GPIO_BSRR_BS11_Msk             (0x1UL << GPIO_BSRR_BS11_Pos)            /*!< 0x00000800 */
#define GPIO_BSRR_BS11                 GPIO_BSRR_BS11_Msk
#define GPIO_BSRR_BS12_Pos             (12U)
#define GPIO_BSRR_BS12_Msk             (0x1UL << GPIO_BSRR_BS12_Pos)            /*!< 0x00001000 */
#define GPIO_BSRR_BS12                 GPIO_BSRR_BS12_Msk
#define GPIO_BSRR_BS13_Pos             (13U)
#define GPIO_BSRR_BS13_Msk             (0x1UL << GPIO_BSRR_BS13_Pos)            /*!< 0x00002000 */
#define GPIO_BSRR_BS13                 GPIO_BSRR_BS13_Msk
#define GPIO_BSRR_BS14_Pos             (14U)
#define GPIO_BSRR_BS14_Msk             (0x1UL << GPIO_BSRR_BS14_Pos)            /*!< 0x00004000 */
#define GPIO_BSRR_BS14                 GPIO_BSRR_BS14_Msk
#define GPIO_BSRR_BS15_Pos             (15U)
#define GPIO_BSRR_BS15_Msk             (0x1UL << GPIO_BSRR_BS15_Pos)            /*!< 0x00008000 */
#define GPIO_BSRR_BS15                 GPIO_BSRR_BS15_Msk
#define GPIO_BSRR_BR0_Pos              (16U)
#define GPIO_BSRR_BR0_Msk              (0x1UL << GPIO_BSRR_BR0_Pos)             /*!< 0x00010000 */
#define GPIO_BSRR_BR0                  GPIO_BSRR_BR0_Msk
#define GPIO_BSRR_BR1_Pos              (17U)
#define GPIO_BSRR_BR1_Msk              (0x1UL << GPIO_BSRR_BR1_Pos)             /*!< 0x00020000 */
#define GPIO_BSRR_BR1                  GPIO_BSRR_BR1_Msk
#define GPIO_BSRR_BR2_Pos              (18U)
#define GPIO_BSRR_BR2_Msk              (0x1UL << GPIO_BSRR_BR2_Pos)             /*!< 0x00040000 */
#define GPIO_BSRR_BR2                  GPIO_BSRR_BR2_Msk
#define GPIO_BSRR_BR3_Pos              (19U)
#define GPIO_BSRR_BR3_Msk              (0x1UL << GPIO_BSRR_BR3_Pos)             /*!< 0x00080000 */
#define GPIO_BSRR_BR3                  GPIO_BSRR_BR3_Msk
#define GPIO_BSRR_BR4_Pos              (20U)
#define GPIO_BSRR_BR4_Msk              (0x1UL << GPIO_BSRR_BR4_Pos)             /*!< 0x00100000 */
#define GPIO_BSRR_BR4                  GPIO_BSRR_BR4_Msk
#define GPIO_BSRR_BR5_Pos              (21U)
#define GPIO_BSRR_BR5_Msk              (0x1UL << GPIO_BSRR_BR5_Pos)             /*!< 0x00200000 */
#define GPIO_BSRR_BR5                  GPIO_BSRR_BR5_Msk
#define GPIO_BSRR_BR6_Pos              (22U)
#define GPIO_BSRR_BR6_Msk              (0x1UL << GPIO_BSRR_BR6_Pos)             /*!< 0x00400000 */
#define GPIO_BSRR_BR6                  GPIO_BSRR_BR6_Msk
#define GPIO_BSRR_BR7_Pos              (23U)
#define GPIO_BSRR_BR7_Msk              (0x1UL << GPIO_BSRR_BR7_Pos)             /*!< 0x00800000 */
#define GPIO_BSRR_BR7                  GPIO_BSRR_BR7_Msk
#define GPIO_BSRR_BR8_Pos              (24U)
#define GPIO_BSRR_BR8_Msk              (0x1UL << GPIO_BSRR_BR8_Pos)             /*!< 0x01000000 */
#define GPIO_BSRR_BR8                  GPIO_BSRR_BR8_Msk
#define GPIO_BSRR_BR9_Pos              (25U)
#define GPIO_BSRR_BR9_Msk              (0x1UL << GPIO_BSRR_BR9_Pos)             /*!< 0x02000000 */
#define GPIO_BSRR_BR9                  GPIO_BSRR_BR9_Msk
#define GPIO_BSRR_BR10_Pos             (26U)
#define GPIO_BSRR_BR10_Msk             (0x1UL << GPIO_BSRR_BR10_Pos)            /*!< 0x04000000 */
#define GPIO_BSRR_BR10                 GPIO_BSRR_BR10_Msk
#define GPIO_BSRR_BR11_Pos             (27U)
#define GPIO_BSRR_BR11_Msk             (0x1UL << GPIO_BSRR_BR11_Pos)            /*!< 0x08000000 */
#define GPIO_BSRR_BR11                 GPIO_BSRR_BR11_Msk
#define GPIO_BSRR_BR12_Pos             (28U)
#define GPIO_BSRR_BR12_Msk             (0x1UL << GPIO_BSRR_BR12_Pos)            /*!< 0x10000000 */
#define GPIO_BSRR_BR12                 GPIO_BSRR_BR12_Msk
#define GPIO_BSRR_BR13_Pos             (29U)
#define GPIO_BSRR_BR13_Msk             (0x1UL << GPIO_BSRR_BR13_Pos)            /*!< 0x20000000 */
#define GPIO_BSRR_BR13                 GPIO_BSRR_BR13_Msk
#define GPIO_BSRR_BR14_Pos             (30U)
#define GPIO_BSRR_BR14_Msk             (0x1UL << GPIO_BSRR_BR14_Pos)            /*!< 0x40000000 */
#define GPIO_BSRR_BR14                 GPIO_BSRR_BR14_Msk
#define GPIO_BSRR_BR15_Pos             (31U)
#define GPIO_BSRR_BR15_Msk             (0x1UL << GPIO_BSRR_BR15_Pos)            /*!< 0x80000000 */
#define GPIO_BSRR_BR15                 GPIO_BSRR_BR15_Msk

/****************** Bit definition for GPIO_LCKR register *******************/
#define GPIO_LCKR_LCK0_Pos             (0U)
#define GPIO_LCKR_LCK0_Msk             (0x1UL << GPIO_LCKR_LCK0_Pos)            /*!< 0x00000001 */
#define GPIO_LCKR_LCK0                 GPIO_LCKR_LCK0_Msk
#define GPIO_LCKR_LCK1_Pos             (1U)
#define GPIO_LCKR_LCK1_Msk             (0x1UL << GPIO_LCKR_LCK1_Pos)            /*!< 0x00000002 */
#define GPIO_LCKR_LCK1                 GPIO_LCKR_LCK1_Msk
#define GPIO_LCKR_LCK2_Pos             (2U)
#define GPIO_LCKR_LCK2_Msk             (0x1UL << GPIO_LCKR_LCK2_Pos)            /*!< 0x00000004 */
#define GPIO_LCKR_LCK2                 GPIO_LCKR_LCK2_Msk
#define GPIO_LCKR_LCK3_Pos             (3U)
#define GPIO_LCKR_LCK3_Msk             (0x1UL << GPIO_LCKR_LCK3_Pos)            /*!< 0x00000008 */
#define GPIO_LCKR_LCK3                 GPIO_LCKR_LCK3_Msk
#define GPIO_LCKR_LCK4_Pos             (4U)
#define GPIO_LCKR_LCK4_Msk             (0x1UL << GPIO_LCKR_LCK4_Pos)            /*!< 0x00000010 */
#define GPIO_LCKR_LCK4                 GPIO_LCKR_LCK4_Msk
#define GPIO_LCKR_LCK5_Pos             (5U)
#define GPIO_LCKR_LCK5_Msk             (0x1UL << GPIO_LCKR_LCK5_Pos)            /*!< 0x00000020 */
#define GPIO_LCKR_LCK5                 GPIO_LCKR_LCK5_Msk
#define GPIO_LCKR_LCK6_Pos             (6U)
#define GPIO_LCKR_LCK6_Msk             (0x1UL << GPIO_LCKR_LCK6_Pos)            /*!< 0x00000040 */
#define GPIO_LCKR_LCK6                 GPIO_LCKR_LCK6_Msk
#define GPIO_LCKR_LCK7_Pos             (7U)
#define GPIO_LCKR_LCK7_Msk             (0x1UL << GPIO_LCKR_LCK7_Pos)            /*!< 0x00000080 */
#define GPIO_LCKR_LCK7                 GPIO_LCKR_LCK7_Msk
#define GPIO_LCKR_LCK8_Pos             (8U)
#define GPIO_LCKR_LCK8_Msk             (0x1UL << GPIO_LCKR_LCK8_Pos)            /*!< 0x00000100 */
#define GPIO_LCKR_LCK8                 GPIO_LCKR_LCK8_Msk
#define GPIO_LCKR_LCK9_Pos             (9U)
#define GPIO_LCKR_LCK9_Msk             (0x1UL << GPIO_LCKR_LCK9_Pos)            /*!< 0x00000200 */
#define GPIO_LCKR_LCK9                 GPIO_LCKR_LCK9_Msk
#define GPIO_LCKR_LCK10_Pos            (10U)
#define GPIO_LCKR_LCK10_Msk            (0x1UL << GPIO_LCKR_LCK10_Pos)           /*!< 0x00000400 */
#define GPIO_LCKR_LCK10                GPIO_LCKR_LCK10_Msk
#define GPIO_LCKR_LCK11_Pos            (11U)
#define GPIO_LCKR_LCK11_Msk            (0x1UL << GPIO_LCKR_LCK11_Pos)           /*!< 0x00000800 */
#define GPIO_LCKR_LCK11                GPIO_LCKR_LCK11_Msk
#define GPIO_LCKR_LCK12_Pos            (12U)
#define GPIO_LCKR_LCK12_Msk            (0x1UL << GPIO_LCKR_LCK12_Pos)           /*!< 0x00001000 */
#define GPIO_LCKR_LCK12                GPIO_LCKR_LCK12_Msk
#define GPIO_LCKR_LCK13_Pos            (13U)
#define GPIO_LCKR_LCK13_Msk            (0x1UL << GPIO_LCKR_LCK13_Pos)           /*!< 0x00002000 */
#define GPIO_LCKR_LCK13                GPIO_LCKR_LCK13_Msk
#define GPIO_LCKR_LCK14_Pos            (14U)
#define GPIO_LCKR_LCK14_Msk            (0x1UL << GPIO_LCKR_LCK14_Pos)           /*!< 0x00004000 */
#define GPIO_LCKR_LCK14                GPIO_LCKR_LCK14_Msk
#define GPIO_LCKR_LCK15_Pos            (15U)
#define GPIO_LCKR_LCK15_Msk            (0x1UL << GPIO_LCKR_LCK15_Pos)           /*!< 0x00008000 */
#define GPIO_LCKR_LCK15                GPIO_LCKR_LCK15_Msk
#define GPIO_LCKR_LCKK_Pos             (16U)
#define GPIO_LCKR_LCKK_Msk             (0x1UL << GPIO_LCKR_LCKK_Pos)            /*!< 0x00010000 */
#define GPIO_LCKR_LCKK                 GPIO_LCKR_LCKK_Msk

/****************** Bit definition for GPIO_AFRL register *******************/
#define GPIO_AFRL_AFSEL0_Pos           (0U)
#define GPIO_AFRL_AFSEL0_Msk           (0xFUL << GPIO_AFRL_AFSEL0_Pos)          /*!< 0x0000000F */
#define GPIO_AFRL_AFSEL0               GPIO_AFRL_AFSEL0_Msk
#define GPIO_AFRL_AFSEL0_0             (0x1UL << GPIO_AFRL_AFSEL0_Pos)          /*!< 0x00000001 */
#define GPIO_AFRL_AFSEL0_1             (0x2UL << GPIO_AFRL_AFSEL0_Pos)          /*!< 0x00000002 */
#define GPIO_AFRL_AFSEL0_2             (0x4UL << GPIO_AFRL_AFSEL0_Pos)          /*!< 0x00000004 */
#define GPIO_AFRL_AFSEL0_3             (0x8UL << GPIO_AFRL_AFSEL0_Pos)          /*!< 0x00000008 */
#define GPIO_AFRL_AFSEL1_Pos           (4U)
#define GPIO_AFRL_AFSEL1_Msk           (0xFUL << GPIO_AFRL_AFSEL1_Pos)          /*!< 0x000000F0 */
#define GPIO_AFRL_AFSEL1               GPIO_AFRL_AFSEL1_Msk
#define GPIO_AFRL_AFSEL1_0             (0x1UL << GPIO_AFRL_AFSEL1_Pos)          /*!< 0x00000010 */
#define GPIO_AFRL_AFSEL1_1             (0x2UL << GPIO_AFRL_AFSEL1_Pos)          /*!< 0x00000020 */
#define GPIO_AFRL_AFSEL1_2             (0x4UL << GPIO_AFRL_AFSEL1_Pos)          /*!< 0x00000040 */
#define GPIO_AFRL_AFSEL1_3             (0x8UL << GPIO_AFRL_AFSEL1_Pos)          /*!< 0x00000080 */
#define GPIO_AFRL_AFSEL2_Pos           (8U)
#define GPIO_AFRL_AFSEL2_Msk           (0xFUL << GPIO_AFRL_AFSEL2_Pos)          /*!< 0x00000F00 */
#define GPIO_AFRL_AFSEL2               GPIO_AFRL_AFSEL2_Msk
#define GPIO_AFRL_AFSEL2_0             (0x1UL << GPIO_AFRL_AFSEL2_Pos)          /*!< 0x00000100 */
#define GPIO_AFRL_AFSEL2_1             (0x2UL << GPIO_AFRL_AFSEL2_Pos)          /*!< 0x00000200 */
#define GPIO_AFRL_AFSEL2_2             (0x4UL << GPIO_AFRL_AFSEL2_Pos)          /*!< 0x00000400 */
#define GPIO_AFRL_AFSEL2_3             (0x8UL << GPIO_AFRL_AFSEL2_Pos)          /*!< 0x00000800 */
#define GPIO_AFRL_AFSEL3_Pos           (12U)
#define GPIO_AFRL_AFSEL3_Msk           (0xFUL << GPIO_AFRL_AFSEL3_Pos)          /*!< 0x0000F000 */
#define GPIO_AFRL_AFSEL3               GPIO_AFRL_AFSEL3_Msk
#define GPIO_AFRL_AFSEL3_0             (0x1UL << GPIO_AFRL_AFSEL3_Pos)          /*!< 0x00001000 */
#define GPIO_AFRL_AFSEL3_1             (0x2UL << GPIO_AFRL_AFSEL3_Pos)          /*!< 0x00002000 */
#define GPIO_AFRL_AFSEL3_2             (0x4UL << GPIO_AFRL_AFSEL3_Pos)          /*!< 0x00004000 */
#define GPIO_AFRL_AFSEL3_3             (0x8UL << GPIO_AFRL_AFSEL3_Pos)          /*!< 0x00008000 */
#define GPIO_AFRL_AFSEL4_Pos           (16U)
#define GPIO_AFRL_AFSEL4_Msk           (0xFUL << GPIO_AFRL_AFSEL4_Pos)          /*!< 0x000F0000 */
#define GPIO_AFRL_AFSEL4               GPIO_AFRL_AFSEL4_Msk
#define GPIO_AFRL_AFSEL4_0             (0x1UL << GPIO_AFRL_AFSEL4_Pos)          /*!< 0x00010000 */
#define GPIO_AFRL_AFSEL4_1             (0x2UL << GPIO_AFRL_AFSEL4_Pos)          /*!< 0x00020000 */
#define GPIO_AFRL_AFSEL4_2             (0x4UL << GPIO_AFRL_AFSEL4_Pos)          /*!< 0x00040000 */
#define GPIO_AFRL_AFSEL4_3             (0x8UL << GPIO_AFRL_AFSEL4_Pos)          /*!< 0x00080000 */
#define GPIO_AFRL_AFSEL5_Pos           (20U)
#define GPIO_AFRL_AFSEL5_Msk           (0xFUL << GPIO_AFRL_AFSEL5_Pos)          /*!< 0x00F00000 */
#define GPIO_AFRL_AFSEL5               GPIO_AFRL_AFSEL5_Msk
#define GPIO_AFRL_AFSEL5_0             (0x1UL << GPIO_AFRL_AFSEL5_Pos)          /*!< 0x00100000 */
#define GPIO_AFRL_AFSEL5_1             (0x2UL << GPIO_AFRL_AFSEL5_Pos)          /*!< 0x00200000 */
#define GPIO_AFRL_AFSEL5_2             (0x4UL << GPIO_AFRL_AFSEL5_Pos)          /*!< 0x00400000 */
#define GPIO_AFRL_AFSEL5_3             (0x8UL << GPIO_AFRL_AFSEL5_Pos)          /*!< 0x00800000 */
#define GPIO_AFRL_AFSEL6_Pos           (24U)
#define GPIO_AFRL_AFSEL6_Msk           (0xFUL << GPIO_AFRL_AFSEL6_Pos)          /*!< 0x0F000000 */
#define GPIO_AFRL_AFSEL6               GPIO_AFRL_AFSEL6_Msk
#define GPIO_AFRL_AFSEL6_0             (0x1UL << GPIO_AFRL_AFSEL6_Pos)          /*!< 0x01000000 */
#define GPIO_AFRL_AFSEL6_1             (0x2UL << GPIO_AFRL_AFSEL6_Pos)          /*!< 0x02000000 */
#define GPIO_AFRL_AFSEL6_2             (0x4UL << GPIO_AFRL_AFSEL6_Pos)          /*!< 0x04000000 */
#define GPIO_AFRL_AFSEL6_3             (0x8UL << GPIO_AFRL_AFSEL6_Pos)          /*!< 0x08000000 */
#define GPIO_AFRL_AFSEL7_Pos           (28U)
#define GPIO_AFRL_AFSEL7_Msk           (0xFUL << GPIO_AFRL_AFSEL7_Pos)          /*!< 0xF0000000 */
#define GPIO_AFRL_AFSEL7               GPIO_AFRL_AFSEL7_Msk
#define GPIO_AFRL_AFSEL7_0             (0x1UL << GPIO_AFRL_AFSEL7_Pos)          /*!< 0x10000000 */
#define GPIO_AFRL_AFSEL7_1             (0x2UL << GPIO_AFRL_AFSEL7_Pos)          /*!< 0x20000000 */
#define GPIO_AFRL_AFSEL7_2             (0x4UL << GPIO_AFRL_AFSEL7_Pos)          /*!< 0x40000000 */
#define GPIO_AFRL_AFSEL7_3             (0x8UL << GPIO_AFRL_AFSEL7_Pos)          /*!< 0x80000000 */

/****************** Bit definition for GPIO_AFRH register *******************/
#define GPIO_AFRH_AFSEL8_Pos           (0U)
#define GPIO_AFRH_AFSEL8_Msk           (0xFUL << GPIO_AFRH_AFSEL8_Pos)          /*!< 0x0000000F */
#define GPIO_AFRH_AFSEL8               GPIO_AFRH_AFSEL8_Msk
#define GPIO_AFRH_AFSEL8_0             (0x1UL << GPIO_AFRH_AFSEL8_Pos)          /*!< 0x00000001 */
#define GPIO_AFRH_AFSEL8_1             (0x2UL << GPIO_AFRH_AFSEL8_Pos)          /*!< 0x00000002 */
#define GPIO_AFRH_AFSEL8_2             (0x4UL << GPIO_AFRH_AFSEL8_Pos)          /*!< 0x00000004 */
#define GPIO_AFRH_AFSEL8_3             (0x8UL << GPIO_AFRH_AFSEL8_Pos)          /*!< 0x00000008 */
#define GPIO_AFRH_AFSEL9_Pos           (4U)
#define GPIO_AFRH_AFSEL9_Msk           (0xFUL << GPIO_AFRH_AFSEL9_Pos)          /*!< 0x000000F0 */
#define GPIO_AFRH_AFSEL9               GPIO_AFRH_AFSEL9_Msk
#define GPIO_AFRH_AFSEL9_0             (0x1UL << GPIO_AFRH_AFSEL9_Pos)          /*!< 0x00000010 */
#define GPIO_AFRH_AFSEL9_1             (0x2UL << GPIO_AFRH_AFSEL9_Pos)          /*!< 0x00000020 */
#define GPIO_AFRH_AFSEL9_2             (0x4UL << GPIO_AFRH_AFSEL9_Pos)          /*!< 0x00000040 */
#define GPIO_AFRH_AFSEL9_3             (0x8UL << GPIO_AFRH_AFSEL9_Pos)          /*!< 0x00000080 */
#define GPIO_AFRH_AFSEL10_Pos          (8U)
#define GPIO_AFRH_AFSEL10_Msk          (0xFUL << GPIO_AFRH_AFSEL10_Pos)         /*!< 0x00000F00 */
#define GPIO_AFRH_AFSEL10              GPIO_AFRH_AFSEL10_Msk
#define GPIO_AFRH_AFSEL10_0            (0x1UL << GPIO_AFRH_AFSEL10_Pos)         /*!< 0x00000100 */
#define GPIO_AFRH_AFSEL10_1            (0x2UL << GPIO_AFRH_AFSEL10_Pos)         /*!< 0x00000200 */
#define GPIO_AFRH_AFSEL10_2            (0x4UL << GPIO_AFRH_AFSEL10_Pos)         /*!< 0x00000400 */
#define GPIO_AFRH_AFSEL10_3            (0x8UL << GPIO_AFRH_AFSEL10_Pos)         /*!< 0x00000800 */
#define GPIO_AFRH_AFSEL11_Pos          (12U)
#define GPIO_AFRH_AFSEL11_Msk          (0xFUL << GPIO_AFRH_AFSEL11_Pos)         /*!< 0x0000F000 */
#define GPIO_AFRH_AFSEL11              GPIO_AFRH_AFSEL11_Msk
#define GPIO_AFRH_AFSEL11_0            (0x1UL << GPIO_AFRH_AFSEL11_Pos)         /*!< 0x00001000 */
#define GPIO_AFRH_AFSEL11_1            (0x2UL << GPIO_AFRH_AFSEL11_Pos)         /*!< 0x00002000 */
#define GPIO_AFRH_AFSEL11_2            (0x4UL << GPIO_AFRH_AFSEL11_Pos)         /*!< 0x00004000 */
#define GPIO_AFRH_AFSEL11_3            (0x8UL << GPIO_AFRH_AFSEL11_Pos)         /*!< 0x00008000 */
#define GPIO_AFRH_AFSEL12_Pos          (16U)
#define GPIO_AFRH_AFSEL12_Msk          (0xFUL << GPIO_AFRH_AFSEL12_Pos)         /*!< 0x000F0000 */
#define GPIO_AFRH_AFSEL12              GPIO_AFRH_AFSEL12_Msk
#define GPIO_AFRH_AFSEL12_0            (0x1UL << GPIO_AFRH_AFSEL12_Pos)         /*!< 0x00010000 */
#define GPIO_AFRH_AFSEL12_1            (0x2UL << GPIO_AFRH_AFSEL12_Pos)         /*!< 0x00020000 */
#define GPIO_AFRH_AFSEL12_2            (0x4UL << GPIO_AFRH_AFSEL12_Pos)         /*!< 0x00040000 */
#define GPIO_AFRH_AFSEL12_3            (0x8UL << GPIO_AFRH_AFSEL12_Pos)         /*!< 0x00080000 */
#define GPIO_AFRH_AFSEL13_Pos          (20U)
#define GPIO_AFRH_AFSEL13_Msk          (0xFUL << GPIO_AFRH_AFSEL13_Pos)         /*!< 0x00F00000 */
#define GPIO_AFRH_AFSEL13              GPIO_AFRH_AFSEL13_Msk
#define GPIO_AFRH_AFSEL13_0            (0x1UL << GPIO_AFRH_AFSEL13_Pos)         /*!< 0x00100000 */
#define GPIO_AFRH_AFSEL13_1            (0x2UL << GPIO_AFRH_AFSEL13_Pos)         /*!< 0x00200000 */
#define GPIO_AFRH_AFSEL13_2            (0x4UL << GPIO_AFRH_AFSEL13_Pos)         /*!< 0x00400000 */
#define GPIO_AFRH_AFSEL13_3            (0x8UL << GPIO_AFRH_AFSEL13_Pos)         /*!< 0x00800000 */
#define GPIO_AFRH_AFSEL14_Pos          (24U)
#define GPIO_AFRH_AFSEL14_Msk          (0xFUL << GPIO_AFRH_AFSEL14_Pos)         /*!< 0x0F000000 */
#define GPIO_AFRH_AFSEL14              GPIO_AFRH_AFSEL14_Msk
#define GPIO_AFRH_AFSEL14_0            (0x1UL << GPIO_AFRH_AFSEL14_Pos)         /*!< 0x01000000 */
#define GPIO_AFRH_AFSEL14_1            (0x2UL << GPIO_AFRH_AFSEL14_Pos)         /*!< 0x02000000 */
#define GPIO_AFRH_AFSEL14_2            (0x4UL << GPIO_AFRH_AFSEL14_Pos)         /*!< 0x04000000 */
#define GPIO_AFRH_AFSEL14_3            (0x8UL << GPIO_AFRH_AFSEL14_Pos)         /*!< 0x08000000 */
#define GPIO_AFRH_AFSEL15_Pos          (28U)
#define GPIO_AFRH_AFSEL15_Msk          (0xFUL << GPIO_AFRH_AFSEL15_Pos)         /*!< 0xF0000000 */
#define GPIO_AFRH_AFSEL15              GPIO_AFRH_AFSEL15_Msk
#define GPIO_AFRH_AFSEL15_0            (0x1UL << GPIO_AFRH_AFSEL15_Pos)         /*!< 0x10000000 */
#define GPIO_AFRH_AFSEL15_1            (0x2UL << GPIO_AFRH_AFSEL15_Pos)         /*!< 0x20000000 */
#define GPIO_AFRH_AFSEL15_2            (0x4UL << GPIO_AFRH_AFSEL15_Pos)         /*!< 0x40000000 */
#define GPIO_AFRH_AFSEL15_3            (0x8UL << GPIO_AFRH_AFSEL15_Pos)         /*!< 0x80000000 */

/******************  Bits definition for GPIO_BRR register  *****************/
#define GPIO_BRR_BR0_Pos               (0U)
#define GPIO_BRR_BR0_Msk               (0x1UL << GPIO_BRR_BR0_Pos)              /*!< 0x00000001 */
#define GPIO_BRR_BR0                   GPIO_BRR_BR0_Msk
#define GPIO_BRR_BR1_Pos               (1U)
#define GPIO_BRR_BR1_Msk               (0x1UL << GPIO_BRR_BR1_Pos)              /*!< 0x00000002 */
#define GPIO_BRR_BR1                   GPIO_BRR_BR1_Msk
#define GPIO_BRR_BR2_Pos               (2U)
#define GPIO_BRR_BR2_Msk               (0x1UL << GPIO_BRR_BR2_Pos)              /*!< 0x00000004 */
#define GPIO_BRR_BR2                   GPIO_BRR_BR2_Msk
#define GPIO_BRR_BR3_Pos               (3U)
#define GPIO_BRR_BR3_Msk               (0x1UL << GPIO_BRR_BR3_Pos)              /*!< 0x00000008 */
#define GPIO_BRR_BR3                   GPIO_BRR_BR3_Msk
#define GPIO_BRR_BR4_Pos               (4U)
#define GPIO_BRR_BR4_Msk               (0x1UL << GPIO_BRR_BR4_Pos)              /*!< 0x00000010 */
#define GPIO_BRR_BR4                   GPIO_BRR_BR4_Msk
#define GPIO_BRR_BR5_Pos               (5U)
#define GPIO_BRR_BR5_Msk               (0x1UL << GPIO_BRR_BR5_Pos)              /*!< 0x00000020 */
#define GPIO_BRR_BR5                   GPIO_BRR_BR5_Msk
#define GPIO_BRR_BR6_Pos               (6U)
#define GPIO_BRR_BR6_Msk               (0x1UL << GPIO_BRR_BR6_Pos)              /*!< 0x00000040 */
#define GPIO_BRR_BR6                   GPIO_BRR_BR6_Msk
#define GPIO_BRR_BR7_Pos               (7U)
#define GPIO_BRR_BR7_Msk               (0x1UL << GPIO_BRR_BR7_Pos)              /*!< 0x00000080 */
#define GPIO_BRR_BR7                   GPIO_BRR_BR7_Msk
#define GPIO_BRR_BR8_Pos               (8U)
#define GPIO_BRR_BR8_Msk               (0x1UL << GPIO_BRR_BR8_Pos)              /*!< 0x00000100 */
#define GPIO_BRR_BR8                   GPIO_BRR_BR8_Msk
#define GPIO_BRR_BR9_Pos               (9U)
#define GPIO_BRR_BR9_Msk               (0x1UL << GPIO_BRR_BR9_Pos)              /*!< 0x00000200 */
#define GPIO_BRR_BR9                   GPIO_BRR_BR9_Msk
#define GPIO_BRR_BR10_Pos              (10U)
#define GPIO_BRR_BR10_Msk              (0x1UL << GPIO_BRR_BR10_Pos)             /*!< 0x00000400 */
#define GPIO_BRR_BR10                  GPIO_BRR_BR10_Msk
#define GPIO_BRR_BR11_Pos              (11U)
#define GPIO_BRR_BR11_Msk              (0x1UL << GPIO_BRR_BR11_Pos)             /*!< 0x00000800 */
#define GPIO_BRR_BR11                  GPIO_BRR_BR11_Msk
#define GPIO_BRR_BR12_Pos              (12U)
#define GPIO_BRR_BR12_Msk              (0x1UL << GPIO_BRR_BR12_Pos)             /*!< 0x00001000 */
#define GPIO_BRR_BR12                  GPIO_BRR_BR12_Msk
#define GPIO_BRR_BR13_Pos              (13U)
#define GPIO_BRR_BR13_Msk              (0x1UL << GPIO_BRR_BR13_Pos)             /*!< 0x00002000 */
#define GPIO_BRR_BR13                  GPIO_BRR_BR13_Msk
#define GPIO_BRR_BR14_Pos              (14U)
#define GPIO_BRR_BR14_Msk              (0x1UL << GPIO_BRR_BR14_Pos)             /*!< 0x00004000 */
#define GPIO_BRR_BR14                  GPIO_BRR_BR14_Msk
#define GPIO_BRR_BR15_Pos              (15U)
#define GPIO_BRR_BR15_Msk              (0x1UL << GPIO_BRR_BR15_Pos)             /*!< 0x00008000 */
#define GPIO_BRR_BR15                  GPIO_BRR_BR15_Msk

/******************************************************************************/
/*                                                                            */
/*                           Hardware Divider(DIV)                            */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for DIV_DEND register  ********************/
#define DIV_DEND_Pos              (0U)
#define DIV_DEND_Msk              (0xFFFFFFFFUL << DIV_DEND_Pos)     /*!< 0xFFFFFFFF */
#define DIV_DEND                  DIV_DEND_Msk

/*******************  Bit definition for DIV_SOR register  *********************/
#define DIV_SOR_Pos               (0U)
#define DIV_SOR_Msk               (0xFFFFFFFFUL << DIV_SOR_Pos)      /*!< 0xFFFFFFFF */
#define DIV_SOR                   DIV_SOR_Msk

/*******************  Bit definition for DIV_QUOT register  ********************/
#define DIV_QUOT_Pos              (0U)
#define DIV_QUOT_Msk              (0xFFFFFFFFUL << DIV_QUOT_Pos)     /*!< 0xFFFFFFFF */
#define DIV_QUOT                  DIV_QUOT_Msk

/*******************  Bit definition for DIV_REMA register  ********************/
#define DIV_REMA_Pos              (0U)
#define DIV_REMA_Msk              (0xFFFFFFFFUL << DIV_REMA_Pos)     /*!< 0xFFFFFFFF */
#define DIV_REMA                  DIV_REMA_Msk

/*******************  Bit definition for DIV_SIGN register  ********************/
#define DIV_SIGN_DIV_SIGN_Pos     (0U)
#define DIV_SIGN_DIV_SIGN_Msk     (0x1UL << DIV_SIGN_DIV_SIGN_Pos)   /*!< 0x00000001 */
#define DIV_SIGN_DIV_SIGN         DIV_SIGN_DIV_SIGN_Msk

/*******************  Bit definition for DIV_STAT register  ********************/
#define DIV_STAT_DIV_END_Pos      (0U)
#define DIV_STAT_DIV_END_Msk      (0x1UL << DIV_STAT_DIV_END_Pos)    /*!< 0x00000001 */
#define DIV_STAT_DIV_END          DIV_STAT_DIV_END_Msk
#define DIV_STAT_DIV_ZERO_Pos     (1U)
#define DIV_STAT_DIV_ZERO_Msk     (0x1UL << DIV_STAT_DIV_ZERO_Pos)   /*!< 0x00000002 */
#define DIV_STAT_DIV_ZERO         DIV_STAT_DIV_ZERO_Msk

/****************************************************************************/
/*                                                                          */
/*                  Inter-integrated Circuit Interface (I2C)                */
/*                                                                          */
/****************************************************************************/
/*********************  Bits Define For Peripheral I2C  *********************/
/*!< I2C_CR1 */
#define I2C_CR1_PE_Pos                            (0U)
#define I2C_CR1_PE_Msk                            (0x1UL << I2C_CR1_PE_Pos)                         /*!< 0x00000001 */
#define I2C_CR1_PE                                I2C_CR1_PE_Msk                                    /*!< desc PE */
#define I2C_CR1_SMBUS_Pos                         (1U)
#define I2C_CR1_SMBUS_Msk                         (0x1UL << I2C_CR1_SMBUS_Pos)                      /*!< 0x00000002 */
#define I2C_CR1_SMBUS                             I2C_CR1_SMBUS_Msk                                 /*!< desc SMBUS */
#define I2C_CR1_SMBTYPE_Pos                       (3U)
#define I2C_CR1_SMBTYPE_Msk                       (0x1UL << I2C_CR1_SMBTYPE_Pos)                    /*!< 0x00000008 */
#define I2C_CR1_SMBTYPE                           I2C_CR1_SMBTYPE_Msk                               /*!< desc SMBTYPE */
#define I2C_CR1_ENARP_Pos                         (4U)
#define I2C_CR1_ENARP_Msk                         (0x1UL << I2C_CR1_ENARP_Pos)                      /*!< 0x00000010 */
#define I2C_CR1_ENARP                             I2C_CR1_ENARP_Msk                                 /*!< desc ENARP */
#define I2C_CR1_ENPEC_Pos                         (5U)
#define I2C_CR1_ENPEC_Msk                         (0x1UL << I2C_CR1_ENPEC_Pos)                      /*!< 0x00000020 */
#define I2C_CR1_ENPEC                             I2C_CR1_ENPEC_Msk                                 /*!< desc ENPEC */
#define I2C_CR1_ENGC_Pos                          (6U)
#define I2C_CR1_ENGC_Msk                          (0x1UL << I2C_CR1_ENGC_Pos)                       /*!< 0x00000040 */
#define I2C_CR1_ENGC                              I2C_CR1_ENGC_Msk                                  /*!< desc ENGC */
#define I2C_CR1_NOSTRETCH_Pos                     (7U)
#define I2C_CR1_NOSTRETCH_Msk                     (0x1UL << I2C_CR1_NOSTRETCH_Pos)                  /*!< 0x00000080 */
#define I2C_CR1_NOSTRETCH                         I2C_CR1_NOSTRETCH_Msk                             /*!< desc NOSTRETCH */
#define I2C_CR1_START_Pos                         (8U)
#define I2C_CR1_START_Msk                         (0x1UL << I2C_CR1_START_Pos)                      /*!< 0x00000100 */
#define I2C_CR1_START                             I2C_CR1_START_Msk                                 /*!< desc START */
#define I2C_CR1_STOP_Pos                          (9U)
#define I2C_CR1_STOP_Msk                          (0x1UL << I2C_CR1_STOP_Pos)                       /*!< 0x00000200 */
#define I2C_CR1_STOP                              I2C_CR1_STOP_Msk                                  /*!< desc STOP */
#define I2C_CR1_ACK_Pos                           (10U)
#define I2C_CR1_ACK_Msk                           (0x1UL << I2C_CR1_ACK_Pos)                        /*!< 0x00000400 */
#define I2C_CR1_ACK                               I2C_CR1_ACK_Msk                                   /*!< desc ACK */
#define I2C_CR1_POS_Pos                           (11U)
#define I2C_CR1_POS_Msk                           (0x1UL << I2C_CR1_POS_Pos)                        /*!< 0x00000800 */
#define I2C_CR1_POS                               I2C_CR1_POS_Msk                                   /*!< desc POS */
#define I2C_CR1_PEC_Pos                           (12U)
#define I2C_CR1_PEC_Msk                           (0x1UL << I2C_CR1_PEC_Pos)                        /*!< 0x00001000 */
#define I2C_CR1_PEC                               I2C_CR1_PEC_Msk                                   /*!< desc PEC */
#define I2C_CR1_ALERT_Pos                         (13U)
#define I2C_CR1_ALERT_Msk                         (0x1UL << I2C_CR1_ALERT_Pos)                      /*!< 0x00002000 */
#define I2C_CR1_ALERT                             I2C_CR1_ALERT_Msk                                 /*!< desc ALERT */
#define I2C_CR1_SWRST_Pos                         (15U)
#define I2C_CR1_SWRST_Msk                         (0x1UL << I2C_CR1_SWRST_Pos)                      /*!< 0x00008000 */
#define I2C_CR1_SWRST                             I2C_CR1_SWRST_Msk                                 /*!< desc SWRST */

/*!< I2C_CR2 */
#define I2C_CR2_FREQ_Pos                          (0U)
#define I2C_CR2_FREQ_Msk                          (0x3FUL << I2C_CR2_FREQ_Pos)                      /*!< 0x0000003F */
#define I2C_CR2_FREQ                              I2C_CR2_FREQ_Msk                                  /*!< FREQ[5:0] bits (desc FREQ) */
#define I2C_CR2_FREQ_0                            (0x1UL << I2C_CR2_FREQ_Pos)                       /*!< 0x00000001 */
#define I2C_CR2_FREQ_1                            (0x2UL << I2C_CR2_FREQ_Pos)                       /*!< 0x00000002 */
#define I2C_CR2_FREQ_2                            (0x4UL << I2C_CR2_FREQ_Pos)                       /*!< 0x00000004 */
#define I2C_CR2_FREQ_3                            (0x8UL << I2C_CR2_FREQ_Pos)                       /*!< 0x00000008 */
#define I2C_CR2_FREQ_4                            (0x10UL << I2C_CR2_FREQ_Pos)                      /*!< 0x00000010 */
#define I2C_CR2_FREQ_5                            (0x20UL << I2C_CR2_FREQ_Pos)                      /*!< 0x00000020 */
#define I2C_CR2_ITERREN_Pos                       (8U)
#define I2C_CR2_ITERREN_Msk                       (0x1UL << I2C_CR2_ITERREN_Pos)                    /*!< 0x00000100 */
#define I2C_CR2_ITERREN                           I2C_CR2_ITERREN_Msk                               /*!< desc ITERREN */
#define I2C_CR2_ITEVTEN_Pos                       (9U)
#define I2C_CR2_ITEVTEN_Msk                       (0x1UL << I2C_CR2_ITEVTEN_Pos)                    /*!< 0x00000200 */
#define I2C_CR2_ITEVTEN                           I2C_CR2_ITEVTEN_Msk                               /*!< desc ITEVTEN */
#define I2C_CR2_ITBUFEN_Pos                       (10U)
#define I2C_CR2_ITBUFEN_Msk                       (0x1UL << I2C_CR2_ITBUFEN_Pos)                    /*!< 0x00000400 */
#define I2C_CR2_ITBUFEN                           I2C_CR2_ITBUFEN_Msk                               /*!< desc ITBUFEN */
#define I2C_CR2_DMAEN_Pos                         (11U)
#define I2C_CR2_DMAEN_Msk                         (0x1UL << I2C_CR2_DMAEN_Pos)                      /*!< 0x00000800 */
#define I2C_CR2_DMAEN                             I2C_CR2_DMAEN_Msk                                 /*!< desc DMAEN */
#define I2C_CR2_LAST_Pos                          (12U)
#define I2C_CR2_LAST_Msk                          (0x1UL << I2C_CR2_LAST_Pos)                       /*!< 0x00001000 */
#define I2C_CR2_LAST                              I2C_CR2_LAST_Msk                                  /*!< desc LAST */

/*!< I2C_OAR1 */
#define I2C_OAR1_ADD0_Pos                         (0U)
#define I2C_OAR1_ADD0_Msk                         (0x1UL << I2C_OAR1_ADD0_Pos)                      /*!< 0x00000001 */
#define I2C_OAR1_ADD0                             I2C_OAR1_ADD0_Msk                                 /*!< desc ADD0 */
#define I2C_OAR1_ADD1_Pos                         (1U)
#define I2C_OAR1_ADD1_Msk                         (0x1UL << I2C_OAR1_ADD1_Pos)                      /*!< 0x00000002 */
#define I2C_OAR1_ADD1                             I2C_OAR1_ADD1_Msk                                 /*!< Bit 1 */
#define I2C_OAR1_ADD2_Pos                         (2U)
#define I2C_OAR1_ADD2_Msk                         (0x1UL << I2C_OAR1_ADD2_Pos)                      /*!< 0x00000004 */
#define I2C_OAR1_ADD2                             I2C_OAR1_ADD2_Msk                                 /*!< Bit 2 */
#define I2C_OAR1_ADD3_Pos                         (3U)
#define I2C_OAR1_ADD3_Msk                         (0x1UL << I2C_OAR1_ADD3_Pos)                      /*!< 0x00000008 */
#define I2C_OAR1_ADD3                             I2C_OAR1_ADD3_Msk                                 /*!< Bit 3 */
#define I2C_OAR1_ADD4_Pos                         (4U)
#define I2C_OAR1_ADD4_Msk                         (0x1UL << I2C_OAR1_ADD4_Pos)                      /*!< 0x00000010 */
#define I2C_OAR1_ADD4                             I2C_OAR1_ADD4_Msk                                 /*!< Bit 4 */
#define I2C_OAR1_ADD5_Pos                         (5U)
#define I2C_OAR1_ADD5_Msk                         (0x1UL << I2C_OAR1_ADD5_Pos)                      /*!< 0x00000020 */
#define I2C_OAR1_ADD5                             I2C_OAR1_ADD5_Msk                                 /*!< Bit 5 */
#define I2C_OAR1_ADD6_Pos                         (6U)
#define I2C_OAR1_ADD6_Msk                         (0x1UL << I2C_OAR1_ADD6_Pos)                      /*!< 0x00000040 */
#define I2C_OAR1_ADD6                             I2C_OAR1_ADD6_Msk                                 /*!< Bit 6 */
#define I2C_OAR1_ADD7_Pos                         (7U)
#define I2C_OAR1_ADD7_Msk                         (0x1UL << I2C_OAR1_ADD7_Pos)                      /*!< 0x00000080 */
#define I2C_OAR1_ADD7                             I2C_OAR1_ADD7_Msk                                 /*!< Bit 7 */
#define I2C_OAR1_ADD8_Pos                         (8U)
#define I2C_OAR1_ADD8_Msk                         (0x1UL << I2C_OAR1_ADD8_Pos)                      /*!< 0x00000100 */
#define I2C_OAR1_ADD8                             I2C_OAR1_ADD8_Msk                                 /*!< Bit 8 */
#define I2C_OAR1_ADD9_Pos                         (9U)
#define I2C_OAR1_ADD9_Msk                         (0x1UL << I2C_OAR1_ADD9_Pos)                      /*!< 0x00000200 */
#define I2C_OAR1_ADD9                             I2C_OAR1_ADD9_Msk                                 /*!< Bit 9 */
#define I2C_OAR1_ADD1_7                           0x000000FEU                                       /*!< Interface Address */
#define I2C_OAR1_ADD8_9                           0x00000300U                                       /*!< Interface Address */
#define I2C_OAR1_ADD_Pos                          (1U)
#define I2C_OAR1_ADD_Msk                          (0x1FFUL << I2C_OAR1_ADD_Pos)                     /*!< 0x000003FE */
#define I2C_OAR1_ADD                              I2C_OAR1_ADD_Msk                                  /*!< ADD[7:1] bits (desc ADD) */
#define I2C_OAR1_ADDMODE_Pos                      (15U)
#define I2C_OAR1_ADDMODE_Msk                      (0x1UL << I2C_OAR1_ADDMODE_Pos)                   /*!< 0x00008000 */
#define I2C_OAR1_ADDMODE                          I2C_OAR1_ADDMODE_Msk                              /*!< desc ADDMODE */

/*!< I2C_OAR2 */
#define I2C_OAR2_ENDUAL_Pos                       (0U)
#define I2C_OAR2_ENDUAL_Msk                       (0x1UL << I2C_OAR2_ENDUAL_Pos)                    /*!< 0x00000001 */
#define I2C_OAR2_ENDUAL                           I2C_OAR2_ENDUAL_Msk                               /*!< desc ENDUAL */
#define I2C_OAR2_ADD2_Pos                         (1U)
#define I2C_OAR2_ADD2_Msk                         (0x7FUL << I2C_OAR2_ADD2_Pos)                     /*!< 0x000000FE */
#define I2C_OAR2_ADD2                             I2C_OAR2_ADD2_Msk                                 /*!< ADD2[7:1] bits (desc ADD2) */

/*!< I2C_DR */
#define I2C_DR_DR_Pos                             (0U)
#define I2C_DR_DR_Msk                             (0xFFUL << I2C_DR_DR_Pos)                         /*!< 0x000000FF */
#define I2C_DR_DR                                 I2C_DR_DR_Msk                                     /*!< DR[7:0] bits (desc DR) */

/*!< I2C_SR1 */
#define I2C_SR1_SB_Pos                            (0U)
#define I2C_SR1_SB_Msk                            (0x1UL << I2C_SR1_SB_Pos)                         /*!< 0x00000001 */
#define I2C_SR1_SB                                I2C_SR1_SB_Msk                                    /*!< desc SB */
#define I2C_SR1_ADDR_Pos                          (1U)
#define I2C_SR1_ADDR_Msk                          (0x1UL << I2C_SR1_ADDR_Pos)                       /*!< 0x00000002 */
#define I2C_SR1_ADDR                              I2C_SR1_ADDR_Msk                                  /*!< desc ADDR */
#define I2C_SR1_BTF_Pos                           (2U)
#define I2C_SR1_BTF_Msk                           (0x1UL << I2C_SR1_BTF_Pos)                        /*!< 0x00000004 */
#define I2C_SR1_BTF                               I2C_SR1_BTF_Msk                                   /*!< desc BTF */
#define I2C_SR1_ADD10_Pos                         (3U)
#define I2C_SR1_ADD10_Msk                         (0x1UL << I2C_SR1_ADD10_Pos)                      /*!< 0x00000008 */
#define I2C_SR1_ADD10                             I2C_SR1_ADD10_Msk                                 /*!< desc ADD10 */
#define I2C_SR1_STOPF_Pos                         (4U)
#define I2C_SR1_STOPF_Msk                         (0x1UL << I2C_SR1_STOPF_Pos)                      /*!< 0x00000010 */
#define I2C_SR1_STOPF                             I2C_SR1_STOPF_Msk                                 /*!< desc STOPF */
#define I2C_SR1_RXNE_Pos                          (6U)
#define I2C_SR1_RXNE_Msk                          (0x1UL << I2C_SR1_RXNE_Pos)                       /*!< 0x00000040 */
#define I2C_SR1_RXNE                              I2C_SR1_RXNE_Msk                                  /*!< desc RXNE */
#define I2C_SR1_TXE_Pos                           (7U)
#define I2C_SR1_TXE_Msk                           (0x1UL << I2C_SR1_TXE_Pos)                        /*!< 0x00000080 */
#define I2C_SR1_TXE                               I2C_SR1_TXE_Msk                                   /*!< desc TXE */
#define I2C_SR1_BERR_Pos                          (8U)
#define I2C_SR1_BERR_Msk                          (0x1UL << I2C_SR1_BERR_Pos)                       /*!< 0x00000100 */
#define I2C_SR1_BERR                              I2C_SR1_BERR_Msk                                  /*!< desc BERR */
#define I2C_SR1_ARLO_Pos                          (9U)
#define I2C_SR1_ARLO_Msk                          (0x1UL << I2C_SR1_ARLO_Pos)                       /*!< 0x00000200 */
#define I2C_SR1_ARLO                              I2C_SR1_ARLO_Msk                                  /*!< desc ARLO */
#define I2C_SR1_AF_Pos                            (10U)
#define I2C_SR1_AF_Msk                            (0x1UL << I2C_SR1_AF_Pos)                         /*!< 0x00000400 */
#define I2C_SR1_AF                                I2C_SR1_AF_Msk                                    /*!< desc AF */
#define I2C_SR1_OVR_Pos                           (11U)
#define I2C_SR1_OVR_Msk                           (0x1UL << I2C_SR1_OVR_Pos)                        /*!< 0x00000800 */
#define I2C_SR1_OVR                               I2C_SR1_OVR_Msk                                   /*!< desc OVR */
#define I2C_SR1_PECERR_Pos                        (12U)
#define I2C_SR1_PECERR_Msk                        (0x1UL << I2C_SR1_PECERR_Pos)                     /*!< 0x00001000 */
#define I2C_SR1_PECERR                            I2C_SR1_PECERR_Msk                                /*!< desc PECERR */
#define I2C_SR1_TIMEOUT_Pos                       (14U)
#define I2C_SR1_TIMEOUT_Msk                       (0x1UL << I2C_SR1_TIMEOUT_Pos)                    /*!< 0x00004000 */
#define I2C_SR1_TIMEOUT                           I2C_SR1_TIMEOUT_Msk                               /*!< desc TIMEOUT */
#define I2C_SR1_SMBALERT_Pos                      (15U)
#define I2C_SR1_SMBALERT_Msk                      (0x1UL << I2C_SR1_SMBALERT_Pos)                   /*!< 0x00008000 */
#define I2C_SR1_SMBALERT                          I2C_SR1_SMBALERT_Msk                              /*!< desc SMBALERT */

/*!< I2C_SR2 */
#define I2C_SR2_MSL_Pos                           (0U)
#define I2C_SR2_MSL_Msk                           (0x1UL << I2C_SR2_MSL_Pos)                        /*!< 0x00000001 */
#define I2C_SR2_MSL                               I2C_SR2_MSL_Msk                                   /*!< desc MSL */
#define I2C_SR2_BUSY_Pos                          (1U)
#define I2C_SR2_BUSY_Msk                          (0x1UL << I2C_SR2_BUSY_Pos)                       /*!< 0x00000002 */
#define I2C_SR2_BUSY                              I2C_SR2_BUSY_Msk                                  /*!< desc BUSY */
#define I2C_SR2_TRA_Pos                           (2U)
#define I2C_SR2_TRA_Msk                           (0x1UL << I2C_SR2_TRA_Pos)                        /*!< 0x00000004 */
#define I2C_SR2_TRA                               I2C_SR2_TRA_Msk                                   /*!< desc TRA */
#define I2C_SR2_GENCALL_Pos                       (4U)
#define I2C_SR2_GENCALL_Msk                       (0x1UL << I2C_SR2_GENCALL_Pos)                    /*!< 0x00000010 */
#define I2C_SR2_GENCALL                           I2C_SR2_GENCALL_Msk                               /*!< desc GENCALL */
#define I2C_SR2_SMBDEFAULT_Pos                    (5U)
#define I2C_SR2_SMBDEFAULT_Msk                    (0x1UL << I2C_SR2_SMBDEFAULT_Pos)                 /*!< 0x00000020 */
#define I2C_SR2_SMBDEFAULT                        I2C_SR2_SMBDEFAULT_Msk                            /*!< desc SMBDEFAULT */
#define I2C_SR2_SMBHOST_Pos                       (6U)
#define I2C_SR2_SMBHOST_Msk                       (0x1UL << I2C_SR2_SMBHOST_Pos)                    /*!< 0x00000040 */
#define I2C_SR2_SMBHOST                           I2C_SR2_SMBHOST_Msk                               /*!< desc SMBHOST */
#define I2C_SR2_DUALF_Pos                         (7U)
#define I2C_SR2_DUALF_Msk                         (0x1UL << I2C_SR2_DUALF_Pos)                      /*!< 0x00000080 */
#define I2C_SR2_DUALF                             I2C_SR2_DUALF_Msk                                 /*!< desc DUALF */
#define I2C_SR2_PEC_Pos                           (8U)
#define I2C_SR2_PEC_Msk                           (0xFFUL << I2C_SR2_PEC_Pos)                       /*!< 0x0000FF00 */
#define I2C_SR2_PEC                               I2C_SR2_PEC_Msk                                   /*!< PEC[15:8] bits (desc PEC) */

/*!< I2C_CCR */
#define I2C_CCR_CCR_Pos                           (0U)
#define I2C_CCR_CCR_Msk                           (0xFFFUL << I2C_CCR_CCR_Pos)                      /*!< 0x00000FFF */
#define I2C_CCR_CCR                               I2C_CCR_CCR_Msk                                   /*!< CCR[11:0] bits (desc CCR) */
#define I2C_CCR_DUTY_Pos                          (14U)
#define I2C_CCR_DUTY_Msk                          (0x1UL << I2C_CCR_DUTY_Pos)                       /*!< 0x00004000 */
#define I2C_CCR_DUTY                              I2C_CCR_DUTY_Msk                                  /*!< desc DUTY */
#define I2C_CCR_FS_Pos                            (15U)
#define I2C_CCR_FS_Msk                            (0x1UL << I2C_CCR_FS_Pos)                         /*!< 0x00008000 */
#define I2C_CCR_FS                                I2C_CCR_FS_Msk                                    /*!< desc FS */

/*!< I2C_TRISE */
#define I2C_TRISE_TRISE_Pos                       (0U)
#define I2C_TRISE_TRISE_Msk                       (0x3FUL << I2C_TRISE_TRISE_Pos)                   /*!< 0x0000003F */
#define I2C_TRISE_TRISE                           I2C_TRISE_TRISE_Msk                               /*!< TRISE[5:0] bits (desc TRISE) */

/****************************************************************************/
/*                                                                          */
/*                        Independent WATCHDOG (IWDG)                       */
/*                                                                          */
/****************************************************************************/
/*******************  Bit definition for IWDG_KR register  ******************/
#define IWDG_KR_KEY_Pos      (0U)
#define IWDG_KR_KEY_Msk      (0xFFFFUL << IWDG_KR_KEY_Pos)                     /*!< 0x0000FFFF */
#define IWDG_KR_KEY          IWDG_KR_KEY_Msk                                   /*!<Key value (write only, read 0000h)  */

/*******************  Bit definition for IWDG_PR register  ******************/
#define IWDG_PR_PR_Pos       (0U)
#define IWDG_PR_PR_Msk       (0x7UL << IWDG_PR_PR_Pos)                         /*!< 0x00000007 */
#define IWDG_PR_PR           IWDG_PR_PR_Msk                                    /*!<PR[2:0] (Prescaler divider)         */
#define IWDG_PR_PR_0         (0x1UL << IWDG_PR_PR_Pos)                         /*!< 0x00000001 */
#define IWDG_PR_PR_1         (0x2UL << IWDG_PR_PR_Pos)                         /*!< 0x00000002 */
#define IWDG_PR_PR_2         (0x4UL << IWDG_PR_PR_Pos)                         /*!< 0x00000004 */

/*******************  Bit definition for IWDG_RLR register  *****************/
#define IWDG_RLR_RL_Pos      (0U)
#define IWDG_RLR_RL_Msk      (0xFFFUL << IWDG_RLR_RL_Pos)                      /*!< 0x00000FFF */
#define IWDG_RLR_RL          IWDG_RLR_RL_Msk                                   /*!<Watchdog counter reload value        */

/*******************  Bit definition for IWDG_SR register  ******************/
#define IWDG_SR_PVU_Pos      (0U)
#define IWDG_SR_PVU_Msk      (0x1UL << IWDG_SR_PVU_Pos)                        /*!< 0x00000001 */
#define IWDG_SR_PVU          IWDG_SR_PVU_Msk                                   /*!< Watchdog prescaler value update */
#define IWDG_SR_RVU_Pos      (1U)
#define IWDG_SR_RVU_Msk      (0x1UL << IWDG_SR_RVU_Pos)                        /*!< 0x00000002 */
#define IWDG_SR_RVU          IWDG_SR_RVU_Msk                                   /*!< Watchdog counter reload value update */

/****************************************************************************/
/*                                                                          */
/*                          OPA calculation unit (OPA)                      */
/*                                                                          */
/****************************************************************************/
/*******************  Bit definition for OPA_CR0 register  ******************/
#define OPA_CR0_OP1OEN1_Pos              (1U)
#define OPA_CR0_OP1OEN1_Msk              (0x1UL << OPA_CR0_OP1OEN1_Pos)
#define OPA_CR0_OP1OEN1                  OPA_CR0_OP1OEN1_Msk
#define OPA_CR0_OP2OEN1_Pos              (6U)
#define OPA_CR0_OP2OEN1_Msk              (0x1UL << OPA_CR0_OP2OEN1_Pos)
#define OPA_CR0_OP2OEN1                  OPA_CR0_OP2OEN1_Msk
#define OPA_CR0_OP3OEN1_Pos              (11U)
#define OPA_CR0_OP3OEN1_Msk              (0x1UL << OPA_CR0_OP3OEN1_Pos)
#define OPA_CR0_OP3OEN1                  OPA_CR0_OP3OEN1_Msk

/*******************  Bit definition for OPA_CR1 register  ******************/
#define OPA_CR1_EN1_Pos                (5U)
#define OPA_CR1_EN1_Msk                (0x1UL << OPA_CR1_EN1_Pos)
#define OPA_CR1_EN1                    OPA_CR1_EN1_Msk
#define OPA_CR1_EN2_Pos                (6U)
#define OPA_CR1_EN2_Msk                (0x1UL << OPA_CR1_EN2_Pos)
#define OPA_CR1_EN2                    OPA_CR1_EN2_Msk
#define OPA_CR1_EN3_Pos                (7U)
#define OPA_CR1_EN3_Msk                (0x1UL << OPA_CR1_EN3_Pos)
#define OPA_CR1_EN3                    OPA_CR1_EN3_Msk

/*******************  Bit definition for OPA_CR1 register  ******************/
#define OPA_CR2_OPA_TRIM1_Pos                (0U)
#define OPA_CR2_OPA_TRIM1_Msk                (0xFUL << OPA_CR2_OPA_TRIM1_Pos)
#define OPA_CR2_OPA_TRIM1                    OPA_CR2_OPA_TRIM1_Msk
#define OPA_CR2_OPA_TRIM2_Pos                (4U)
#define OPA_CR2_OPA_TRIM2_Msk                (0xFUL << OPA_CR2_OPA_TRIM2_Pos)
#define OPA_CR2_OPA_TRIM2                    OPA_CR2_OPA_TRIM2_Msk
#define OPA_CR2_OPA_TRIM3_Pos                (8U)
#define OPA_CR2_OPA_TRIM3_Msk                (0xFUL << OPA_CR2_OPA_TRIM3_Pos)
#define OPA_CR2_OPA_TRIM3                    OPA_CR2_OPA_TRIM3_Msk

/****************************************************************************/
/*                                                                          */
/*                          Power Control (PWR)                             */
/*                                                                          */
/****************************************************************************/
#define PWR_PVD_SUPPORT                       /*!< PWR feature available only on specific devices: Power Voltage Detection feature */
#define PWR_BOR_SUPPORT                       /*!< PWR feature available only on specific devices: Brown-Out Reset feature         */
#define PWR_SHDW_SUPPORT                      /*!< PWR feature available only on specific devices: Shutdown mode */

/*********************  Bits Define For Peripheral PWR  *********************/
/*!< PWR_CR1 */
#define PWR_CR1_BIAS_CR_Pos                (0U)
#define PWR_CR1_BIAS_CR_Msk                (0xFUL << PWR_CR1_BIAS_CR_Pos)                     /*!< 0x000000E0 */
#define PWR_CR1_BIAS_CR                    PWR_CR1_BIAS_CR_Msk                                /*!< PLS[7:5] bits (desc PLS) */
#define PWR_CR1_BIAS_CR_0                  (0x1UL << PWR_CR1_BIAS_CR_Pos)                     /*!< 0x00000020 */
#define PWR_CR1_BIAS_CR_1                  (0x2UL << PWR_CR1_BIAS_CR_Pos)                     /*!< 0x00000040 */
#define PWR_CR1_BIAS_CR_2                  (0x4UL << PWR_CR1_BIAS_CR_Pos)
#define PWR_CR1_BIAS_CR_SEL_Pos            (4U)
#define PWR_CR1_BIAS_CR_SEL_Msk            (0x1UL << PWR_CR1_BIAS_CR_SEL_Pos)                 /*!< 0x000000E0 */
#define PWR_CR1_BIAS_CR_SEL                PWR_CR1_BIAS_CR_SEL_Msk
#define PWR_CR1_DBP_Pos                    (8U)
#define PWR_CR1_DBP_Msk                    (0x1UL << PWR_CR1_DBP_Pos)                         /*!< 0x00000100 */
#define PWR_CR1_DBP                        PWR_CR1_DBP_Msk
#define PWR_CR1_VOS_Pos                    (9U)
#define PWR_CR1_VOS_Msk                    (0x3UL << PWR_CR1_VOS_Pos)                         /*!< 0x00006000 */
#define PWR_CR1_VOS                        PWR_CR1_VOS_Msk                                    /*!< VOS[14:13] bits (desc VOS) */
#define PWR_CR1_VOS_0                      (0x1UL << PWR_CR1_VOS_Pos)                         /*!< 0x00002000 */
#define PWR_CR1_VOS_1                      (0x2UL << PWR_CR1_VOS_Pos)
#define PWR_CR1_FLS_SLPTIME_Pos            (12U)
#define PWR_CR1_FLS_SLPTIME_Msk            (0x3UL << PWR_CR1_FLS_SLPTIME_Pos)                 /*!< 0x00006000 */
#define PWR_CR1_FLS_SLPTIME                PWR_CR1_FLS_SLPTIME_Msk                            /*!< VOS[14:13] bits (desc VOS) */
#define PWR_CR1_FLS_SLPTIME_0              (0x1UL << PWR_CR1_FLS_SLPTIME_Pos)                 /*!< 0x00002000 */
#define PWR_CR1_FLS_SLPTIME_1              (0x2UL << PWR_CR1_FLS_SLPTIME_Pos)
#define PWR_CR1_LPR_Pos                    (14U)
#define PWR_CR1_LPR_Msk                    (0x1UL << PWR_CR1_LPR_Pos)                         /*!< 0x00000100 */
#define PWR_CR1_LPR                        PWR_CR1_LPR_Msk
#define PWR_CR1_HSION_CTRL_Pos             (19U)
#define PWR_CR1_HSION_CTRL_Msk             (0x1UL << PWR_CR1_HSION_CTRL_Pos)                  /*!< 0x00000100 */
#define PWR_CR1_HSION_CTRL                 PWR_CR1_HSION_CTRL_Msk

/*!< PWR_CR2 */
#define PWR_CR2_PVDE_Pos                   (0U)
#define PWR_CR2_PVDE_Msk                   (0x1UL << PWR_CR2_PVDE_Pos)                        /*!< 0x00000100 */
#define PWR_CR2_PVDE                       PWR_CR2_PVDE_Msk
#define PWR_CR2_SRCSEL_Pos                 (2U)
#define PWR_CR2_SRCSEL_Msk                 (0x1UL << PWR_CR2_SRCSEL_Pos)                      /*!< 0x00000100 */
#define PWR_CR2_SRCSEL                     PWR_CR2_SRCSEL_Msk
#define PWR_CR2_PVDT_Pos                   (4U)
#define PWR_CR2_PVDT_Msk                   (0x7UL << PWR_CR2_PVDT_Pos)                        /*!< 0x000000E0 */
#define PWR_CR2_PVDT                       PWR_CR2_PVDT_Msk                                   /*!< PLS[7:5] bits (desc PLS) */
#define PWR_CR2_PVDT_0                     (0x1UL << PWR_CR2_PVDT_Pos)                        /*!< 0x00000020 */
#define PWR_CR2_PVDT_1                     (0x2UL << PWR_CR2_PVDT_Pos)                        /*!< 0x00000040 */
#define PWR_CR2_PVDT_2                     (0x4UL << PWR_CR2_PVDT_Pos)
#define PWR_CR2_FLTEN_Pos                  (8U)
#define PWR_CR2_FLTEN_Msk                  (0x1UL << PWR_CR2_FLTEN_Pos)                       /*!< 0x000000E0 */
#define PWR_CR2_FLTEN                      PWR_CR2_FLTEN_Msk
#define PWR_CR2_FLT_TIME_Pos               (9U)
#define PWR_CR2_FLT_TIME_Msk               (0x7UL << PWR_CR2_FLT_TIME_Pos)                    /*!< 0x000000E0 */
#define PWR_CR2_FLT_TIME                   PWR_CR2_FLT_TIME_Msk                               /*!< PLS[7:5] bits (desc PLS) */
#define PWR_CR2_FLT_TIME_0                 (0x1UL << PWR_CR2_FLT_TIME_Pos)                    /*!< 0x00000020 */
#define PWR_CR2_FLT_TIME_1                 (0x2UL << PWR_CR2_FLT_TIME_Pos)                    /*!< 0x00000040 */
#define PWR_CR2_FLT_TIME_2                 (0x4UL << PWR_CR2_FLT_TIME_Pos)

/*!< PWR_SR */
#define PWR_SR_PVDO_Pos                    (11U)
#define PWR_SR_PVDO_Msk                    (0x1UL << PWR_SR_PVDO_Pos)                         /*!< 0x00004000 */
#define PWR_SR_PVDO                        PWR_SR_PVDO_Msk

/****************************************************************************/
/*                                                                          */
/*                           Reset and Clock Control (RCC)                  */
/*                                                                          */
/****************************************************************************/
/*
* @brief Specific device feature definitions
*/
#define RCC_LSE_SUPPORT
#define RCC_PLL_SUPPORT

/********************  Bit definition for RCC_CR register  ******************/
#define RCC_CR_HSION_Pos                 (8U)
#define RCC_CR_HSION_Msk                 (0x1UL << RCC_CR_HSION_Pos)           /*!< 0x00000100 */
#define RCC_CR_HSION                     RCC_CR_HSION_Msk                      /*!< Internal High Speed clock enable */
#define RCC_CR_HSIRDY_Pos                (10U)
#define RCC_CR_HSIRDY_Msk                (0x1UL << RCC_CR_HSIRDY_Pos)          /*!< 0x00000400 */
#define RCC_CR_HSIRDY                    RCC_CR_HSIRDY_Msk                     /*!< Internal High Speed clock ready flag */
#define RCC_CR_HSIDIV_Pos                (11U)
#define RCC_CR_HSIDIV_Msk                (0x7UL << RCC_CR_HSIDIV_Pos)          /*!< 0x00003800 */
#define RCC_CR_HSIDIV                    RCC_CR_HSIDIV_Msk                     /*!< HSIDIV[13:11] Internal High Speed clock division factor */
#define RCC_CR_HSIDIV_0                  (0x1UL << RCC_CR_HSIDIV_Pos)          /*!< 0x00000800 */
#define RCC_CR_HSIDIV_1                  (0x2UL << RCC_CR_HSIDIV_Pos)          /*!< 0x00001000 */
#define RCC_CR_HSIDIV_2                  (0x4UL << RCC_CR_HSIDIV_Pos)          /*!< 0x00002000 */
#define RCC_CR_HSEON_Pos                 (16U)
#define RCC_CR_HSEON_Msk                 (0x1UL << RCC_CR_HSEON_Pos)           /*!< 0x00010000 */
#define RCC_CR_HSEON                     RCC_CR_HSEON_Msk                      /*!< External High Speed clock enable */
#define RCC_CR_HSERDY_Pos                (17U)
#define RCC_CR_HSERDY_Msk                (0x1UL << RCC_CR_HSERDY_Pos)          /*!< 0x00020000 */
#define RCC_CR_HSERDY                    RCC_CR_HSERDY_Msk                     /*!< External High Speed clock ready */
#define RCC_CR_HSEBYP_Pos                (18U)
#define RCC_CR_HSEBYP_Msk                (0x1UL << RCC_CR_HSEBYP_Pos)          /*!< 0x00040000 */
#define RCC_CR_HSEBYP                    RCC_CR_HSEBYP_Msk                     /*!< External High Speed clock Bypass */
#define RCC_CR_CSSON_Pos                 (19U)
#define RCC_CR_CSSON_Msk                 (0x1UL << RCC_CR_CSSON_Pos)           /*!< 0x00080000 */
#define RCC_CR_CSSON                     RCC_CR_CSSON_Msk                      /*!< HSE Clock Security System enable */
#define RCC_CR_ADC_DIV_Pos               (21U)
#define RCC_CR_ADC_DIV_Msk               (0x3UL << RCC_CR_ADC_DIV_Pos)         /*!< 0x00600000 */
#define RCC_CR_ADC_DIV                   RCC_CR_ADC_DIV_Msk                    /*!< ADC Frequency Division Factor */
#define RCC_CR_ADC_DIV_0                 (0x1UL << RCC_CR_ADC_DIV_Pos)
#define RCC_CR_ADC_DIV_1                 (0x2UL << RCC_CR_ADC_DIV_Pos)
#define RCC_CR_PLLON_Pos                 (24U)
#define RCC_CR_PLLON_Msk                 (0x1UL << RCC_CR_PLLON_Pos)           /*!< 0x01000000 */
#define RCC_CR_PLLON                     RCC_CR_PLLON_Msk                      /*!< System PLL clock enable */
#define RCC_CR_PLLRDY_Pos                (25U)
#define RCC_CR_PLLRDY_Msk                (0x1UL << RCC_CR_PLLRDY_Pos)          /*!< 0x02000000 */
#define RCC_CR_PLLRDY                    RCC_CR_PLLRDY_Msk                     /*!< System PLL clock ready */

/********************  Bit definition for RCC_ICSCR register  ***************/
#define RCC_ICSCR_HSI_TRIM_Pos            (0U)
#define RCC_ICSCR_HSI_TRIM_Msk            (0x1FFFUL << RCC_ICSCR_HSI_TRIM_Pos)     /*!< 0x00007F00 */
#define RCC_ICSCR_HSI_TRIM                RCC_ICSCR_HSI_TRIM_Msk                   /*!< HSITRIM[14:8] bits */
#define RCC_ICSCR_HSI_TRIM_0              (0x01UL << RCC_ICSCR_HSI_TRIM_Pos)       /*!< 0x00000001 */
#define RCC_ICSCR_HSI_TRIM_1              (0x02UL << RCC_ICSCR_HSI_TRIM_Pos)       /*!< 0x00000002 */
#define RCC_ICSCR_HSI_TRIM_2              (0x04UL << RCC_ICSCR_HSI_TRIM_Pos)       /*!< 0x00000004 */
#define RCC_ICSCR_HSI_TRIM_3              (0x08UL << RCC_ICSCR_HSI_TRIM_Pos)       /*!< 0x00000008 */
#define RCC_ICSCR_HSI_TRIM_4              (0x10UL << RCC_ICSCR_HSI_TRIM_Pos)       /*!< 0x00000010 */
#define RCC_ICSCR_HSI_TRIM_5              (0x20UL << RCC_ICSCR_HSI_TRIM_Pos)       /*!< 0x00000020 */
#define RCC_ICSCR_HSI_TRIM_6              (0x40UL << RCC_ICSCR_HSI_TRIM_Pos)       /*!< 0x00000040 */
#define RCC_ICSCR_HSI_TRIM_7              (0x80UL << RCC_ICSCR_HSI_TRIM_Pos)       /*!< 0x00000080 */
#define RCC_ICSCR_HSI_TRIM_8              (0x100UL << RCC_ICSCR_HSI_TRIM_Pos)      /*!< 0x00000100 */
#define RCC_ICSCR_HSI_TRIM_9              (0x200UL << RCC_ICSCR_HSI_TRIM_Pos)      /*!< 0x00000200 */
#define RCC_ICSCR_HSI_TRIM_10             (0x400UL << RCC_ICSCR_HSI_TRIM_Pos)      /*!< 0x00000400 */
#define RCC_ICSCR_HSI_TRIM_11             (0x800UL << RCC_ICSCR_HSI_TRIM_Pos)      /*!< 0x00000800 */
#define RCC_ICSCR_HSI_TRIM_12             (0x1000UL << RCC_ICSCR_HSI_TRIM_Pos)     /*!< 0x00001000 */
#define RCC_ICSCR_HSI_FS_Pos              (13U)
#define RCC_ICSCR_HSI_FS_Msk              (0x7UL << RCC_ICSCR_HSI_FS_Pos)          /*!< 0x0000E000 */
#define RCC_ICSCR_HSI_FS                  RCC_ICSCR_HSI_FS_Msk                     /*!< HSIFS[15:13] bits */
#define RCC_ICSCR_HSI_FS_0                (0x01UL << RCC_ICSCR_HSI_FS_Pos)         /*!< 0x00002000 */
#define RCC_ICSCR_HSI_FS_1                (0x02UL << RCC_ICSCR_HSI_FS_Pos)         /*!< 0x00004000 */
#define RCC_ICSCR_HSI_FS_2                (0x04UL << RCC_ICSCR_HSI_FS_Pos)         /*!< 0x00008000 */
#define RCC_ICSCR_LSI_TRIM_Pos            (16U)
#define RCC_ICSCR_LSI_TRIM_Msk            (0x1FFUL << RCC_ICSCR_LSI_TRIM_Pos)
#define RCC_ICSCR_LSI_TRIM                RCC_ICSCR_LSI_TRIM_Msk
#define RCC_ICSCR_LSI_TRIM_0              (0x01UL << RCC_ICSCR_LSI_TRIM_Pos)
#define RCC_ICSCR_LSI_TRIM_1              (0x02UL << RCC_ICSCR_LSI_TRIM_Pos)
#define RCC_ICSCR_LSI_TRIM_2              (0x04UL << RCC_ICSCR_LSI_TRIM_Pos)
#define RCC_ICSCR_LSI_TRIM_3              (0x08UL << RCC_ICSCR_LSI_TRIM_Pos)
#define RCC_ICSCR_LSI_TRIM_4              (0x10UL << RCC_ICSCR_LSI_TRIM_Pos)
#define RCC_ICSCR_LSI_TRIM_5              (0x20UL << RCC_ICSCR_LSI_TRIM_Pos)
#define RCC_ICSCR_LSI_TRIM_6              (0x40UL << RCC_ICSCR_LSI_TRIM_Pos)
#define RCC_ICSCR_LSI_TRIM_7              (0x80UL << RCC_ICSCR_LSI_TRIM_Pos)
#define RCC_ICSCR_LSI_TRIM_8              (0x100UL << RCC_ICSCR_LSI_TRIM_Pos)

/********************  Bit definition for RCC_CFGR register  ****************/
/*!< SW configuration */
#define RCC_CFGR_SW_Pos                (0U)
#define RCC_CFGR_SW_Msk                (0x7UL << RCC_CFGR_SW_Pos)              /*!< 0x00000007 */
#define RCC_CFGR_SW                    RCC_CFGR_SW_Msk                         /*!< SW[2:0] bits (System clock Switch) */
#define RCC_CFGR_SW_0                  (0x1UL << RCC_CFGR_SW_Pos)              /*!< 0x00000001 */
#define RCC_CFGR_SW_1                  (0x2UL << RCC_CFGR_SW_Pos)              /*!< 0x00000002 */
#define RCC_CFGR_SW_2                  (0x4UL << RCC_CFGR_SW_Pos)              /*!< 0x00000004 */

/*!< SWS configuration */
#define RCC_CFGR_SWS_Pos               (3U)
#define RCC_CFGR_SWS_Msk               (0x7UL << RCC_CFGR_SWS_Pos)             /*!< 0x00000038 */
#define RCC_CFGR_SWS                   RCC_CFGR_SWS_Msk                        /*!< SWS[2:0] bits (System Clock Switch Status) */
#define RCC_CFGR_SWS_0                 (0x1UL << RCC_CFGR_SWS_Pos)             /*!< 0x00000008 */
#define RCC_CFGR_SWS_1                 (0x2UL << RCC_CFGR_SWS_Pos)             /*!< 0x00000010 */
#define RCC_CFGR_SWS_2                 (0x4UL << RCC_CFGR_SWS_Pos)             /*!< 0x00000020 */
#define RCC_CFGR_SWS_HSI               (0UL)                                   /*!< HSI used as system clock */
#define RCC_CFGR_SWS_HSE               (0x00000008UL)                          /*!< HSE used as system clock */
#define RCC_CFGR_SWS_PLL               (0x00000010UL)                          /*!< PLL used as system clock */
#define RCC_CFGR_SWS_LSI               (0x00000018UL)                          /*!< LSI used as system clock */
#define RCC_CFGR_SWS_LSE               (0x00000020UL)                          /*!< LSE used as system clock */

/*!< HPRE configuration */
#define RCC_CFGR_HPRE_Pos              (8U)
#define RCC_CFGR_HPRE_Msk              (0xFUL << RCC_CFGR_HPRE_Pos)            /*!< 0x00000F00 */
#define RCC_CFGR_HPRE                  RCC_CFGR_HPRE_Msk                       /*!< HPRE[3:0] bits (AHB prescaler) */
#define RCC_CFGR_HPRE_0                (0x1UL << RCC_CFGR_HPRE_Pos)            /*!< 0x00000100 */
#define RCC_CFGR_HPRE_1                (0x2UL << RCC_CFGR_HPRE_Pos)            /*!< 0x00000200 */
#define RCC_CFGR_HPRE_2                (0x4UL << RCC_CFGR_HPRE_Pos)            /*!< 0x00000400 */
#define RCC_CFGR_HPRE_3                (0x8UL << RCC_CFGR_HPRE_Pos)            /*!< 0x00000800 */

/*!< PPRE configuration */
#define RCC_CFGR_PPRE_Pos              (12U)
#define RCC_CFGR_PPRE_Msk              (0x7UL << RCC_CFGR_PPRE_Pos)            /*!< 0x00007000 */
#define RCC_CFGR_PPRE                  RCC_CFGR_PPRE_Msk                       /*!< PRE1[2:0] bits (APB prescaler) */
#define RCC_CFGR_PPRE_0                (0x1UL << RCC_CFGR_PPRE_Pos)            /*!< 0x00001000 */
#define RCC_CFGR_PPRE_1                (0x2UL << RCC_CFGR_PPRE_Pos)            /*!< 0x00002000 */
#define RCC_CFGR_PPRE_2                (0x4UL << RCC_CFGR_PPRE_Pos)            /*!< 0x00004000 */

/*!< MCOSEL configuration */
#define RCC_CFGR_MCOSEL_Pos            (24U)
#define RCC_CFGR_MCOSEL_Msk            (0xFUL << RCC_CFGR_MCOSEL_Pos)          /*!< 0x0F000000 */
#define RCC_CFGR_MCOSEL                RCC_CFGR_MCOSEL_Msk                     /*!< MCOSEL [3:0] bits (Clock output selection) */
#define RCC_CFGR_MCOSEL_0              (0x1UL << RCC_CFGR_MCOSEL_Pos)          /*!< 0x01000000 */
#define RCC_CFGR_MCOSEL_1              (0x2UL << RCC_CFGR_MCOSEL_Pos)          /*!< 0x02000000 */
#define RCC_CFGR_MCOSEL_2              (0x4UL << RCC_CFGR_MCOSEL_Pos)          /*!< 0x04000000 */
#define RCC_CFGR_MCOSEL_3              (0x8UL << RCC_CFGR_MCOSEL_Pos)          /*!< 0x08000000 */

/*!< MCO Prescaler configuration */
#define RCC_CFGR_MCOPRE_Pos            (28U)
#define RCC_CFGR_MCOPRE_Msk            (0x7UL << RCC_CFGR_MCOPRE_Pos)          /*!< 0x70000000 */
#define RCC_CFGR_MCOPRE                RCC_CFGR_MCOPRE_Msk                     /*!< MCO prescaler [2:0] */
#define RCC_CFGR_MCOPRE_0              (0x1UL << RCC_CFGR_MCOPRE_Pos)          /*!< 0x10000000 */
#define RCC_CFGR_MCOPRE_1              (0x2UL << RCC_CFGR_MCOPRE_Pos)          /*!< 0x20000000 */
#define RCC_CFGR_MCOPRE_2              (0x4UL << RCC_CFGR_MCOPRE_Pos)          /*!< 0x40000000 */

/********************  Bit definition for RCC_PLLCFGR register  **************/
#define RCC_PLLCFGR_PLLSRC_Pos           (0U)
#define RCC_PLLCFGR_PLLSRC_Msk           (0x3UL << RCC_PLLCFGR_PLLSRC_Pos)     /*!< 0x00000003 */
#define RCC_PLLCFGR_PLLSRC               RCC_PLLCFGR_PLLSRC_Msk
#define RCC_PLLCFGR_PLLSRC_0             (0x1UL << RCC_PLLCFGR_PLLSRC_Pos)     /*!< 0x00000001 */
#define RCC_PLLCFGR_PLLSRC_1             (0x2UL << RCC_PLLCFGR_PLLSRC_Pos)     /*!< 0x00000002 */
#define RCC_PLLCFGR_PLLSRC_NONE          (0x00000000UL)                        /*!< No clock sent to PLL      */
#define RCC_PLLCFGR_PLLSRC_HSI_Pos       (0U)
#define RCC_PLLCFGR_PLLSRC_HSI_Msk       (0x2UL << RCC_PLLCFGR_PLLSRC_HSI_Pos) /*!< 0x00000002 */
#define RCC_PLLCFGR_PLLSRC_HSI           RCC_PLLCFGR_PLLSRC_HSI_Msk            /*!< HSI source clock selected */
#define RCC_PLLCFGR_PLLSRC_HSE_Pos       (0U)
#define RCC_PLLCFGR_PLLSRC_HSE_Msk       (0x3UL << RCC_PLLCFGR_PLLSRC_HSE_Pos) /*!< 0x00000003 */
#define RCC_PLLCFGR_PLLSRC_HSE           RCC_PLLCFGR_PLLSRC_HSE_Msk            /*!< HSE source clock selected */
#define RCC_PLLCFGR_PLLMUL_Pos           (2U)
#define RCC_PLLCFGR_PLLMUL_Msk           (0x3UL << RCC_PLLCFGR_PLLMUL_Pos)     /*!< 0x0000000C */
#define RCC_PLLCFGR_PLLMUL               RCC_PLLCFGR_PLLMUL_Msk
#define RCC_PLLCFGR_PLLMUL_0             (0x1UL << RCC_PLLCFGR_PLLMUL_Pos)     /*!< 0x00000004 */
#define RCC_PLLCFGR_PLLMUL_1             (0x2UL << RCC_PLLCFGR_PLLMUL_Pos)     /*!< 0x00000008 */

/********************  Bit definition for RCC_ECSCR register  ****************/
#define RCC_ECSCR_HSE_DRV_Pos            (0U)
#define RCC_ECSCR_HSE_DRV_Msk            (3UL << RCC_ECSCR_HSE_DRV_Pos)        /*!< 0x00000003 */
#define RCC_ECSCR_HSE_DRV                RCC_ECSCR_HSE_DRV_Msk
#define RCC_ECSCR_HSE_DRV_0              (0x1UL <<RCC_ECSCR_HSE_DRV_Pos)       /*!< 0x00000001 */
#define RCC_ECSCR_HSE_DRV_1              (0x2UL <<RCC_ECSCR_HSE_DRV_Pos)       /*!< 0x00000002 */
#define RCC_ECSCR_HSE_STARTUP_Pos       (3U)
#define RCC_ECSCR_HSE_STARTUP_Msk       (3UL << RCC_ECSCR_HSE_STARTUP_Pos)     /*!< 0x00000018 */
#define RCC_ECSCR_HSE_STARTUP           RCC_ECSCR_HSE_STARTUP_Msk
#define RCC_ECSCR_HSE_STARTUP_0         (0x1UL <<RCC_ECSCR_HSE_STARTUP_Pos)    /*!< 0x00000008 */
#define RCC_ECSCR_HSE_STARTUP_1         (0x2UL <<RCC_ECSCR_HSE_STARTUP_Pos)    /*!< 0x00000010 */
#define RCC_ECSCR_LSE_DRIVER_Pos        (16U)
#define RCC_ECSCR_LSE_DRIVER_Msk        (0x3UL << RCC_ECSCR_LSE_DRIVER_Pos)    /*!< 0x00030000 */
#define RCC_ECSCR_LSE_DRIVER            RCC_ECSCR_LSE_DRIVER_Msk
#define RCC_ECSCR_LSE_DRIVER_0          (0x1UL << RCC_ECSCR_LSE_DRIVER_Pos)    /*!< 0x00010000 */
#define RCC_ECSCR_LSE_DRIVER_1          (0x2UL << RCC_ECSCR_LSE_DRIVER_Pos)    /*!< 0x00020000 */
#define RCC_ECSCR_LSE_STARTUP_Pos       (20U)
#define RCC_ECSCR_LSE_STARTUP_Msk       (3UL << RCC_ECSCR_LSE_STARTUP_Pos)     /*!< 0x00300000 */
#define RCC_ECSCR_LSE_STARTUP           RCC_ECSCR_LSE_STARTUP_Msk
#define RCC_ECSCR_LSE_STARTUP_0         (0x1UL <<RCC_ECSCR_LSE_STARTUP_Pos)    /*!< 0x00100000 */
#define RCC_ECSCR_LSE_STARTUP_1         (0x2UL <<RCC_ECSCR_LSE_STARTUP_Pos)    /*!< 0x00200000 */

/********************  Bit definition for RCC_CIER register  ******************/
#define RCC_CIER_LSIRDYIE_Pos            (0U)
#define RCC_CIER_LSIRDYIE_Msk            (0x1UL << RCC_CIER_LSIRDYIE_Pos)      /*!< 0x00000001 */
#define RCC_CIER_LSIRDYIE                RCC_CIER_LSIRDYIE_Msk
#define RCC_CIER_LSERDYIE_Pos            (1U)
#define RCC_CIER_LSERDYIE_Msk            (0x1UL << RCC_CIER_LSERDYIE_Pos)      /*!< 0x00000002 */
#define RCC_CIER_LSERDYIE                RCC_CIER_LSERDYIE_Msk
#define RCC_CIER_HSIRDYIE_Pos            (3U)
#define RCC_CIER_HSIRDYIE_Msk            (0x1UL << RCC_CIER_HSIRDYIE_Pos)      /*!< 0x00000008 */
#define RCC_CIER_HSIRDYIE                RCC_CIER_HSIRDYIE_Msk
#define RCC_CIER_HSERDYIE_Pos            (4U)
#define RCC_CIER_HSERDYIE_Msk            (0x1UL << RCC_CIER_HSERDYIE_Pos)      /*!< 0x00000010 */
#define RCC_CIER_HSERDYIE                RCC_CIER_HSERDYIE_Msk
#define RCC_CIER_PLLRDYIE_Pos            (5U)
#define RCC_CIER_PLLRDYIE_Msk            (0x1UL << RCC_CIER_PLLRDYIE_Pos)      /*!< 0x00000020 */
#define RCC_CIER_PLLRDYIE                RCC_CIER_PLLRDYIE_Msk

/********************  Bit definition for RCC_CIFR register  ******************/
#define RCC_CIFR_LSIRDYF_Pos             (0U)
#define RCC_CIFR_LSIRDYF_Msk             (0x1UL << RCC_CIFR_LSIRDYF_Pos)       /*!< 0x00000001 */
#define RCC_CIFR_LSIRDYF                 RCC_CIFR_LSIRDYF_Msk
#define RCC_CIFR_LSERDYF_Pos             (1U)
#define RCC_CIFR_LSERDYF_Msk             (0x1UL << RCC_CIFR_LSERDYF_Pos)       /*!< 0x00000002 */
#define RCC_CIFR_LSERDYF                 RCC_CIFR_LSERDYF_Msk
#define RCC_CIFR_HSIRDYF_Pos             (3U)
#define RCC_CIFR_HSIRDYF_Msk             (0x1UL << RCC_CIFR_HSIRDYF_Pos)       /*!< 0x00000008 */
#define RCC_CIFR_HSIRDYF                 RCC_CIFR_HSIRDYF_Msk
#define RCC_CIFR_HSERDYF_Pos             (4U)
#define RCC_CIFR_HSERDYF_Msk             (0x1UL << RCC_CIFR_HSERDYF_Pos)       /*!< 0x00000010 */
#define RCC_CIFR_HSERDYF                 RCC_CIFR_HSERDYF_Msk
#define RCC_CIFR_PLLRDYF_Pos             (5U)
#define RCC_CIFR_PLLRDYF_Msk             (0x1UL << RCC_CIFR_PLLRDYF_Pos)       /*!< 0x00000020 */
#define RCC_CIFR_PLLRDYF                 RCC_CIFR_PLLRDYF_Msk
#define RCC_CIFR_CSSF_Pos                (8U)
#define RCC_CIFR_CSSF_Msk                (0x1UL << RCC_CIFR_CSSF_Pos)          /*!< 0x00000100 */
#define RCC_CIFR_CSSF                    RCC_CIFR_CSSF_Msk
#define RCC_CIFR_LSECSSF_Pos             (9U)
#define RCC_CIFR_LSECSSF_Msk             (0x1UL << RCC_CIFR_LSECSSF_Pos)       /*!< 0x00000200 */
#define RCC_CIFR_LSECSSF                 RCC_CIFR_LSECSSF_Msk

/********************  Bit definition for RCC_CICR register  ******************/
#define RCC_CICR_LSIRDYC_Pos             (0U)
#define RCC_CICR_LSIRDYC_Msk             (0x1UL << RCC_CICR_LSIRDYC_Pos)       /*!< 0x00000001 */
#define RCC_CICR_LSIRDYC                 RCC_CICR_LSIRDYC_Msk
#define RCC_CICR_LSERDYC_Pos             (1U)
#define RCC_CICR_LSERDYC_Msk             (0x1UL << RCC_CICR_LSERDYC_Pos)       /*!< 0x00000002 */
#define RCC_CICR_LSERDYC                 RCC_CICR_LSERDYC_Msk
#define RCC_CICR_HSIRDYC_Pos             (3U)
#define RCC_CICR_HSIRDYC_Msk             (0x1UL << RCC_CICR_HSIRDYC_Pos)       /*!< 0x00000008 */
#define RCC_CICR_HSIRDYC                 RCC_CICR_HSIRDYC_Msk
#define RCC_CICR_HSERDYC_Pos             (4U)
#define RCC_CICR_HSERDYC_Msk             (0x1UL << RCC_CICR_HSERDYC_Pos)       /*!< 0x00000010 */
#define RCC_CICR_HSERDYC                 RCC_CICR_HSERDYC_Msk
#define RCC_CICR_PLLRDYC_Pos             (5U)
#define RCC_CICR_PLLRDYC_Msk             (0x1UL << RCC_CICR_PLLRDYC_Pos)       /*!< 0x00000020 */
#define RCC_CICR_PLLRDYC                 RCC_CICR_PLLRDYC_Msk
#define RCC_CICR_CSSC_Pos                (8U)
#define RCC_CICR_CSSC_Msk                (0x1UL << RCC_CICR_CSSC_Pos)          /*!< 0x00000100 */
#define RCC_CICR_CSSC                    RCC_CICR_CSSC_Msk
#define RCC_CICR_LSECSSC_Pos             (9U)
#define RCC_CICR_LSECSSC_Msk             (0x1UL << RCC_CICR_LSECSSC_Pos)       /*!< 0x00000200 */
#define RCC_CICR_LSECSSC                 RCC_CICR_LSECSSC_Msk

/********************  Bit definition for RCC_IOPRSTR register  ***************/
#define RCC_IOPRSTR_GPIOARST_Pos         (0U)
#define RCC_IOPRSTR_GPIOARST_Msk         (0x1UL << RCC_IOPRSTR_GPIOARST_Pos)   /*!< 0x00000001 */
#define RCC_IOPRSTR_GPIOARST             RCC_IOPRSTR_GPIOARST_Msk
#define RCC_IOPRSTR_GPIOBRST_Pos         (1U)
#define RCC_IOPRSTR_GPIOBRST_Msk         (0x1UL << RCC_IOPRSTR_GPIOBRST_Pos)   /*!< 0x00000002 */
#define RCC_IOPRSTR_GPIOBRST             RCC_IOPRSTR_GPIOBRST_Msk
#define RCC_IOPRSTR_GPIOCRST_Pos         (2U)
#define RCC_IOPRSTR_GPIOCRST_Msk         (0x1UL << RCC_IOPRSTR_GPIOCRST_Pos)   /*!< 0x00000004 */
#define RCC_IOPRSTR_GPIOCRST             RCC_IOPRSTR_GPIOCRST_Msk
#define RCC_IOPRSTR_GPIOFRST_Pos         (5U)
#define RCC_IOPRSTR_GPIOFRST_Msk         (0x1UL << RCC_IOPRSTR_GPIOFRST_Pos)   /*!< 0x00000020 */
#define RCC_IOPRSTR_GPIOFRST             RCC_IOPRSTR_GPIOFRST_Msk

/********************  Bit definition for RCC_AHBRSTR register  ***************/
#define RCC_AHBRSTR_DMARST_Pos           (0U)
#define RCC_AHBRSTR_DMARST_Msk           (0x1UL << RCC_AHBRSTR_DMARST_Pos)      /*!< 0x00000001 */
#define RCC_AHBRSTR_DMARST               RCC_AHBRSTR_DMARST_Msk
#define RCC_AHBRSTR_CRCRST_Pos           (12U)
#define RCC_AHBRSTR_CRCRST_Msk           (0x1UL << RCC_AHBRSTR_CRCRST_Pos)     /*!< 0x00001000 */
#define RCC_AHBRSTR_CRCRST               RCC_AHBRSTR_CRCRST_Msk
#define RCC_AHBRSTR_DIVRST_Pos           (24U)
#define RCC_AHBRSTR_DIVRST_Msk           (0x1UL << RCC_AHBRSTR_DIVRST_Pos)     /*!< 0x01000000 */
#define RCC_AHBRSTR_DIVRST               RCC_AHBRSTR_DIVRST_Msk

/********************  Bit definition for RCC_APBRSTR1 register  **************/
#define RCC_APBRSTR1_TIM2RST_Pos         (0U)
#define RCC_APBRSTR1_TIM2RST_Msk         (0x1UL << RCC_APBRSTR1_TIM2RST_Pos)   /*!< 0x00000001 */
#define RCC_APBRSTR1_TIM2RST             RCC_APBRSTR1_TIM2RST_Msk
#define RCC_APBRSTR1_TIM3RST_Pos         (1U)
#define RCC_APBRSTR1_TIM3RST_Msk         (0x1UL << RCC_APBRSTR1_TIM3RST_Pos)   /*!< 0x00000002 */
#define RCC_APBRSTR1_TIM3RST             RCC_APBRSTR1_TIM3RST_Msk
#define RCC_APBRSTR1_TIM6RST_Pos         (4U)
#define RCC_APBRSTR1_TIM6RST_Msk         (0x1UL << RCC_APBRSTR1_TIM6RST_Pos)   /*!< 0x00000010 */
#define RCC_APBRSTR1_TIM6RST             RCC_APBRSTR1_TIM6RST_Msk
#define RCC_APBRSTR1_TIM7RST_Pos         (5U)
#define RCC_APBRSTR1_TIM7RST_Msk         (0x1UL << RCC_APBRSTR1_TIM7RST_Pos)   /*!< 0x00000020 */
#define RCC_APBRSTR1_TIM7RST             RCC_APBRSTR1_TIM7RST_Msk
#define RCC_APBRSTR1_RTCAPBRST_Pos       (10U)
#define RCC_APBRSTR1_RTCAPBRST_Msk       (0x1UL << RCC_APBRSTR1_RTCAPBRST_Pos) /*!< 0x00000400 */
#define RCC_APBRSTR1_RTCAPBRST           RCC_APBRSTR1_RTCAPBRST_Msk
#define RCC_APBRSTR1_WWDGRST_Pos         (11U)
#define RCC_APBRSTR1_WWDGRST_Msk         (0x1UL << RCC_APBRSTR1_WWDGRST_Pos)   /*!< 0x00000800 */
#define RCC_APBRSTR1_WWDGRST             RCC_APBRSTR1_WWDGRST_Msk
#define RCC_APBRSTR1_SPI2RST_Pos         (14U)
#define RCC_APBRSTR1_SPI2RST_Msk         (0x1UL << RCC_APBRSTR1_SPI2RST_Pos)   /*!< 0x00004000 */
#define RCC_APBRSTR1_SPI2RST             RCC_APBRSTR1_SPI2RST_Msk
#define RCC_APBRSTR1_USART2RST_Pos       (17U)
#define RCC_APBRSTR1_USART2RST_Msk       (0x1UL << RCC_APBRSTR1_USART2RST_Pos) /*!< 0x00020000 */
#define RCC_APBRSTR1_USART2RST           RCC_APBRSTR1_USART2RST_Msk
#define RCC_APBRSTR1_USART3RST_Pos       (18U)
#define RCC_APBRSTR1_USART3RST_Msk       (0x1UL << RCC_APBRSTR1_USART3RST_Pos) /*!< 0x00040000 */
#define RCC_APBRSTR1_USART3RST           RCC_APBRSTR1_USART3RST_Msk
#define RCC_APBRSTR1_USART4RST_Pos       (19U)
#define RCC_APBRSTR1_USART4RST_Msk       (0x1UL << RCC_APBRSTR1_USART4RST_Pos) /*!< 0x00080000 */
#define RCC_APBRSTR1_USART4RST           RCC_APBRSTR1_USART4RST_Msk
#define RCC_APBRSTR1_I2C1RST_Pos         (21U)
#define RCC_APBRSTR1_I2C1RST_Msk         (0x1UL << RCC_APBRSTR1_I2C1RST_Pos)   /*!< 0x00200000 */
#define RCC_APBRSTR1_I2C1RST             RCC_APBRSTR1_I2C1RST_Msk
#define RCC_APBRSTR1_I2C2RST_Pos         (22U)
#define RCC_APBRSTR1_I2C2RST_Msk         (0x1UL << RCC_APBRSTR1_I2C2RST_Pos)   /*!< 0x00400000 */
#define RCC_APBRSTR1_I2C2RST             RCC_APBRSTR1_I2C2RST_Msk
#define RCC_APBRSTR1_USBDRST_Pos         (23U)
#define RCC_APBRSTR1_USBDRST_Msk         (0x1UL << RCC_APBRSTR1_USBDRST_Pos)    /*!< 0x00800000 */
#define RCC_APBRSTR1_USBDRST             RCC_APBRSTR1_USBDRST_Msk
#define RCC_APBRSTR1_CANRST_Pos          (25U)
#define RCC_APBRSTR1_CANRST_Msk          (0x1UL << RCC_APBRSTR1_CANRST_Pos)    /*!< 0x02000000 */
#define RCC_APBRSTR1_CANRST              RCC_APBRSTR1_CANRST_Msk
#define RCC_APBRSTR1_CAN1RST             RCC_APBRSTR1_CANRST
#define RCC_APBRSTR1_CTCRST_Pos          (27U)
#define RCC_APBRSTR1_CTCRST_Msk          (0x1UL << RCC_APBRSTR1_CTCRST_Pos)    /*!< 0x04000000 */
#define RCC_APBRSTR1_CTCRST              RCC_APBRSTR1_CTCRST_Msk
#define RCC_APBRSTR1_PWRRST_Pos          (28U)
#define RCC_APBRSTR1_PWRRST_Msk          (0x1UL << RCC_APBRSTR1_PWRRST_Pos)    /*!< 0x10000000 */
#define RCC_APBRSTR1_PWRRST              RCC_APBRSTR1_PWRRST_Msk
#define RCC_APBRSTR1_DACRST_Pos          (29U)
#define RCC_APBRSTR1_DACRST_Msk          (0x1UL << RCC_APBRSTR1_DACRST_Pos)    /*!< 0x20000000 */
#define RCC_APBRSTR1_DACRST              RCC_APBRSTR1_DACRST_Msk
#define RCC_APBRSTR1_OPARST_Pos          (30U)
#define RCC_APBRSTR1_OPARST_Msk          (0x1UL << RCC_APBRSTR1_OPARST_Pos)    /*!< 0x40000000 */
#define RCC_APBRSTR1_OPARST              RCC_APBRSTR1_OPARST_Msk
#define RCC_APBRSTR1_LPTIMRST_Pos        (31U)
#define RCC_APBRSTR1_LPTIMRST_Msk        (0x1UL << RCC_APBRSTR1_LPTIMRST_Pos)  /*!< 0x80000000 */
#define RCC_APBRSTR1_LPTIMRST            RCC_APBRSTR1_LPTIMRST_Msk
#define RCC_APBRSTR1_LPTIM1RST           RCC_APBRSTR1_LPTIMRST_Msk

/********************  Bit definition for RCC_APBRSTR2 register  **************/
#define RCC_APBRSTR2_SYSCFGRST_Pos       (0U)
#define RCC_APBRSTR2_SYSCFGRST_Msk       (0x1UL << RCC_APBRSTR2_SYSCFGRST_Pos)  /*!< 0x00000001 */
#define RCC_APBRSTR2_SYSCFGRST           RCC_APBRSTR2_SYSCFGRST_Msk
#define RCC_APBRSTR2_ADCRST_Pos          (9U)
#define RCC_APBRSTR2_ADCRST_Msk          (0x1UL << RCC_APBRSTR2_ADCRST_Pos)     /*!< 0x00000200 */
#define RCC_APBRSTR2_ADCRST              RCC_APBRSTR2_ADCRST_Msk
#define RCC_APBRSTR2_DBGRST_Pos          (10U)
#define RCC_APBRSTR2_DBGRST_Msk          (0x1UL << RCC_APBRSTR2_DBGRST_Pos)     /*!< 0x00000400 */
#define RCC_APBRSTR2_DBGRST              RCC_APBRSTR2_DBGRST_Msk
#define RCC_APBRSTR2_TIM1RST_Pos         (11U)
#define RCC_APBRSTR2_TIM1RST_Msk         (0x1UL << RCC_APBRSTR2_TIM1RST_Pos)    /*!< 0x00000800 */
#define RCC_APBRSTR2_TIM1RST             RCC_APBRSTR2_TIM1RST_Msk
#define RCC_APBRSTR2_SPI1RST_Pos         (12U)
#define RCC_APBRSTR2_SPI1RST_Msk         (0x1UL << RCC_APBRSTR2_SPI1RST_Pos)    /*!< 0x00001000 */
#define RCC_APBRSTR2_SPI1RST             RCC_APBRSTR2_SPI1RST_Msk
#define RCC_APBRSTR2_USART1RST_Pos       (14U)
#define RCC_APBRSTR2_USART1RST_Msk       (0x1UL << RCC_APBRSTR2_USART1RST_Pos)  /*!< 0x00004000 */
#define RCC_APBRSTR2_USART1RST           RCC_APBRSTR2_USART1RST_Msk
#define RCC_APBRSTR2_TIM14RST_Pos        (15U)
#define RCC_APBRSTR2_TIM14RST_Msk        (0x1UL << RCC_APBRSTR2_TIM14RST_Pos)   /*!< 0x00008000 */
#define RCC_APBRSTR2_TIM14RST            RCC_APBRSTR2_TIM14RST_Msk
#define RCC_APBRSTR2_TIM15RST_Pos        (16U)
#define RCC_APBRSTR2_TIM15RST_Msk        (0x1UL << RCC_APBRSTR2_TIM15RST_Pos)   /*!< 0x00010000 */
#define RCC_APBRSTR2_TIM15RST            RCC_APBRSTR2_TIM15RST_Msk
#define RCC_APBRSTR2_TIM16RST_Pos        (17U)
#define RCC_APBRSTR2_TIM16RST_Msk        (0x1UL << RCC_APBRSTR2_TIM16RST_Pos)   /*!< 0x00020000 */
#define RCC_APBRSTR2_TIM16RST            RCC_APBRSTR2_TIM16RST_Msk
#define RCC_APBRSTR2_TIM17RST_Pos        (18U)
#define RCC_APBRSTR2_TIM17RST_Msk        (0x1UL << RCC_APBRSTR2_TIM17RST_Pos)   /*!< 0x00040000 */
#define RCC_APBRSTR2_TIM17RST            RCC_APBRSTR2_TIM17RST_Msk
#define RCC_APBRSTR2_COMP1RST_Pos        (20U)
#define RCC_APBRSTR2_COMP1RST_Msk        (0x1UL << RCC_APBRSTR2_COMP1RST_Pos)   /*!< 0x00100000 */
#define RCC_APBRSTR2_COMP1RST            RCC_APBRSTR2_COMP1RST_Msk
#define RCC_APBRSTR2_COMP2RST_Pos        (21U)
#define RCC_APBRSTR2_COMP2RST_Msk        (0x1UL << RCC_APBRSTR2_COMP2RST_Pos)   /*!< 0x00200000 */
#define RCC_APBRSTR2_COMP2RST            RCC_APBRSTR2_COMP2RST_Msk
#define RCC_APBRSTR2_COMP3RST_Pos        (22U)
#define RCC_APBRSTR2_COMP3RST_Msk        (0x1UL << RCC_APBRSTR2_COMP3RST_Pos)   /*!< 0x00400000 */
#define RCC_APBRSTR2_COMP3RST            RCC_APBRSTR2_COMP3RST_Msk
#define RCC_APBRSTR2_LCDRST_Pos          (23U)
#define RCC_APBRSTR2_LCDRST_Msk          (0x1UL << RCC_APBRSTR2_LCDRST_Pos)     /*!< 0x00800000 */
#define RCC_APBRSTR2_LCDRST              RCC_APBRSTR2_LCDRST_Msk

/********************  Bit definition for RCC_IOPENR register  ****************/
#define RCC_IOPENR_GPIOAEN_Pos           (0U)
#define RCC_IOPENR_GPIOAEN_Msk           (0x1UL << RCC_IOPENR_GPIOAEN_Pos)      /*!< 0x00000001 */
#define RCC_IOPENR_GPIOAEN               RCC_IOPENR_GPIOAEN_Msk
#define RCC_IOPENR_GPIOBEN_Pos           (1U)
#define RCC_IOPENR_GPIOBEN_Msk           (0x1UL << RCC_IOPENR_GPIOBEN_Pos)      /*!< 0x00000002 */
#define RCC_IOPENR_GPIOBEN               RCC_IOPENR_GPIOBEN_Msk
#define RCC_IOPENR_GPIOCEN_Pos           (2U)
#define RCC_IOPENR_GPIOCEN_Msk           (0x1UL << RCC_IOPENR_GPIOCEN_Pos)      /*!< 0x00000004 */
#define RCC_IOPENR_GPIOCEN               RCC_IOPENR_GPIOCEN_Msk
#define RCC_IOPENR_GPIOFEN_Pos           (5U)
#define RCC_IOPENR_GPIOFEN_Msk           (0x1UL << RCC_IOPENR_GPIOFEN_Pos)      /*!< 0x00000020 */
#define RCC_IOPENR_GPIOFEN               RCC_IOPENR_GPIOFEN_Msk

/********************  Bit definition for RCC_AHBENR register  ****************/
#define RCC_AHBENR_DMAEN_Pos            (0U)
#define RCC_AHBENR_DMAEN_Msk            (0x1UL << RCC_AHBENR_DMAEN_Pos)         /*!< 0x00000001 */
#define RCC_AHBENR_DMAEN                RCC_AHBENR_DMAEN_Msk
#define RCC_AHBENR_FLASHEN_Pos           (8U)
#define RCC_AHBENR_FLASHEN_Msk           (0x1UL << RCC_AHBENR_FLASHEN_Pos)      /*!< 0x00000100 */
#define RCC_AHBENR_FLASHEN               RCC_AHBENR_FLASHEN_Msk
#define RCC_AHBENR_SRAMEN_Pos           (9U)
#define RCC_AHBENR_SRAMEN_Msk           (0x1UL << RCC_AHBENR_SRAMEN_Pos)        /*!< 0x00000100 */
#define RCC_AHBENR_SRAMEN               RCC_AHBENR_SRAMEN_Msk
#define RCC_AHBENR_CRCEN_Pos             (12U)
#define RCC_AHBENR_CRCEN_Msk             (0x1UL << RCC_AHBENR_CRCEN_Pos)        /*!< 0x00001000 */
#define RCC_AHBENR_CRCEN                 RCC_AHBENR_CRCEN_Msk
#define RCC_AHBENR_DIVEN_Pos             (24U)
#define RCC_AHBENR_DIVEN_Msk             (0x1UL << RCC_AHBENR_DIVEN_Pos)        /*!< 0x01000000 */
#define RCC_AHBENR_DIVEN                 RCC_AHBENR_DIVEN_Msk

/********************  Bit definition for RCC_APBENR1 register  ***************/
#define RCC_APBENR1_TIM2EN_Pos           (0U)
#define RCC_APBENR1_TIM2EN_Msk           (0x1UL << RCC_APBENR1_TIM2EN_Pos)      /*!< 0x00000001 */
#define RCC_APBENR1_TIM2EN               RCC_APBENR1_TIM2EN_Msk
#define RCC_APBENR1_TIM3EN_Pos           (1U)
#define RCC_APBENR1_TIM3EN_Msk           (0x1UL << RCC_APBENR1_TIM3EN_Pos)      /*!< 0x00000002 */
#define RCC_APBENR1_TIM3EN               RCC_APBENR1_TIM3EN_Msk
#define RCC_APBENR1_TIM6EN_Pos           (4U)
#define RCC_APBENR1_TIM6EN_Msk           (0x1UL << RCC_APBENR1_TIM6EN_Pos)      /*!< 0x00000010 */
#define RCC_APBENR1_TIM6EN               RCC_APBENR1_TIM6EN_Msk
#define RCC_APBENR1_TIM7EN_Pos           (5U)
#define RCC_APBENR1_TIM7EN_Msk           (0x1UL << RCC_APBENR1_TIM7EN_Pos)      /*!< 0x00000020 */
#define RCC_APBENR1_TIM7EN               RCC_APBENR1_TIM7EN_Msk
#define RCC_APBENR1_RTCAPBEN_Pos         (10U)
#define RCC_APBENR1_RTCAPBEN_Msk         (0x1UL << RCC_APBENR1_RTCAPBEN_Pos)    /*!< 0x00000400 */
#define RCC_APBENR1_RTCAPBEN             RCC_APBENR1_RTCAPBEN_Msk
#define RCC_APBENR1_WWDGEN_Pos           (11U)
#define RCC_APBENR1_WWDGEN_Msk           (0x1UL << RCC_APBENR1_WWDGEN_Pos)      /*!< 0x00000800 */
#define RCC_APBENR1_WWDGEN               RCC_APBENR1_WWDGEN_Msk
#define RCC_APBENR1_SPI2EN_Pos           (14U)
#define RCC_APBENR1_SPI2EN_Msk           (0x1UL << RCC_APBENR1_SPI2EN_Pos)      /*!< 0x00004000 */
#define RCC_APBENR1_SPI2EN               RCC_APBENR1_SPI2EN_Msk
#define RCC_APBENR1_USART2EN_Pos         (17U)
#define RCC_APBENR1_USART2EN_Msk         (0x1UL << RCC_APBENR1_USART2EN_Pos)    /*!< 0x00020000 */
#define RCC_APBENR1_USART2EN             RCC_APBENR1_USART2EN_Msk
#define RCC_APBENR1_USART3EN_Pos         (18U)
#define RCC_APBENR1_USART3EN_Msk         (0x1UL << RCC_APBENR1_USART3EN_Pos)    /*!< 0x00040000 */
#define RCC_APBENR1_USART3EN             RCC_APBENR1_USART3EN_Msk
#define RCC_APBENR1_USART4EN_Pos         (19U)
#define RCC_APBENR1_USART4EN_Msk         (0x1UL << RCC_APBENR1_USART4EN_Pos)    /*!< 0x00080000 */
#define RCC_APBENR1_USART4EN             RCC_APBENR1_USART4EN_Msk
#define RCC_APBENR1_I2C1EN_Pos           (21U)
#define RCC_APBENR1_I2C1EN_Msk           (0x1UL << RCC_APBENR1_I2C1EN_Pos)      /*!< 0x00200000 */
#define RCC_APBENR1_I2C1EN               RCC_APBENR1_I2C1EN_Msk
#define RCC_APBENR1_I2C2EN_Pos           (22U)
#define RCC_APBENR1_I2C2EN_Msk           (0x1UL << RCC_APBENR1_I2C2EN_Pos)      /*!< 0x00400000 */
#define RCC_APBENR1_I2C2EN               RCC_APBENR1_I2C2EN_Msk
#define RCC_APBENR1_USBEN_Pos           (23U)
#define RCC_APBENR1_USBEN_Msk           (0x1UL << RCC_APBENR1_USBEN_Pos)        /*!< 0x00400000 */
#define RCC_APBENR1_USBEN               RCC_APBENR1_USBEN_Msk
#define RCC_APBENR1_CANEN_Pos           (25U)
#define RCC_APBENR1_CANEN_Msk           (0x1UL << RCC_APBENR1_CANEN_Pos)        /*!< 0x00400000 */
#define RCC_APBENR1_CANEN               RCC_APBENR1_CANEN_Msk
#define RCC_APBENR1_CAN1EN              RCC_APBENR1_CANEN
#define RCC_APBENR1_CTCEN_Pos           (27U)
#define RCC_APBENR1_CTCEN_Msk           (0x1UL << RCC_APBENR1_CTCEN_Pos)        /*!< 0x00400000 */
#define RCC_APBENR1_CTCEN               RCC_APBENR1_CTCEN_Msk
#define RCC_APBENR1_PWREN_Pos            (28U)
#define RCC_APBENR1_PWREN_Msk            (0x1UL << RCC_APBENR1_PWREN_Pos)       /*!< 0x10000000 */
#define RCC_APBENR1_PWREN                RCC_APBENR1_PWREN_Msk
#define RCC_APBENR1_DACEN_Pos           (29U)
#define RCC_APBENR1_DACEN_Msk           (0x1UL << RCC_APBENR1_DACEN_Pos)        /*!< 0x00400000 */
#define RCC_APBENR1_DACEN               RCC_APBENR1_DACEN_Msk
#define RCC_APBENR1_DAC1EN_Pos          (29U)
#define RCC_APBENR1_DAC1EN_Msk          (0x1UL << RCC_APBENR1_DAC1EN_Pos)        /*!< 0x00400000 */
#define RCC_APBENR1_DAC1EN              RCC_APBENR1_DAC1EN_Msk
#define RCC_APBENR1_OPAEN_Pos           (30U)
#define RCC_APBENR1_OPAEN_Msk           (0x1UL << RCC_APBENR1_OPAEN_Pos)        /*!< 0x00400000 */
#define RCC_APBENR1_OPAEN               RCC_APBENR1_OPAEN_Msk
#define RCC_APBENR1_LPTIMEN_Pos         (31U)
#define RCC_APBENR1_LPTIMEN_Msk         (0x1UL << RCC_APBENR1_LPTIMEN_Pos)      /*!< 0x80000000 */
#define RCC_APBENR1_LPTIMEN             RCC_APBENR1_LPTIMEN_Msk
#define RCC_APBENR1_LPTIM1EN            RCC_APBENR1_LPTIMEN_Msk

/********************  Bit definition for RCC_APBENR2 register  ***************/
#define RCC_APBENR2_SYSCFGEN_Pos         (0U)
#define RCC_APBENR2_SYSCFGEN_Msk         (0x1UL << RCC_APBENR2_SYSCFGEN_Pos)    /*!< 0x00000001 */
#define RCC_APBENR2_SYSCFGEN             RCC_APBENR2_SYSCFGEN_Msk
#define RCC_APBENR2_ADCEN_Pos            (9U)
#define RCC_APBENR2_ADCEN_Msk            (0x1UL << RCC_APBENR2_ADCEN_Pos)       /*!< 0x00000200 */
#define RCC_APBENR2_ADCEN                RCC_APBENR2_ADCEN_Msk
#define RCC_APBENR2_DBGEN_Pos            (10U)
#define RCC_APBENR2_DBGEN_Msk            (0x1UL << RCC_APBENR2_DBGEN_Pos)       /*!< 0x00000400 */
#define RCC_APBENR2_DBGEN                RCC_APBENR2_DBGEN_Msk
#define RCC_APBENR2_TIM1EN_Pos           (11U)
#define RCC_APBENR2_TIM1EN_Msk           (0x1UL << RCC_APBENR2_TIM1EN_Pos)      /*!< 0x00000800 */
#define RCC_APBENR2_TIM1EN               RCC_APBENR2_TIM1EN_Msk
#define RCC_APBENR2_SPI1EN_Pos           (12U)
#define RCC_APBENR2_SPI1EN_Msk           (0x1UL << RCC_APBENR2_SPI1EN_Pos)      /*!< 0x00001000 */
#define RCC_APBENR2_SPI1EN               RCC_APBENR2_SPI1EN_Msk
#define RCC_APBENR2_USART1EN_Pos         (14U)
#define RCC_APBENR2_USART1EN_Msk         (0x1UL << RCC_APBENR2_USART1EN_Pos)    /*!< 0x00004000 */
#define RCC_APBENR2_USART1EN             RCC_APBENR2_USART1EN_Msk
#define RCC_APBENR2_TIM14EN_Pos          (15U)
#define RCC_APBENR2_TIM14EN_Msk          (0x1UL << RCC_APBENR2_TIM14EN_Pos)     /*!< 0x00008000 */
#define RCC_APBENR2_TIM14EN              RCC_APBENR2_TIM14EN_Msk
#define RCC_APBENR2_TIM15EN_Pos          (16U)
#define RCC_APBENR2_TIM15EN_Msk          (0x1UL << RCC_APBENR2_TIM15EN_Pos)     /*!< 0x00010000 */
#define RCC_APBENR2_TIM15EN              RCC_APBENR2_TIM15EN_Msk
#define RCC_APBENR2_TIM16EN_Pos          (17U)
#define RCC_APBENR2_TIM16EN_Msk          (0x1UL << RCC_APBENR2_TIM16EN_Pos)     /*!< 0x00020000 */
#define RCC_APBENR2_TIM16EN              RCC_APBENR2_TIM16EN_Msk
#define RCC_APBENR2_TIM17EN_Pos          (18U)
#define RCC_APBENR2_TIM17EN_Msk          (0x1UL << RCC_APBENR2_TIM17EN_Pos)     /*!< 0x00040000 */
#define RCC_APBENR2_TIM17EN              RCC_APBENR2_TIM17EN_Msk
#define RCC_APBENR2_COMP1EN_Pos          (20U)
#define RCC_APBENR2_COMP1EN_Msk          (0x1UL << RCC_APBENR2_COMP1EN_Pos)     /*!< 0x00100000 */
#define RCC_APBENR2_COMP1EN              RCC_APBENR2_COMP1EN_Msk
#define RCC_APBENR2_COMP2EN_Pos          (21U)
#define RCC_APBENR2_COMP2EN_Msk          (0x1UL << RCC_APBENR2_COMP2EN_Pos)     /*!< 0x00200000 */
#define RCC_APBENR2_COMP2EN              RCC_APBENR2_COMP2EN_Msk
#define RCC_APBENR2_COMP3EN_Pos          (22U)
#define RCC_APBENR2_COMP3EN_Msk          (0x1UL << RCC_APBENR2_COMP3EN_Pos)     /*!< 0x00400000 */
#define RCC_APBENR2_COMP3EN              RCC_APBENR2_COMP3EN_Msk
#define RCC_APBENR2_LCDEN_Pos            (23U)
#define RCC_APBENR2_LCDEN_Msk            (0x1UL << RCC_APBENR2_LCDEN_Pos)       /*!< 0x00800000 */
#define RCC_APBENR2_LCDEN                RCC_APBENR2_LCDEN_Msk

/********************  Bit definition for RCC_CCIPR register  *****************/
#define RCC_CCIPR_CANSEL_Pos             (6U)
#define RCC_CCIPR_CANSEL_Msk             (0x1UL << RCC_CCIPR_CANSEL_Pos)      /*!< 0x00000040 */
#define RCC_CCIPR_CANSEL                 RCC_CCIPR_CANSEL_Msk
#define RCC_CCIPR_CAN1SEL                RCC_CCIPR_CANSEL
#define RCC_CCIPR_PVDSEL_Pos             (7U)
#define RCC_CCIPR_PVDSEL_Msk             (0x1UL << RCC_CCIPR_PVDSEL_Pos)      /*!< 0x00000080 */
#define RCC_CCIPR_PVDSEL                 RCC_CCIPR_PVDSEL_Msk
#define RCC_CCIPR_COMP1SEL_Pos           (8U)
#define RCC_CCIPR_COMP1SEL_Msk           (0x1UL << RCC_CCIPR_COMP1SEL_Pos)    /*!< 0x00000100 */
#define RCC_CCIPR_COMP1SEL               RCC_CCIPR_COMP1SEL_Msk
#define RCC_CCIPR_COMP2SEL_Pos           (9U)
#define RCC_CCIPR_COMP2SEL_Msk           (0x1UL << RCC_CCIPR_COMP2SEL_Pos)    /*!< 0x00000200 */
#define RCC_CCIPR_COMP2SEL               RCC_CCIPR_COMP2SEL_Msk
#define RCC_CCIPR_COMP3SEL_Pos           (10U)
#define RCC_CCIPR_COMP3SEL_Msk           (0x1UL << RCC_CCIPR_COMP3SEL_Pos)    /*!< 0x00000400 */
#define RCC_CCIPR_COMP3SEL               RCC_CCIPR_COMP3SEL_Msk
#define RCC_CCIPR_LPTIMSEL_Pos          (18U)
#define RCC_CCIPR_LPTIMSEL_Msk          (0x3UL << RCC_CCIPR_LPTIMSEL_Pos)     /*!< 0x000C0000 */
#define RCC_CCIPR_LPTIMSEL              RCC_CCIPR_LPTIMSEL_Msk
#define RCC_CCIPR_LPTIM1SEL             RCC_CCIPR_LPTIMSEL_Msk
#define RCC_CCIPR_LPTIMSEL_0            (0x1UL << RCC_CCIPR_LPTIMSEL_Pos)     /*!< 0x00040000 */
#define RCC_CCIPR_LPTIMSEL_1            (0x2UL << RCC_CCIPR_LPTIMSEL_Pos)     /*!< 0x00080000 */

/********************  Bit definition for RCC_BDCR register  ******************/
#define RCC_BDCR_LSEON_Pos               (0U)
#define RCC_BDCR_LSEON_Msk               (0x1UL << RCC_BDCR_LSEON_Pos)          /*!< 0x00000001 */
#define RCC_BDCR_LSEON                   RCC_BDCR_LSEON_Msk
#define RCC_BDCR_LSERDY_Pos              (1U)
#define RCC_BDCR_LSERDY_Msk              (0x1UL << RCC_BDCR_LSERDY_Pos)         /*!< 0x00000002 */
#define RCC_BDCR_LSERDY                  RCC_BDCR_LSERDY_Msk
#define RCC_BDCR_LSEBYP_Pos              (2U)
#define RCC_BDCR_LSEBYP_Msk              (0x1UL << RCC_BDCR_LSEBYP_Pos)         /*!< 0x00000004 */
#define RCC_BDCR_LSEBYP                  RCC_BDCR_LSEBYP_Msk
#define RCC_BDCR_LSECSSON_Pos            (5U)
#define RCC_BDCR_LSECSSON_Msk            (0x1UL << RCC_BDCR_LSECSSON_Pos)       /*!< 0x00000020 */
#define RCC_BDCR_LSECSSON                 RCC_BDCR_LSECSSON_Msk
#define RCC_BDCR_LSECSSD_Pos             (6U)
#define RCC_BDCR_LSECSSD_Msk             (0x1UL << RCC_BDCR_LSECSSD_Pos)        /*!< 0x00000040 */
#define RCC_BDCR_LSECSSD                  RCC_BDCR_LSECSSD_Msk
#define RCC_BDCR_RTCSEL_Pos              (8U)
#define RCC_BDCR_RTCSEL_Msk              (0x3UL << RCC_BDCR_RTCSEL_Pos)         /*!< 0x00000300 */
#define RCC_BDCR_RTCSEL                  RCC_BDCR_RTCSEL_Msk
#define RCC_BDCR_RTCSEL_0                (0x1UL << RCC_BDCR_RTCSEL_Pos)         /*!< 0x00000100 */
#define RCC_BDCR_RTCSEL_1                (0x2UL << RCC_BDCR_RTCSEL_Pos)         /*!< 0x00000200 */
#define RCC_BDCR_RTCEN_Pos               (15U)
#define RCC_BDCR_RTCEN_Msk               (0x1UL << RCC_BDCR_RTCEN_Pos)          /*!< 0x00008000 */
#define RCC_BDCR_RTCEN                   RCC_BDCR_RTCEN_Msk
#define RCC_BDCR_BDRST_Pos               (16U)
#define RCC_BDCR_BDRST_Msk               (0x1UL << RCC_BDCR_BDRST_Pos)          /*!< 0x00010000 */
#define RCC_BDCR_BDRST                   RCC_BDCR_BDRST_Msk
#define RCC_BDCR_LSCOEN_Pos              (24U)
#define RCC_BDCR_LSCOEN_Msk              (0x1UL << RCC_BDCR_LSCOEN_Pos)         /*!< 0x01000000 */
#define RCC_BDCR_LSCOEN                  RCC_BDCR_LSCOEN_Msk
#define RCC_BDCR_LSCOSEL_Pos             (25U)
#define RCC_BDCR_LSCOSEL_Msk             (0x1UL << RCC_BDCR_LSCOSEL_Pos)        /*!< 0x02000000 */
#define RCC_BDCR_LSCOSEL                 RCC_BDCR_LSCOSEL_Msk

/********************  Bit definition for RCC_CSR register  *******************/
#define RCC_CSR_LSION_Pos                (0U)
#define RCC_CSR_LSION_Msk                (0x1UL << RCC_CSR_LSION_Pos)           /*!< 0x00000001 */
#define RCC_CSR_LSION                    RCC_CSR_LSION_Msk
#define RCC_CSR_LSIRDY_Pos               (1U)
#define RCC_CSR_LSIRDY_Msk               (0x1UL << RCC_CSR_LSIRDY_Pos)          /*!< 0x00000002 */
#define RCC_CSR_LSIRDY                   RCC_CSR_LSIRDY_Msk
#define RCC_CSR_NRST_FLTDIS_Pos          (8U)
#define RCC_CSR_NRST_FLTDIS_Msk          (0x1UL << RCC_CSR_NRST_FLTDIS_Pos)     /*!< 0x00000100 */
#define RCC_CSR_NRST_FLTDIS              RCC_CSR_NRST_FLTDIS_Msk
#define RCC_CSR_RMVF_Pos                 (23U)
#define RCC_CSR_RMVF_Msk                 (0x1UL << RCC_CSR_RMVF_Pos)            /*!< 0x00800000 */
#define RCC_CSR_RMVF                     RCC_CSR_RMVF_Msk
#define RCC_CSR_OBLRSTF_Pos              (25U)
#define RCC_CSR_OBLRSTF_Msk              (0x1UL << RCC_CSR_OBLRSTF_Pos)         /*!< 0x02000000 */
#define RCC_CSR_OBLRSTF                  RCC_CSR_OBLRSTF_Msk
#define RCC_CSR_PINRSTF_Pos              (26U)
#define RCC_CSR_PINRSTF_Msk              (0x1UL << RCC_CSR_PINRSTF_Pos)         /*!< 0x04000000 */
#define RCC_CSR_PINRSTF                  RCC_CSR_PINRSTF_Msk
#define RCC_CSR_PWRRSTF_Pos              (27U)
#define RCC_CSR_PWRRSTF_Msk              (0x1UL << RCC_CSR_PWRRSTF_Pos)         /*!< 0x08000000 */
#define RCC_CSR_PWRRSTF                  RCC_CSR_PWRRSTF_Msk
#define RCC_CSR_SFTRSTF_Pos              (28U)
#define RCC_CSR_SFTRSTF_Msk              (0x1UL << RCC_CSR_SFTRSTF_Pos)         /*!< 0x10000000 */
#define RCC_CSR_SFTRSTF                  RCC_CSR_SFTRSTF_Msk
#define RCC_CSR_IWDGRSTF_Pos             (29U)
#define RCC_CSR_IWDGRSTF_Msk             (0x1UL << RCC_CSR_IWDGRSTF_Pos)        /*!< 0x20000000 */
#define RCC_CSR_IWDGRSTF                 RCC_CSR_IWDGRSTF_Msk
#define RCC_CSR_WWDGRSTF_Pos             (30U)
#define RCC_CSR_WWDGRSTF_Msk             (0x1UL << RCC_CSR_WWDGRSTF_Pos)        /*!< 0x40000000 */
#define RCC_CSR_WWDGRSTF                 RCC_CSR_WWDGRSTF_Msk

/******************************************************************************/
/*                                                                            */
/*                             Real-Time Clock (RTC)                          */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for RTC_CRH register  ********************/
#define RTC_CRH_SECIE_Pos                   (0U)
#define RTC_CRH_SECIE_Msk                   (0x1UL << RTC_CRH_SECIE_Pos)        /*!< 0x00000001 */
#define RTC_CRH_SECIE                       RTC_CRH_SECIE_Msk                   /*!< Second Interrupt Enable */
#define RTC_CRH_ALRIE_Pos                   (1U)
#define RTC_CRH_ALRIE_Msk                   (0x1UL << RTC_CRH_ALRIE_Pos)        /*!< 0x00000002 */
#define RTC_CRH_ALRIE                       RTC_CRH_ALRIE_Msk                   /*!< Alarm Interrupt Enable */
#define RTC_CRH_OWIE_Pos                    (2U)
#define RTC_CRH_OWIE_Msk                    (0x1UL << RTC_CRH_OWIE_Pos)         /*!< 0x00000004 */
#define RTC_CRH_OWIE                        RTC_CRH_OWIE_Msk                    /*!< OverfloW Interrupt Enable */

/*******************  Bit definition for RTC_CRL register  ********************/
#define RTC_CRL_SECF_Pos                    (0U)
#define RTC_CRL_SECF_Msk                    (0x1UL << RTC_CRL_SECF_Pos)         /*!< 0x00000001 */
#define RTC_CRL_SECF                        RTC_CRL_SECF_Msk                    /*!< Second Flag */
#define RTC_CRL_ALRF_Pos                    (1U)
#define RTC_CRL_ALRF_Msk                    (0x1UL << RTC_CRL_ALRF_Pos)         /*!< 0x00000002 */
#define RTC_CRL_ALRF                        RTC_CRL_ALRF_Msk                    /*!< Alarm Flag */
#define RTC_CRL_OWF_Pos                     (2U)
#define RTC_CRL_OWF_Msk                     (0x1UL << RTC_CRL_OWF_Pos)          /*!< 0x00000004 */
#define RTC_CRL_OWF                         RTC_CRL_OWF_Msk                     /*!< OverfloW Flag */
#define RTC_CRL_RSF_Pos                     (3U)
#define RTC_CRL_RSF_Msk                     (0x1UL << RTC_CRL_RSF_Pos)          /*!< 0x00000008 */
#define RTC_CRL_RSF                         RTC_CRL_RSF_Msk                     /*!< Registers Synchronized Flag */
#define RTC_CRL_CNF_Pos                     (4U)
#define RTC_CRL_CNF_Msk                     (0x1UL << RTC_CRL_CNF_Pos)          /*!< 0x00000010 */
#define RTC_CRL_CNF                         RTC_CRL_CNF_Msk                     /*!< Configuration Flag */
#define RTC_CRL_RTOFF_Pos                   (5U)
#define RTC_CRL_RTOFF_Msk                   (0x1UL << RTC_CRL_RTOFF_Pos)        /*!< 0x00000020 */
#define RTC_CRL_RTOFF                       RTC_CRL_RTOFF_Msk                   /*!< RTC operation OFF */

/*******************  Bit definition for RTC_PRLH register  *******************/
#define RTC_PRLH_PRL_Pos                    (0U)
#define RTC_PRLH_PRL_Msk                    (0xFUL << RTC_PRLH_PRL_Pos)         /*!< 0x0000000F */
#define RTC_PRLH_PRL                        RTC_PRLH_PRL_Msk                    /*!< RTC Prescaler Reload Value High */

/*******************  Bit definition for RTC_PRLL register  *******************/
#define RTC_PRLL_PRL_Pos                    (0U)
#define RTC_PRLL_PRL_Msk                    (0xFFFFUL << RTC_PRLL_PRL_Pos)      /*!< 0x0000FFFF */
#define RTC_PRLL_PRL                        RTC_PRLL_PRL_Msk                    /*!< RTC Prescaler Reload Value Low */

/*******************  Bit definition for RTC_DIVH register  *******************/
#define RTC_DIVH_RTC_DIV_Pos                (0U)
#define RTC_DIVH_RTC_DIV_Msk                (0xFUL << RTC_DIVH_RTC_DIV_Pos)     /*!< 0x0000000F */
#define RTC_DIVH_RTC_DIV                    RTC_DIVH_RTC_DIV_Msk                /*!< RTC Clock Divider High */

/*******************  Bit definition for RTC_DIVL register  *******************/
#define RTC_DIVL_RTC_DIV_Pos                (0U)
#define RTC_DIVL_RTC_DIV_Msk                (0xFFFFUL << RTC_DIVL_RTC_DIV_Pos)  /*!< 0x0000FFFF */
#define RTC_DIVL_RTC_DIV                    RTC_DIVL_RTC_DIV_Msk                /*!< RTC Clock Divider Low */

/*******************  Bit definition for RTC_CNTH register  *******************/
#define RTC_CNTH_RTC_CNT_Pos                (0U)
#define RTC_CNTH_RTC_CNT_Msk                (0xFFFFUL << RTC_CNTH_RTC_CNT_Pos)  /*!< 0x0000FFFF */
#define RTC_CNTH_RTC_CNT                    RTC_CNTH_RTC_CNT_Msk                /*!< RTC Counter High */

/*******************  Bit definition for RTC_CNTL register  *******************/
#define RTC_CNTL_RTC_CNT_Pos                (0U)
#define RTC_CNTL_RTC_CNT_Msk                (0xFFFFUL << RTC_CNTL_RTC_CNT_Pos)  /*!< 0x0000FFFF */
#define RTC_CNTL_RTC_CNT                    RTC_CNTL_RTC_CNT_Msk                /*!< RTC Counter Low */

/*******************  Bit definition for RTC_ALRH register  *******************/
#define RTC_ALRH_RTC_ALR_Pos                (0U)
#define RTC_ALRH_RTC_ALR_Msk                (0xFFFFUL << RTC_ALRH_RTC_ALR_Pos)  /*!< 0x0000FFFF */
#define RTC_ALRH_RTC_ALR                    RTC_ALRH_RTC_ALR_Msk                /*!< RTC Alarm High */

/*******************  Bit definition for RTC_ALRL register  *******************/
#define RTC_ALRL_RTC_ALR_Pos                (0U)
#define RTC_ALRL_RTC_ALR_Msk                (0xFFFFUL << RTC_ALRL_RTC_ALR_Pos)  /*!< 0x0000FFFF */
#define RTC_ALRL_RTC_ALR                    RTC_ALRL_RTC_ALR_Msk                /*!< RTC Alarm Low */

/*******************  Bit definition for BKP_RTCCR register  ******************/
#define BKP_RTCCR_CAL_Pos                   (0U)
#define BKP_RTCCR_CAL_Msk                   (0x7FUL << BKP_RTCCR_CAL_Pos)       /*!< 0x0000007F */
#define BKP_RTCCR_CAL                       BKP_RTCCR_CAL_Msk                   /*!< Calibration value */
#define BKP_RTCCR_CAL_0                     (0x01UL << BKP_RTCCR_CAL_Pos)
#define BKP_RTCCR_CAL_1                     (0x02UL << BKP_RTCCR_CAL_Pos)
#define BKP_RTCCR_CAL_2                     (0x04UL << BKP_RTCCR_CAL_Pos)
#define BKP_RTCCR_CAL_3                     (0x08UL << BKP_RTCCR_CAL_Pos)
#define BKP_RTCCR_CAL_4                     (0x10UL << BKP_RTCCR_CAL_Pos)
#define BKP_RTCCR_CAL_5                     (0x20UL << BKP_RTCCR_CAL_Pos)
#define BKP_RTCCR_CAL_6                     (0x40UL << BKP_RTCCR_CAL_Pos)
#define BKP_RTCCR_CCO_Pos                   (7U)
#define BKP_RTCCR_CCO_Msk                   (0x1UL << BKP_RTCCR_CCO_Pos)        /*!< 0x00000080 */
#define BKP_RTCCR_CCO                       BKP_RTCCR_CCO_Msk                   /*!< Calibration Clock Output */
#define BKP_RTCCR_ASOE_Pos                  (8U)
#define BKP_RTCCR_ASOE_Msk                  (0x1UL << BKP_RTCCR_ASOE_Pos)       /*!< 0x00000100 */
#define BKP_RTCCR_ASOE                      BKP_RTCCR_ASOE_Msk                  /*!< Alarm or Second Output Enable */
#define BKP_RTCCR_ASOS_Pos                  (9U)
#define BKP_RTCCR_ASOS_Msk                  (0x1UL << BKP_RTCCR_ASOS_Pos)       /*!< 0x00000200 */
#define BKP_RTCCR_ASOS                      BKP_RTCCR_ASOS_Msk                  /*!< Alarm or Second Output Selection */

/******************************************************************************/
/*                                                                            */
/*                        Serial Peripheral Interface (SPI)                   */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for SPI_CR1 register  ********************/
#define SPI_CR1_CPHA_Pos                    (0U)
#define SPI_CR1_CPHA_Msk                    (0x1UL << SPI_CR1_CPHA_Pos)         /*!< 0x00000001 */
#define SPI_CR1_CPHA                        SPI_CR1_CPHA_Msk                    /*!< Clock Phase */
#define SPI_CR1_CPOL_Pos                    (1U)
#define SPI_CR1_CPOL_Msk                    (0x1UL << SPI_CR1_CPOL_Pos)         /*!< 0x00000002 */
#define SPI_CR1_CPOL                        SPI_CR1_CPOL_Msk                    /*!< Clock Polarity */
#define SPI_CR1_MSTR_Pos                    (2U)
#define SPI_CR1_MSTR_Msk                    (0x1UL << SPI_CR1_MSTR_Pos)         /*!< 0x00000004 */
#define SPI_CR1_MSTR                        SPI_CR1_MSTR_Msk                    /*!< Master Selection */
#define SPI_CR1_BR_Pos                      (3U)
#define SPI_CR1_BR_Msk                      (0x7UL << SPI_CR1_BR_Pos)           /*!< 0x00000038 */
#define SPI_CR1_BR                          SPI_CR1_BR_Msk                      /*!< BR[2:0] bits (Baud Rate Control) */
#define SPI_CR1_BR_0                        (0x1UL << SPI_CR1_BR_Pos)           /*!< 0x00000008 */
#define SPI_CR1_BR_1                        (0x2UL << SPI_CR1_BR_Pos)           /*!< 0x00000010 */
#define SPI_CR1_BR_2                        (0x4UL << SPI_CR1_BR_Pos)           /*!< 0x00000020 */
#define SPI_CR1_SPE_Pos                     (6U)
#define SPI_CR1_SPE_Msk                     (0x1UL << SPI_CR1_SPE_Pos)          /*!< 0x00000040 */
#define SPI_CR1_SPE                         SPI_CR1_SPE_Msk                     /*!< SPI Enable */
#define SPI_CR1_LSBFIRST_Pos                (7U)
#define SPI_CR1_LSBFIRST_Msk                (0x1UL << SPI_CR1_LSBFIRST_Pos)     /*!< 0x00000080 */
#define SPI_CR1_LSBFIRST                    SPI_CR1_LSBFIRST_Msk                /*!< Frame Format */
#define SPI_CR1_SSI_Pos                     (8U)
#define SPI_CR1_SSI_Msk                     (0x1UL << SPI_CR1_SSI_Pos)          /*!< 0x00000100 */
#define SPI_CR1_SSI                         SPI_CR1_SSI_Msk                     /*!< Internal slave select */
#define SPI_CR1_SSM_Pos                     (9U)
#define SPI_CR1_SSM_Msk                     (0x1UL << SPI_CR1_SSM_Pos)          /*!< 0x00000200 */
#define SPI_CR1_SSM                         SPI_CR1_SSM_Msk                     /*!< Software slave management */
#define SPI_CR1_RXONLY_Pos                  (10U)
#define SPI_CR1_RXONLY_Msk                  (0x1UL << SPI_CR1_RXONLY_Pos)       /*!< 0x00000400 */
#define SPI_CR1_RXONLY                      SPI_CR1_RXONLY_Msk                  /*!< Receive only */
#define SPI_CR1_DFF_Pos                     (11U)
#define SPI_CR1_DFF_Msk                     (0x1UL << SPI_CR1_DFF_Pos)          /*!< 0x00000800 */
#define SPI_CR1_DFF                         SPI_CR1_DFF_Msk                     /*!< Data frame format */
#define SPI_CR1_CRCNEXT_Pos                 (12U)
#define SPI_CR1_CRCNEXT_Msk                 (0x1UL << SPI_CR1_CRCNEXT_Pos)      /*!< 0x00001000 */
#define SPI_CR1_CRCNEXT                     SPI_CR1_CRCNEXT_Msk                 /*!< Transmit CRC next */
#define SPI_CR1_CRCEN_Pos                   (13U)
#define SPI_CR1_CRCEN_Msk                   (0x1UL << SPI_CR1_CRCEN_Pos)        /*!< 0x00002000 */
#define SPI_CR1_CRCEN                       SPI_CR1_CRCEN_Msk                   /*!< Hardware CRC calculation enable */
#define SPI_CR1_BIDIOE_Pos                  (14U)
#define SPI_CR1_BIDIOE_Msk                  (0x1UL << SPI_CR1_BIDIOE_Pos)       /*!< 0x00004000 */
#define SPI_CR1_BIDIOE                      SPI_CR1_BIDIOE_Msk                  /*!< Output enable in bidirectional mode */
#define SPI_CR1_BIDIMODE_Pos                (15U)
#define SPI_CR1_BIDIMODE_Msk                (0x1UL << SPI_CR1_BIDIMODE_Pos)     /*!< 0x00008000 */
#define SPI_CR1_BIDIMODE                    SPI_CR1_BIDIMODE_Msk                /*!< Bidirectional data mode enable */

/*******************  Bit definition for SPI_CR2 register  ********************/
#define SPI_CR2_RXDMAEN_Pos                 (0U)
#define SPI_CR2_RXDMAEN_Msk                 (0x1UL << SPI_CR2_RXDMAEN_Pos)      /*!< 0x00000001 */
#define SPI_CR2_RXDMAEN                     SPI_CR2_RXDMAEN_Msk                 /*!< Rx Buffer DMA Enable */
#define SPI_CR2_TXDMAEN_Pos                 (1U)
#define SPI_CR2_TXDMAEN_Msk                 (0x1UL << SPI_CR2_TXDMAEN_Pos)      /*!< 0x00000002 */
#define SPI_CR2_TXDMAEN                     SPI_CR2_TXDMAEN_Msk                 /*!< Tx Buffer DMA Enable */
#define SPI_CR2_SSOE_Pos                    (2U)
#define SPI_CR2_SSOE_Msk                    (0x1UL << SPI_CR2_SSOE_Pos)         /*!< 0x00000004 */
#define SPI_CR2_SSOE                        SPI_CR2_SSOE_Msk                    /*!< SS Output Enable */
#define SPI_CR2_CLRTXFIFO_Pos               (4U)
#define SPI_CR2_CLRTXFIFO_Msk               (0x1UL << SPI_CR2_CLRTXFIFO_Pos)    /*!< 0x00000010 */
#define SPI_CR2_CLRTXFIFO                   SPI_CR2_CLRTXFIFO_Msk               /*!< Clear TX FIFO */
#define SPI_CR2_ERRIE_Pos                   (5U)
#define SPI_CR2_ERRIE_Msk                   (0x1UL << SPI_CR2_ERRIE_Pos)        /*!< 0x00000020 */
#define SPI_CR2_ERRIE                       SPI_CR2_ERRIE_Msk                   /*!< Error Interrupt Enable */
#define SPI_CR2_RXNEIE_Pos                  (6U)
#define SPI_CR2_RXNEIE_Msk                  (0x1UL << SPI_CR2_RXNEIE_Pos)       /*!< 0x00000040 */
#define SPI_CR2_RXNEIE                      SPI_CR2_RXNEIE_Msk                  /*!< RX buffer Not Empty Interrupt Enable */
#define SPI_CR2_TXEIE_Pos                   (7U)
#define SPI_CR2_TXEIE_Msk                   (0x1UL << SPI_CR2_TXEIE_Pos)        /*!< 0x00000080 */
#define SPI_CR2_TXEIE                       SPI_CR2_TXEIE_Msk                   /*!< Tx buffer Empty Interrupt Enable */

/********************  Bit definition for SPI_SR register  ********************/
#define SPI_SR_RXNE_Pos                     (0U)
#define SPI_SR_RXNE_Msk                     (0x1UL << SPI_SR_RXNE_Pos)          /*!< 0x00000001 */
#define SPI_SR_RXNE                         SPI_SR_RXNE_Msk                     /*!< Receive buffer Not Empty */
#define SPI_SR_TXE_Pos                      (1U)
#define SPI_SR_TXE_Msk                      (0x1UL << SPI_SR_TXE_Pos)           /*!< 0x00000002 */
#define SPI_SR_TXE                          SPI_SR_TXE_Msk                      /*!< Transmit buffer Empty */
#define SPI_SR_CHSIDE_Pos                   (2U)
#define SPI_SR_CHSIDE_Msk                   (0x1UL << SPI_SR_CHSIDE_Pos)        /*!< 0x00000004 */
#define SPI_SR_CHSIDE                       SPI_SR_CHSIDE_Msk                   /*!< Channel side */
#define SPI_SR_UDR_Pos                      (3U)
#define SPI_SR_UDR_Msk                      (0x1UL << SPI_SR_UDR_Pos)           /*!< 0x00000008 */
#define SPI_SR_UDR                          SPI_SR_UDR_Msk                      /*!< Underrun flag */
#define SPI_SR_CRCERR_Pos                   (4U)
#define SPI_SR_CRCERR_Msk                   (0x1UL << SPI_SR_CRCERR_Pos)        /*!< 0x00000010 */
#define SPI_SR_CRCERR                       SPI_SR_CRCERR_Msk                   /*!< CRC error flag */
#define SPI_SR_MODF_Pos                     (5U)
#define SPI_SR_MODF_Msk                     (0x1UL << SPI_SR_MODF_Pos)          /*!< 0x00000020 */
#define SPI_SR_MODF                         SPI_SR_MODF_Msk                     /*!< Mode fault */
#define SPI_SR_OVR_Pos                      (6U)
#define SPI_SR_OVR_Msk                      (0x1UL << SPI_SR_OVR_Pos)           /*!< 0x00000040 */
#define SPI_SR_OVR                          SPI_SR_OVR_Msk                      /*!< Overrun flag */
#define SPI_SR_BSY_Pos                      (7U)
#define SPI_SR_BSY_Msk                      (0x1UL << SPI_SR_BSY_Pos)           /*!< 0x00000080 */
#define SPI_SR_BSY                          SPI_SR_BSY_Msk                      /*!< Busy flag */
#define SPI_SR_FRLVL_Pos                    (9U)
#define SPI_SR_FRLVL_Msk                    (0x3UL << SPI_SR_FRLVL_Pos)         /*!< 0x00000600 */
#define SPI_SR_FRLVL                        SPI_SR_FRLVL_Msk                    /*!< FIFO Reception Level */
#define SPI_SR_FRLVL_0                      (0x1UL << SPI_SR_FRLVL_Pos)         /*!< 0x00000200 */
#define SPI_SR_FRLVL_1                      (0x2UL << SPI_SR_FRLVL_Pos)         /*!< 0x00000400 */
#define SPI_SR_FTLVL_Pos                    (11U)
#define SPI_SR_FTLVL_Msk                    (0x3UL << SPI_SR_FTLVL_Pos)         /*!< 0x00001800 */
#define SPI_SR_FTLVL                        SPI_SR_FTLVL_Msk                    /*!< FIFO Transmission Level */
#define SPI_SR_FTLVL_0                      (0x1UL << SPI_SR_FTLVL_Pos)         /*!< 0x00000800 */
#define SPI_SR_FTLVL_1                      (0x2UL << SPI_SR_FTLVL_Pos)         /*!< 0x00001000 */

/********************  Bit definition for SPI_DR register  ********************/
#define SPI_DR_DR_Pos                       (0U)
#define SPI_DR_DR_Msk                       (0xFFFFUL << SPI_DR_DR_Pos)         /*!< 0x0000FFFF */
#define SPI_DR_DR                           SPI_DR_DR_Msk                       /*!< Data Register */

/*!< SPI_CRCPR */
#define SPI_CRCPR_CRCPOLY_Pos                     (0U)
#define SPI_CRCPR_CRCPOLY_Msk                     (0xFFFFUL << SPI_CRCPR_CRCPOLY_Pos)               /*!< 0x0000FFFF */
#define SPI_CRCPR_CRCPOLY                         SPI_CRCPR_CRCPOLY_Msk                             /*!< CRCPOLY[15:0] bits (desc CRCPOLY) */

/*!< SPI_RXCRCR */
#define SPI_RXCRCR_RXCRC_Pos                      (0U)
#define SPI_RXCRCR_RXCRC_Msk                      (0xFFFFUL << SPI_RXCRCR_RXCRC_Pos)                /*!< 0x0000FFFF */
#define SPI_RXCRCR_RXCRC                          SPI_RXCRCR_RXCRC_Msk                              /*!< RXCRC[15:0] bits (desc RXCRC) */

/*!< SPI_TXCRCR */
#define SPI_TXCRCR_TXCRC_Pos                      (0U)
#define SPI_TXCRCR_TXCRC_Msk                      (0xFFFFUL << SPI_TXCRCR_TXCRC_Pos)                /*!< 0x0000FFFF */
#define SPI_TXCRCR_TXCRC                          SPI_TXCRCR_TXCRC_Msk                              /*!< TXCRC[15:0] bits (desc TXCRC) */

/*!< SPI_I2SCFGR */
#define SPI_I2SCFGR_CHLEN_Pos                     (0U)
#define SPI_I2SCFGR_CHLEN_Msk                     (0x1UL << SPI_I2SCFGR_CHLEN_Pos)                  /*!< 0x00000001 */
#define SPI_I2SCFGR_CHLEN                         SPI_I2SCFGR_CHLEN_Msk                             /*!< desc CHLEN */
#define SPI_I2SCFGR_DATLEN_Pos                    (1U)
#define SPI_I2SCFGR_DATLEN_Msk                    (0x3UL << SPI_I2SCFGR_DATLEN_Pos)                 /*!< 0x00000006 */
#define SPI_I2SCFGR_DATLEN                        SPI_I2SCFGR_DATLEN_Msk                            /*!< DATLEN[2:1] bits (desc DATLEN) */
#define SPI_I2SCFGR_DATLEN_0                      (0x1UL << SPI_I2SCFGR_DATLEN_Pos)                 /*!< 0x00000002 */
#define SPI_I2SCFGR_DATLEN_1                      (0x2UL << SPI_I2SCFGR_DATLEN_Pos)                 /*!< 0x00000004 */
#define SPI_I2SCFGR_CKPOL_Pos                     (3U)
#define SPI_I2SCFGR_CKPOL_Msk                     (0x1UL << SPI_I2SCFGR_CKPOL_Pos)                  /*!< 0x00000008 */
#define SPI_I2SCFGR_CKPOL                         SPI_I2SCFGR_CKPOL_Msk                             /*!< desc CKPOL */
#define SPI_I2SCFGR_I2SSTD_Pos                    (4U)
#define SPI_I2SCFGR_I2SSTD_Msk                    (0x3UL << SPI_I2SCFGR_I2SSTD_Pos)                 /*!< 0x00000030 */
#define SPI_I2SCFGR_I2SSTD                        SPI_I2SCFGR_I2SSTD_Msk                            /*!< I2SSTD[5:4] bits (desc I2SSTD) */
#define SPI_I2SCFGR_I2SSTD_0                      (0x1UL << SPI_I2SCFGR_I2SSTD_Pos)                 /*!< 0x00000010 */
#define SPI_I2SCFGR_I2SSTD_1                      (0x2UL << SPI_I2SCFGR_I2SSTD_Pos)                 /*!< 0x00000020 */
#define SPI_I2SCFGR_PCMSYNC_Pos                   (7U)
#define SPI_I2SCFGR_PCMSYNC_Msk                   (0x1UL << SPI_I2SCFGR_PCMSYNC_Pos)                /*!< 0x00000080 */
#define SPI_I2SCFGR_PCMSYNC                       SPI_I2SCFGR_PCMSYNC_Msk                           /*!< desc PCMSYNC */
#define SPI_I2SCFGR_I2SCFG_Pos                    (8U)
#define SPI_I2SCFGR_I2SCFG_Msk                    (0x3UL << SPI_I2SCFGR_I2SCFG_Pos)                 /*!< 0x00000300 */
#define SPI_I2SCFGR_I2SCFG                        SPI_I2SCFGR_I2SCFG_Msk                            /*!< I2SCFG[9:8] bits (desc I2SCFG) */
#define SPI_I2SCFGR_I2SCFG_0                      (0x1UL << SPI_I2SCFGR_I2SCFG_Pos)                 /*!< 0x00000100 */
#define SPI_I2SCFGR_I2SCFG_1                      (0x2UL << SPI_I2SCFGR_I2SCFG_Pos)                 /*!< 0x00000200 */
#define SPI_I2SCFGR_I2SE_Pos                      (10U)
#define SPI_I2SCFGR_I2SE_Msk                      (0x1UL << SPI_I2SCFGR_I2SE_Pos)                   /*!< 0x00000400 */
#define SPI_I2SCFGR_I2SE                          SPI_I2SCFGR_I2SE_Msk                              /*!< desc I2SE */
#define SPI_I2SCFGR_I2SMOD_Pos                    (11U)
#define SPI_I2SCFGR_I2SMOD_Msk                    (0x1UL << SPI_I2SCFGR_I2SMOD_Pos)                 /*!< 0x00000800 */
#define SPI_I2SCFGR_I2SMOD                        SPI_I2SCFGR_I2SMOD_Msk                            /*!< desc I2SMOD */

/*!< SPI_I2SPR */
#define SPI_I2SPR_I2SDIV_Pos                      (0U)
#define SPI_I2SPR_I2SDIV_Msk                      (0xFFUL << SPI_I2SPR_I2SDIV_Pos)                  /*!< 0x000000FF */
#define SPI_I2SPR_I2SDIV                          SPI_I2SPR_I2SDIV_Msk                              /*!< I2SDIV[7:0] bits (desc I2SDIV) */
#define SPI_I2SPR_ODD_Pos                         (8U)
#define SPI_I2SPR_ODD_Msk                         (0x1UL << SPI_I2SPR_ODD_Pos)                      /*!< 0x00000100 */
#define SPI_I2SPR_ODD                             SPI_I2SPR_ODD_Msk                                 /*!< desc ODD */
#define SPI_I2SPR_MCKOE_Pos                       (9U)
#define SPI_I2SPR_MCKOE_Msk                       (0x1UL << SPI_I2SPR_MCKOE_Pos)                    /*!< 0x00000200 */
#define SPI_I2SPR_MCKOE                           SPI_I2SPR_MCKOE_Msk                               /*!< desc MCKOE */

#define SPI_I2S_SUPPORT       /*!< I2S support */

/******************************************************************************/
/*                                                                            */
/*                       System Configuration (SYSCFG)                        */
/*                                                                            */
/******************************************************************************/
/*****************  Bit definition for SYSCFG_CFGR1 register  *****************/
#define SYSCFG_CFGR1_MEM_MODE_Pos                 (0U)
#define SYSCFG_CFGR1_MEM_MODE_Msk                 (0x3UL << SYSCFG_CFGR1_MEM_MODE_Pos)     /*!< 0x00000003 */
#define SYSCFG_CFGR1_MEM_MODE                     SYSCFG_CFGR1_MEM_MODE_Msk                /*!< SYSCFG_Memory Remap Config */
#define SYSCFG_CFGR1_MEM_MODE_0                   (0x1UL << SYSCFG_CFGR1_MEM_MODE_Pos)     /*!< 0x00000001 */
#define SYSCFG_CFGR1_MEM_MODE_1                   (0x2UL << SYSCFG_CFGR1_MEM_MODE_Pos)     /*!< 0x00000002 */
#define SYSCFG_CFGR1_TIM1_IC1_SRC_Pos             (2U)
#define SYSCFG_CFGR1_TIM1_IC1_SRC_Msk             (0x3UL << SYSCFG_CFGR1_TIM1_IC1_SRC_Pos) /*!< 0x0000000C */
#define SYSCFG_CFGR1_TIM1_IC1_SRC                 SYSCFG_CFGR1_TIM1_IC1_SRC_Msk            /*!< TIM1 IC1 SRC */
#define SYSCFG_CFGR1_TIM1_IC1_SRC_0               (0x1UL << SYSCFG_CFGR1_TIM1_IC1_SRC_Pos) /*!< 0x00000004 */
#define SYSCFG_CFGR1_TIM1_IC1_SRC_1               (0x2UL << SYSCFG_CFGR1_TIM1_IC1_SRC_Pos) /*!< 0x00000008 */
#define SYSCFG_CFGR1_TIM2_IC4_SRC_Pos             (4U)
#define SYSCFG_CFGR1_TIM2_IC4_SRC_Msk             (0x3UL << SYSCFG_CFGR1_TIM2_IC4_SRC_Pos) /*!< 0x00000030 */
#define SYSCFG_CFGR1_TIM2_IC4_SRC                 SYSCFG_CFGR1_TIM2_IC4_SRC_Msk            /*!< TIM2 IC4 SRC */
#define SYSCFG_CFGR1_TIM2_IC4_SRC_0               (0x1UL << SYSCFG_CFGR1_TIM2_IC4_SRC_Pos) /*!< 0x00000010 */
#define SYSCFG_CFGR1_TIM2_IC4_SRC_1               (0x2UL << SYSCFG_CFGR1_TIM2_IC4_SRC_Pos) /*!< 0x00000020 */
#define SYSCFG_CFGR1_TIM3_IC1_SRC_Pos             (6U)
#define SYSCFG_CFGR1_TIM3_IC1_SRC_Msk             (0x3UL << SYSCFG_CFGR1_TIM3_IC1_SRC_Pos) /*!< 0x00000C00 */
#define SYSCFG_CFGR1_TIM3_IC1_SRC                 SYSCFG_CFGR1_TIM3_IC1_SRC_Msk            /*!< TIM3 IC1 SRC */
#define SYSCFG_CFGR1_TIM3_IC1_SRC_0               (0x1UL << SYSCFG_CFGR1_TIM3_IC1_SRC_Pos) /*!< 0x00000040 */
#define SYSCFG_CFGR1_TIM3_IC1_SRC_1               (0x2UL << SYSCFG_CFGR1_TIM3_IC1_SRC_Pos) /*!< 0x00000080 */
#define SYSCFG_CFGR1_ETR_SRC_TIM1_Pos             (8U)
#define SYSCFG_CFGR1_ETR_SRC_TIM1_Msk             (0x7UL << SYSCFG_CFGR1_ETR_SRC_TIM1_Pos) /*!< 0x00000700 */
#define SYSCFG_CFGR1_ETR_SRC_TIM1                 SYSCFG_CFGR1_ETR_SRC_TIM1_Msk            /*!< ETR SRC TIM1 */
#define SYSCFG_CFGR1_ETR_SRC_TIM1_0               (0x1UL << SYSCFG_CFGR1_ETR_SRC_TIM1_Pos) /*!< 0x00000100 */
#define SYSCFG_CFGR1_ETR_SRC_TIM1_1               (0x2UL << SYSCFG_CFGR1_ETR_SRC_TIM1_Pos) /*!< 0x00000200 */
#define SYSCFG_CFGR1_ETR_SRC_TIM1_2               (0x4UL << SYSCFG_CFGR1_ETR_SRC_TIM1_Pos) /*!< 0x00000400 */
#define SYSCFG_CFGR1_ETR_SRC_TIM2_Pos             (12U)
#define SYSCFG_CFGR1_ETR_SRC_TIM2_Msk             (0x7UL << SYSCFG_CFGR1_ETR_SRC_TIM2_Pos) /*!< 0x00001C00 */
#define SYSCFG_CFGR1_ETR_SRC_TIM2                 SYSCFG_CFGR1_ETR_SRC_TIM2_Msk            /*!< ETR SRC TIM2 */
#define SYSCFG_CFGR1_ETR_SRC_TIM2_0               (0x1UL << SYSCFG_CFGR1_ETR_SRC_TIM2_Pos) /*!< 0x00000400 */
#define SYSCFG_CFGR1_ETR_SRC_TIM2_1               (0x2UL << SYSCFG_CFGR1_ETR_SRC_TIM2_Pos) /*!< 0x00000800 */
#define SYSCFG_CFGR1_ETR_SRC_TIM2_2               (0x4UL << SYSCFG_CFGR1_ETR_SRC_TIM2_Pos) /*!< 0x00001000 */
#define SYSCFG_CFGR1_ETR_SRC_TIM3_Pos             (16U)
#define SYSCFG_CFGR1_ETR_SRC_TIM3_Msk             (0x7UL << SYSCFG_CFGR1_ETR_SRC_TIM3_Pos) /*!< 0x00070000 */
#define SYSCFG_CFGR1_ETR_SRC_TIM3                 SYSCFG_CFGR1_ETR_SRC_TIM3_Msk            /*!< ETR SRC TIM3 */
#define SYSCFG_CFGR1_ETR_SRC_TIM3_0               (0x1UL << SYSCFG_CFGR1_ETR_SRC_TIM3_Pos) /*!< 0x00010000 */
#define SYSCFG_CFGR1_ETR_SRC_TIM3_1               (0x2UL << SYSCFG_CFGR1_ETR_SRC_TIM3_Pos) /*!< 0x00020000 */
#define SYSCFG_CFGR1_ETR_SRC_TIM3_2               (0x4UL << SYSCFG_CFGR1_ETR_SRC_TIM3_Pos) /*!< 0x00040000 */
#define SYSCFG_CFGR1_GPIO_AHB_SEL_Pos             (24U)
#define SYSCFG_CFGR1_GPIO_AHB_SEL_Msk             (0x1UL << SYSCFG_CFGR1_GPIO_AHB_SEL_Pos) /*!< 0x01000000 */
#define SYSCFG_CFGR1_GPIO_AHB_SEL                 SYSCFG_CFGR1_GPIO_AHB_SEL_Msk            /*!< GPIO AHB SEL */

/******************  Bit definition for SYSCFG_CFGR2 register  ****************/
#define SYSCFG_CFGR2_LOCKUP_LOCK_Pos                   (0U)
#define SYSCFG_CFGR2_LOCKUP_LOCK_Msk                   (0x1UL << SYSCFG_CFGR2_LOCKUP_LOCK_Pos)      /*!< 0x00000001 */
#define SYSCFG_CFGR2_LOCKUP_LOCK                       SYSCFG_CFGR2_LOCKUP_LOCK_Msk                 /*!< Enables and locks the LOCKUP_LOCK (Hardfault) output of CortexM0 with Break Input of TIMER1 */
#define SYSCFG_CFGR2_PVD_LOCK_Pos                      (2U)
#define SYSCFG_CFGR2_PVD_LOCK_Msk                      (0x1UL << SYSCFG_CFGR2_PVD_LOCK_Pos)         /*!< 0x00000004 */
#define SYSCFG_CFGR2_PVD_LOCK                          SYSCFG_CFGR2_PVD_LOCK_Msk                    /*!< Enables and locks the PVD connection with Timer1 Break Input and also the PVD_EN and PVDSEL[2:0] bits of the Power Control Interface */
#define SYSCFG_CFGR2_COMP1_BRK_TIM1_Pos                (3U)
#define SYSCFG_CFGR2_COMP1_BRK_TIM1_Msk                (0x1UL << SYSCFG_CFGR2_COMP1_BRK_TIM1_Pos)   /*!< 0x00000008 */
#define SYSCFG_CFGR2_COMP1_BRK_TIM1                    SYSCFG_CFGR2_COMP1_BRK_TIM1_Msk              /*!< COMP1_BRK_TIM1 */
#define SYSCFG_CFGR2_COMP2_BRK_TIM1_Pos                (4U)
#define SYSCFG_CFGR2_COMP2_BRK_TIM1_Msk                (0x1UL << SYSCFG_CFGR2_COMP2_BRK_TIM1_Pos)   /*!< 0x00000010 */
#define SYSCFG_CFGR2_COMP2_BRK_TIM1                    SYSCFG_CFGR2_COMP2_BRK_TIM1_Msk              /*!< COMP2_BRK_TIM1 */
#define SYSCFG_CFGR2_COMP3_BRK_TIM1_Pos                (5U)
#define SYSCFG_CFGR2_COMP3_BRK_TIM1_Msk                (0x1UL << SYSCFG_CFGR2_COMP3_BRK_TIM1_Pos)   /*!< 0x00000020 */
#define SYSCFG_CFGR2_COMP3_BRK_TIM1                    SYSCFG_CFGR2_COMP3_BRK_TIM1_Msk              /*!< COMP3_BRK_TIM1 */
#define SYSCFG_CFGR2_COMP1_BRK_TIM15_Pos               (6U)
#define SYSCFG_CFGR2_COMP1_BRK_TIM15_Msk               (0x1UL << SYSCFG_CFGR2_COMP1_BRK_TIM15_Pos)  /*!< 0x00000040 */
#define SYSCFG_CFGR2_COMP1_BRK_TIM15                   SYSCFG_CFGR2_COMP1_BRK_TIM15_Msk             /*!< COMP1_BRK_TIM15 */
#define SYSCFG_CFGR2_COMP2_BRK_TIM15_Pos               (7U)
#define SYSCFG_CFGR2_COMP2_BRK_TIM15_Msk               (0x1UL << SYSCFG_CFGR2_COMP2_BRK_TIM15_Pos)  /*!< 0x00000080 */
#define SYSCFG_CFGR2_COMP2_BRK_TIM15                   SYSCFG_CFGR2_COMP2_BRK_TIM15_Msk             /*!< COMP2_BRK_TIM15 */
#define SYSCFG_CFGR2_COMP3_BRK_TIM15_Pos               (8U)
#define SYSCFG_CFGR2_COMP3_BRK_TIM15_Msk               (0x1UL << SYSCFG_CFGR2_COMP3_BRK_TIM15_Pos)  /*!< 0x00000100 */
#define SYSCFG_CFGR2_COMP3_BRK_TIM15                   SYSCFG_CFGR2_COMP3_BRK_TIM15_Msk             /*!< COMP3_BRK_TIM15 */
#define SYSCFG_CFGR2_COMP1_BRK_TIM16_Pos               (9U)
#define SYSCFG_CFGR2_COMP1_BRK_TIM16_Msk               (0x1UL << SYSCFG_CFGR2_COMP1_BRK_TIM16_Pos)  /*!< 0x00000200 */
#define SYSCFG_CFGR2_COMP1_BRK_TIM16                   SYSCFG_CFGR2_COMP1_BRK_TIM16_Msk             /*!< COMP1_BRK_TIM16 */
#define SYSCFG_CFGR2_COMP2_BRK_TIM16_Pos               (10U)
#define SYSCFG_CFGR2_COMP2_BRK_TIM16_Msk               (0x1UL << SYSCFG_CFGR2_COMP2_BRK_TIM16_Pos)  /*!< 0x00000400 */
#define SYSCFG_CFGR2_COMP2_BRK_TIM16                   SYSCFG_CFGR2_COMP2_BRK_TIM16_Msk             /*!< COMP2_BRK_TIM16 */
#define SYSCFG_CFGR2_COMP3_BRK_TIM16_Pos               (11U)
#define SYSCFG_CFGR2_COMP3_BRK_TIM16_Msk               (0x1UL << SYSCFG_CFGR2_COMP3_BRK_TIM16_Pos)  /*!< 0x00000800 */
#define SYSCFG_CFGR2_COMP3_BRK_TIM16                   SYSCFG_CFGR2_COMP3_BRK_TIM16_Msk             /*!< COMP3_BRK_TIM16 */
#define SYSCFG_CFGR2_COMP1_BRK_TIM17_Pos               (12U)
#define SYSCFG_CFGR2_COMP1_BRK_TIM17_Msk               (0x1UL << SYSCFG_CFGR2_COMP1_BRK_TIM17_Pos)  /*!< 0x00001000 */
#define SYSCFG_CFGR2_COMP1_BRK_TIM17                   SYSCFG_CFGR2_COMP1_BRK_TIM17_Msk             /*!< COMP1_BRK_TIM17 */
#define SYSCFG_CFGR2_COMP2_BRK_TIM17_Pos               (13U)
#define SYSCFG_CFGR2_COMP2_BRK_TIM17_Msk               (0x1UL << SYSCFG_CFGR2_COMP2_BRK_TIM17_Pos)  /*!< 0x00002000 */
#define SYSCFG_CFGR2_COMP2_BRK_TIM17                   SYSCFG_CFGR2_COMP2_BRK_TIM17_Msk             /*!< COMP2_BRK_TIM17 */
#define SYSCFG_CFGR2_COMP3_BRK_TIM17_Pos               (14U)
#define SYSCFG_CFGR2_COMP3_BRK_TIM17_Msk               (0x1UL << SYSCFG_CFGR2_COMP3_BRK_TIM17_Pos)  /*!< 0x00004000 */
#define SYSCFG_CFGR2_COMP3_BRK_TIM17                   SYSCFG_CFGR2_COMP3_BRK_TIM17_Msk             /*!< COMP3_BRK_TIM17 */
#define SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM1_Pos          (15U)
#define SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM1_Msk          (0x1UL << SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM1_Pos)   /*!< 0x00008000 */
#define SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM1              SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM1_Msk              /*!< COMP1_OCREF_CLR_TIM1 */
#define SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM2_Pos          (16U)
#define SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM2_Msk          (0x1UL << SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM2_Pos)   /*!< 0x00010000 */
#define SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM2              SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM2_Msk              /*!< COMP1_OCREF_CLR_TIM2 */
#define SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM3_Pos          (17U)
#define SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM3_Msk          (0x1UL << SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM3_Pos)   /*!< 0x00020000 */
#define SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM3              SYSCFG_CFGR2_COMP1_OCREF_CLR_TIM3_Msk              /*!< COMP1_OCREF_CLR_TIM3 */
#define SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM1_Pos          (18U)
#define SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM1_Msk          (0x1UL << SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM1_Pos)   /*!< 0x00040000 */
#define SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM1              SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM1_Msk              /*!< COMP2_OCREF_CLR_TIM1 */
#define SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM2_Pos          (19U)
#define SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM2_Msk          (0x1UL << SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM2_Pos)   /*!< 0x00080000 */
#define SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM2              SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM2_Msk              /*!< COMP2_OCREF_CLR_TIM2 */
#define SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM3_Pos          (20U)
#define SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM3_Msk          (0x1UL << SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM3_Pos)   /*!< 0x00100000 */
#define SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM3              SYSCFG_CFGR2_COMP2_OCREF_CLR_TIM3_Msk              /*!< COMP2_OCREF_CLR_TIM3 */
#define SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM1_Pos          (21U)
#define SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM1_Msk          (0x1UL << SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM1_Pos)   /*!< 0x00200000 */
#define SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM1              SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM1_Msk              /*!< COMP3_OCREF_CLR_TIM1 */
#define SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM2_Pos          (22U)
#define SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM2_Msk          (0x1UL << SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM2_Pos)   /*!< 0x00400000 */
#define SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM2              SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM2_Msk              /*!< COMP3_OCREF_CLR_TIM2 */
#define SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM3_Pos          (23U)
#define SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM3_Msk          (0x1UL << SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM3_Pos)   /*!< 0x00800000 */
#define SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM3              SYSCFG_CFGR2_COMP3_OCREF_CLR_TIM3_Msk              /*!< COMP3_OCREF_CLR_TIM3 */

/*****************  Bit definition for SYSCFG_CFGR3 register  *****************/
#define SYSCFG_CFGR3_DMA1_MAP_Pos                  (0U)
#define SYSCFG_CFGR3_DMA1_MAP_Msk                  (0x3FUL << SYSCFG_CFGR3_DMA1_MAP_Pos)   /*!< 0x0000003F */
#define SYSCFG_CFGR3_DMA1_MAP                      SYSCFG_CFGR3_DMA1_MAP_Msk
#define SYSCFG_CFGR3_DMA1_MAP_0                    (0x1UL << SYSCFG_CFGR3_DMA1_MAP_Pos)
#define SYSCFG_CFGR3_DMA1_MAP_1                    (0x2UL << SYSCFG_CFGR3_DMA1_MAP_Pos)
#define SYSCFG_CFGR3_DMA1_MAP_2                    (0x4UL << SYSCFG_CFGR3_DMA1_MAP_Pos)
#define SYSCFG_CFGR3_DMA1_MAP_3                    (0x8UL << SYSCFG_CFGR3_DMA1_MAP_Pos)
#define SYSCFG_CFGR3_DMA1_MAP_4                    (0x10UL << SYSCFG_CFGR3_DMA1_MAP_Pos)
#define SYSCFG_CFGR3_DMA1_MAP_5                    (0x20UL << SYSCFG_CFGR3_DMA1_MAP_Pos)
#define SYSCFG_CFGR3_DMA2_MAP_Pos                  (8U)
#define SYSCFG_CFGR3_DMA2_MAP_Msk                  (0x3FUL << SYSCFG_CFGR3_DMA2_MAP_Pos)   /*!< 0x00003F00 */
#define SYSCFG_CFGR3_DMA2_MAP                      SYSCFG_CFGR3_DMA2_MAP_Msk
#define SYSCFG_CFGR3_DMA2_MAP_0                    (0x1UL << SYSCFG_CFGR3_DMA2_MAP_Pos)
#define SYSCFG_CFGR3_DMA2_MAP_1                    (0x2UL << SYSCFG_CFGR3_DMA2_MAP_Pos)
#define SYSCFG_CFGR3_DMA2_MAP_2                    (0x4UL << SYSCFG_CFGR3_DMA2_MAP_Pos)
#define SYSCFG_CFGR3_DMA2_MAP_3                    (0x8UL << SYSCFG_CFGR3_DMA2_MAP_Pos)
#define SYSCFG_CFGR3_DMA2_MAP_4                    (0x10UL << SYSCFG_CFGR3_DMA2_MAP_Pos)
#define SYSCFG_CFGR3_DMA2_MAP_5                    (0x20UL << SYSCFG_CFGR3_DMA2_MAP_Pos)
#define SYSCFG_CFGR3_DMA3_MAP_Pos                  (16U)
#define SYSCFG_CFGR3_DMA3_MAP_Msk                  (0x3FUL << SYSCFG_CFGR3_DMA3_MAP_Pos)   /*!< 0x003F0000 */
#define SYSCFG_CFGR3_DMA3_MAP                      SYSCFG_CFGR3_DMA3_MAP_Msk
#define SYSCFG_CFGR3_DMA3_MAP_0                    (0x1UL << SYSCFG_CFGR3_DMA3_MAP_Pos)
#define SYSCFG_CFGR3_DMA3_MAP_1                    (0x2UL << SYSCFG_CFGR3_DMA3_MAP_Pos)
#define SYSCFG_CFGR3_DMA3_MAP_2                    (0x4UL << SYSCFG_CFGR3_DMA3_MAP_Pos)
#define SYSCFG_CFGR3_DMA3_MAP_3                    (0x8UL << SYSCFG_CFGR3_DMA3_MAP_Pos)
#define SYSCFG_CFGR3_DMA3_MAP_4                    (0x10UL << SYSCFG_CFGR3_DMA3_MAP_Pos)
#define SYSCFG_CFGR3_DMA3_MAP_5                    (0x20UL << SYSCFG_CFGR3_DMA3_MAP_Pos)
#define SYSCFG_CFGR3_DMA4_MAP_Pos                  (24U)
#define SYSCFG_CFGR3_DMA4_MAP_Msk                  (0x3FUL << SYSCFG_CFGR3_DMA4_MAP_Pos)   /*!< 0x3F000000 */
#define SYSCFG_CFGR3_DMA4_MAP                      SYSCFG_CFGR3_DMA4_MAP_Msk
#define SYSCFG_CFGR3_DMA4_MAP_0                    (0x1UL << SYSCFG_CFGR3_DMA4_MAP_Pos)
#define SYSCFG_CFGR3_DMA4_MAP_1                    (0x2UL << SYSCFG_CFGR3_DMA4_MAP_Pos)
#define SYSCFG_CFGR3_DMA4_MAP_2                    (0x4UL << SYSCFG_CFGR3_DMA4_MAP_Pos)
#define SYSCFG_CFGR3_DMA4_MAP_3                    (0x8UL << SYSCFG_CFGR3_DMA4_MAP_Pos)
#define SYSCFG_CFGR3_DMA4_MAP_4                    (0x10UL << SYSCFG_CFGR3_DMA4_MAP_Pos)
#define SYSCFG_CFGR3_DMA4_MAP_5                    (0x20UL << SYSCFG_CFGR3_DMA4_MAP_Pos)

/*****************  Bit definition for SYSCFG_CFGR4 register  *****************/
#define SYSCFG_CFGR4_DMA5_MAP_Pos                  (0U)
#define SYSCFG_CFGR4_DMA5_MAP_Msk                  (0x3FUL << SYSCFG_CFGR4_DMA5_MAP_Pos)   /*!< 0x0000003F */
#define SYSCFG_CFGR4_DMA5_MAP                      SYSCFG_CFGR4_DMA5_MAP_Msk
#define SYSCFG_CFGR4_DMA5_MAP_0                    (0x1UL << SYSCFG_CFGR4_DMA5_MAP_Pos)
#define SYSCFG_CFGR4_DMA5_MAP_1                    (0x2UL << SYSCFG_CFGR4_DMA5_MAP_Pos)
#define SYSCFG_CFGR4_DMA5_MAP_2                    (0x4UL << SYSCFG_CFGR4_DMA5_MAP_Pos)
#define SYSCFG_CFGR4_DMA5_MAP_3                    (0x8UL << SYSCFG_CFGR4_DMA5_MAP_Pos)
#define SYSCFG_CFGR4_DMA5_MAP_4                    (0x10UL << SYSCFG_CFGR4_DMA5_MAP_Pos)
#define SYSCFG_CFGR4_DMA5_MAP_5                    (0x20UL << SYSCFG_CFGR4_DMA5_MAP_Pos)
#define SYSCFG_CFGR4_DMA6_MAP_Pos                  (8U)
#define SYSCFG_CFGR4_DMA6_MAP_Msk                  (0x3FUL << SYSCFG_CFGR4_DMA6_MAP_Pos)   /*!< 0x00003F00 */
#define SYSCFG_CFGR4_DMA6_MAP                      SYSCFG_CFGR4_DMA6_MAP_Msk
#define SYSCFG_CFGR4_DMA6_MAP_0                    (0x1UL << SYSCFG_CFGR4_DMA6_MAP_Pos)
#define SYSCFG_CFGR4_DMA6_MAP_1                    (0x2UL << SYSCFG_CFGR4_DMA6_MAP_Pos)
#define SYSCFG_CFGR4_DMA6_MAP_2                    (0x4UL << SYSCFG_CFGR4_DMA6_MAP_Pos)
#define SYSCFG_CFGR4_DMA6_MAP_3                    (0x8UL << SYSCFG_CFGR4_DMA6_MAP_Pos)
#define SYSCFG_CFGR4_DMA6_MAP_4                    (0x10UL << SYSCFG_CFGR4_DMA6_MAP_Pos)
#define SYSCFG_CFGR4_DMA6_MAP_5                    (0x20UL << SYSCFG_CFGR4_DMA6_MAP_Pos)
#define SYSCFG_CFGR4_DMA7_MAP_Pos                  (16U)
#define SYSCFG_CFGR4_DMA7_MAP_Msk                  (0x3FUL << SYSCFG_CFGR4_DMA7_MAP_Pos)   /*!< 0x003F0000 */
#define SYSCFG_CFGR4_DMA7_MAP                      SYSCFG_CFGR4_DMA7_MAP_Msk
#define SYSCFG_CFGR4_DMA7_MAP_0                    (0x1UL << SYSCFG_CFGR4_DMA7_MAP_Pos)
#define SYSCFG_CFGR4_DMA7_MAP_1                    (0x2UL << SYSCFG_CFGR4_DMA7_MAP_Pos)
#define SYSCFG_CFGR4_DMA7_MAP_2                    (0x4UL << SYSCFG_CFGR4_DMA7_MAP_Pos)
#define SYSCFG_CFGR4_DMA7_MAP_3                    (0x8UL << SYSCFG_CFGR4_DMA7_MAP_Pos)
#define SYSCFG_CFGR4_DMA7_MAP_4                    (0x10UL << SYSCFG_CFGR4_DMA7_MAP_Pos)
#define SYSCFG_CFGR4_DMA7_MAP_5                    (0x20UL << SYSCFG_CFGR4_DMA7_MAP_Pos)

/*****************  Bit definition for PAENS register  ************************/
#define SYSCFG_PAENS_PA_ENS_Pos                   (0U)
#define SYSCFG_PAENS_PA_ENS_Msk                   (0xFFFFUL << SYSCFG_PAENS_PA_ENS_Pos)             /*!< 0x0000FFFF */
#define SYSCFG_PAENS_PA_ENS                       SYSCFG_PAENS_PA_ENS_Msk                           /*!< PA_ENS[15:0] bits (desc PA_ENS) */

/*****************  Bit definition for PBENS register  ************************/
#define SYSCFG_PBENS_PB_ENS_Pos                   (0U)
#define SYSCFG_PBENS_PB_ENS_Msk                   (0xFFFFUL << SYSCFG_PBENS_PB_ENS_Pos)             /*!< 0x0000FFFF */
#define SYSCFG_PBENS_PB_ENS                       SYSCFG_PBENS_PB_ENS_Msk                           /*!< PB_ENS[15:0] bits (desc PB_ENS) */

/*****************  Bit definition for PCENS register  ************************/
#define SYSCFG_PCENS_PC_ENS_Pos                   (0U)
#define SYSCFG_PCENS_PC_ENS_Msk                   (0xFFFFUL << SYSCFG_PCENS_PC_ENS_Pos)             /*!< 0x0000FFFF */
#define SYSCFG_PCENS_PC_ENS                       SYSCFG_PCENS_PC_ENS_Msk                           /*!< PC_ENS[15:0] bits (desc PC_ENS) */

/*****************  Bit definition for PFENS register  ************************/
#define SYSCFG_PFENS_PF_ENS_Pos                   (0U)
#define SYSCFG_PFENS_PF_ENS_Msk                   (0xFFFFUL << SYSCFG_PFENS_PF_ENS_Pos)             /*!< 0x0000FFFF */
#define SYSCFG_PFENS_PF_ENS                       SYSCFG_PFENS_PF_ENS_Msk                           /*!< PF_ENS[15:0] bits (desc PF_ENS) */

/*****************  Bit definition for SYSCFG_EIIC register  ******************/
#define SYSCFG_EIIC_PA_EIIC_Pos            (0U)
#define SYSCFG_EIIC_PA_EIIC_Msk            (0x3UL << SYSCFG_EIIC_PA_EIIC_Pos)     /*!< 0x00000003 */
#define SYSCFG_EIIC_PA_EIIC                SYSCFG_EIIC_PA_EIIC_Msk
#define SYSCFG_EIIC_PA_EIIC_0              (0x1UL << SYSCFG_EIIC_PA_EIIC_Pos)
#define SYSCFG_EIIC_PA_EIIC_1              (0x2UL << SYSCFG_EIIC_PA_EIIC_Pos)
#define SYSCFG_EIIC_PB_EIIC_Pos            (8U)
#define SYSCFG_EIIC_PB_EIIC_Msk            (0x1FFUL << SYSCFG_EIIC_PB_EIIC_Pos)   /*!< 0x0001FF00 */
#define SYSCFG_EIIC_PB_EIIC                SYSCFG_EIIC_PB_EIIC_Msk
#define SYSCFG_EIIC_PB_EIIC_0              (0x1UL << SYSCFG_EIIC_PB_EIIC_Pos)
#define SYSCFG_EIIC_PB_EIIC_1              (0x2UL << SYSCFG_EIIC_PB_EIIC_Pos)
#define SYSCFG_EIIC_PB_EIIC_2              (0x4UL << SYSCFG_EIIC_PB_EIIC_Pos)
#define SYSCFG_EIIC_PB_EIIC_3              (0x8UL << SYSCFG_EIIC_PB_EIIC_Pos)
#define SYSCFG_EIIC_PB_EIIC_4              (0x10UL << SYSCFG_EIIC_PB_EIIC_Pos)
#define SYSCFG_EIIC_PB_EIIC_5              (0x20UL << SYSCFG_EIIC_PB_EIIC_Pos)
#define SYSCFG_EIIC_PB_EIIC_6              (0x40UL << SYSCFG_EIIC_PB_EIIC_Pos)
#define SYSCFG_EIIC_PB_EIIC_7              (0x80UL << SYSCFG_EIIC_PB_EIIC_Pos)
#define SYSCFG_EIIC_PB_EIIC_8              (0x100UL << SYSCFG_EIIC_PB_EIIC_Pos)
#define SYSCFG_EIIC_PF_EIIC_Pos            (24U)
#define SYSCFG_EIIC_PF_EIIC_Msk            (0x3UL << SYSCFG_EIIC_PF_EIIC_Pos)     /*!< 0x03000000 */
#define SYSCFG_EIIC_PF_EIIC                SYSCFG_EIIC_PF_EIIC_Msk
#define SYSCFG_EIIC_PF_EIIC_0              (0x1UL << SYSCFG_EIIC_PF_EIIC_Pos)
#define SYSCFG_EIIC_PF_EIIC_1              (0x2UL << SYSCFG_EIIC_PF_EIIC_Pos)

/******************************************************************************/
/*                                                                            */
/*                               Timers (TIM)                                 */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for TIM_CR1 register  ********************/
#define TIM_CR1_CEN_Pos           (0U)
#define TIM_CR1_CEN_Msk           (0x1UL << TIM_CR1_CEN_Pos)                    /*!< 0x00000001 */
#define TIM_CR1_CEN               TIM_CR1_CEN_Msk                               /*!<Counter enable */
#define TIM_CR1_UDIS_Pos          (1U)
#define TIM_CR1_UDIS_Msk          (0x1UL << TIM_CR1_UDIS_Pos)                   /*!< 0x00000002 */
#define TIM_CR1_UDIS              TIM_CR1_UDIS_Msk                              /*!<Update disable */
#define TIM_CR1_URS_Pos           (2U)
#define TIM_CR1_URS_Msk           (0x1UL << TIM_CR1_URS_Pos)                    /*!< 0x00000004 */
#define TIM_CR1_URS               TIM_CR1_URS_Msk                               /*!<Update request source */
#define TIM_CR1_OPM_Pos           (3U)
#define TIM_CR1_OPM_Msk           (0x1UL << TIM_CR1_OPM_Pos)                    /*!< 0x00000008 */
#define TIM_CR1_OPM               TIM_CR1_OPM_Msk                               /*!<One pulse mode */
#define TIM_CR1_DIR_Pos           (4U)
#define TIM_CR1_DIR_Msk           (0x1UL << TIM_CR1_DIR_Pos)                    /*!< 0x00000010 */
#define TIM_CR1_DIR               TIM_CR1_DIR_Msk                               /*!<Direction */

#define TIM_CR1_CMS_Pos           (5U)
#define TIM_CR1_CMS_Msk           (0x3UL << TIM_CR1_CMS_Pos)                    /*!< 0x00000060 */
#define TIM_CR1_CMS               TIM_CR1_CMS_Msk                               /*!<CMS[1:0] bits (Center-aligned mode selection) */
#define TIM_CR1_CMS_0             (0x1UL << TIM_CR1_CMS_Pos)                    /*!< 0x00000020 */
#define TIM_CR1_CMS_1             (0x2UL << TIM_CR1_CMS_Pos)                    /*!< 0x00000040 */

#define TIM_CR1_ARPE_Pos          (7U)
#define TIM_CR1_ARPE_Msk          (0x1UL << TIM_CR1_ARPE_Pos)                   /*!< 0x00000080 */
#define TIM_CR1_ARPE              TIM_CR1_ARPE_Msk                              /*!<Auto-reload preload enable */

#define TIM_CR1_CKD_Pos           (8U)
#define TIM_CR1_CKD_Msk           (0x3UL << TIM_CR1_CKD_Pos)                    /*!< 0x00000300 */
#define TIM_CR1_CKD               TIM_CR1_CKD_Msk                               /*!<CKD[1:0] bits (clock division) */
#define TIM_CR1_CKD_0             (0x1UL << TIM_CR1_CKD_Pos)                    /*!< 0x00000100 */
#define TIM_CR1_CKD_1             (0x2UL << TIM_CR1_CKD_Pos)                    /*!< 0x00000200 */

/*******************  Bit definition for TIM_CR2 register  ********************/
#define TIM_CR2_CCPC_Pos          (0U)
#define TIM_CR2_CCPC_Msk          (0x1UL << TIM_CR2_CCPC_Pos)                   /*!< 0x00000001 */
#define TIM_CR2_CCPC              TIM_CR2_CCPC_Msk                              /*!<Capture/Compare Preloaded Control */
#define TIM_CR2_CCUS_Pos          (2U)
#define TIM_CR2_CCUS_Msk          (0x1UL << TIM_CR2_CCUS_Pos)                   /*!< 0x00000004 */
#define TIM_CR2_CCUS              TIM_CR2_CCUS_Msk                              /*!<Capture/Compare Control Update Selection */
#define TIM_CR2_CCDS_Pos          (3U)
#define TIM_CR2_CCDS_Msk          (0x1UL << TIM_CR2_CCDS_Pos)                   /*!< 0x00000008 */
#define TIM_CR2_CCDS              TIM_CR2_CCDS_Msk                              /*!<Capture/Compare DMA Selection */

#define TIM_CR2_MMS_Pos           (4U)
#define TIM_CR2_MMS_Msk           (0x7UL << TIM_CR2_MMS_Pos)                    /*!< 0x00000070 */
#define TIM_CR2_MMS               TIM_CR2_MMS_Msk                               /*!<MMS[2:0] bits (Master Mode Selection) */
#define TIM_CR2_MMS_0             (0x1UL << TIM_CR2_MMS_Pos)                    /*!< 0x00000010 */
#define TIM_CR2_MMS_1             (0x2UL << TIM_CR2_MMS_Pos)                    /*!< 0x00000020 */
#define TIM_CR2_MMS_2             (0x4UL << TIM_CR2_MMS_Pos)                    /*!< 0x00000040 */

#define TIM_CR2_TI1S_Pos          (7U)
#define TIM_CR2_TI1S_Msk          (0x1UL << TIM_CR2_TI1S_Pos)                   /*!< 0x00000080 */
#define TIM_CR2_TI1S              TIM_CR2_TI1S_Msk                              /*!<TI1 Selection */
#define TIM_CR2_OIS1_Pos          (8U)
#define TIM_CR2_OIS1_Msk          (0x1UL << TIM_CR2_OIS1_Pos)                   /*!< 0x00000100 */
#define TIM_CR2_OIS1              TIM_CR2_OIS1_Msk                              /*!<Output Idle state 1 (OC1 output) */
#define TIM_CR2_OIS1N_Pos         (9U)
#define TIM_CR2_OIS1N_Msk         (0x1UL << TIM_CR2_OIS1N_Pos)                  /*!< 0x00000200 */
#define TIM_CR2_OIS1N             TIM_CR2_OIS1N_Msk                             /*!<Output Idle state 1 (OC1N output) */
#define TIM_CR2_OIS2_Pos          (10U)
#define TIM_CR2_OIS2_Msk          (0x1UL << TIM_CR2_OIS2_Pos)                   /*!< 0x00000400 */
#define TIM_CR2_OIS2              TIM_CR2_OIS2_Msk                              /*!<Output Idle state 2 (OC2 output) */
#define TIM_CR2_OIS2N_Pos         (11U)
#define TIM_CR2_OIS2N_Msk         (0x1UL << TIM_CR2_OIS2N_Pos)                  /*!< 0x00000800 */
#define TIM_CR2_OIS2N             TIM_CR2_OIS2N_Msk                             /*!<Output Idle state 2 (OC2N output) */
#define TIM_CR2_OIS3_Pos          (12U)
#define TIM_CR2_OIS3_Msk          (0x1UL << TIM_CR2_OIS3_Pos)                   /*!< 0x00001000 */
#define TIM_CR2_OIS3              TIM_CR2_OIS3_Msk                              /*!<Output Idle state 3 (OC3 output) */
#define TIM_CR2_OIS3N_Pos         (13U)
#define TIM_CR2_OIS3N_Msk         (0x1UL << TIM_CR2_OIS3N_Pos)                  /*!< 0x00002000 */
#define TIM_CR2_OIS3N             TIM_CR2_OIS3N_Msk                             /*!<Output Idle state 3 (OC3N output) */
#define TIM_CR2_OIS4_Pos          (14U)
#define TIM_CR2_OIS4_Msk          (0x1UL << TIM_CR2_OIS4_Pos)                   /*!< 0x00004000 */
#define TIM_CR2_OIS4              TIM_CR2_OIS4_Msk                              /*!<Output Idle state 4 (OC4 output) */

/*******************  Bit definition for TIM_SMCR register  *******************/
#define TIM_SMCR_SMS_Pos          (0U)
#define TIM_SMCR_SMS_Msk          (0x7UL << TIM_SMCR_SMS_Pos)                   /*!< 0x00000007 */
#define TIM_SMCR_SMS              TIM_SMCR_SMS_Msk                              /*!<SMS[2:0] bits (Slave mode selection) */
#define TIM_SMCR_SMS_0            (0x1UL << TIM_SMCR_SMS_Pos)                   /*!< 0x00000001 */
#define TIM_SMCR_SMS_1            (0x2UL << TIM_SMCR_SMS_Pos)                   /*!< 0x00000002 */
#define TIM_SMCR_SMS_2            (0x4UL << TIM_SMCR_SMS_Pos)                   /*!< 0x00000004 */

#define TIM_SMCR_OCCS_Pos         (3U)
#define TIM_SMCR_OCCS_Msk         (0x1UL << TIM_SMCR_OCCS_Pos)                  /*!< 0x00000008 */
#define TIM_SMCR_OCCS             TIM_SMCR_OCCS_Msk                             /*!< OCREF clear selection */

#define TIM_SMCR_TS_Pos           (4U)
#define TIM_SMCR_TS_Msk           (0x7UL << TIM_SMCR_TS_Pos)                    /*!< 0x00000070 */
#define TIM_SMCR_TS               TIM_SMCR_TS_Msk                               /*!<TS[2:0] bits (Trigger selection) */
#define TIM_SMCR_TS_0             (0x1UL << TIM_SMCR_TS_Pos)                    /*!< 0x00000010 */
#define TIM_SMCR_TS_1             (0x2UL << TIM_SMCR_TS_Pos)                    /*!< 0x00000020 */
#define TIM_SMCR_TS_2             (0x4UL << TIM_SMCR_TS_Pos)                    /*!< 0x00000040 */

#define TIM_SMCR_MSM_Pos          (7U)
#define TIM_SMCR_MSM_Msk          (0x1UL << TIM_SMCR_MSM_Pos)                   /*!< 0x00000080 */
#define TIM_SMCR_MSM              TIM_SMCR_MSM_Msk                              /*!<Master/slave mode */

#define TIM_SMCR_ETF_Pos          (8U)
#define TIM_SMCR_ETF_Msk          (0xFUL << TIM_SMCR_ETF_Pos)                   /*!< 0x00000F00 */
#define TIM_SMCR_ETF              TIM_SMCR_ETF_Msk                              /*!<ETF[3:0] bits (External trigger filter) */
#define TIM_SMCR_ETF_0            (0x1UL << TIM_SMCR_ETF_Pos)                   /*!< 0x00000100 */
#define TIM_SMCR_ETF_1            (0x2UL << TIM_SMCR_ETF_Pos)                   /*!< 0x00000200 */
#define TIM_SMCR_ETF_2            (0x4UL << TIM_SMCR_ETF_Pos)                   /*!< 0x00000400 */
#define TIM_SMCR_ETF_3            (0x8UL << TIM_SMCR_ETF_Pos)                   /*!< 0x00000800 */

#define TIM_SMCR_ETPS_Pos         (12U)
#define TIM_SMCR_ETPS_Msk         (0x3UL << TIM_SMCR_ETPS_Pos)                  /*!< 0x00003000 */
#define TIM_SMCR_ETPS             TIM_SMCR_ETPS_Msk                             /*!<ETPS[1:0] bits (External trigger prescaler) */
#define TIM_SMCR_ETPS_0           (0x1UL << TIM_SMCR_ETPS_Pos)                  /*!< 0x00001000 */
#define TIM_SMCR_ETPS_1           (0x2UL << TIM_SMCR_ETPS_Pos)                  /*!< 0x00002000 */

#define TIM_SMCR_ECE_Pos          (14U)
#define TIM_SMCR_ECE_Msk          (0x1UL << TIM_SMCR_ECE_Pos)                   /*!< 0x00004000 */
#define TIM_SMCR_ECE              TIM_SMCR_ECE_Msk                              /*!<External clock enable */
#define TIM_SMCR_ETP_Pos          (15U)
#define TIM_SMCR_ETP_Msk          (0x1UL << TIM_SMCR_ETP_Pos)                   /*!< 0x00008000 */
#define TIM_SMCR_ETP              TIM_SMCR_ETP_Msk                              /*!<External trigger polarity */

/*******************  Bit definition for TIM_DIER register  *******************/
#define TIM_DIER_UIE_Pos          (0U)
#define TIM_DIER_UIE_Msk          (0x1UL << TIM_DIER_UIE_Pos)                   /*!< 0x00000001 */
#define TIM_DIER_UIE              TIM_DIER_UIE_Msk                              /*!<Update interrupt enable */
#define TIM_DIER_CC1IE_Pos        (1U)
#define TIM_DIER_CC1IE_Msk        (0x1UL << TIM_DIER_CC1IE_Pos)                 /*!< 0x00000002 */
#define TIM_DIER_CC1IE            TIM_DIER_CC1IE_Msk                            /*!<Capture/Compare 1 interrupt enable */
#define TIM_DIER_CC2IE_Pos        (2U)
#define TIM_DIER_CC2IE_Msk        (0x1UL << TIM_DIER_CC2IE_Pos)                 /*!< 0x00000004 */
#define TIM_DIER_CC2IE            TIM_DIER_CC2IE_Msk                            /*!<Capture/Compare 2 interrupt enable */
#define TIM_DIER_CC3IE_Pos        (3U)
#define TIM_DIER_CC3IE_Msk        (0x1UL << TIM_DIER_CC3IE_Pos)                 /*!< 0x00000008 */
#define TIM_DIER_CC3IE            TIM_DIER_CC3IE_Msk                            /*!<Capture/Compare 3 interrupt enable */
#define TIM_DIER_CC4IE_Pos        (4U)
#define TIM_DIER_CC4IE_Msk        (0x1UL << TIM_DIER_CC4IE_Pos)                 /*!< 0x00000010 */
#define TIM_DIER_CC4IE            TIM_DIER_CC4IE_Msk                            /*!<Capture/Compare 4 interrupt enable */
#define TIM_DIER_COMIE_Pos        (5U)
#define TIM_DIER_COMIE_Msk        (0x1UL << TIM_DIER_COMIE_Pos)                 /*!< 0x00000020 */
#define TIM_DIER_COMIE            TIM_DIER_COMIE_Msk                            /*!<COM interrupt enable */
#define TIM_DIER_TIE_Pos          (6U)
#define TIM_DIER_TIE_Msk          (0x1UL << TIM_DIER_TIE_Pos)                   /*!< 0x00000040 */
#define TIM_DIER_TIE              TIM_DIER_TIE_Msk                              /*!<Trigger interrupt enable */
#define TIM_DIER_BIE_Pos          (7U)
#define TIM_DIER_BIE_Msk          (0x1UL << TIM_DIER_BIE_Pos)                   /*!< 0x00000080 */
#define TIM_DIER_BIE              TIM_DIER_BIE_Msk                              /*!<Break interrupt enable */
#define TIM_DIER_UDE_Pos          (8U)
#define TIM_DIER_UDE_Msk          (0x1UL << TIM_DIER_UDE_Pos)                   /*!< 0x00000100 */
#define TIM_DIER_UDE              TIM_DIER_UDE_Msk                              /*!<Update DMA request enable */
#define TIM_DIER_CC1DE_Pos        (9U)
#define TIM_DIER_CC1DE_Msk        (0x1UL << TIM_DIER_CC1DE_Pos)                 /*!< 0x00000200 */
#define TIM_DIER_CC1DE            TIM_DIER_CC1DE_Msk                            /*!<Capture/Compare 1 DMA request enable */
#define TIM_DIER_CC2DE_Pos        (10U)
#define TIM_DIER_CC2DE_Msk        (0x1UL << TIM_DIER_CC2DE_Pos)                 /*!< 0x00000400 */
#define TIM_DIER_CC2DE            TIM_DIER_CC2DE_Msk                            /*!<Capture/Compare 2 DMA request enable */
#define TIM_DIER_CC3DE_Pos        (11U)
#define TIM_DIER_CC3DE_Msk        (0x1UL << TIM_DIER_CC3DE_Pos)                 /*!< 0x00000800 */
#define TIM_DIER_CC3DE            TIM_DIER_CC3DE_Msk                            /*!<Capture/Compare 3 DMA request enable */
#define TIM_DIER_CC4DE_Pos        (12U)
#define TIM_DIER_CC4DE_Msk        (0x1UL << TIM_DIER_CC4DE_Pos)                 /*!< 0x00001000 */
#define TIM_DIER_CC4DE            TIM_DIER_CC4DE_Msk                            /*!<Capture/Compare 4 DMA request enable */
#define TIM_DIER_COMDE_Pos        (13U)
#define TIM_DIER_COMDE_Msk        (0x1UL << TIM_DIER_COMDE_Pos)                 /*!< 0x00002000 */
#define TIM_DIER_COMDE            TIM_DIER_COMDE_Msk                            /*!<COM DMA request enable */
#define TIM_DIER_TDE_Pos          (14U)
#define TIM_DIER_TDE_Msk          (0x1UL << TIM_DIER_TDE_Pos)                   /*!< 0x00004000 */
#define TIM_DIER_TDE              TIM_DIER_TDE_Msk                              /*!<Trigger DMA request enable */

/********************  Bit definition for TIM_SR register  ********************/
#define TIM_SR_UIF_Pos            (0U)
#define TIM_SR_UIF_Msk            (0x1UL << TIM_SR_UIF_Pos)                     /*!< 0x00000001 */
#define TIM_SR_UIF                TIM_SR_UIF_Msk                                /*!<Update interrupt Flag */
#define TIM_SR_CC1IF_Pos          (1U)
#define TIM_SR_CC1IF_Msk          (0x1UL << TIM_SR_CC1IF_Pos)                   /*!< 0x00000002 */
#define TIM_SR_CC1IF              TIM_SR_CC1IF_Msk                              /*!<Capture/Compare 1 interrupt Flag */
#define TIM_SR_CC2IF_Pos          (2U)
#define TIM_SR_CC2IF_Msk          (0x1UL << TIM_SR_CC2IF_Pos)                   /*!< 0x00000004 */
#define TIM_SR_CC2IF              TIM_SR_CC2IF_Msk                              /*!<Capture/Compare 2 interrupt Flag */
#define TIM_SR_CC3IF_Pos          (3U)
#define TIM_SR_CC3IF_Msk          (0x1UL << TIM_SR_CC3IF_Pos)                   /*!< 0x00000008 */
#define TIM_SR_CC3IF              TIM_SR_CC3IF_Msk                              /*!<Capture/Compare 3 interrupt Flag */
#define TIM_SR_CC4IF_Pos          (4U)
#define TIM_SR_CC4IF_Msk          (0x1UL << TIM_SR_CC4IF_Pos)                   /*!< 0x00000010 */
#define TIM_SR_CC4IF              TIM_SR_CC4IF_Msk                              /*!<Capture/Compare 4 interrupt Flag */
#define TIM_SR_COMIF_Pos          (5U)
#define TIM_SR_COMIF_Msk          (0x1UL << TIM_SR_COMIF_Pos)                   /*!< 0x00000020 */
#define TIM_SR_COMIF              TIM_SR_COMIF_Msk                              /*!<COM interrupt Flag */
#define TIM_SR_TIF_Pos            (6U)
#define TIM_SR_TIF_Msk            (0x1UL << TIM_SR_TIF_Pos)                     /*!< 0x00000040 */
#define TIM_SR_TIF                TIM_SR_TIF_Msk                                /*!<Trigger interrupt Flag */
#define TIM_SR_BIF_Pos            (7U)
#define TIM_SR_BIF_Msk            (0x1UL << TIM_SR_BIF_Pos)                     /*!< 0x00000080 */
#define TIM_SR_BIF                TIM_SR_BIF_Msk                                /*!<Break interrupt Flag */
#define TIM_SR_CC1OF_Pos          (9U)
#define TIM_SR_CC1OF_Msk          (0x1UL << TIM_SR_CC1OF_Pos)                   /*!< 0x00000200 */
#define TIM_SR_CC1OF              TIM_SR_CC1OF_Msk                              /*!<Capture/Compare 1 Overcapture Flag */
#define TIM_SR_CC2OF_Pos          (10U)
#define TIM_SR_CC2OF_Msk          (0x1UL << TIM_SR_CC2OF_Pos)                   /*!< 0x00000400 */
#define TIM_SR_CC2OF              TIM_SR_CC2OF_Msk                              /*!<Capture/Compare 2 Overcapture Flag */
#define TIM_SR_CC3OF_Pos          (11U)
#define TIM_SR_CC3OF_Msk          (0x1UL << TIM_SR_CC3OF_Pos)                   /*!< 0x00000800 */
#define TIM_SR_CC3OF              TIM_SR_CC3OF_Msk                              /*!<Capture/Compare 3 Overcapture Flag */
#define TIM_SR_CC4OF_Pos          (12U)
#define TIM_SR_CC4OF_Msk          (0x1UL << TIM_SR_CC4OF_Pos)                   /*!< 0x00001000 */
#define TIM_SR_CC4OF              TIM_SR_CC4OF_Msk                              /*!<Capture/Compare 4 Overcapture Flag */

#define TIM_SR_IC1IR_Pos          (16U)
#define TIM_SR_IC1IR_Msk          (0x1UL << TIM_SR_IC1IR_Pos)                       /*!< 0x00010000 */
#define TIM_SR_IC1IR              TIM_SR_IC1IR_Msk                                  /*!< desc IC1IR */
#define TIM_SR_IC2IR_Pos          (17U)
#define TIM_SR_IC2IR_Msk          (0x1UL << TIM_SR_IC2IR_Pos)                       /*!< 0x00020000 */
#define TIM_SR_IC2IR              TIM_SR_IC2IR_Msk                                  /*!< desc IC2IR */
#define TIM_SR_IC3IR_Pos          (18U)
#define TIM_SR_IC3IR_Msk          (0x1UL << TIM_SR_IC3IR_Pos)                       /*!< 0x00040000 */
#define TIM_SR_IC3IR              TIM_SR_IC3IR_Msk                                  /*!< desc IC3IR */
#define TIM_SR_IC4IR_Pos          (19U)
#define TIM_SR_IC4IR_Msk          (0x1UL << TIM_SR_IC4IR_Pos)                       /*!< 0x00080000 */
#define TIM_SR_IC4IR              TIM_SR_IC4IR_Msk                                  /*!< desc IC4IR */

#define TIM_SR_IC1IF_Pos          (20U)
#define TIM_SR_IC1IF_Msk          (0x1UL << TIM_SR_IC1IF_Pos)                       /*!< 0x00100000 */
#define TIM_SR_IC1IF              TIM_SR_IC1IF_Msk                                  /*!< desc IC1IF */
#define TIM_SR_IC2IF_Pos          (21U)
#define TIM_SR_IC2IF_Msk          (0x1UL << TIM_SR_IC2IF_Pos)                       /*!< 0x00200000 */
#define TIM_SR_IC2IF              TIM_SR_IC2IF_Msk                                  /*!< desc IC2IF */
#define TIM_SR_IC3IF_Pos          (22U)
#define TIM_SR_IC3IF_Msk          (0x1UL << TIM_SR_IC3IR_Pos)                       /*!< 0x00400000 */
#define TIM_SR_IC3IF              TIM_SR_IC3IF_Msk                                  /*!< desc IC3IF */
#define TIM_SR_IC4IF_Pos          (23U)
#define TIM_SR_IC4IF_Msk          (0x1UL << TIM_SR_IC4IR_Pos)                       /*!< 0x00800000 */
#define TIM_SR_IC4IF              TIM_SR_IC4IF_Msk                                  /*!< desc IC4IF */

/*******************  Bit definition for TIM_EGR register  ********************/
#define TIM_EGR_UG_Pos            (0U)
#define TIM_EGR_UG_Msk            (0x1UL << TIM_EGR_UG_Pos)                     /*!< 0x00000001 */
#define TIM_EGR_UG                TIM_EGR_UG_Msk                                /*!<Update Generation */
#define TIM_EGR_CC1G_Pos          (1U)
#define TIM_EGR_CC1G_Msk          (0x1UL << TIM_EGR_CC1G_Pos)                   /*!< 0x00000002 */
#define TIM_EGR_CC1G              TIM_EGR_CC1G_Msk                              /*!<Capture/Compare 1 Generation */
#define TIM_EGR_CC2G_Pos          (2U)
#define TIM_EGR_CC2G_Msk          (0x1UL << TIM_EGR_CC2G_Pos)                   /*!< 0x00000004 */
#define TIM_EGR_CC2G              TIM_EGR_CC2G_Msk                              /*!<Capture/Compare 2 Generation */
#define TIM_EGR_CC3G_Pos          (3U)
#define TIM_EGR_CC3G_Msk          (0x1UL << TIM_EGR_CC3G_Pos)                   /*!< 0x00000008 */
#define TIM_EGR_CC3G              TIM_EGR_CC3G_Msk                              /*!<Capture/Compare 3 Generation */
#define TIM_EGR_CC4G_Pos          (4U)
#define TIM_EGR_CC4G_Msk          (0x1UL << TIM_EGR_CC4G_Pos)                   /*!< 0x00000010 */
#define TIM_EGR_CC4G              TIM_EGR_CC4G_Msk                              /*!<Capture/Compare 4 Generation */
#define TIM_EGR_COMG_Pos          (5U)
#define TIM_EGR_COMG_Msk          (0x1UL << TIM_EGR_COMG_Pos)                   /*!< 0x00000020 */
#define TIM_EGR_COMG              TIM_EGR_COMG_Msk                              /*!<Capture/Compare Control Update Generation */
#define TIM_EGR_TG_Pos            (6U)
#define TIM_EGR_TG_Msk            (0x1UL << TIM_EGR_TG_Pos)                     /*!< 0x00000040 */
#define TIM_EGR_TG                TIM_EGR_TG_Msk                                /*!<Trigger Generation */
#define TIM_EGR_BG_Pos            (7U)
#define TIM_EGR_BG_Msk            (0x1UL << TIM_EGR_BG_Pos)                     /*!< 0x00000080 */
#define TIM_EGR_BG                TIM_EGR_BG_Msk                                /*!<Break Generation */

/******************  Bit definition for TIM_CCMR1 register  *******************/
#define TIM_CCMR1_CC1S_Pos        (0U)
#define TIM_CCMR1_CC1S_Msk        (0x3UL << TIM_CCMR1_CC1S_Pos)                 /*!< 0x00000003 */
#define TIM_CCMR1_CC1S            TIM_CCMR1_CC1S_Msk                            /*!<CC1S[1:0] bits (Capture/Compare 1 Selection) */
#define TIM_CCMR1_CC1S_0          (0x1UL << TIM_CCMR1_CC1S_Pos)                 /*!< 0x00000001 */
#define TIM_CCMR1_CC1S_1          (0x2UL << TIM_CCMR1_CC1S_Pos)                 /*!< 0x00000002 */

#define TIM_CCMR1_OC1FE_Pos       (2U)
#define TIM_CCMR1_OC1FE_Msk       (0x1UL << TIM_CCMR1_OC1FE_Pos)                /*!< 0x00000004 */
#define TIM_CCMR1_OC1FE           TIM_CCMR1_OC1FE_Msk                           /*!<Output Compare 1 Fast enable */
#define TIM_CCMR1_OC1PE_Pos       (3U)
#define TIM_CCMR1_OC1PE_Msk       (0x1UL << TIM_CCMR1_OC1PE_Pos)                /*!< 0x00000008 */
#define TIM_CCMR1_OC1PE           TIM_CCMR1_OC1PE_Msk                           /*!<Output Compare 1 Preload enable */

#define TIM_CCMR1_OC1M_Pos        (4U)
#define TIM_CCMR1_OC1M_Msk        (0x7UL << TIM_CCMR1_OC1M_Pos)                 /*!< 0x00000070 */
#define TIM_CCMR1_OC1M            TIM_CCMR1_OC1M_Msk                            /*!<OC1M[2:0] bits (Output Compare 1 Mode) */
#define TIM_CCMR1_OC1M_0          (0x1UL << TIM_CCMR1_OC1M_Pos)                 /*!< 0x00000010 */
#define TIM_CCMR1_OC1M_1          (0x2UL << TIM_CCMR1_OC1M_Pos)                 /*!< 0x00000020 */
#define TIM_CCMR1_OC1M_2          (0x4UL << TIM_CCMR1_OC1M_Pos)                 /*!< 0x00000040 */

#define TIM_CCMR1_OC1CE_Pos       (7U)
#define TIM_CCMR1_OC1CE_Msk       (0x1UL << TIM_CCMR1_OC1CE_Pos)                /*!< 0x00000080 */
#define TIM_CCMR1_OC1CE           TIM_CCMR1_OC1CE_Msk                           /*!<Output Compare 1Clear Enable */

#define TIM_CCMR1_CC2S_Pos        (8U)
#define TIM_CCMR1_CC2S_Msk        (0x3UL << TIM_CCMR1_CC2S_Pos)                 /*!< 0x00000300 */
#define TIM_CCMR1_CC2S            TIM_CCMR1_CC2S_Msk                            /*!<CC2S[1:0] bits (Capture/Compare 2 Selection) */
#define TIM_CCMR1_CC2S_0          (0x1UL << TIM_CCMR1_CC2S_Pos)                 /*!< 0x00000100 */
#define TIM_CCMR1_CC2S_1          (0x2UL << TIM_CCMR1_CC2S_Pos)                 /*!< 0x00000200 */

#define TIM_CCMR1_OC2FE_Pos       (10U)
#define TIM_CCMR1_OC2FE_Msk       (0x1UL << TIM_CCMR1_OC2FE_Pos)                /*!< 0x00000400 */
#define TIM_CCMR1_OC2FE           TIM_CCMR1_OC2FE_Msk                           /*!<Output Compare 2 Fast enable */
#define TIM_CCMR1_OC2PE_Pos       (11U)
#define TIM_CCMR1_OC2PE_Msk       (0x1UL << TIM_CCMR1_OC2PE_Pos)                /*!< 0x00000800 */
#define TIM_CCMR1_OC2PE           TIM_CCMR1_OC2PE_Msk                           /*!<Output Compare 2 Preload enable */

#define TIM_CCMR1_OC2M_Pos        (12U)
#define TIM_CCMR1_OC2M_Msk        (0x7UL << TIM_CCMR1_OC2M_Pos)                 /*!< 0x00007000 */
#define TIM_CCMR1_OC2M            TIM_CCMR1_OC2M_Msk                            /*!<OC2M[2:0] bits (Output Compare 2 Mode) */
#define TIM_CCMR1_OC2M_0          (0x1UL << TIM_CCMR1_OC2M_Pos)                 /*!< 0x00001000 */
#define TIM_CCMR1_OC2M_1          (0x2UL << TIM_CCMR1_OC2M_Pos)                 /*!< 0x00002000 */
#define TIM_CCMR1_OC2M_2          (0x4UL << TIM_CCMR1_OC2M_Pos)                 /*!< 0x00004000 */

#define TIM_CCMR1_OC2CE_Pos       (15U)
#define TIM_CCMR1_OC2CE_Msk       (0x1UL << TIM_CCMR1_OC2CE_Pos)                /*!< 0x00008000 */
#define TIM_CCMR1_OC2CE           TIM_CCMR1_OC2CE_Msk                           /*!<Output Compare 2 Clear Enable */

/*----------------------------------------------------------------------------*/
#define TIM_CCMR1_IC1PSC_Pos      (2U)
#define TIM_CCMR1_IC1PSC_Msk      (0x3UL << TIM_CCMR1_IC1PSC_Pos)               /*!< 0x0000000C */
#define TIM_CCMR1_IC1PSC          TIM_CCMR1_IC1PSC_Msk                          /*!<IC1PSC[1:0] bits (Input Capture 1 Prescaler) */
#define TIM_CCMR1_IC1PSC_0        (0x1UL << TIM_CCMR1_IC1PSC_Pos)               /*!< 0x00000004 */
#define TIM_CCMR1_IC1PSC_1        (0x2UL << TIM_CCMR1_IC1PSC_Pos)               /*!< 0x00000008 */

#define TIM_CCMR1_IC1F_Pos        (4U)
#define TIM_CCMR1_IC1F_Msk        (0xFUL << TIM_CCMR1_IC1F_Pos)                 /*!< 0x000000F0 */
#define TIM_CCMR1_IC1F            TIM_CCMR1_IC1F_Msk                            /*!<IC1F[3:0] bits (Input Capture 1 Filter) */
#define TIM_CCMR1_IC1F_0          (0x1UL << TIM_CCMR1_IC1F_Pos)                 /*!< 0x00000010 */
#define TIM_CCMR1_IC1F_1          (0x2UL << TIM_CCMR1_IC1F_Pos)                 /*!< 0x00000020 */
#define TIM_CCMR1_IC1F_2          (0x4UL << TIM_CCMR1_IC1F_Pos)                 /*!< 0x00000040 */
#define TIM_CCMR1_IC1F_3          (0x8UL << TIM_CCMR1_IC1F_Pos)                 /*!< 0x00000080 */

#define TIM_CCMR1_IC2PSC_Pos      (10U)
#define TIM_CCMR1_IC2PSC_Msk      (0x3UL << TIM_CCMR1_IC2PSC_Pos)               /*!< 0x00000C00 */
#define TIM_CCMR1_IC2PSC          TIM_CCMR1_IC2PSC_Msk                          /*!<IC2PSC[1:0] bits (Input Capture 2 Prescaler) */
#define TIM_CCMR1_IC2PSC_0        (0x1UL << TIM_CCMR1_IC2PSC_Pos)               /*!< 0x00000400 */
#define TIM_CCMR1_IC2PSC_1        (0x2UL << TIM_CCMR1_IC2PSC_Pos)               /*!< 0x00000800 */

#define TIM_CCMR1_IC2F_Pos        (12U)
#define TIM_CCMR1_IC2F_Msk        (0xFUL << TIM_CCMR1_IC2F_Pos)                 /*!< 0x0000F000 */
#define TIM_CCMR1_IC2F            TIM_CCMR1_IC2F_Msk                            /*!<IC2F[3:0] bits (Input Capture 2 Filter) */
#define TIM_CCMR1_IC2F_0          (0x1UL << TIM_CCMR1_IC2F_Pos)                 /*!< 0x00001000 */
#define TIM_CCMR1_IC2F_1          (0x2UL << TIM_CCMR1_IC2F_Pos)                 /*!< 0x00002000 */
#define TIM_CCMR1_IC2F_2          (0x4UL << TIM_CCMR1_IC2F_Pos)                 /*!< 0x00004000 */
#define TIM_CCMR1_IC2F_3          (0x8UL << TIM_CCMR1_IC2F_Pos)                 /*!< 0x00008000 */

/******************  Bit definition for TIM_CCMR2 register  *******************/
#define TIM_CCMR2_CC3S_Pos        (0U)
#define TIM_CCMR2_CC3S_Msk        (0x3UL << TIM_CCMR2_CC3S_Pos)                 /*!< 0x00000003 */
#define TIM_CCMR2_CC3S            TIM_CCMR2_CC3S_Msk                            /*!<CC3S[1:0] bits (Capture/Compare 3 Selection) */
#define TIM_CCMR2_CC3S_0          (0x1UL << TIM_CCMR2_CC3S_Pos)                 /*!< 0x00000001 */
#define TIM_CCMR2_CC3S_1          (0x2UL << TIM_CCMR2_CC3S_Pos)                 /*!< 0x00000002 */

#define TIM_CCMR2_OC3FE_Pos       (2U)
#define TIM_CCMR2_OC3FE_Msk       (0x1UL << TIM_CCMR2_OC3FE_Pos)                /*!< 0x00000004 */
#define TIM_CCMR2_OC3FE           TIM_CCMR2_OC3FE_Msk                           /*!<Output Compare 3 Fast enable */
#define TIM_CCMR2_OC3PE_Pos       (3U)
#define TIM_CCMR2_OC3PE_Msk       (0x1UL << TIM_CCMR2_OC3PE_Pos)                /*!< 0x00000008 */
#define TIM_CCMR2_OC3PE           TIM_CCMR2_OC3PE_Msk                           /*!<Output Compare 3 Preload enable */

#define TIM_CCMR2_OC3M_Pos        (4U)
#define TIM_CCMR2_OC3M_Msk        (0x7UL << TIM_CCMR2_OC3M_Pos)                 /*!< 0x00000070 */
#define TIM_CCMR2_OC3M            TIM_CCMR2_OC3M_Msk                            /*!<OC3M[2:0] bits (Output Compare 3 Mode) */
#define TIM_CCMR2_OC3M_0          (0x1UL << TIM_CCMR2_OC3M_Pos)                 /*!< 0x00000010 */
#define TIM_CCMR2_OC3M_1          (0x2UL << TIM_CCMR2_OC3M_Pos)                 /*!< 0x00000020 */
#define TIM_CCMR2_OC3M_2          (0x4UL << TIM_CCMR2_OC3M_Pos)                 /*!< 0x00000040 */

#define TIM_CCMR2_OC3CE_Pos       (7U)
#define TIM_CCMR2_OC3CE_Msk       (0x1UL << TIM_CCMR2_OC3CE_Pos)                /*!< 0x00000080 */
#define TIM_CCMR2_OC3CE           TIM_CCMR2_OC3CE_Msk                           /*!<Output Compare 3 Clear Enable */

#define TIM_CCMR2_CC4S_Pos        (8U)
#define TIM_CCMR2_CC4S_Msk        (0x3UL << TIM_CCMR2_CC4S_Pos)                 /*!< 0x00000300 */
#define TIM_CCMR2_CC4S            TIM_CCMR2_CC4S_Msk                            /*!<CC4S[1:0] bits (Capture/Compare 4 Selection) */
#define TIM_CCMR2_CC4S_0          (0x1UL << TIM_CCMR2_CC4S_Pos)                 /*!< 0x00000100 */
#define TIM_CCMR2_CC4S_1          (0x2UL << TIM_CCMR2_CC4S_Pos)                 /*!< 0x00000200 */

#define TIM_CCMR2_OC4FE_Pos       (10U)
#define TIM_CCMR2_OC4FE_Msk       (0x1UL << TIM_CCMR2_OC4FE_Pos)                /*!< 0x00000400 */
#define TIM_CCMR2_OC4FE           TIM_CCMR2_OC4FE_Msk                           /*!<Output Compare 4 Fast enable */
#define TIM_CCMR2_OC4PE_Pos       (11U)
#define TIM_CCMR2_OC4PE_Msk       (0x1UL << TIM_CCMR2_OC4PE_Pos)                /*!< 0x00000800 */
#define TIM_CCMR2_OC4PE           TIM_CCMR2_OC4PE_Msk                           /*!<Output Compare 4 Preload enable */

#define TIM_CCMR2_OC4M_Pos        (12U)
#define TIM_CCMR2_OC4M_Msk        (0x7UL << TIM_CCMR2_OC4M_Pos)                 /*!< 0x00007000 */
#define TIM_CCMR2_OC4M            TIM_CCMR2_OC4M_Msk                            /*!<OC4M[2:0] bits (Output Compare 4 Mode) */
#define TIM_CCMR2_OC4M_0          (0x1UL << TIM_CCMR2_OC4M_Pos)                 /*!< 0x00001000 */
#define TIM_CCMR2_OC4M_1          (0x2UL << TIM_CCMR2_OC4M_Pos)                 /*!< 0x00002000 */
#define TIM_CCMR2_OC4M_2          (0x4UL << TIM_CCMR2_OC4M_Pos)                 /*!< 0x00004000 */

#define TIM_CCMR2_OC4CE_Pos       (15U)
#define TIM_CCMR2_OC4CE_Msk       (0x1UL << TIM_CCMR2_OC4CE_Pos)                /*!< 0x00008000 */
#define TIM_CCMR2_OC4CE           TIM_CCMR2_OC4CE_Msk                           /*!<Output Compare 4 Clear Enable */

/*----------------------------------------------------------------------------*/
#define TIM_CCMR2_IC3PSC_Pos      (2U)
#define TIM_CCMR2_IC3PSC_Msk      (0x3UL << TIM_CCMR2_IC3PSC_Pos)               /*!< 0x0000000C */
#define TIM_CCMR2_IC3PSC          TIM_CCMR2_IC3PSC_Msk                          /*!<IC3PSC[1:0] bits (Input Capture 3 Prescaler) */
#define TIM_CCMR2_IC3PSC_0        (0x1UL << TIM_CCMR2_IC3PSC_Pos)               /*!< 0x00000004 */
#define TIM_CCMR2_IC3PSC_1        (0x2UL << TIM_CCMR2_IC3PSC_Pos)               /*!< 0x00000008 */

#define TIM_CCMR2_IC3F_Pos        (4U)
#define TIM_CCMR2_IC3F_Msk        (0xFUL << TIM_CCMR2_IC3F_Pos)                 /*!< 0x000000F0 */
#define TIM_CCMR2_IC3F            TIM_CCMR2_IC3F_Msk                            /*!<IC3F[3:0] bits (Input Capture 3 Filter) */
#define TIM_CCMR2_IC3F_0          (0x1UL << TIM_CCMR2_IC3F_Pos)                 /*!< 0x00000010 */
#define TIM_CCMR2_IC3F_1          (0x2UL << TIM_CCMR2_IC3F_Pos)                 /*!< 0x00000020 */
#define TIM_CCMR2_IC3F_2          (0x4UL << TIM_CCMR2_IC3F_Pos)                 /*!< 0x00000040 */
#define TIM_CCMR2_IC3F_3          (0x8UL << TIM_CCMR2_IC3F_Pos)                 /*!< 0x00000080 */

#define TIM_CCMR2_IC4PSC_Pos      (10U)
#define TIM_CCMR2_IC4PSC_Msk      (0x3UL << TIM_CCMR2_IC4PSC_Pos)               /*!< 0x00000C00 */
#define TIM_CCMR2_IC4PSC          TIM_CCMR2_IC4PSC_Msk                          /*!<IC4PSC[1:0] bits (Input Capture 4 Prescaler) */
#define TIM_CCMR2_IC4PSC_0        (0x1UL << TIM_CCMR2_IC4PSC_Pos)               /*!< 0x00000400 */
#define TIM_CCMR2_IC4PSC_1        (0x2UL << TIM_CCMR2_IC4PSC_Pos)               /*!< 0x00000800 */

#define TIM_CCMR2_IC4F_Pos        (12U)
#define TIM_CCMR2_IC4F_Msk        (0xFUL << TIM_CCMR2_IC4F_Pos)                 /*!< 0x0000F000 */
#define TIM_CCMR2_IC4F            TIM_CCMR2_IC4F_Msk                            /*!<IC4F[3:0] bits (Input Capture 4 Filter) */
#define TIM_CCMR2_IC4F_0          (0x1UL << TIM_CCMR2_IC4F_Pos)                 /*!< 0x00001000 */
#define TIM_CCMR2_IC4F_1          (0x2UL << TIM_CCMR2_IC4F_Pos)                 /*!< 0x00002000 */
#define TIM_CCMR2_IC4F_2          (0x4UL << TIM_CCMR2_IC4F_Pos)                 /*!< 0x00004000 */
#define TIM_CCMR2_IC4F_3          (0x8UL << TIM_CCMR2_IC4F_Pos)                 /*!< 0x00008000 */

/*******************  Bit definition for TIM_CCER register  *******************/
#define TIM_CCER_CC1E_Pos         (0U)
#define TIM_CCER_CC1E_Msk         (0x1UL << TIM_CCER_CC1E_Pos)                  /*!< 0x00000001 */
#define TIM_CCER_CC1E             TIM_CCER_CC1E_Msk                             /*!<Capture/Compare 1 output enable */
#define TIM_CCER_CC1P_Pos         (1U)
#define TIM_CCER_CC1P_Msk         (0x1UL << TIM_CCER_CC1P_Pos)                  /*!< 0x00000002 */
#define TIM_CCER_CC1P             TIM_CCER_CC1P_Msk                             /*!<Capture/Compare 1 output Polarity */
#define TIM_CCER_CC1NE_Pos        (2U)
#define TIM_CCER_CC1NE_Msk        (0x1UL << TIM_CCER_CC1NE_Pos)                 /*!< 0x00000004 */
#define TIM_CCER_CC1NE            TIM_CCER_CC1NE_Msk                            /*!<Capture/Compare 1 Complementary output enable */
#define TIM_CCER_CC1NP_Pos        (3U)
#define TIM_CCER_CC1NP_Msk        (0x1UL << TIM_CCER_CC1NP_Pos)                 /*!< 0x00000008 */
#define TIM_CCER_CC1NP            TIM_CCER_CC1NP_Msk                            /*!<Capture/Compare 1 Complementary output Polarity */
#define TIM_CCER_CC2E_Pos         (4U)
#define TIM_CCER_CC2E_Msk         (0x1UL << TIM_CCER_CC2E_Pos)                  /*!< 0x00000010 */
#define TIM_CCER_CC2E             TIM_CCER_CC2E_Msk                             /*!<Capture/Compare 2 output enable */
#define TIM_CCER_CC2P_Pos         (5U)
#define TIM_CCER_CC2P_Msk         (0x1UL << TIM_CCER_CC2P_Pos)                  /*!< 0x00000020 */
#define TIM_CCER_CC2P             TIM_CCER_CC2P_Msk                             /*!<Capture/Compare 2 output Polarity */
#define TIM_CCER_CC2NE_Pos        (6U)
#define TIM_CCER_CC2NE_Msk        (0x1UL << TIM_CCER_CC2NE_Pos)                 /*!< 0x00000040 */
#define TIM_CCER_CC2NE            TIM_CCER_CC2NE_Msk                            /*!<Capture/Compare 2 Complementary output enable */
#define TIM_CCER_CC2NP_Pos        (7U)
#define TIM_CCER_CC2NP_Msk        (0x1UL << TIM_CCER_CC2NP_Pos)                 /*!< 0x00000080 */
#define TIM_CCER_CC2NP            TIM_CCER_CC2NP_Msk                            /*!<Capture/Compare 2 Complementary output Polarity */
#define TIM_CCER_CC3E_Pos         (8U)
#define TIM_CCER_CC3E_Msk         (0x1UL << TIM_CCER_CC3E_Pos)                  /*!< 0x00000100 */
#define TIM_CCER_CC3E             TIM_CCER_CC3E_Msk                             /*!<Capture/Compare 3 output enable */
#define TIM_CCER_CC3P_Pos         (9U)
#define TIM_CCER_CC3P_Msk         (0x1UL << TIM_CCER_CC3P_Pos)                  /*!< 0x00000200 */
#define TIM_CCER_CC3P             TIM_CCER_CC3P_Msk                             /*!<Capture/Compare 3 output Polarity */
#define TIM_CCER_CC3NE_Pos        (10U)
#define TIM_CCER_CC3NE_Msk        (0x1UL << TIM_CCER_CC3NE_Pos)                 /*!< 0x00000400 */
#define TIM_CCER_CC3NE            TIM_CCER_CC3NE_Msk                            /*!<Capture/Compare 3 Complementary output enable */
#define TIM_CCER_CC3NP_Pos        (11U)
#define TIM_CCER_CC3NP_Msk        (0x1UL << TIM_CCER_CC3NP_Pos)                 /*!< 0x00000800 */
#define TIM_CCER_CC3NP            TIM_CCER_CC3NP_Msk                            /*!<Capture/Compare 3 Complementary output Polarity */
#define TIM_CCER_CC4E_Pos         (12U)
#define TIM_CCER_CC4E_Msk         (0x1UL << TIM_CCER_CC4E_Pos)                  /*!< 0x00001000 */
#define TIM_CCER_CC4E             TIM_CCER_CC4E_Msk                             /*!<Capture/Compare 4 output enable */
#define TIM_CCER_CC4P_Pos         (13U)
#define TIM_CCER_CC4P_Msk         (0x1UL << TIM_CCER_CC4P_Pos)                  /*!< 0x00002000 */
#define TIM_CCER_CC4P             TIM_CCER_CC4P_Msk                             /*!<Capture/Compare 4 output Polarity */
#define TIM_CCER_CC4NP_Pos        (15U)
#define TIM_CCER_CC4NP_Msk        (0x1UL << TIM_CCER_CC4NP_Pos)                 /*!< 0x00008000 */
#define TIM_CCER_CC4NP            TIM_CCER_CC4NP_Msk                            /*!<Capture/Compare 4 Complementary output Polarity */

/*******************  Bit definition for TIM_CNT register  ********************/
#define TIM_CNT_CNT_Pos           (0U)
#define TIM_CNT_CNT_Msk           (0xFFFFUL << TIM_CNT_CNT_Pos)                /*!< 0x0000FFFF */
#define TIM_CNT_CNT               TIM_CNT_CNT_Msk                              /*!<Counter Value */

/*******************  Bit definition for TIM_PSC register  ********************/
#define TIM_PSC_PSC_Pos           (0U)
#define TIM_PSC_PSC_Msk           (0xFFFFUL << TIM_PSC_PSC_Pos)                /*!< 0x0000FFFF */
#define TIM_PSC_PSC               TIM_PSC_PSC_Msk                              /*!<Prescaler Value */

/*******************  Bit definition for TIM_ARR register  ********************/
#define TIM_ARR_ARR_Pos           (0U)
#define TIM_ARR_ARR_Msk           (0xFFFFUL << TIM_ARR_ARR_Pos)                /*!< 0x0000FFFF */
#define TIM_ARR_ARR               TIM_ARR_ARR_Msk                              /*!<actual auto-reload Value */

/*******************  Bit definition for TIM_RCR register  ********************/
#define TIM_RCR_REP_Pos           (0U)
#define TIM_RCR_REP_Msk           (0xFFUL << TIM_RCR_REP_Pos)                  /*!< 0x000000FF */
#define TIM_RCR_REP               TIM_RCR_REP_Msk                              /*!<Repetition Counter Value */

/*******************  Bit definition for TIM_CCR1 register  *******************/
#define TIM_CCR1_CCR1_Pos         (0U)
#define TIM_CCR1_CCR1_Msk         (0xFFFFUL << TIM_CCR1_CCR1_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR1_CCR1             TIM_CCR1_CCR1_Msk                            /*!<Capture/Compare 1 Value */

/*******************  Bit definition for TIM_CCR2 register  *******************/
#define TIM_CCR2_CCR2_Pos         (0U)
#define TIM_CCR2_CCR2_Msk         (0xFFFFUL << TIM_CCR2_CCR2_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR2_CCR2             TIM_CCR2_CCR2_Msk                            /*!<Capture/Compare 2 Value */

/*******************  Bit definition for TIM_CCR3 register  *******************/
#define TIM_CCR3_CCR3_Pos         (0U)
#define TIM_CCR3_CCR3_Msk         (0xFFFFUL << TIM_CCR3_CCR3_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR3_CCR3             TIM_CCR3_CCR3_Msk                            /*!<Capture/Compare 3 Value */

/*******************  Bit definition for TIM_CCR4 register  *******************/
#define TIM_CCR4_CCR4_Pos         (0U)
#define TIM_CCR4_CCR4_Msk         (0xFFFFUL << TIM_CCR4_CCR4_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR4_CCR4             TIM_CCR4_CCR4_Msk                            /*!<Capture/Compare 4 Value */

/*******************  Bit definition for TIM_BDTR register  *******************/
#define TIM_BDTR_DTG_Pos          (0U)
#define TIM_BDTR_DTG_Msk          (0xFFUL << TIM_BDTR_DTG_Pos)                  /*!< 0x000000FF */
#define TIM_BDTR_DTG              TIM_BDTR_DTG_Msk                              /*!<DTG[0:7] bits (Dead-Time Generator set-up) */
#define TIM_BDTR_DTG_0            (0x01UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000001 */
#define TIM_BDTR_DTG_1            (0x02UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000002 */
#define TIM_BDTR_DTG_2            (0x04UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000004 */
#define TIM_BDTR_DTG_3            (0x08UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000008 */
#define TIM_BDTR_DTG_4            (0x10UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000010 */
#define TIM_BDTR_DTG_5            (0x20UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000020 */
#define TIM_BDTR_DTG_6            (0x40UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000040 */
#define TIM_BDTR_DTG_7            (0x80UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000080 */

#define TIM_BDTR_LOCK_Pos         (8U)
#define TIM_BDTR_LOCK_Msk         (0x3UL << TIM_BDTR_LOCK_Pos)                  /*!< 0x00000300 */
#define TIM_BDTR_LOCK             TIM_BDTR_LOCK_Msk                             /*!<LOCK[1:0] bits (Lock Configuration) */
#define TIM_BDTR_LOCK_0           (0x1UL << TIM_BDTR_LOCK_Pos)                  /*!< 0x00000100 */
#define TIM_BDTR_LOCK_1           (0x2UL << TIM_BDTR_LOCK_Pos)                  /*!< 0x00000200 */

#define TIM_BDTR_OSSI_Pos         (10U)
#define TIM_BDTR_OSSI_Msk         (0x1UL << TIM_BDTR_OSSI_Pos)                  /*!< 0x00000400 */
#define TIM_BDTR_OSSI             TIM_BDTR_OSSI_Msk                             /*!<Off-State Selection for Idle mode */
#define TIM_BDTR_OSSR_Pos         (11U)
#define TIM_BDTR_OSSR_Msk         (0x1UL << TIM_BDTR_OSSR_Pos)                  /*!< 0x00000800 */
#define TIM_BDTR_OSSR             TIM_BDTR_OSSR_Msk                             /*!<Off-State Selection for Run mode */
#define TIM_BDTR_BKE_Pos          (12U)
#define TIM_BDTR_BKE_Msk          (0x1UL << TIM_BDTR_BKE_Pos)                   /*!< 0x00001000 */
#define TIM_BDTR_BKE              TIM_BDTR_BKE_Msk                              /*!<Break enable */
#define TIM_BDTR_BKP_Pos          (13U)
#define TIM_BDTR_BKP_Msk          (0x1UL << TIM_BDTR_BKP_Pos)                   /*!< 0x00002000 */
#define TIM_BDTR_BKP              TIM_BDTR_BKP_Msk                              /*!<Break Polarity */
#define TIM_BDTR_AOE_Pos          (14U)
#define TIM_BDTR_AOE_Msk          (0x1UL << TIM_BDTR_AOE_Pos)                   /*!< 0x00004000 */
#define TIM_BDTR_AOE              TIM_BDTR_AOE_Msk                              /*!<Automatic Output enable */
#define TIM_BDTR_MOE_Pos          (15U)
#define TIM_BDTR_MOE_Msk          (0x1UL << TIM_BDTR_MOE_Pos)                   /*!< 0x00008000 */
#define TIM_BDTR_MOE              TIM_BDTR_MOE_Msk                              /*!<Main Output enable */

/*******************  Bit definition for TIM_DCR register  ********************/
#define TIM_DCR_DBA_Pos           (0U)
#define TIM_DCR_DBA_Msk           (0x1FUL << TIM_DCR_DBA_Pos)                   /*!< 0x0000001F */
#define TIM_DCR_DBA               TIM_DCR_DBA_Msk                               /*!<DBA[4:0] bits (DMA Base Address) */
#define TIM_DCR_DBA_0             (0x01UL << TIM_DCR_DBA_Pos)                   /*!< 0x00000001 */
#define TIM_DCR_DBA_1             (0x02UL << TIM_DCR_DBA_Pos)                   /*!< 0x00000002 */
#define TIM_DCR_DBA_2             (0x04UL << TIM_DCR_DBA_Pos)                   /*!< 0x00000004 */
#define TIM_DCR_DBA_3             (0x08UL << TIM_DCR_DBA_Pos)                   /*!< 0x00000008 */
#define TIM_DCR_DBA_4             (0x10UL << TIM_DCR_DBA_Pos)                   /*!< 0x00000010 */

#define TIM_DCR_DBL_Pos           (8U)
#define TIM_DCR_DBL_Msk           (0x1FUL << TIM_DCR_DBL_Pos)                   /*!< 0x00001F00 */
#define TIM_DCR_DBL               TIM_DCR_DBL_Msk                               /*!<DBL[4:0] bits (DMA Burst Length) */
#define TIM_DCR_DBL_0             (0x01UL << TIM_DCR_DBL_Pos)                   /*!< 0x00000100 */
#define TIM_DCR_DBL_1             (0x02UL << TIM_DCR_DBL_Pos)                   /*!< 0x00000200 */
#define TIM_DCR_DBL_2             (0x04UL << TIM_DCR_DBL_Pos)                   /*!< 0x00000400 */
#define TIM_DCR_DBL_3             (0x08UL << TIM_DCR_DBL_Pos)                   /*!< 0x00000800 */
#define TIM_DCR_DBL_4             (0x10UL << TIM_DCR_DBL_Pos)                   /*!< 0x00001000 */

/*******************  Bit definition for TIM_DMAR register  *******************/
#define TIM_DMAR_DMAB_Pos         (0U)
#define TIM_DMAR_DMAB_Msk         (0xFFFFUL << TIM_DMAR_DMAB_Pos)               /*!< 0x0000FFFF */
#define TIM_DMAR_DMAB             TIM_DMAR_DMAB_Msk                             /*!<DMA register for burst accesses */

/*******************  Bit definition for TIM14_OR register  *******************/
#define TIM14_OR_TI1_RMP_Pos      (0U)
#define TIM14_OR_TI1_RMP_Msk      (0x3UL << TIM14_OR_TI1_RMP_Pos)               /*!< 0x00000003 */
#define TIM14_OR_TI1_RMP          TIM14_OR_TI1_RMP_Msk                          /*!<TI1_RMP[1:0] bits (TIM14 Input 4 remap) */
#define TIM14_OR_TI1_RMP_0        (0x1UL << TIM14_OR_TI1_RMP_Pos)               /*!< 0x00000001 */
#define TIM14_OR_TI1_RMP_1        (0x2UL << TIM14_OR_TI1_RMP_Pos)               /*!< 0x00000002 */

/******************************************************************************/
/*                                                                            */
/*                       Liquid Crystal Display (LCD)                         */
/*                                                                            */
/******************************************************************************/
/******************   Bit definition for LCD_CR0 register   *******************/
#define LCD_CR0_EN_Pos              (0U)
#define LCD_CR0_EN_Msk              (0x1UL << LCD_CR0_EN_Pos)                   /*!< 0x00000001 */
#define LCD_CR0_EN                  LCD_CR0_EN_Msk                              /*!< desc EN */

#define LCD_CR0_LCDCLK_Pos          (1U)
#define LCD_CR0_LCDCLK_Msk          (0x3UL << LCD_CR0_LCDCLK_Pos)               /*!< 0x00000006 */
#define LCD_CR0_LCDCLK              LCD_CR0_LCDCLK_Msk                          /*!< desc LCDCLK */
#define LCD_CR0_LCDCLK_0            (0x1UL << LCD_CR0_LCDCLK_Pos)               /*!< 0x00000002 */
#define LCD_CR0_LCDCLK_1            (0x2UL << LCD_CR0_LCDCLK_Pos)               /*!< 0x00000004 */

#define LCD_CR0_BIAS_Pos            (5U)
#define LCD_CR0_BIAS_Msk            (0x1UL << LCD_CR0_BIAS_Pos)                 /*!< 0x00000020 */
#define LCD_CR0_BIAS                LCD_CR0_BIAS_Msk                            /*!< desc BIAS */

#define LCD_CR0_DUTY_Pos            (6U)
#define LCD_CR0_DUTY_Msk            (0x7UL << LCD_CR0_DUTY_Pos)                 /*!< 0x000001C0 */
#define LCD_CR0_DUTY                LCD_CR0_DUTY_Msk                            /*!< desc DUTY */
#define LCD_CR0_DUTY_0              (0x1UL << LCD_CR0_DUTY_Pos)                 /*!< 0x00000040 */
#define LCD_CR0_DUTY_1              (0x2UL << LCD_CR0_DUTY_Pos)                 /*!< 0x00000080 */
#define LCD_CR0_DUTY_2              (0x4UL << LCD_CR0_DUTY_Pos)                 /*!< 0x00000100 */

#define LCD_CR0_BSEL_Pos            (9U)
#define LCD_CR0_BSEL_Msk            (0x7UL << LCD_CR0_BSEL_Pos)                 /*!< 0x00000E00 */
#define LCD_CR0_BSEL                LCD_CR0_BSEL_Msk                            /*!< desc BSEL */
#define LCD_CR0_BSEL_0              (0x1UL << LCD_CR0_BSEL_Pos)                 /*!< 0x00000200 */
#define LCD_CR0_BSEL_1              (0x2UL << LCD_CR0_BSEL_Pos)                 /*!< 0x00000400 */
#define LCD_CR0_BSEL_2              (0x4UL << LCD_CR0_BSEL_Pos)                 /*!< 0x00000800 */

#define LCD_CR0_CONTRAST_Pos        (12U)
#define LCD_CR0_CONTRAST_Msk        (0xFUL << LCD_CR0_CONTRAST_Pos)             /*!< 0x0000F000 */
#define LCD_CR0_CONTRAST            LCD_CR0_CONTRAST_Msk                        /*!< desc CONTRAST */
#define LCD_CR0_CONTRAST_0          (0x1UL << LCD_CR0_CONTRAST_Pos)             /*!< 0x00001000 */
#define LCD_CR0_CONTRAST_1          (0x2UL << LCD_CR0_CONTRAST_Pos)             /*!< 0x00002000 */
#define LCD_CR0_CONTRAST_2          (0x4UL << LCD_CR0_CONTRAST_Pos)             /*!< 0x00004000 */
#define LCD_CR0_CONTRAST_3          (0x8UL << LCD_CR0_CONTRAST_Pos)             /*!< 0x00008000 */

/******************   Bit definition for LCD_CR1 register   *******************/
#define LCD_CR1_BLINKCNT_Pos          (0U)
#define LCD_CR1_BLINKCNT_Msk          (0x3FUL << LCD_CR1_BLINKCNT_Pos)          /*!< 0x0000003F */
#define LCD_CR1_BLINKCNT              LCD_CR1_BLINKCNT_Msk                      /*!< desc BLINKCNT */
#define LCD_CR1_BLINKCNT_0            (0x1UL << LCD_CR1_BLINKCNT_Pos)           /*!< 0x00000001 */
#define LCD_CR1_BLINKCNT_1            (0x2UL << LCD_CR1_BLINKCNT_Pos)           /*!< 0x00000002 */
#define LCD_CR1_BLINKCNT_2            (0x4UL << LCD_CR1_BLINKCNT_Pos)           /*!< 0x00000004 */
#define LCD_CR1_BLINKCNT_3            (0x8UL << LCD_CR1_BLINKCNT_Pos)           /*!< 0x00000008 */
#define LCD_CR1_BLINKCNT_4            (0x10UL << LCD_CR1_BLINKCNT_Pos)          /*!< 0x00000010 */
#define LCD_CR1_BLINKCNT_5            (0x20UL << LCD_CR1_BLINKCNT_Pos)          /*!< 0x00000020 */

#define LCD_CR1_BLINKEN_Pos           (6U)
#define LCD_CR1_BLINKEN_Msk           (0x1UL << LCD_CR1_BLINKEN_Pos)            /*!< 0x00000040 */
#define LCD_CR1_BLINKEN               LCD_CR1_BLINKEN_Msk                       /*!< desc BLINKEN */

#define LCD_CR1_MODE_Pos              (8U)
#define LCD_CR1_MODE_Msk              (0x1UL << LCD_CR1_MODE_Pos)               /*!< 0x00000100 */
#define LCD_CR1_MODE                  LCD_CR1_MODE_Msk                          /*!< desc MODE */

#define LCD_CR1_IE_Pos                (9U)
#define LCD_CR1_IE_Msk                (0x1UL << LCD_CR1_IE_Pos)                 /*!< 0x00000200 */
#define LCD_CR1_IE                    LCD_CR1_IE_Msk                            /*!< desc IE */

#define LCD_CR1_DMAEN_Pos             (10U)
#define LCD_CR1_DMAEN_Msk             (0x1UL << LCD_CR1_DMAEN_Pos)              /*!< 0x00000400 */
#define LCD_CR1_DMAEN                 LCD_CR1_DMAEN_Msk                         /*!< desc DMAEN */

#define LCD_CR1_INTF_Pos              (11U)
#define LCD_CR1_INTF_Msk              (0x1UL << LCD_CR1_INTF_Pos)               /*!< 0x00000800 */
#define LCD_CR1_INTF                  LCD_CR1_INTF_Msk                          /*!< desc INTF */

/******************   Bit definition for LCD_INTCLR register   ****************/
#define LCD_INTCLR_INTF_CLR_Pos       (10U)
#define LCD_INTCLR_INTF_CLR_Msk       (0x1UL << LCD_INTCLR_INTF_CLR_Pos)        /*!< 0x00000001 */
#define LCD_INTCLR_INTF_CLR           LCD_INTCLR_INTF_CLR_Msk                   /*!< desc INTF_CLR */

/******************  Bits definition for LCD_POEN0 register  ******************/
#define LCD_POEN0_S0_Pos               (0U)
#define LCD_POEN0_S0_Msk               (0x1UL << LCD_POEN0_S0_Pos)              /*!< 0x00000001 */
#define LCD_POEN0_S0                   LCD_POEN0_S0_Msk
#define LCD_POEN0_S1_Pos               (1U)
#define LCD_POEN0_S1_Msk               (0x1UL << LCD_POEN0_S1_Pos)              /*!< 0x00000002 */
#define LCD_POEN0_S1                   LCD_POEN0_S1_Msk
#define LCD_POEN0_S2_Pos               (2U)
#define LCD_POEN0_S2_Msk               (0x1UL << LCD_POEN0_S2_Pos)              /*!< 0x00000004 */
#define LCD_POEN0_S2                   LCD_POEN0_S2_Msk
#define LCD_POEN0_S3_Pos               (3U)
#define LCD_POEN0_S3_Msk               (0x1UL << LCD_POEN0_S3_Pos)              /*!< 0x00000008 */
#define LCD_POEN0_S3                   LCD_POEN0_S3_Msk
#define LCD_POEN0_S4_Pos               (4U)
#define LCD_POEN0_S4_Msk               (0x1UL << LCD_POEN0_S4_Pos)              /*!< 0x00000010 */
#define LCD_POEN0_S4                   LCD_POEN0_S4_Msk
#define LCD_POEN0_S5_Pos               (5U)
#define LCD_POEN0_S5_Msk               (0x1UL << LCD_POEN0_S5_Pos)              /*!< 0x00000020 */
#define LCD_POEN0_S5                   LCD_POEN0_S5_Msk
#define LCD_POEN0_S6_Pos               (6U)
#define LCD_POEN0_S6_Msk               (0x1UL << LCD_POEN0_S6_Pos)              /*!< 0x00000040 */
#define LCD_POEN0_S6                   LCD_POEN0_S6_Msk
#define LCD_POEN0_S7_Pos               (7U)
#define LCD_POEN0_S7_Msk               (0x1UL << LCD_POEN0_S7_Pos)              /*!< 0x00000080 */
#define LCD_POEN0_S7                   LCD_POEN0_S7_Msk
#define LCD_POEN0_S8_Pos               (8U)
#define LCD_POEN0_S8_Msk               (0x1UL << LCD_POEN0_S8_Pos)              /*!< 0x00000100 */
#define LCD_POEN0_S8                   LCD_POEN0_S8_Msk
#define LCD_POEN0_S9_Pos               (9U)
#define LCD_POEN0_S9_Msk               (0x1UL << LCD_POEN0_S9_Pos)              /*!< 0x00000200 */
#define LCD_POEN0_S9                   LCD_POEN0_S9_Msk
#define LCD_POEN0_S10_Pos              (10U)
#define LCD_POEN0_S10_Msk              (0x1UL << LCD_POEN0_S10_Pos)             /*!< 0x00000400 */
#define LCD_POEN0_S10                  LCD_POEN0_S10_Msk
#define LCD_POEN0_S11_Pos              (11U)
#define LCD_POEN0_S11_Msk              (0x1UL << LCD_POEN0_S11_Pos)             /*!< 0x00000800 */
#define LCD_POEN0_S11                  LCD_POEN0_S11_Msk
#define LCD_POEN0_S12_Pos              (12U)
#define LCD_POEN0_S12_Msk              (0x1UL << LCD_POEN0_S12_Pos)             /*!< 0x00001000 */
#define LCD_POEN0_S12                  LCD_POEN0_S12_Msk
#define LCD_POEN0_S13_Pos              (13U)
#define LCD_POEN0_S13_Msk              (0x1UL << LCD_POEN0_S13_Pos)             /*!< 0x00002000 */
#define LCD_POEN0_S13                  LCD_POEN0_S13_Msk
#define LCD_POEN0_S14_Pos              (14U)
#define LCD_POEN0_S14_Msk              (0x1UL << LCD_POEN0_S14_Pos)             /*!< 0x00004000 */
#define LCD_POEN0_S14                  LCD_POEN0_S14_Msk
#define LCD_POEN0_S15_Pos              (15U)
#define LCD_POEN0_S15_Msk              (0x1UL << LCD_POEN0_S15_Pos)             /*!< 0x00008000 */
#define LCD_POEN0_S15                  LCD_POEN0_S15_Msk
#define LCD_POEN0_S16_Pos              (16U)
#define LCD_POEN0_S16_Msk              (0x1UL << LCD_POEN0_S16_Pos)             /*!< 0x00010000 */
#define LCD_POEN0_S16                  LCD_POEN0_S16_Msk
#define LCD_POEN0_S17_Pos              (17U)
#define LCD_POEN0_S17_Msk              (0x1UL << LCD_POEN0_S17_Pos)             /*!< 0x00020000 */
#define LCD_POEN0_S17                  LCD_POEN0_S17_Msk
#define LCD_POEN0_S18_Pos              (18U)
#define LCD_POEN0_S18_Msk              (0x1UL << LCD_POEN0_S18_Pos)             /*!< 0x00040000 */
#define LCD_POEN0_S18                  LCD_POEN0_S18_Msk
#define LCD_POEN0_S19_Pos              (19U)
#define LCD_POEN0_S19_Msk              (0x1UL << LCD_POEN0_S19_Pos)             /*!< 0x00080000 */
#define LCD_POEN0_S19                  LCD_POEN0_S19_Msk
#define LCD_POEN0_S20_Pos              (20U)
#define LCD_POEN0_S20_Msk              (0x1UL << LCD_POEN0_S20_Pos)             /*!< 0x00100000 */
#define LCD_POEN0_S20                  LCD_POEN0_S20_Msk
#define LCD_POEN0_S21_Pos              (21U)
#define LCD_POEN0_S21_Msk              (0x1UL << LCD_POEN0_S21_Pos)             /*!< 0x00200000 */
#define LCD_POEN0_S21                  LCD_POEN0_S21_Msk
#define LCD_POEN0_S22_Pos              (22U)
#define LCD_POEN0_S22_Msk              (0x1UL << LCD_POEN0_S22_Pos)             /*!< 0x00400000 */
#define LCD_POEN0_S22                  LCD_POEN0_S22_Msk
#define LCD_POEN0_S23_Pos              (23U)
#define LCD_POEN0_S23_Msk              (0x1UL << LCD_POEN0_S23_Pos)             /*!< 0x00800000 */
#define LCD_POEN0_S23                  LCD_POEN0_S23_Msk
#define LCD_POEN0_S24_Pos              (24U)
#define LCD_POEN0_S24_Msk              (0x1UL << LCD_POEN0_S24_Pos)             /*!< 0x01000000 */
#define LCD_POEN0_S24                  LCD_POEN0_S24_Msk
#define LCD_POEN0_S25_Pos              (25U)
#define LCD_POEN0_S25_Msk              (0x1UL << LCD_POEN0_S25_Pos)             /*!< 0x02000000 */
#define LCD_POEN0_S25                  LCD_POEN0_S25_Msk
#define LCD_POEN0_S26_Pos              (26U)
#define LCD_POEN0_S26_Msk              (0x1UL << LCD_POEN0_S26_Pos)             /*!< 0x04000000 */
#define LCD_POEN0_S26                  LCD_POEN0_S26_Msk
#define LCD_POEN0_S27_Pos              (27U)
#define LCD_POEN0_S27_Msk              (0x1UL << LCD_POEN0_S27_Pos)             /*!< 0x08000000 */
#define LCD_POEN0_S27                  LCD_POEN0_S27_Msk
#define LCD_POEN0_S28_Pos              (28U)
#define LCD_POEN0_S28_Msk              (0x1UL << LCD_POEN0_S28_Pos)             /*!< 0x10000000 */
#define LCD_POEN0_S28                  LCD_POEN0_S28_Msk
#define LCD_POEN0_S29_Pos              (29U)
#define LCD_POEN0_S29_Msk              (0x1UL << LCD_POEN0_S29_Pos)             /*!< 0x20000000 */
#define LCD_POEN0_S29                  LCD_POEN0_S29_Msk
#define LCD_POEN0_S30_Pos              (30U)
#define LCD_POEN0_S30_Msk              (0x1UL << LCD_POEN0_S30_Pos)             /*!< 0x40000000 */
#define LCD_POEN0_S30                  LCD_POEN0_S30_Msk
#define LCD_POEN0_S31_Pos              (31U)
#define LCD_POEN0_S31_Msk              (0x1UL << LCD_POEN0_S31_Pos)             /*!< 0x80000000 */
#define LCD_POEN0_S31                  LCD_POEN0_S31_Msk

/******************  Bits definition for LCD_POEN1 register  ******************/
#define LCD_POEN1_S32_Pos               (0U)
#define LCD_POEN1_S32_Msk               (0x1UL << LCD_POEN1_S32_Pos)              /*!< 0x00000001 */
#define LCD_POEN1_S32                   LCD_POEN1_S32_Msk
#define LCD_POEN1_S33_Pos               (1U)
#define LCD_POEN1_S33_Msk               (0x1UL << LCD_POEN1_S33_Pos)              /*!< 0x00000002 */
#define LCD_POEN1_S33                   LCD_POEN1_S33_Msk
#define LCD_POEN1_S34_Pos               (2U)
#define LCD_POEN1_S34_Msk               (0x1UL << LCD_POEN1_S34_Pos)              /*!< 0x00000004 */
#define LCD_POEN1_S34                   LCD_POEN1_S34_Msk
#define LCD_POEN1_S35_Pos               (3U)
#define LCD_POEN1_S35_Msk               (0x1UL << LCD_POEN1_S35_Pos)              /*!< 0x00000008 */
#define LCD_POEN1_S35                   LCD_POEN1_S35_Msk
#define LCD_POEN1_S36_Pos               (4U)
#define LCD_POEN1_S36_Msk               (0x1UL << LCD_POEN1_S36_Pos)              /*!< 0x00000010 */
#define LCD_POEN1_S36                   LCD_POEN1_S36_Msk
#define LCD_POEN1_S37_Pos               (5U)
#define LCD_POEN1_S37_Msk               (0x1UL << LCD_POEN1_S37_Pos)              /*!< 0x00000020 */
#define LCD_POEN1_S37                   LCD_POEN1_S37_Msk
#define LCD_POEN1_S38_Pos               (6U)
#define LCD_POEN1_S38_Msk               (0x1UL << LCD_POEN1_S38_Pos)              /*!< 0x00000040 */
#define LCD_POEN1_S38                   LCD_POEN1_S38_Msk
#define LCD_POEN1_S39_Pos               (7U)
#define LCD_POEN1_S39_Msk               (0x1UL << LCD_POEN1_S39_Pos)              /*!< 0x00000080 */
#define LCD_POEN1_S39                   LCD_POEN1_S39_Msk

#define LCD_POEN1_C0_Pos                (8U)
#define LCD_POEN1_C0_Msk                (0x1UL << LCD_POEN1_C0_Pos)               /*!< 0x00000100 */
#define LCD_POEN1_C0                    LCD_POEN1_C0_Msk                          /*!< desc C0 */
#define LCD_POEN1_C1_Pos                (9U)
#define LCD_POEN1_C1_Msk                (0x1UL << LCD_POEN1_C1_Pos)               /*!< 0x00000200 */
#define LCD_POEN1_C1                    LCD_POEN1_C1_Msk                          /*!< desc C1 */
#define LCD_POEN1_C2_Pos                (10U)
#define LCD_POEN1_C2_Msk                (0x1UL << LCD_POEN1_C2_Pos)               /*!< 0x00000400 */
#define LCD_POEN1_C2                    LCD_POEN1_C2_Msk                          /*!< desc C2 */
#define LCD_POEN1_C3_Pos                (11U)
#define LCD_POEN1_C3_Msk                (0x1UL << LCD_POEN1_C3_Pos)               /*!< 0x00000800 */
#define LCD_POEN1_C3                    LCD_POEN1_C3_Msk                          /*!< desc C3 */

#define LCD_POEN1_MUX_Pos               (12U)
#define LCD_POEN1_MUX_Msk               (0x1UL << LCD_POEN1_MUX_Pos)              /*!< 0x00001000 */
#define LCD_POEN1_MUX                   LCD_POEN1_MUX_Msk                         /*!< desc MUX */

/******************  Bits definition for LCD_RAMx register  *******************/
#define LCD_RAM_D0_Pos               (0U)
#define LCD_RAM_D0_Msk               (0x1UL << LCD_RAM_D0_Pos)              /*!< 0x00000001 */
#define LCD_RAM_D0                   LCD_RAM_D0_Msk
#define LCD_RAM_D1_Pos               (1U)
#define LCD_RAM_D1_Msk               (0x1UL << LCD_RAM_D1_Pos)              /*!< 0x00000002 */
#define LCD_RAM_D1                   LCD_RAM_D1_Msk
#define LCD_RAM_D2_Pos               (2U)
#define LCD_RAM_D2_Msk               (0x1UL << LCD_RAM_D2_Pos)              /*!< 0x00000004 */
#define LCD_RAM_D2                   LCD_RAM_D2_Msk
#define LCD_RAM_D3_Pos               (3U)
#define LCD_RAM_D3_Msk               (0x1UL << LCD_RAM_D3_Pos)              /*!< 0x00000008 */
#define LCD_RAM_D3                   LCD_RAM_D3_Msk
#define LCD_RAM_D4_Pos               (4U)
#define LCD_RAM_D4_Msk               (0x1UL << LCD_RAM_D4_Pos)              /*!< 0x00000010 */
#define LCD_RAM_D4                   LCD_RAM_D4_Msk
#define LCD_RAM_D5_Pos               (5U)
#define LCD_RAM_D5_Msk               (0x1UL << LCD_RAM_D5_Pos)              /*!< 0x00000020 */
#define LCD_RAM_D5                   LCD_RAM_D5_Msk
#define LCD_RAM_D6_Pos               (6U)
#define LCD_RAM_D6_Msk               (0x1UL << LCD_RAM_D6_Pos)              /*!< 0x00000040 */
#define LCD_RAM_D6                   LCD_RAM_D6_Msk
#define LCD_RAM_D7_Pos               (7U)
#define LCD_RAM_D7_Msk               (0x1UL << LCD_RAM_D7_Pos)              /*!< 0x00000080 */
#define LCD_RAM_D7                   LCD_RAM_D7_Msk
#define LCD_RAM_D8_Pos               (8U)
#define LCD_RAM_D8_Msk               (0x1UL << LCD_RAM_D8_Pos)              /*!< 0x00000100 */
#define LCD_RAM_D8                   LCD_RAM_D8_Msk
#define LCD_RAM_D9_Pos               (9U)
#define LCD_RAM_D9_Msk               (0x1UL << LCD_RAM_D9_Pos)              /*!< 0x00000200 */
#define LCD_RAM_D9                   LCD_RAM_D9_Msk
#define LCD_RAM_D10_Pos              (10U)
#define LCD_RAM_D10_Msk              (0x1UL << LCD_RAM_D10_Pos)             /*!< 0x00000400 */
#define LCD_RAM_D10                  LCD_RAM_D10_Msk
#define LCD_RAM_D11_Pos              (11U)
#define LCD_RAM_D11_Msk              (0x1UL << LCD_RAM_D11_Pos)             /*!< 0x00000800 */
#define LCD_RAM_D11                  LCD_RAM_D11_Msk
#define LCD_RAM_D12_Pos              (12U)
#define LCD_RAM_D12_Msk              (0x1UL << LCD_RAM_D12_Pos)             /*!< 0x00001000 */
#define LCD_RAM_D12                  LCD_RAM_D12_Msk
#define LCD_RAM_D13_Pos              (13U)
#define LCD_RAM_D13_Msk              (0x1UL << LCD_RAM_D13_Pos)             /*!< 0x00002000 */
#define LCD_RAM_D13                  LCD_RAM_D13_Msk
#define LCD_RAM_D14_Pos              (14U)
#define LCD_RAM_D14_Msk              (0x1UL << LCD_RAM_D14_Pos)             /*!< 0x00004000 */
#define LCD_RAM_D14                  LCD_RAM_D14_Msk
#define LCD_RAM_D15_Pos              (15U)
#define LCD_RAM_D15_Msk              (0x1UL << LCD_RAM_D15_Pos)             /*!< 0x00008000 */
#define LCD_RAM_D15                  LCD_RAM_D15_Msk
#define LCD_RAM_D16_Pos              (16U)
#define LCD_RAM_D16_Msk              (0x1UL << LCD_RAM_D16_Pos)             /*!< 0x00010000 */
#define LCD_RAM_D16                  LCD_RAM_D16_Msk
#define LCD_RAM_D17_Pos              (17U)
#define LCD_RAM_D17_Msk              (0x1UL << LCD_RAM_D17_Pos)             /*!< 0x00020000 */
#define LCD_RAM_D17                  LCD_RAM_D17_Msk
#define LCD_RAM_D18_Pos              (18U)
#define LCD_RAM_D18_Msk              (0x1UL << LCD_RAM_D18_Pos)             /*!< 0x00040000 */
#define LCD_RAM_D18                  LCD_RAM_D18_Msk
#define LCD_RAM_D19_Pos              (19U)
#define LCD_RAM_D19_Msk              (0x1UL << LCD_RAM_D19_Pos)             /*!< 0x00080000 */
#define LCD_RAM_D19                  LCD_RAM_D19_Msk
#define LCD_RAM_D20_Pos              (20U)
#define LCD_RAM_D20_Msk              (0x1UL << LCD_RAM_D20_Pos)             /*!< 0x00100000 */
#define LCD_RAM_D20                  LCD_RAM_D20_Msk
#define LCD_RAM_D21_Pos              (21U)
#define LCD_RAM_D21_Msk              (0x1UL << LCD_RAM_D21_Pos)             /*!< 0x00200000 */
#define LCD_RAM_D21                  LCD_RAM_D21_Msk
#define LCD_RAM_D22_Pos              (22U)
#define LCD_RAM_D22_Msk              (0x1UL << LCD_RAM_D22_Pos)             /*!< 0x00400000 */
#define LCD_RAM_D22                  LCD_RAM_D22_Msk
#define LCD_RAM_D23_Pos              (23U)
#define LCD_RAM_D23_Msk              (0x1UL << LCD_RAM_D23_Pos)             /*!< 0x00800000 */
#define LCD_RAM_D23                  LCD_RAM_D23_Msk
#define LCD_RAM_D24_Pos              (24U)
#define LCD_RAM_D24_Msk              (0x1UL << LCD_RAM_D24_Pos)             /*!< 0x01000000 */
#define LCD_RAM_D24                  LCD_RAM_D24_Msk
#define LCD_RAM_D25_Pos              (25U)
#define LCD_RAM_D25_Msk              (0x1UL << LCD_RAM_D25_Pos)             /*!< 0x02000000 */
#define LCD_RAM_D25                  LCD_RAM_D25_Msk
#define LCD_RAM_D26_Pos              (26U)
#define LCD_RAM_D26_Msk              (0x1UL << LCD_RAM_D26_Pos)             /*!< 0x04000000 */
#define LCD_RAM_D26                  LCD_RAM_D26_Msk
#define LCD_RAM_D27_Pos              (27U)
#define LCD_RAM_D27_Msk              (0x1UL << LCD_RAM_D27_Pos)             /*!< 0x08000000 */
#define LCD_RAM_D27                  LCD_RAM_D27_Msk
#define LCD_RAM_D28_Pos              (28U)
#define LCD_RAM_D28_Msk              (0x1UL << LCD_RAM_D28_Pos)             /*!< 0x10000000 */
#define LCD_RAM_D28                  LCD_RAM_D28_Msk
#define LCD_RAM_D29_Pos              (29U)
#define LCD_RAM_D29_Msk              (0x1UL << LCD_RAM_D29_Pos)             /*!< 0x20000000 */
#define LCD_RAM_D29                  LCD_RAM_D29_Msk
#define LCD_RAM_D30_Pos              (30U)
#define LCD_RAM_D30_Msk              (0x1UL << LCD_RAM_D30_Pos)             /*!< 0x40000000 */
#define LCD_RAM_D30                  LCD_RAM_D30_Msk
#define LCD_RAM_D31_Pos              (31U)
#define LCD_RAM_D31_Msk              (0x1UL << LCD_RAM_D31_Pos)             /*!< 0x80000000 */
#define LCD_RAM_D31                  LCD_RAM_D31_Msk

/******************************************************************************/
/*                                                                            */
/*                         Low Power Timer (LPTIM)                            */
/*                                                                            */
/******************************************************************************/
/******************  Bit definition for LPTIM_ISR register  *******************/
#define LPTIM_ISR_ARRM_Pos          (1U)
#define LPTIM_ISR_ARRM_Msk          (0x1UL << LPTIM_ISR_ARRM_Pos)              /*!< 0x00000002 */
#define LPTIM_ISR_ARRM              LPTIM_ISR_ARRM_Msk                         /*!< Autoreload match */
#define LPTIM_ISR_ARROK_Pos         (4U)
#define LPTIM_ISR_ARROK_Msk         (0x1UL << LPTIM_ISR_ARROK_Pos)              /*!< 0x00000010 */
#define LPTIM_ISR_ARROK             LPTIM_ISR_ARROK_Msk                         /*!< Autoreload Update completed */

/******************  Bit definition for LPTIM_ICR register  *******************/
#define LPTIM_ICR_ARRMCF_Pos        (1U)
#define LPTIM_ICR_ARRMCF_Msk        (0x1UL << LPTIM_ICR_ARRMCF_Pos)            /*!< 0x00000002 */
#define LPTIM_ICR_ARRMCF            LPTIM_ICR_ARRMCF_Msk                       /*!< Autoreload match Clear Flag */
#define LPTIM_ICR_ARROKCF_Pos       (4U)
#define LPTIM_ICR_ARROKCF_Msk       (0x1UL << LPTIM_ICR_ARROKCF_Pos)            /*!< 0x00000010 */
#define LPTIM_ICR_ARROKCF           LPTIM_ICR_ARROKCF_Msk                       /*!< Autoreload Update completed Clear Flag */

/******************  Bit definition for LPTIM_IER register ********************/
#define LPTIM_IER_ARRMIE_Pos        (1U)
#define LPTIM_IER_ARRMIE_Msk        (0x1UL << LPTIM_IER_ARRMIE_Pos)            /*!< 0x00000002 */
#define LPTIM_IER_ARRMIE            LPTIM_IER_ARRMIE_Msk                       /*!< Autoreload match Interrupt Enable */
#define LPTIM_IER_ARROKIE_Pos       (4U)
#define LPTIM_IER_ARROKIE_Msk       (0x1UL << LPTIM_IER_ARROKIE_Pos)            /*!< 0x00000010 */
#define LPTIM_IER_ARROKIE           LPTIM_IER_ARROKIE_Msk                       /*!< Autoreload Update completed Interrupt Enable */

/******************  Bit definition for LPTIM_CFGR register *******************/
#define LPTIM_CFGR_PRESC_Pos        (9U)
#define LPTIM_CFGR_PRESC_Msk        (0x7UL << LPTIM_CFGR_PRESC_Pos)            /*!< 0x00000E00 */
#define LPTIM_CFGR_PRESC            LPTIM_CFGR_PRESC_Msk                       /*!< PRESC[2:0] bits (Clock prescaler) */
#define LPTIM_CFGR_PRESC_0          (0x1UL << LPTIM_CFGR_PRESC_Pos)            /*!< 0x00000200 */
#define LPTIM_CFGR_PRESC_1          (0x2UL << LPTIM_CFGR_PRESC_Pos)            /*!< 0x00000400 */
#define LPTIM_CFGR_PRESC_2          (0x4UL << LPTIM_CFGR_PRESC_Pos)            /*!< 0x00000800 */

#define LPTIM_CFGR_PRELOAD_Pos      (22U)
#define LPTIM_CFGR_PRELOAD_Msk      (0x1UL << LPTIM_CFGR_PRELOAD_Pos)          /*!< 0x00400000 */
#define LPTIM_CFGR_PRELOAD          LPTIM_CFGR_PRELOAD_Msk                     /*!< Reg update mode */

/******************  Bit definition for LPTIM_CR register  ********************/
#define LPTIM_CR_ENABLE_Pos         (0U)
#define LPTIM_CR_ENABLE_Msk         (0x1UL << LPTIM_CR_ENABLE_Pos)             /*!< 0x00000001 */
#define LPTIM_CR_ENABLE             LPTIM_CR_ENABLE_Msk                        /*!< LPTIMer enable */
#define LPTIM_CR_SNGSTRT_Pos        (1U)
#define LPTIM_CR_SNGSTRT_Msk        (0x1UL << LPTIM_CR_SNGSTRT_Pos)            /*!< 0x00000002 */
#define LPTIM_CR_SNGSTRT            LPTIM_CR_SNGSTRT_Msk                       /*!< Timer start in single mode */
#define LPTIM_CR_CNTSTRT_Pos        (2U)
#define LPTIM_CR_CNTSTRT_Msk        (0x1UL << LPTIM_CR_CNTSTRT_Pos)            /*!< 0x00000004 */
#define LPTIM_CR_CNTSTRT            LPTIM_CR_CNTSTRT_Msk                       /*!< Timer start in continue mode */
#define LPTIM_CR_COUNTRST_Pos       (3U)
#define LPTIM_CR_COUNTRST_Msk       (0x1UL << LPTIM_CR_COUNTRST_Pos)           /*!< 0x00000008 */
#define LPTIM_CR_COUNTRST           LPTIM_CR_COUNTRST_Msk                      /*!< Timer start in continue mode */
#define LPTIM_CR_RSTARE_Pos         (4U)
#define LPTIM_CR_RSTARE_Msk         (0x1UL << LPTIM_CR_RSTARE_Pos)             /*!< 0x00000010 */
#define LPTIM_CR_RSTARE             LPTIM_CR_RSTARE_Msk                        /*!< Reset after read enable */

/******************  Bit definition for LPTIM_ARR register  *******************/
#define LPTIM_ARR_ARR_Pos           (0U)
#define LPTIM_ARR_ARR_Msk           (0xFFFFUL << LPTIM_ARR_ARR_Pos)            /*!< 0x0000FFFF */
#define LPTIM_ARR_ARR               LPTIM_ARR_ARR_Msk                          /*!< Auto reload register */

/******************  Bit definition for LPTIM_CNT register  *******************/
#define LPTIM_CNT_CNT_Pos           (0U)
#define LPTIM_CNT_CNT_Msk           (0xFFFFUL << LPTIM_CNT_CNT_Pos)            /*!< 0x0000FFFF */
#define LPTIM_CNT_CNT               LPTIM_CNT_CNT_Msk                          /*!< Counter register */

/******************************************************************************/
/*                                                                            */
/*      Universal Synchronous Asynchronous Receiver Transmitter (USART)       */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for USART_SR register  *******************/
#define USART_SR_PE_Pos                     (0U)
#define USART_SR_PE_Msk                     (0x1UL << USART_SR_PE_Pos)          /*!< 0x00000001 */
#define USART_SR_PE                         USART_SR_PE_Msk                     /*!< Parity Error */
#define USART_SR_FE_Pos                     (1U)
#define USART_SR_FE_Msk                     (0x1UL << USART_SR_FE_Pos)          /*!< 0x00000002 */
#define USART_SR_FE                         USART_SR_FE_Msk                     /*!< Framing Error */
#define USART_SR_NE_Pos                     (2U)
#define USART_SR_NE_Msk                     (0x1UL << USART_SR_NE_Pos)          /*!< 0x00000004 */
#define USART_SR_NE                         USART_SR_NE_Msk                     /*!< Noise Error Flag */
#define USART_SR_ORE_Pos                    (3U)
#define USART_SR_ORE_Msk                    (0x1UL << USART_SR_ORE_Pos)         /*!< 0x00000008 */
#define USART_SR_ORE                        USART_SR_ORE_Msk                    /*!< OverRun Error */
#define USART_SR_IDLE_Pos                   (4U)
#define USART_SR_IDLE_Msk                   (0x1UL << USART_SR_IDLE_Pos)        /*!< 0x00000010 */
#define USART_SR_IDLE                       USART_SR_IDLE_Msk                   /*!< IDLE line detected */
#define USART_SR_RXNE_Pos                   (5U)
#define USART_SR_RXNE_Msk                   (0x1UL << USART_SR_RXNE_Pos)        /*!< 0x00000020 */
#define USART_SR_RXNE                       USART_SR_RXNE_Msk                   /*!< Read Data Register Not Empty */
#define USART_SR_TC_Pos                     (6U)
#define USART_SR_TC_Msk                     (0x1UL << USART_SR_TC_Pos)          /*!< 0x00000040 */
#define USART_SR_TC                         USART_SR_TC_Msk                     /*!< Transmission Complete */
#define USART_SR_TXE_Pos                    (7U)
#define USART_SR_TXE_Msk                    (0x1UL << USART_SR_TXE_Pos)         /*!< 0x00000080 */
#define USART_SR_TXE                        USART_SR_TXE_Msk                    /*!< Transmit Data Register Empty */
#define USART_SR_LBD_Pos                    (8U)
#define USART_SR_LBD_Msk                    (0x1UL << USART_SR_LBD_Pos)         /*!< 0x00000100 */
#define USART_SR_LBD                        USART_SR_LBD_Msk                    /*!< LIN break detection flag */
#define USART_SR_CTS_Pos                    (9U)
#define USART_SR_CTS_Msk                    (0x1UL << USART_SR_CTS_Pos)         /*!< 0x00000200 */
#define USART_SR_CTS                        USART_SR_CTS_Msk                    /*!< CTS Flag */
#define USART_SR_ABRF_Pos                   (10U)
#define USART_SR_ABRF_Msk                   (0x1UL << USART_SR_ABRF_Pos)        /*!< 0x00000400 */
#define USART_SR_ABRF                       USART_SR_ABRF_Msk                   /*!< Auto brr detection Flag */
#define USART_SR_ABRE_Pos                   (11U)
#define USART_SR_ABRE_Msk                   (0x1UL << USART_SR_ABRE_Pos)        /*!< 0x00000800 */
#define USART_SR_ABRE                       USART_SR_ABRE_Msk                   /*!< Auto brr detection err Flag */
#define USART_SR_ABRRQ_Pos                  (12U)
#define USART_SR_ABRRQ_Msk                  (0x1UL << USART_SR_ABRRQ_Pos)       /*!< 0x00001000 */
#define USART_SR_ABRRQ                      USART_SR_ABRRQ_Msk                  /*!< Auto brr detection err Flag */

/*******************  Bit definition for USART_DR register  *******************/
#define USART_DR_DR_Pos                     (0U)
#define USART_DR_DR_Msk                     (0x1FFUL << USART_DR_DR_Pos)        /*!< 0x000001FF */
#define USART_DR_DR                         USART_DR_DR_Msk                     /*!< Data value */

/******************  Bit definition for USART_BRR register  *******************/
#define USART_BRR_DIV_Fraction_Pos          (0U)
#define USART_BRR_DIV_Fraction_Msk          (0xFUL << USART_BRR_DIV_Fraction_Pos)   /*!< 0x0000000F */
#define USART_BRR_DIV_Fraction              USART_BRR_DIV_Fraction_Msk              /*!< Fraction of USARTDIV */
#define USART_BRR_DIV_Mantissa_Pos          (4U)
#define USART_BRR_DIV_Mantissa_Msk          (0xFFFUL << USART_BRR_DIV_Mantissa_Pos) /*!< 0x0000FFF0 */
#define USART_BRR_DIV_Mantissa              USART_BRR_DIV_Mantissa_Msk              /*!< Mantissa of USARTDIV */

/******************  Bit definition for USART_CR1 register  *******************/
#define USART_CR1_SBK_Pos                   (0U)
#define USART_CR1_SBK_Msk                   (0x1UL << USART_CR1_SBK_Pos)        /*!< 0x00000001 */
#define USART_CR1_SBK                       USART_CR1_SBK_Msk                   /*!< Send Break */
#define USART_CR1_RWU_Pos                   (1U)
#define USART_CR1_RWU_Msk                   (0x1UL << USART_CR1_RWU_Pos)        /*!< 0x00000002 */
#define USART_CR1_RWU                       USART_CR1_RWU_Msk                   /*!< Receiver wakeup */
#define USART_CR1_RE_Pos                    (2U)
#define USART_CR1_RE_Msk                    (0x1UL << USART_CR1_RE_Pos)         /*!< 0x00000004 */
#define USART_CR1_RE                        USART_CR1_RE_Msk                    /*!< Receiver Enable */
#define USART_CR1_TE_Pos                    (3U)
#define USART_CR1_TE_Msk                    (0x1UL << USART_CR1_TE_Pos)         /*!< 0x00000008 */
#define USART_CR1_TE                        USART_CR1_TE_Msk                    /*!< Transmitter Enable */
#define USART_CR1_IDLEIE_Pos                (4U)
#define USART_CR1_IDLEIE_Msk                (0x1UL << USART_CR1_IDLEIE_Pos)     /*!< 0x00000010 */
#define USART_CR1_IDLEIE                    USART_CR1_IDLEIE_Msk                /*!< IDLE Interrupt Enable */
#define USART_CR1_RXNEIE_Pos                (5U)
#define USART_CR1_RXNEIE_Msk                (0x1UL << USART_CR1_RXNEIE_Pos)     /*!< 0x00000020 */
#define USART_CR1_RXNEIE                    USART_CR1_RXNEIE_Msk                /*!< RXNE Interrupt Enable */
#define USART_CR1_TCIE_Pos                  (6U)
#define USART_CR1_TCIE_Msk                  (0x1UL << USART_CR1_TCIE_Pos)       /*!< 0x00000040 */
#define USART_CR1_TCIE                      USART_CR1_TCIE_Msk                  /*!< Transmission Complete Interrupt Enable */
#define USART_CR1_TXEIE_Pos                 (7U)
#define USART_CR1_TXEIE_Msk                 (0x1UL << USART_CR1_TXEIE_Pos)      /*!< 0x00000080 */
#define USART_CR1_TXEIE                     USART_CR1_TXEIE_Msk                 /*!< PE Interrupt Enable */
#define USART_CR1_PEIE_Pos                  (8U)
#define USART_CR1_PEIE_Msk                  (0x1UL << USART_CR1_PEIE_Pos)       /*!< 0x00000100 */
#define USART_CR1_PEIE                      USART_CR1_PEIE_Msk                  /*!< PE Interrupt Enable */
#define USART_CR1_PS_Pos                    (9U)
#define USART_CR1_PS_Msk                    (0x1UL << USART_CR1_PS_Pos)         /*!< 0x00000200 */
#define USART_CR1_PS                        USART_CR1_PS_Msk                    /*!< Parity Selection */
#define USART_CR1_PCE_Pos                   (10U)
#define USART_CR1_PCE_Msk                   (0x1UL << USART_CR1_PCE_Pos)        /*!< 0x00000400 */
#define USART_CR1_PCE                       USART_CR1_PCE_Msk                   /*!< Parity Control Enable */
#define USART_CR1_WAKE_Pos                  (11U)
#define USART_CR1_WAKE_Msk                  (0x1UL << USART_CR1_WAKE_Pos)       /*!< 0x00000800 */
#define USART_CR1_WAKE                      USART_CR1_WAKE_Msk                  /*!< Wakeup method */
#define USART_CR1_M_Pos                     (12U)
#define USART_CR1_M_Msk                     (0x1UL << USART_CR1_M_Pos)          /*!< 0x00001000 */
#define USART_CR1_M                         USART_CR1_M_Msk                     /*!< Word length */
#define USART_CR1_UE_Pos                    (13U)
#define USART_CR1_UE_Msk                    (0x1UL << USART_CR1_UE_Pos)         /*!< 0x00002000 */
#define USART_CR1_UE                        USART_CR1_UE_Msk                    /*!< USART Enable */

/******************  Bit definition for USART_CR2 register  *******************/
#define USART_CR2_ADD_Pos                   (0U)
#define USART_CR2_ADD_Msk                   (0xFUL << USART_CR2_ADD_Pos)       /*!< 0x0000000F */
#define USART_CR2_ADD                       USART_CR2_ADD_Msk                  /*!< Address of the USART node */
#define USART_CR2_ADD_0                     (0x1UL << USART_CR2_ADD_Pos)       /*!< 0x00000001 */
#define USART_CR2_ADD_1                     (0x2UL << USART_CR2_ADD_Pos)       /*!< 0x00000002 */
#define USART_CR2_ADD_2                     (0x4UL << USART_CR2_ADD_Pos)       /*!< 0x00000004 */
#define USART_CR2_ADD_3                     (0x8UL << USART_CR2_ADD_Pos)       /*!< 0x00000008 */
#define USART_CR2_LBDL_Pos                  (5U)
#define USART_CR2_LBDL_Msk                  (0x1UL << USART_CR2_LBDL_Pos)      /*!< 0x00000020 */
#define USART_CR2_LBDL                      USART_CR2_LBDL_Msk                 /*!< LIN break detect length */
#define USART_CR2_LBDIE_Pos                 (6U)
#define USART_CR2_LBDIE_Msk                 (0x1UL << USART_CR2_LBDIE_Pos)     /*!< 0x00000040 */
#define USART_CR2_LBDIE                     USART_CR2_LBDIE_Msk                /*!< LIN break interrupt enable */
#define USART_CR2_LBCL_Pos                  (8U)
#define USART_CR2_LBCL_Msk                  (0x1UL << USART_CR2_LBCL_Pos)      /*!< 0x00000100 */
#define USART_CR2_LBCL                      USART_CR2_LBCL_Msk                 /*!< Last Bit Clock pulse */
#define USART_CR2_CPHA_Pos                  (9U)
#define USART_CR2_CPHA_Msk                  (0x1UL << USART_CR2_CPHA_Pos)      /*!< 0x00000200 */
#define USART_CR2_CPHA                      USART_CR2_CPHA_Msk                 /*!< Clock Phase */
#define USART_CR2_CPOL_Pos                  (10U)
#define USART_CR2_CPOL_Msk                  (0x1UL << USART_CR2_CPOL_Pos)      /*!< 0x00000400 */
#define USART_CR2_CPOL                      USART_CR2_CPOL_Msk                 /*!< Clock Polarity */
#define USART_CR2_CLKEN_Pos                 (11U)
#define USART_CR2_CLKEN_Msk                 (0x1UL << USART_CR2_CLKEN_Pos)     /*!< 0x00000800 */
#define USART_CR2_CLKEN                     USART_CR2_CLKEN_Msk                /*!< Clock Enable */
#define USART_CR2_STOP_Pos                  (12U)
#define USART_CR2_STOP_Msk                  (0x3UL << USART_CR2_STOP_Pos)      /*!< 0x00003000 */
#define USART_CR2_STOP                      USART_CR2_STOP_Msk                 /*!< STOP bits*/
#define USART_CR2_STOP_0                    (0x1UL << USART_CR2_STOP_Pos)      /*!< 0x00001000 */
#define USART_CR2_STOP_1                    (0x2UL << USART_CR2_STOP_Pos)      /*!< 0x00002000 */
#define USART_CR2_LINEN_Pos                 (14U)
#define USART_CR2_LINEN_Msk                 (0x1UL << USART_CR2_LINEN_Pos)     /*!< 0x00004000 */
#define USART_CR2_LINEN                     USART_CR2_LINEN_Msk                /*!< LIN enable */

/******************  Bit definition for USART_CR3 register  *******************/
#define USART_CR3_EIE_Pos                   (0U)
#define USART_CR3_EIE_Msk                   (0x1UL << USART_CR3_EIE_Pos)       /*!< 0x00000001 */
#define USART_CR3_EIE                       USART_CR3_EIE_Msk                  /*!< Error Interrupt Enable */
#define USART_CR3_IREN_Pos                  (1U)
#define USART_CR3_IREN_Msk                  (0x1UL << USART_CR3_IREN_Pos)      /*!< 0x00000002 */
#define USART_CR3_IREN                      USART_CR3_IREN_Msk                 /*!< desc IREN */
#define USART_CR3_IRLP_Pos                  (2U)
#define USART_CR3_IRLP_Msk                  (0x1UL << USART_CR3_IRLP_Pos)      /*!< 0x00000004 */
#define USART_CR3_IRLP                      USART_CR3_IRLP_Msk                 /*!< desc IRLP */
#define USART_CR3_HDSEL_Pos                 (3U)
#define USART_CR3_HDSEL_Msk                 (0x1UL << USART_CR3_HDSEL_Pos)     /*!< 0x00000008 */
#define USART_CR3_HDSEL                     USART_CR3_HDSEL_Msk                /*!< Half-Duplex Selection */
#define USART_CR3_NACK_Pos                  (4U)
#define USART_CR3_NACK_Msk                  (0x1UL << USART_CR3_NACK_Pos)      /*!< 0x00000010 */
#define USART_CR3_NACK                      USART_CR3_NACK_Msk                 /*!< desc NACK */
#define USART_CR3_SCEN_Pos                  (5U)
#define USART_CR3_SCEN_Msk                  (0x1UL << USART_CR3_SCEN_Pos)      /*!< 0x00000020 */
#define USART_CR3_SCEN                      USART_CR3_SCEN_Msk                 /*!< desc SCEN */
#define USART_CR3_DMAR_Pos                  (6U)
#define USART_CR3_DMAR_Msk                  (0x1UL << USART_CR3_DMAR_Pos)      /*!< 0x00000040 */
#define USART_CR3_DMAR                      USART_CR3_DMAR_Msk                 /*!< DMA Enable Receiver */
#define USART_CR3_DMAT_Pos                  (7U)
#define USART_CR3_DMAT_Msk                  (0x1UL << USART_CR3_DMAT_Pos)      /*!< 0x00000080 */
#define USART_CR3_DMAT                      USART_CR3_DMAT_Msk                 /*!< DMA Enable Transmitter */
#define USART_CR3_RTSE_Pos                  (8U)
#define USART_CR3_RTSE_Msk                  (0x1UL << USART_CR3_RTSE_Pos)      /*!< 0x00000100 */
#define USART_CR3_RTSE                      USART_CR3_RTSE_Msk                 /*!< RTS Enable */
#define USART_CR3_CTSE_Pos                  (9U)
#define USART_CR3_CTSE_Msk                  (0x1UL << USART_CR3_CTSE_Pos)      /*!< 0x00000200 */
#define USART_CR3_CTSE                      USART_CR3_CTSE_Msk                 /*!< CTS Enable */
#define USART_CR3_CTSIE_Pos                 (10U)
#define USART_CR3_CTSIE_Msk                 (0x1UL << USART_CR3_CTSIE_Pos)     /*!< 0x00000400 */
#define USART_CR3_CTSIE                     USART_CR3_CTSIE_Msk                /*!< CTS Interrupt Enable */
#define USART_CR3_OVER8_Pos                 (11U)
#define USART_CR3_OVER8_Msk                 (0x1UL <<USART_CR3_OVER8_Pos)      /*!< 0x00000800 */
#define USART_CR3_OVER8                     USART_CR3_OVER8_Msk                /*!< Oversampling mode */
#define USART_CR3_ABREN_Pos                 (12U)
#define USART_CR3_ABREN_Msk                 (0x1UL <<USART_CR3_ABREN_Pos)      /*!< 0x00001000 */
#define USART_CR3_ABREN                     USART_CR3_ABREN_Msk                /*!< Auto baudrate enable */
#define USART_CR3_ABRMODE_Pos               (13U)
#define USART_CR3_ABRMODE_Msk               (0x3UL <<USART_CR3_ABRMODE_Pos)    /*!< 0x00006000 */
#define USART_CR3_ABRMODE                   USART_CR3_ABRMODE_Msk              /*!< ABRMODE[14:13] bits (desc ABRMODE) */
#define USART_CR3_ABRMODE_0                 (0x1UL <<USART_CR3_ABRMODE_Pos)    /*!< 0x00002000 */
#define USART_CR3_ABRMODE_1                 (0x2UL <<USART_CR3_ABRMODE_Pos)    /*!< 0x00004000 */

/*!< USART_GTPR */
#define USART_GTPR_PSC_Pos                  (0U)
#define USART_GTPR_PSC_Msk                  (0xFFUL << USART_GTPR_PSC_Pos)     /*!< 0x000000FF */
#define USART_GTPR_PSC                      USART_GTPR_PSC_Msk                 /*!< PSC[7:0] bits (desc PSC) */
#define USART_GTPR_GT_Pos                   (8U)
#define USART_GTPR_GT_Msk                   (0xFFUL << USART_GTPR_GT_Pos)      /*!< 0x0000FF00 */
#define USART_GTPR_GT                       USART_GTPR_GT_Msk                  /*!< GT[15:8] bits (desc GT) */

/*********************  Bits Define For Peripheral USBD  *********************/
/*!< USBD_CR */
#define USBD_CR_ADD_Pos                         (0U)
#define USBD_CR_ADD_Msk                         (0x7FUL << USBD_CR_ADD_Pos)                       /*!< 0x0000007F */
#define USBD_CR_ADD                             USBD_CR_ADD_Msk                                   /*!< desc ADD */
#define USBD_CR_ADD_0                           (0x1UL << USBD_CR_ADD_Pos)                        /*!< 0x00000001 */
#define USBD_CR_ADD_1                           (0x2UL << USBD_CR_ADD_Pos)                        /*!< 0x00000002 */
#define USBD_CR_ADD_2                           (0x4UL << USBD_CR_ADD_Pos)                        /*!< 0x00000004 */
#define USBD_CR_ADD_3                           (0x8UL << USBD_CR_ADD_Pos)                        /*!< 0x00000008 */
#define USBD_CR_ADD_4                           (0x10UL << USBD_CR_ADD_Pos)                       /*!< 0x00000010 */
#define USBD_CR_ADD_5                           (0x20UL << USBD_CR_ADD_Pos)                       /*!< 0x00000020 */
#define USBD_CR_ADD_6                           (0x40UL << USBD_CR_ADD_Pos)                       /*!< 0x00000040 */
#define USBD_CR_UPDATE_Pos                      (7U)
#define USBD_CR_UPDATE_Msk                      (0x1UL << USBD_CR_UPDATE_Pos)                     /*!< 0x00000080 */
#define USBD_CR_UPDATE                          USBD_CR_UPDATE_Msk                                /*!< desc UPDATE */
#define USBD_CR_Enable_Suspend_Pos              (8U)
#define USBD_CR_Enable_Suspend_Msk              (0x1UL << USBD_CR_Enable_Suspend_Pos)             /*!< 0x00000100 */
#define USBD_CR_Enable_Suspend                  USBD_CR_Enable_Suspend_Msk                        /*!< desc Enable_Suspend */
#define USBD_CR_Suspend_Mode_Pos                (9U)
#define USBD_CR_Suspend_Mode_Msk                (0x1UL << USBD_CR_Suspend_Mode_Pos)               /*!< 0x00000200 */
#define USBD_CR_Suspend_Mode                    USBD_CR_Suspend_Mode_Msk                          /*!< desc Suspend_Mode */
#define USBD_CR_Resume_Pos                      (10U)
#define USBD_CR_Resume_Msk                      (0x1UL << USBD_CR_Resume_Pos)                     /*!< 0x00000400 */
#define USBD_CR_Resume                          USBD_CR_Resume_Msk                                /*!< desc Resume */
#define USBD_CR_Reset_Pos                       (11U)
#define USBD_CR_Reset_Msk                       (0x1UL << USBD_CR_Reset_Pos)                      /*!< 0x00000800 */
#define USBD_CR_Reset                           USBD_CR_Reset_Msk                                 /*!< desc Reset */
#define USBD_CR_ISO_Update_Pos                  (15U)
#define USBD_CR_ISO_Update_Msk                  (0x1UL << USBD_CR_ISO_Update_Pos)                 /*!< 0x00008000 */
#define USBD_CR_ISO_Update                      USBD_CR_ISO_Update_Msk                            /*!< desc ISO_Update */

/*!< USBD_INTR */
#define USBD_INTR_Suspend_Pos                   (0U)
#define USBD_INTR_Suspend_Msk                   (0x1UL << USBD_INTR_Suspend_Pos)                  /*!< 0x00000001 */
#define USBD_INTR_Suspend                       USBD_INTR_Suspend_Msk                             /*!< desc Suspend */
#define USBD_INTR_Resume_Pos                    (1U)
#define USBD_INTR_Resume_Msk                    (0x1UL << USBD_INTR_Resume_Pos)                   /*!< 0x00000002 */
#define USBD_INTR_Resume                        USBD_INTR_Resume_Msk                              /*!< desc Resume */
#define USBD_INTR_Reset_Pos                     (2U)
#define USBD_INTR_Reset_Msk                     (0x1UL << USBD_INTR_Reset_Pos)                    /*!< 0x00000004 */
#define USBD_INTR_Reset                         USBD_INTR_Reset_Msk                               /*!< desc Reset */
#define USBD_INTR_SOF_Pos                       (3U)
#define USBD_INTR_SOF_Msk                       (0x1UL << USBD_INTR_SOF_Pos)                      /*!< 0x00000008 */
#define USBD_INTR_SOF                           USBD_INTR_SOF_Msk                                 /*!< desc SOF */
#define USBD_INTR_EP1OUT_Pos                    (9U)
#define USBD_INTR_EP1OUT_Msk                    (0x1UL << USBD_INTR_EP1OUT_Pos)                   /*!< 0x00000200 */
#define USBD_INTR_EP1OUT                        USBD_INTR_EP1OUT_Msk                              /*!< desc EP1OUT */
#define USBD_INTR_EP2OUT_Pos                    (10U)
#define USBD_INTR_EP2OUT_Msk                    (0x1UL << USBD_INTR_EP2OUT_Pos)                   /*!< 0x00000400 */
#define USBD_INTR_EP2OUT                        USBD_INTR_EP2OUT_Msk                              /*!< desc EP2OUT */
#define USBD_INTR_EP3OUT_Pos                    (11U)
#define USBD_INTR_EP3OUT_Msk                    (0x1UL << USBD_INTR_EP3OUT_Pos)                   /*!< 0x00000800 */
#define USBD_INTR_EP3OUT                        USBD_INTR_EP3OUT_Msk                              /*!< desc EP3OUT */
#define USBD_INTR_EP4OUT_Pos                    (12U)
#define USBD_INTR_EP4OUT_Msk                    (0x1UL << USBD_INTR_EP4OUT_Pos)                   /*!< 0x00001000 */
#define USBD_INTR_EP4OUT                        USBD_INTR_EP4OUT_Msk                              /*!< desc EP4OUT */
#define USBD_INTR_EP5OUT_Pos                    (13U)
#define USBD_INTR_EP5OUT_Msk                    (0x1UL << USBD_INTR_EP5OUT_Pos)                   /*!< 0x00002000 */
#define USBD_INTR_EP5OUT                        USBD_INTR_EP5OUT_Msk                              /*!< desc EP5OUT */
#define USBD_INTR_EP0_Pos                       (16U)
#define USBD_INTR_EP0_Msk                       (0x1UL << USBD_INTR_EP0_Pos)                      /*!< 0x00010000 */
#define USBD_INTR_EP0                           USBD_INTR_EP0_Msk                                 /*!< desc EP0 */
#define USBD_INTR_EP1IN_Pos                     (17U)
#define USBD_INTR_EP1IN_Msk                     (0x1UL << USBD_INTR_EP1IN_Pos)                    /*!< 0x00020000 */
#define USBD_INTR_EP1IN                         USBD_INTR_EP1IN_Msk                               /*!< desc EP1IN */
#define USBD_INTR_EP2IN_Pos                     (18U)
#define USBD_INTR_EP2IN_Msk                     (0x1UL << USBD_INTR_EP2IN_Pos)                    /*!< 0x00040000 */
#define USBD_INTR_EP2IN                         USBD_INTR_EP2IN_Msk                               /*!< desc EP2IN */
#define USBD_INTR_EP3IN_Pos                     (19U)
#define USBD_INTR_EP3IN_Msk                     (0x1UL << USBD_INTR_EP3IN_Pos)                    /*!< 0x00080000 */
#define USBD_INTR_EP3IN                         USBD_INTR_EP3IN_Msk                               /*!< desc EP3IN */
#define USBD_INTR_EP4IN_Pos                     (20U)
#define USBD_INTR_EP4IN_Msk                     (0x1UL << USBD_INTR_EP4IN_Pos)                    /*!< 0x00100000 */
#define USBD_INTR_EP4IN                         USBD_INTR_EP4IN_Msk                               /*!< desc EP4IN */
#define USBD_INTR_EP5IN_Pos                     (21U)
#define USBD_INTR_EP5IN_Msk                     (0x1UL << USBD_INTR_EP5IN_Pos)                    /*!< 0x00200000 */
#define USBD_INTR_EP5IN                         USBD_INTR_EP5IN_Msk                               /*!< desc EP5IN */

/*!< USBD_INTRE */
#define USBD_INTRE_EN_Suspend_Pos               (0U)
#define USBD_INTRE_EN_Suspend_Msk               (0x1UL << USBD_INTRE_EN_Suspend_Pos)              /*!< 0x00000001 */
#define USBD_INTRE_EN_Suspend                   USBD_INTRE_EN_Suspend_Msk                         /*!< desc EN_Suspend */
#define USBD_INTRE_EN_Resume_Pos                (1U)
#define USBD_INTRE_EN_Resume_Msk                (0x1UL << USBD_INTRE_EN_Resume_Pos)               /*!< 0x00000002 */
#define USBD_INTRE_EN_Resume                    USBD_INTRE_EN_Resume_Msk                          /*!< desc EN_Resume */
#define USBD_INTRE_EN_Reset_Pos                 (2U)
#define USBD_INTRE_EN_Reset_Msk                 (0x1UL << USBD_INTRE_EN_Reset_Pos)                /*!< 0x00000004 */
#define USBD_INTRE_EN_Reset                     USBD_INTRE_EN_Reset_Msk                           /*!< desc EN_Reset */
#define USBD_INTRE_EN_SOF_Pos                   (3U)
#define USBD_INTRE_EN_SOF_Msk                   (0x1UL << USBD_INTRE_EN_SOF_Pos)                  /*!< 0x00000008 */
#define USBD_INTRE_EN_SOF                       USBD_INTRE_EN_SOF_Msk                             /*!< desc EN_SOF */
#define USBD_INTRE_EP1OUTE_Pos                  (9U)
#define USBD_INTRE_EP1OUTE_Msk                  (0x1UL << USBD_INTRE_EP1OUTE_Pos)                 /*!< 0x00000200 */
#define USBD_INTRE_EP1OUTE                      USBD_INTRE_EP1OUTE_Msk                            /*!< desc EP1OUTE */
#define USBD_INTRE_EP2OUTE_Pos                  (10U)
#define USBD_INTRE_EP2OUTE_Msk                  (0x1UL << USBD_INTRE_EP2OUTE_Pos)                 /*!< 0x00000400 */
#define USBD_INTRE_EP2OUTE                      USBD_INTRE_EP2OUTE_Msk                            /*!< desc EP2OUTE */
#define USBD_INTRE_EP3OUTE_Pos                  (11U)
#define USBD_INTRE_EP3OUTE_Msk                  (0x1UL << USBD_INTRE_EP3OUTE_Pos)                 /*!< 0x00000800 */
#define USBD_INTRE_EP3OUTE                      USBD_INTRE_EP3OUTE_Msk                            /*!< desc EP3OUTE */
#define USBD_INTRE_EP4OUTE_Pos                  (12U)
#define USBD_INTRE_EP4OUTE_Msk                  (0x1UL << USBD_INTRE_EP4OUTE_Pos)                 /*!< 0x00001000 */
#define USBD_INTRE_EP4OUTE                      USBD_INTRE_EP4OUTE_Msk                            /*!< desc EP4OUTE */
#define USBD_INTRE_EP5OUTE_Pos                  (13U)
#define USBD_INTRE_EP5OUTE_Msk                  (0x1UL << USBD_INTRE_EP5OUTE_Pos)                 /*!< 0x00002000 */
#define USBD_INTRE_EP5OUTE                      USBD_INTRE_EP5OUTE_Msk                            /*!< desc EP5OUTE */
#define USBD_INTRE_EP0_Pos                      (16U)
#define USBD_INTRE_EP0_Msk                      (0x1UL << USBD_INTRE_EP0_Pos)                     /*!< 0x00010000 */
#define USBD_INTRE_EP0                          USBD_INTRE_EP0_Msk                                /*!< desc EP0 */
#define USBD_INTRE_EP1INE_Pos                   (17U)
#define USBD_INTRE_EP1INE_Msk                   (0x1UL << USBD_INTRE_EP1INE_Pos)                  /*!< 0x00020000 */
#define USBD_INTRE_EP1INE                       USBD_INTRE_EP1INE_Msk                             /*!< desc EP1INE */
#define USBD_INTRE_EP2INE_Pos                   (18U)
#define USBD_INTRE_EP2INE_Msk                   (0x1UL << USBD_INTRE_EP2INE_Pos)                  /*!< 0x00040000 */
#define USBD_INTRE_EP2INE                       USBD_INTRE_EP2INE_Msk                             /*!< desc EP2INE */
#define USBD_INTRE_EP3INE_Pos                   (19U)
#define USBD_INTRE_EP3INE_Msk                   (0x1UL << USBD_INTRE_EP3INE_Pos)                  /*!< 0x00080000 */
#define USBD_INTRE_EP3INE                       USBD_INTRE_EP3INE_Msk                             /*!< desc EP3INE */
#define USBD_INTRE_EP4INE_Pos                   (20U)
#define USBD_INTRE_EP4INE_Msk                   (0x1UL << USBD_INTRE_EP4INE_Pos)                  /*!< 0x00100000 */
#define USBD_INTRE_EP4INE                       USBD_INTRE_EP4INE_Msk                             /*!< desc EP4INE */
#define USBD_INTRE_EP5INE_Pos                   (21U)
#define USBD_INTRE_EP5INE_Msk                   (0x1UL << USBD_INTRE_EP5INE_Pos)                  /*!< 0x00200000 */
#define USBD_INTRE_EP5INE                       USBD_INTRE_EP5INE_Msk                             /*!< desc EP5INE */

/*!< USBD_FRAME */
#define USBD_FRAME_FramNUM_Pos                  (0U)
#define USBD_FRAME_FramNUM_Msk                  (0x7FFUL << USBD_FRAME_FramNUM_Pos)               /*!< 0x0000007F */
#define USBD_FRAME_FramNUM                      USBD_FRAME_FramNUM_Msk                            /*!< desc FramNUM */
#define USBD_FRAME_FramNUM_0                    (0x1UL << USBD_FRAME_FramNUM_Pos)                 /*!< 0x00000001 */
#define USBD_FRAME_FramNUM_1                    (0x2UL << USBD_FRAME_FramNUM_Pos)                 /*!< 0x00000002 */
#define USBD_FRAME_FramNUM_2                    (0x4UL << USBD_FRAME_FramNUM_Pos)                 /*!< 0x00000004 */
#define USBD_FRAME_FramNUM_3                    (0x8UL << USBD_FRAME_FramNUM_Pos)                 /*!< 0x00000008 */
#define USBD_FRAME_FramNUM_4                    (0x10UL << USBD_FRAME_FramNUM_Pos)                /*!< 0x00000010 */
#define USBD_FRAME_FramNUM_5                    (0x20UL << USBD_FRAME_FramNUM_Pos)                /*!< 0x00000020 */
#define USBD_FRAME_FramNUM_6                    (0x40UL << USBD_FRAME_FramNUM_Pos)                /*!< 0x00000040 */
#define USBD_FRAME_FramNUM_7                    (0x8UL << USBD_FRAME_FramNUM_Pos)                 /*!< 0x00000008 */
#define USBD_FRAME_FramNUM_8                    (0x10UL << USBD_FRAME_FramNUM_Pos)                /*!< 0x00000010 */
#define USBD_FRAME_FramNUM_9                    (0x20UL << USBD_FRAME_FramNUM_Pos)                /*!< 0x00000020 */
#define USBD_FRAME_FramNUM_10                   (0x40UL << USBD_FRAME_FramNUM_Pos)                /*!< 0x00000040 */
#define USBD_FRAME_INDEX_Pos                    (16U)
#define USBD_FRAME_INDEX_Msk                    (0xFUL << USBD_FRAME_INDEX_Pos)                   /*!< 0x000F0000 */
#define USBD_FRAME_INDEX                        USBD_FRAME_INDEX_Msk                              /*!< desc INDEX */
#define USBD_FRAME_INDEX_0                      (0x1UL << USBD_FRAME_INDEX_Pos)                   /*!< 0x00000001 */
#define USBD_FRAME_INDEX_1                      (0x2UL << USBD_FRAME_INDEX_Pos)                   /*!< 0x00000002 */
#define USBD_FRAME_INDEX_2                      (0x4UL << USBD_FRAME_INDEX_Pos)                   /*!< 0x00000004 */
#define USBD_FRAME_INDEX_3                      (0x8UL << USBD_FRAME_INDEX_Pos)                   /*!< 0x00000008 */

/*!< USBD_EP0CSR */
#define USBD_EP0CSR_OutPktRdy_Pos               (0U)
#define USBD_EP0CSR_OutPktRdy_Msk               (0x1UL << USBD_EP0CSR_OutPktRdy_Pos)              /*!< 0x00000001 */
#define USBD_EP0CSR_OutPktRdy                   USBD_EP0CSR_OutPktRdy_Msk                         /*!< desc OutPktRdy */
#define USBD_EP0CSR_InPktRdy_Pos                (1U)
#define USBD_EP0CSR_InPktRdy_Msk                (0x1UL << USBD_EP0CSR_InPktRdy_Pos)               /*!< 0x00000002 */
#define USBD_EP0CSR_InPktRdy                    USBD_EP0CSR_InPktRdy_Msk                          /*!< desc InPktRdy */
#define USBD_EP0CSR_SentStall_Pos               (2U)
#define USBD_EP0CSR_SentStall_Msk               (0x1UL << USBD_EP0CSR_SentStall_Pos)              /*!< 0x00000004 */
#define USBD_EP0CSR_SentStall                   USBD_EP0CSR_SentStall_Msk                         /*!< desc SentStall */
#define USBD_EP0CSR_DataEnd_Pos                 (3U)
#define USBD_EP0CSR_DataEnd_Msk                 (0x1UL << USBD_EP0CSR_DataEnd_Pos)                /*!< 0x00000008 */
#define USBD_EP0CSR_DataEnd                     USBD_EP0CSR_DataEnd_Msk                           /*!< desc DataEnd */
#define USBD_EP0CSR_SetupEnd_Pos                (4U)
#define USBD_EP0CSR_SetupEnd_Msk                (0x1UL << USBD_EP0CSR_SetupEnd_Pos)               /*!< 0x00000010 */
#define USBD_EP0CSR_SetupEnd                    USBD_EP0CSR_SetupEnd_Msk                          /*!< desc SetupEnd */
#define USBD_EP0CSR_SendStall_Pos               (5U)
#define USBD_EP0CSR_SendStall_Msk               (0x1UL << USBD_EP0CSR_SendStall_Pos)              /*!< 0x00000020 */
#define USBD_EP0CSR_SendStall                   USBD_EP0CSR_SendStall_Msk                         /*!< desc SendStall */
#define USBD_EP0CSR_ServicedOutPktRdy_Pos       (6U)
#define USBD_EP0CSR_ServicedOutPktRdy_Msk       (0x1UL << USBD_EP0CSR_ServicedOutPktRdy_Pos)      /*!< 0x00000040 */
#define USBD_EP0CSR_ServicedOutPktRdy           USBD_EP0CSR_ServicedOutPktRdy_Msk                 /*!< desc ServicedOutPktRdy */
#define USBD_EP0CSR_ServicedSetupEnd_Pos        (7U)
#define USBD_EP0CSR_ServicedSetupEnd_Msk        (0x1UL << USBD_EP0CSR_ServicedSetupEnd_Pos)       /*!< 0x00000080 */
#define USBD_EP0CSR_ServicedSetupEnd            USBD_EP0CSR_ServicedSetupEnd_Msk                  /*!< desc ServicedSetupEnd */
#define USBD_EP0CSR_COUNT0_Pos                  (8U)
#define USBD_EP0CSR_COUNT0_Msk                  (0x1UL << USBD_EP0CSR_COUNT0_Pos)                 /*!< 0x00007F00 */
#define USBD_EP0CSR_COUNT0                      USBD_EP0CSR_COUNT0_Msk                            /*!< desc COUNT0 */

/*!< USBD_INEPxCSR */
#define USBD_INEPxCSR_FrcDataTog_Pos            (3U)
#define USBD_INEPxCSR_FrcDataTog_Msk            (0x1UL << USBD_INEPxCSR_FrcDataTog_Pos)           /*!< 0x00000008 */
#define USBD_INEPxCSR_FrcDataTog                USBD_INEPxCSR_FrcDataTog_Msk                      /*!< desc FrcDataTog */
#define USBD_INEPxCSR_DMAEnab_Pos               (4U)
#define USBD_INEPxCSR_DMAEnab_Msk               (0x1UL << USBD_INEPxCSR_DMAEnab_Pos)              /*!< 0x00000010 */
#define USBD_INEPxCSR_DMAEnab                   USBD_INEPxCSR_DMAEnab_Msk                         /*!< desc DMAEnab */
#define USBD_INEPxCSR_Mode_Pos                  (5U)
#define USBD_INEPxCSR_Mode_Msk                  (0x1UL << USBD_INEPxCSR_Mode_Pos)                 /*!< 0x00000020 */
#define USBD_INEPxCSR_Mode                      USBD_INEPxCSR_Mode_Msk                            /*!< desc Mode */
#define USBD_INEPxCSR_ISO_Pos                   (6U)
#define USBD_INEPxCSR_ISO_Msk                   (0x1UL << USBD_INEPxCSR_ISO_Pos)                  /*!< 0x00000040 */
#define USBD_INEPxCSR_ISO                       USBD_INEPxCSR_ISO_Msk                             /*!< desc ISO */
#define USBD_INEPxCSR_AutoSet_Pos               (7U)
#define USBD_INEPxCSR_AutoSet_Msk               (0x1UL << USBD_INEPxCSR_AutoSet_Pos)              /*!< 0x00000080 */
#define USBD_INEPxCSR_AutoSet                   USBD_INEPxCSR_AutoSet_Msk                         /*!< desc AutoSet */
#define USBD_INEPxCSR_InPktRdy_Pos              (8U)
#define USBD_INEPxCSR_InPktRdy_Msk              (0x1UL << USBD_INEPxCSR_InPktRdy_Pos)             /*!< 0x00000100 */
#define USBD_INEPxCSR_InPktRdy                  USBD_INEPxCSR_InPktRdy_Msk                        /*!< desc InPktRdy */
#define USBD_INEPxCSR_FIFONotEmpty_Pos          (9U)
#define USBD_INEPxCSR_FIFONotEmpty_Msk          (0x1UL << USBD_INEPxCSR_FIFONotEmpty_Pos)         /*!< 0x00000200 */
#define USBD_INEPxCSR_FIFONotEmpty              USBD_INEPxCSR_FIFONotEmpty_Msk                    /*!< desc FIFONotEmpty */
#define USBD_INEPxCSR_UnderRun_Pos              (10U)
#define USBD_INEPxCSR_UnderRun_Msk              (0x1UL << USBD_INEPxCSR_UnderRun_Pos)             /*!< 0x00000400 */
#define USBD_INEPxCSR_UnderRun                  USBD_INEPxCSR_UnderRun_Msk                        /*!< desc UnderRun */
#define USBD_INEPxCSR_FlushFIFO_Pos             (11U)
#define USBD_INEPxCSR_FlushFIFO_Msk             (0x1UL << USBD_INEPxCSR_FlushFIFO_Pos)            /*!< 0x00000800 */
#define USBD_INEPxCSR_FlushFIFO                 USBD_INEPxCSR_FlushFIFO_Msk                       /*!< desc FlushFIFO */
#define USBD_INEPxCSR_SendStall_Pos             (12U)
#define USBD_INEPxCSR_SendStall_Msk             (0x1UL << USBD_INEPxCSR_SendStall_Pos)            /*!< 0x00001000 */
#define USBD_INEPxCSR_SendStall                 USBD_INEPxCSR_SendStall_Msk                       /*!< desc SendStall */
#define USBD_INEPxCSR_SentStall_Pos             (13U)
#define USBD_INEPxCSR_SentStall_Msk             (0x1UL << USBD_INEPxCSR_SentStall_Pos)            /*!< 0x00002000 */
#define USBD_INEPxCSR_SentStall                 USBD_INEPxCSR_SentStall_Msk                       /*!< desc SentStall */
#define USBD_INEPxCSR_ClrDataTog_Pos            (14U)
#define USBD_INEPxCSR_ClrDataTog_Msk            (0x1UL << USBD_INEPxCSR_ClrDataTog_Pos)           /*!< 0x00004000 */
#define USBD_INEPxCSR_ClrDataTog                USBD_INEPxCSR_ClrDataTog_Msk                      /*!< desc ClrDataTog */
#define USBD_INEPxCSR_INMAXP_Pos                (16U)
#define USBD_INEPxCSR_INMAXP_Msk                (0x1UL << USBD_INEPxCSR_INMAXP_Pos)               /*!< 0x00FF0000 */
#define USBD_INEPxCSR_INMAXP                    USBD_INEPxCSR_INMAXP_Msk                          /*!< desc INMAXP */

/*!< USBD_OUTEPxCSR */
#define USBD_OUTEPxCSR_DMAMode_Pos              (4U)
#define USBD_OUTEPxCSR_DMAMode_Msk              (0x1UL << USBD_OUTEPxCSR_DMAMode_Pos)             /*!< 0x00000010 */
#define USBD_OUTEPxCSR_DMAMode                  USBD_OUTEPxCSR_DMAMode_Msk                        /*!< desc DMAMode */
#define USBD_OUTEPxCSR_DMAEnab_Pos              (5U)
#define USBD_OUTEPxCSR_DMAEnab_Msk              (0x1UL << USBD_OUTEPxCSR_DMAEnab_Pos)             /*!< 0x00000020 */
#define USBD_OUTEPxCSR_DMAEnab                  USBD_OUTEPxCSR_DMAEnab_Msk                        /*!< desc DMAEnab */
#define USBD_OUTEPxCSR_ISO_Pos                  (6U)
#define USBD_OUTEPxCSR_ISO_Msk                  (0x1UL << USBD_OUTEPxCSR_ISO_Pos)                 /*!< 0x00000040 */
#define USBD_OUTEPxCSR_ISO                      USBD_OUTEPxCSR_ISO_Msk                            /*!< desc ISO */
#define USBD_OUTEPxCSR_AutoClear_Pos            (7U)
#define USBD_OUTEPxCSR_AutoClear_Msk            (0x1UL << USBD_OUTEPxCSR_AutoClear_Pos)           /*!< 0x00000080 */
#define USBD_OUTEPxCSR_AutoClear                USBD_OUTEPxCSR_AutoClear_Msk                      /*!< desc AutoClear */
#define USBD_OUTEPxCSR_OutPktRdy_Pos            (8U)
#define USBD_OUTEPxCSR_OutPktRdy_Msk            (0x1UL << USBD_OUTEPxCSR_OutPktRdy_Pos)           /*!< 0x00000100 */
#define USBD_OUTEPxCSR_OutPktRdy                USBD_OUTEPxCSR_OutPktRdy_Msk                      /*!< desc OutPktRdy */
#define USBD_OUTEPxCSR_FIFOFull_Pos             (9U)
#define USBD_OUTEPxCSR_FIFOFull_Msk             (0x1UL << USBD_OUTEPxCSR_FIFOFull_Pos)            /*!< 0x00000200 */
#define USBD_OUTEPxCSR_FIFOFull                 USBD_OUTEPxCSR_FIFOFull_Msk                       /*!< desc FIFOFull */
#define USBD_OUTEPxCSR_OverRun_Pos              (10U)
#define USBD_OUTEPxCSR_OverRun_Msk              (0x1UL << USBD_OUTEPxCSR_OverRun_Pos)             /*!< 0x00000400 */
#define USBD_OUTEPxCSR_OverRun                  USBD_OUTEPxCSR_OverRun_Msk                        /*!< desc OverRun */
#define USBD_OUTEPxCSR_DataError_Pos            (11U)
#define USBD_OUTEPxCSR_DataError_Msk            (0x1UL << USBD_OUTEPxCSR_DataError_Pos)           /*!< 0x00000800 */
#define USBD_OUTEPxCSR_DataError                USBD_OUTEPxCSR_DataError_Msk                      /*!< desc DataError */
#define USBD_OUTEPxCSR_FlushFIFO_Pos            (12U)
#define USBD_OUTEPxCSR_FlushFIFO_Msk            (0x1UL << USBD_OUTEPxCSR_FlushFIFO_Pos)           /*!< 0x00001000 */
#define USBD_OUTEPxCSR_FlushFIFO                USBD_OUTEPxCSR_FlushFIFO_Msk                      /*!< desc FlushFIFO */
#define USBD_OUTEPxCSR_SendStall_Pos            (13U)
#define USBD_OUTEPxCSR_SendStall_Msk            (0x1UL << USBD_OUTEPxCSR_SendStall_Pos)           /*!< 0x00002000 */
#define USBD_OUTEPxCSR_SendStall                USBD_OUTEPxCSR_SendStall_Msk                      /*!< desc SendStall */
#define USBD_OUTEPxCSR_SentStall_Pos            (14U)
#define USBD_OUTEPxCSR_SentStall_Msk            (0x1UL << USBD_OUTEPxCSR_SentStall_Pos)           /*!< 0x00004000 */
#define USBD_OUTEPxCSR_SentStall                USBD_OUTEPxCSR_SentStall_Msk                      /*!< desc SentStall */
#define USBD_OUTEPxCSR_ClrDataTog_Pos           (15U)
#define USBD_OUTEPxCSR_ClrDataTog_Msk           (0x1UL << USBD_OUTEPxCSR_ClrDataTog_Pos)          /*!< 0x00008000 */
#define USBD_OUTEPxCSR_ClrDataTog               USBD_OUTEPxCSR_ClrDataTog_Msk                     /*!< desc ClrDataTog */
#define USBD_OUTEPxCSR_INMAXP_Pos               (16U)
#define USBD_OUTEPxCSR_INMAXP_Msk               (0x1UL << USBD_OUTEPxCSR_INMAXP_Pos)              /*!< 0x00FF0000 */
#define USBD_OUTEPxCSR_INMAXP                   USBD_OUTEPxCSR_INMAXP_Msk                         /*!< desc INMAXP */

/*!< USBD_OUTCOUNT */
#define USBD_OUTCOUNT_OUTCOUNT_Pos              (0U)
#define USBD_OUTCOUNT_OUTCOUNT_Msk              (0x3FFUL << USBD_OUTCOUNT_OUTCOUNT_Pos)           /*!< 0x000003FF */
#define USBD_OUTCOUNT_OUTCOUNT                  USBD_OUTCOUNT_OUTCOUNT_Msk                        /*!< desc OUTCOUNT */

/******************************************************************************/
/*                                                                            */
/*                            Window WATCHDOG (WWDG)                          */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for WWDG_CR register  ********************/
#define WWDG_CR_T_Pos           (0U)
#define WWDG_CR_T_Msk           (0x7FUL << WWDG_CR_T_Pos)                      /*!< 0x0000007F */
#define WWDG_CR_T               WWDG_CR_T_Msk                                  /*!<T[6:0] bits (7-Bit counter (MSB to LSB)) */
#define WWDG_CR_T_0             (0x01UL << WWDG_CR_T_Pos)                      /*!< 0x00000001 */
#define WWDG_CR_T_1             (0x02UL << WWDG_CR_T_Pos)                      /*!< 0x00000002 */
#define WWDG_CR_T_2             (0x04UL << WWDG_CR_T_Pos)                      /*!< 0x00000004 */
#define WWDG_CR_T_3             (0x08UL << WWDG_CR_T_Pos)                      /*!< 0x00000008 */
#define WWDG_CR_T_4             (0x10UL << WWDG_CR_T_Pos)                      /*!< 0x00000010 */
#define WWDG_CR_T_5             (0x20UL << WWDG_CR_T_Pos)                      /*!< 0x00000020 */
#define WWDG_CR_T_6             (0x40UL << WWDG_CR_T_Pos)                      /*!< 0x00000040 */
#define WWDG_CR_WDGA_Pos        (7U)
#define WWDG_CR_WDGA_Msk        (0x1UL << WWDG_CR_WDGA_Pos)                    /*!< 0x00000080 */
#define WWDG_CR_WDGA            WWDG_CR_WDGA_Msk                               /*!<Activation bit */

/*******************  Bit definition for WWDG_CFR register  *******************/
#define WWDG_CFR_W_Pos          (0U)
#define WWDG_CFR_W_Msk          (0x7FUL << WWDG_CFR_W_Pos)                     /*!< 0x0000007F */
#define WWDG_CFR_W              WWDG_CFR_W_Msk                                 /*!<W[6:0] bits (7-bit window value) */
#define WWDG_CFR_W_0            (0x01UL << WWDG_CFR_W_Pos)                     /*!< 0x00000001 */
#define WWDG_CFR_W_1            (0x02UL << WWDG_CFR_W_Pos)                     /*!< 0x00000002 */
#define WWDG_CFR_W_2            (0x04UL << WWDG_CFR_W_Pos)                     /*!< 0x00000004 */
#define WWDG_CFR_W_3            (0x08UL << WWDG_CFR_W_Pos)                     /*!< 0x00000008 */
#define WWDG_CFR_W_4            (0x10UL << WWDG_CFR_W_Pos)                     /*!< 0x00000010 */
#define WWDG_CFR_W_5            (0x20UL << WWDG_CFR_W_Pos)                     /*!< 0x00000020 */
#define WWDG_CFR_W_6            (0x40UL << WWDG_CFR_W_Pos)                     /*!< 0x00000040 */
#define WWDG_CFR_WDGTB_Pos      (7U)
#define WWDG_CFR_WDGTB_Msk      (0x3UL << WWDG_CFR_WDGTB_Pos)                  /*!< 0x00001800 */
#define WWDG_CFR_WDGTB          WWDG_CFR_WDGTB_Msk                             /*!<WDGTB[1:0] bits (Timer Base) */
#define WWDG_CFR_WDGTB_0        (0x1UL << WWDG_CFR_WDGTB_Pos)                  /*!< 0x00000800 */
#define WWDG_CFR_WDGTB_1        (0x2UL << WWDG_CFR_WDGTB_Pos)                  /*!< 0x00001000 */
#define WWDG_CFR_EWI_Pos        (9U)
#define WWDG_CFR_EWI_Msk        (0x1UL << WWDG_CFR_EWI_Pos)                    /*!< 0x00000200 */
#define WWDG_CFR_EWI            WWDG_CFR_EWI_Msk                               /*!<Early Wakeup Interrupt */

/*******************  Bit definition for WWDG_SR register  ********************/
#define WWDG_SR_EWIF_Pos        (0U)
#define WWDG_SR_EWIF_Msk        (0x1UL << WWDG_SR_EWIF_Pos)                    /*!< 0x00000001 */
#define WWDG_SR_EWIF            WWDG_SR_EWIF_Msk                               /*!<Early Wakeup Interrupt Flag */

/** @addtogroup Exported_macros
  * @{
  */

/******************************* ADC Instances ********************************/
#define IS_ADC_ALL_INSTANCE(INSTANCE) ((INSTANCE) == ADC1)

#define IS_ADC_COMMON_INSTANCE(INSTANCE) ((INSTANCE) == ADC)

/****************************** CAN Instances *********************************/
#define IS_CAN_ALL_INSTANCE(INSTANCE) ((INSTANCE) == CAN1)

/******************************* CRC Instances ********************************/
#define IS_CRC_ALL_INSTANCE(INSTANCE) ((INSTANCE) == CRC)

/****************************** CRC Instances *********************************/
#define IS_CTC_ALL_INSTANCE(INSTANCE) ((INSTANCE) == CTC)

/****************************** DAC Instances *********************************/
#define IS_DAC_ALL_INSTANCE(INSTANCE) ((INSTANCE) == DAC1)

/******************************** DMA Instances *******************************/
#define IS_DMA_ALL_INSTANCE(INSTANCE)  (((INSTANCE) == DMA1_Channel1) || \
                                        ((INSTANCE) == DMA1_Channel2) || \
                                        ((INSTANCE) == DMA1_Channel3) || \
                                        ((INSTANCE) == DMA1_Channel4) || \
                                        ((INSTANCE) == DMA1_Channel5) || \
                                        ((INSTANCE) == DMA1_Channel6) || \
                                        ((INSTANCE) == DMA1_Channel7))

/******************************* GPIO Instances *******************************/
#define IS_GPIO_ALL_INSTANCE(INSTANCE) (((INSTANCE) == GPIOA) || \
                                        ((INSTANCE) == GPIOB) || \
                                        ((INSTANCE) == GPIOC) || \
                                        ((INSTANCE) == GPIOF))

/********************** GPIO Alternate Function Instances *********************/
#define IS_GPIO_AF_INSTANCE(INSTANCE)   IS_GPIO_ALL_INSTANCE(INSTANCE)

/**************************** GPIO Lock Instances *****************************/
#define IS_GPIO_LOCK_INSTANCE(INSTANCE) IS_GPIO_ALL_INSTANCE(INSTANCE)

/****************************** DIV Instances *********************************/
#define IS_DIV_ALL_INSTANCE(INSTANCE) ((INSTANCE) == DIV)

/******************************** I2C Instances *******************************/
#define IS_I2C_ALL_INSTANCE(INSTANCE)  (((INSTANCE) == I2C1) || \
                                        ((INSTANCE) == I2C2))

/****************************** WAKEUP_FROMSTOP Instances *********************/
#define IS_I2C_WAKEUP_FROMSTOP_INSTANCE(INSTANCE) IS_I2C_ALL_INSTANCE(INSTANCE)

/****************************** OPA Instances *********************************/
#define IS_OPA_ALL_INSTANCE(INSTANCE) ((INSTANCE) == OPA)

/****************************** RTC Instances *********************************/
#define IS_RTC_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == RTC)

/******************************* SMBUS Instances ******************************/
#define IS_SMBUS_ALL_INSTANCE         IS_I2C_ALL_INSTANCE

/******************************** SPI Instances *******************************/
#define IS_SPI_ALL_INSTANCE(INSTANCE)  (((INSTANCE) == SPI1) || \
                                        ((INSTANCE) == SPI2))

/******************************** SPI Instances *******************************/
#define IS_I2S_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == SPI1)

/****************** LPTIM Instances : All supported instances *****************/
#define IS_LPTIM_INSTANCE(INSTANCE)     ((INSTANCE) == LPTIM1)

/****************** LPTIM Instances : All supported instances *****************/
#define IS_LPTIM_ENCODER_INTERFACE_INSTANCE(INSTANCE) ((INSTANCE) == LPTIM1)

/****************************** START TIM Instances ***************************/
/****************** TIM Instances : All supported instances *******************/
#define IS_TIM_INSTANCE(INSTANCE)        (((INSTANCE) == TIM1)   || \
                                          ((INSTANCE) == TIM2)   || \
                                          ((INSTANCE) == TIM3)   || \
                                          ((INSTANCE) == TIM6)   || \
                                          ((INSTANCE) == TIM7)   || \
                                          ((INSTANCE) == TIM14)  || \
                                          ((INSTANCE) == TIM15)  || \
                                          ((INSTANCE) == TIM16)  || \
                                          ((INSTANCE) == TIM17))

/****************** TIM Instances : supporting the break function *************/
#define IS_TIM_BREAK_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)    || \
                                            ((INSTANCE) == TIM15)   || \
                                            ((INSTANCE) == TIM16)   || \
                                            ((INSTANCE) == TIM17))

/************** TIM Instances : supporting Break source selection *************/
#define IS_TIM_BREAKSOURCE_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1)   || \
                                                ((INSTANCE) == TIM16)  || \
                                                ((INSTANCE) == TIM17))

/****************** TIM Instances : supporting 2 break inputs *****************/
#define IS_TIM_BKIN2_INSTANCE(INSTANCE)    ((INSTANCE) == TIM1)

/************* TIM Instances : at least 1 capture/compare channel *************/
#define IS_TIM_CC1_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)   || \
                                          ((INSTANCE) == TIM3)   || \
                                          ((INSTANCE) == TIM14)  || \
                                          ((INSTANCE) == TIM16)  || \
                                          ((INSTANCE) == TIM17))

/************ TIM Instances : at least 2 capture/compare channels *************/
#define IS_TIM_CC2_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)   || \
                                          ((INSTANCE) == TIM3))

/************ TIM Instances : at least 3 capture/compare channels *************/
#define IS_TIM_CC3_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)   || \
                                          ((INSTANCE) == TIM3))

/************ TIM Instances : at least 4 capture/compare channels *************/
#define IS_TIM_CC4_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)   || \
                                          ((INSTANCE) == TIM3))

/****************** TIM Instances : at least 5 capture/compare channels *******/
#define IS_TIM_CC5_INSTANCE(INSTANCE)   ((INSTANCE) == TIM1)

/****************** TIM Instances : at least 6 capture/compare channels *******/
#define IS_TIM_CC6_INSTANCE(INSTANCE)   ((INSTANCE) == TIM1)

/************ TIM Instances : DMA requests generation (TIMx_DIER.COMDE) *******/
#define IS_TIM_CCDMA_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)   || \
                                            ((INSTANCE) == TIM16)  || \
                                            ((INSTANCE) == TIM17))

/************ TIM Instances : DMA requests generation (TIMx_DIER.UDE) *********/
#define IS_TIM_DMA_INSTANCE(INSTANCE)      (((INSTANCE) == TIM1)   || \
                                            ((INSTANCE) == TIM3)   || \
                                            ((INSTANCE) == TIM16)  || \
                                            ((INSTANCE) == TIM17))

/************ TIM Instances : DMA requests generation (TIMx_DIER.CCxDE) *******/
#define IS_TIM_DMA_CC_INSTANCE(INSTANCE)   (((INSTANCE) == TIM1)   || \
                                            ((INSTANCE) == TIM3)   || \
                                            ((INSTANCE) == TIM14)  || \
                                            ((INSTANCE) == TIM16)  || \
                                            ((INSTANCE) == TIM17))

/******************** TIM Instances : DMA burst feature ***********************/
#define IS_TIM_DMABURST_INSTANCE(INSTANCE) (((INSTANCE) == TIM1)   || \
                                            ((INSTANCE) == TIM3)   || \
                                            ((INSTANCE) == TIM16)  || \
                                            ((INSTANCE) == TIM17))

/******************* TIM Instances : output(s) available **********************/
#define IS_TIM_CCX_INSTANCE(INSTANCE, CHANNEL) \
    ((((INSTANCE) == TIM1) &&                  \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
     ||                                        \
     (((INSTANCE) == TIM3) &&                  \
     (((CHANNEL) == TIM_CHANNEL_1) ||          \
      ((CHANNEL) == TIM_CHANNEL_2) ||          \
      ((CHANNEL) == TIM_CHANNEL_3) ||          \
      ((CHANNEL) == TIM_CHANNEL_4)))           \
     ||                                        \
     (((INSTANCE) == TIM14) &&                 \
     (((CHANNEL) == TIM_CHANNEL_1)))           \
     ||                                        \
     (((INSTANCE) == TIM16) &&                 \
     (((CHANNEL) == TIM_CHANNEL_1)))           \
     ||                                        \
     (((INSTANCE) == TIM17) &&                 \
      (((CHANNEL) == TIM_CHANNEL_1))))

/****************** TIM Instances : supporting complementary output(s) ********/
#define IS_TIM_CCXN_INSTANCE(INSTANCE, CHANNEL) \
    ((((INSTANCE) == TIM1) &&                   \
     (((CHANNEL) == TIM_CHANNEL_1) ||           \
      ((CHANNEL) == TIM_CHANNEL_2) ||           \
      ((CHANNEL) == TIM_CHANNEL_3)))            \
    ||                                          \
    (((INSTANCE) == TIM16) &&                   \
     ((CHANNEL) == TIM_CHANNEL_1))              \
    ||                                          \
    (((INSTANCE) == TIM17) &&                   \
     ((CHANNEL) == TIM_CHANNEL_1)))

/****************** TIM Instances : supporting clock division *****************/
#define IS_TIM_CLOCK_DIVISION_INSTANCE(INSTANCE)   (((INSTANCE) == TIM1)    || \
                                                    ((INSTANCE) == TIM3)    || \
                                                    ((INSTANCE) == TIM14)   || \
                                                    ((INSTANCE) == TIM16)   || \
                                                    ((INSTANCE) == TIM17))

/****** TIM Instances : supporting external clock mode 1 for ETRF input *******/
#define IS_TIM_CLOCKSOURCE_ETRMODE1_INSTANCE(INSTANCE) (((INSTANCE) == TIM1) || \
                                                        ((INSTANCE) == TIM3))

/****** TIM Instances : supporting external clock mode 2 for ETRF input *******/
#define IS_TIM_CLOCKSOURCE_ETRMODE2_INSTANCE(INSTANCE) (((INSTANCE) == TIM1) || \
                                                        ((INSTANCE) == TIM3))

/********** TIM Instances : supporting external clock mode 1 for TIX inputs****/
#define IS_TIM_CLOCKSOURCE_TIX_INSTANCE(INSTANCE)      (((INSTANCE) == TIM1) || \
                                                        ((INSTANCE) == TIM3))

/********** TIM Instances : supporting internal trigger inputs(ITRX) **********/
#define IS_TIM_CLOCKSOURCE_ITRX_INSTANCE(INSTANCE)     (((INSTANCE) == TIM1) || \
                                                        ((INSTANCE) == TIM3))

/****************** TIM Instances : supporting combined 3-phase PWM mode ******/
#define IS_TIM_COMBINED3PHASEPWM_INSTANCE(INSTANCE)    ((INSTANCE) == TIM1)

/****************** TIM Instances : supporting commutation event generation ***/
#define IS_TIM_COMMUTATION_EVENT_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1)   || \
                                                      ((INSTANCE) == TIM16)  || \
                                                      ((INSTANCE) == TIM17))

/****************** TIM Instances : supporting counting mode selection ********/
#define IS_TIM_COUNTER_MODE_SELECT_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1) || \
                                                        ((INSTANCE) == TIM3))

/****************** TIM Instances : supporting encoder interface **************/
#define IS_TIM_ENCODER_INTERFACE_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1)  || \
                                                      ((INSTANCE) == TIM3))

/****************** TIM Instances : supporting Hall sensor interface **********/
#define IS_TIM_HALL_SENSOR_INTERFACE_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1)   || \
                                                          ((INSTANCE) == TIM3))

/**************** TIM Instances : external trigger input available ************/
#define IS_TIM_ETR_INSTANCE(INSTANCE)      (((INSTANCE) == TIM1)  || \
                                            ((INSTANCE) == TIM3))

/************* TIM Instances : supporting ETR source selection ****************/
#define IS_TIM_ETRSEL_INSTANCE(INSTANCE)     (((INSTANCE) == TIM1)  || \
                                              ((INSTANCE) == TIM3))

/****** TIM Instances : Master mode available (TIMx_CR2.MMS available )********/
#define IS_TIM_MASTER_INSTANCE(INSTANCE)   (((INSTANCE) == TIM1)  || \
                                            ((INSTANCE) == TIM3))

/*********** TIM Instances : Slave mode available (TIMx_SMCR available )*******/
#define IS_TIM_SLAVE_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)  || \
                                            ((INSTANCE) == TIM3))

/****************** TIM Instances : supporting OCxREF clear *******************/
#define IS_TIM_OCXREF_CLEAR_INSTANCE(INSTANCE)         (((INSTANCE) == TIM1) || \
                                                        ((INSTANCE) == TIM3))

/****************** TIM Instances : remapping capability **********************/
#define IS_TIM_REMAP_INSTANCE(INSTANCE)      (((INSTANCE) == TIM1)   || \
                                              ((INSTANCE) == TIM2)   || \
                                              ((INSTANCE) == TIM3)   || \
                                              ((INSTANCE) == TIM14)  || \
                                              ((INSTANCE) == TIM15)  || \
                                              ((INSTANCE) == TIM16)  || \
                                              ((INSTANCE) == TIM17))

/****************** TIM Instances : supporting repetition counter *************/
#define IS_TIM_REPETITION_COUNTER_INSTANCE(INSTANCE)   (((INSTANCE) == TIM1)  || \
                                                        ((INSTANCE) == TIM2)  || \
                                                        ((INSTANCE) == TIM3))

/****************** TIM Instances : supporting synchronization ****************/
#define IS_TIM_SYNCHRO_INSTANCE(INSTANCE)  IS_TIM_MASTER_INSTANCE(INSTANCE)

/****************** TIM Instances : supporting ADC triggering through TRGO2 ***/
#define IS_TIM_TRGO2_INSTANCE(INSTANCE)    (((INSTANCE) == TIM1)   || \
                                            ((INSTANCE) == TIM2)   || \
                                            ((INSTANCE) == TIM3)   || \
                                            ((INSTANCE) == TIM15))

/******************* TIM Instances : Timer input XOR function *****************/
#define IS_TIM_XOR_INSTANCE(INSTANCE)      (((INSTANCE) == TIM1)   || \
                                            ((INSTANCE) == TIM2)   || \
                                            ((INSTANCE) == TIM3))

/******************* TIM Instances : Timer input selection ********************/
#define IS_TIM_TISEL_INSTANCE(INSTANCE)  (((INSTANCE) == TIM1)   || \
                                          ((INSTANCE) == TIM2)   || \
                                          ((INSTANCE) == TIM3)   || \
                                          ((INSTANCE) == TIM14)  || \
                                          ((INSTANCE) == TIM15)  || \
                                          ((INSTANCE) == TIM16)  || \
                                          ((INSTANCE) == TIM17))

/************ TIM Instances : Advanced timers  ********************************/
#define IS_TIM_ADVANCED_INSTANCE(INSTANCE) (((INSTANCE) == TIM1))

/******************** UART Instances : Asynchronous mode **********************/
#define IS_UART_INSTANCE(INSTANCE)         (((INSTANCE) == USART1) || \
                                            ((INSTANCE) == USART2) || \
                                            ((INSTANCE) == USART3) || \
                                            ((INSTANCE) == USART4))
/******************** USART Instances : Synchronous mode **********************/
#define IS_USART_INSTANCE(INSTANCE)        (((INSTANCE) == USART1) || \
                                            ((INSTANCE) == USART2) || \
                                            ((INSTANCE) == USART3) || \
                                            ((INSTANCE) == USART4))
/****************** UART Instances : Hardware Flow control ********************/
#define IS_UART_HWFLOW_INSTANCE(INSTANCE)  (((INSTANCE) == USART1) || \
                                            ((INSTANCE) == USART2) || \
                                            ((INSTANCE) == USART3) || \
                                            ((INSTANCE) == USART4))
/********************* USART Instances : Smard card mode **********************/
#define IS_SMARTCARD_INSTANCE(INSTANCE)    (((INSTANCE) == USART1) || \
                                            ((INSTANCE) == USART2))
/****************** UART Instances : Auto Baud Rate detection *****************/
#define IS_USART_AUTOBAUDRATE_DETECTION_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                                            ((INSTANCE) == USART2) || \
                                                            ((INSTANCE) == USART3) || \
                                                            ((INSTANCE) == USART4))
/******************** UART Instances : Half-Duplex mode ***********************/
#define IS_UART_HALFDUPLEX_INSTANCE(INSTANCE)  (((INSTANCE) == USART1) || \
                                                ((INSTANCE) == USART2) || \
                                                ((INSTANCE) == USART3) || \
                                                ((INSTANCE) == USART4))
/******************** UART Instances : LIN mode *******************************/
#define IS_UART_LIN_INSTANCE(INSTANCE)   (((INSTANCE) == USART1) || \
                                          ((INSTANCE) == USART2))
/******************** UART Instances : Wake-up from Stop mode *****************/
#define IS_UART_WAKEUP_FROMSTOP_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                                    ((INSTANCE) == USART2) || \
                                                    ((INSTANCE) == USART3) || \
                                                    ((INSTANCE) == USART4))
/****************** UART Instances : Driver Enable ****************************/
#define IS_UART_DRIVER_ENABLE_INSTANCE(INSTANCE)   (((INSTANCE) == USART1) || \
                                                    ((INSTANCE) == USART2) || \
                                                    ((INSTANCE) == USART3) || \
                                                    ((INSTANCE) == USART4))
/****************** UART Instances : SPI Slave selection mode *****************/
#define IS_UART_SPI_SLAVE_INSTANCE(INSTANCE) 	   (((INSTANCE) == USART1) || \
                                                  ((INSTANCE) == USART2) || \
                                                  ((INSTANCE) == USART3) || \
                                                  ((INSTANCE) == USART4))
/****************** UART Instances : Driver Enable ****************************/
#define IS_UART_FIFO_INSTANCE(INSTANCE)     	   (((INSTANCE) == USART1) || \
                                                  ((INSTANCE) == USART2) || \
                                                  ((INSTANCE) == USART3) || \
                                                  ((INSTANCE) == USART4))
/*********************** IRDA Instances : IRDA mode ***************************/
#define IS_IRDA_INSTANCE(INSTANCE) (((INSTANCE) == USART1) || \
                                    ((INSTANCE) == USART2))

/****************************** IWDG Instances ********************************/
#define IS_IWDG_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == IWDG)

/****************************** WWDG Instances ********************************/
#define IS_WWDG_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == WWDG)

/****************************** LCD Instances *********************************/
#define IS_LCD_ALL_INSTANCE(INSTANCE)  ((INSTANCE) == LCD)

/****************************** COMP Instances ********************************/
#define IS_COMP_ALL_INSTANCE(INSTANCE) (((INSTANCE) == COMP1) || \
                                        ((INSTANCE) == COMP2) || \
                                        ((INSTANCE) == COMP3))

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
#endif /* __cplusplus */

#endif /* __PY32F072XB_H */

/**
  * @}
  */

/**
* @}
*/

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/

