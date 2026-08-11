/**
  ******************************************************************************
  * @file      startup_py32f072.s
  * @brief     PY32F072 devices vector table for GCC toolchain.
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == Reset_Handler,
  *                - Set the vector table entries with the exceptions ISR address
  *                - Branches to main in the C library (which eventually calls main()).
  *            After Reset the Cortex-M0 processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
  ******************************************************************************
  */

  .syntax unified
  .cpu cortex-m0plus
  .fpu softvfp
  .thumb

.global g_pfnVectors
.global Default_Handler

/* start address for the initialization values of the .data section.
defined in linker script */
.word _sidata
/* start address for the .data section. defined in linker script */
.word _sdata
/* end address for the .data section. defined in linker script */
.word _edata
/* start address for the .bss section. defined in linker script */
.word _sbss
/* end address for the .bss section. defined in linker script */
.word _ebss

  .section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:
  ldr   r0, =_estack
  mov   sp, r0          /* set stack pointer */

/* Copy the data segment initializers from flash to SRAM */
  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

LoopCopyDataInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc CopyDataInit

/* Zero fill the bss segment. */
  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss

FillZerobss:
  str  r3, [r2]
  adds r2, r2, #4

LoopFillZerobss:
  cmp r2, r4
  bcc FillZerobss

/* Call the clock system intitialization function.*/
  bl  SystemInit
/* Call static constructors. Remove this line if compile with `-nostartfiles` reports error */
  bl __libc_init_array
/* Call the application's entry point.*/
  bl main

LoopForever:
    b LoopForever


.size Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None
 * @retval : None
*/
    .section .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b Infinite_Loop
  .size Default_Handler, .-Default_Handler
/******************************************************************************
*
* The minimal vector table for a Cortex M0.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/
   .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object
  .size g_pfnVectors, .-g_pfnVectors


g_pfnVectors:
  .word  _estack
  .word  Reset_Handler                  /* Reset Handler */
  .word  NMI_Handler                    /* NMI Handler */
  .word  HardFault_Handler              /* Hard Fault Handler */
  .word  0                              /* Reserved */
  .word  0                              /* Reserved */
  .word  0                              /* Reserved */
  .word  0                              /* Reserved */
  .word  0                              /* Reserved */
  .word  0                              /* Reserved */
  .word  0                              /* Reserved */
  .word  SVC_Handler                    /* SVCall Handler */
  .word  0                              /* Reserved */
  .word  0                              /* Reserved */
  .word  PendSV_Handler                 /* PendSV Handler */
  .word  SysTick_Handler                /* SysTick Handler */
  .word  WWDG_IRQHandler                /* 0Window Watchdog */
  .word  PVD_IRQHandler                 /* 1PVD through EXTI Line detect */
  .word  RTC_IRQHandler                 /* 2RTC through EXTI Line */
  .word  FLASH_IRQHandler               /* 3FLASH */
  .word  RCC_IRQHandler                 /* 4RCC */
  .word  EXTI0_1_IRQHandler             /* 5EXTI Line 0 and 1 */
  .word  EXTI2_3_IRQHandler             /* 6EXTI Line 2 and 3 */
  .word  EXTI4_15_IRQHandler            /* 7EXTI Line 4 to 15 */
  .word  LCD_IRQHandler                 /* 8LCD  */
  .word  DMA1_Channel1_IRQHandler       /* 9DMA1 Channel 1 */
  .word  DMA1_Channel2_3_IRQHandler     /* 10DMA1 Channel 2 and Channel 3 */
  .word  DMA1_Channel4_5_6_7_IRQHandler /* 11DMA1 Channel 4, 5, 6, 7 */
  .word  ADC_COMP_IRQHandler            /* 12ADC&COMP1  */
  .word  TIM1_BRK_UP_TRG_COM_IRQHandler /* 13TIM1 Break, Update, Trigger and Commutation */
  .word  TIM1_CC_IRQHandler             /* 14TIM1 Capture Compare */
  .word  TIM2_IRQHandler                /* 15TIM2 */
  .word  TIM3_IRQHandler                /* 16TIM3 */
  .word  TIM6_LPTIM1_DAC_IRQHandler     /* 17TIM6, LPTIM1, DAC */
  .word  TIM7_IRQHandler                /* 18TIM7  */
  .word  TIM14_IRQHandler               /* 19TIM14 */
  .word  TIM15_IRQHandler               /* 20TIM15  */
  .word  TIM16_IRQHandler               /* 21TIM16 */
  .word  TIM17_IRQHandler               /* 22TIM17 */
  .word  I2C1_IRQHandler                /* 23I2C1 */
  .word  I2C2_IRQHandler                /* 24I2C2  */
  .word  SPI1_IRQHandler                /* 25SPI1 */
  .word  SPI2_IRQHandler                /* 26SPI2 */
  .word  USART1_IRQHandler              /* 27USART1 */
  .word  USART2_IRQHandler              /* 28USART2 */
  .word  USART3_4_IRQHandler            /* 29USART3, USART4 */
  .word  CAN_IRQHandler                 /* 30CAN */
  .word  USB_IRQHandler                 /* 31USB */

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/

  .weak      NMI_Handler
  .thumb_set NMI_Handler,Default_Handler

  .weak      HardFault_Handler
  .thumb_set HardFault_Handler,Default_Handler

  .weak      SVC_Handler
  .thumb_set SVC_Handler,Default_Handler

  .weak      PendSV_Handler
  .thumb_set PendSV_Handler,Default_Handler

  .weak      SysTick_Handler
  .thumb_set SysTick_Handler,Default_Handler

  .weak      WWDG_IRQHandler
  .thumb_set WWDG_IRQHandler,Default_Handler

  .weak      PVD_IRQHandler
  .thumb_set PVD_IRQHandler,Default_Handler

  .weak      RTC_IRQHandler
  .thumb_set RTC_IRQHandler,Default_Handler

  .weak      FLASH_IRQHandler
  .thumb_set FLASH_IRQHandler,Default_Handler

  .weak      RCC_IRQHandler
  .thumb_set RCC_IRQHandler,Default_Handler

  .weak      EXTI0_1_IRQHandler
  .thumb_set EXTI0_1_IRQHandler,Default_Handler

  .weak      EXTI2_3_IRQHandler
  .thumb_set EXTI2_3_IRQHandler,Default_Handler

  .weak      EXTI4_15_IRQHandler
  .thumb_set EXTI4_15_IRQHandler,Default_Handler

  .weak      LCD_IRQHandler
  .thumb_set LCD_IRQHandler,Default_Handler

  .weak      DMA1_Channel1_IRQHandler
  .thumb_set DMA1_Channel1_IRQHandler,Default_Handler

  .weak      DMA1_Channel2_3_IRQHandler
  .thumb_set DMA1_Channel2_3_IRQHandler,Default_Handler

  .weak      DMA1_Channel4_5_6_7_IRQHandler
  .thumb_set DMA1_Channel4_5_6_7_IRQHandler,Default_Handler

  .weak      ADC_COMP_IRQHandler
  .thumb_set ADC_COMP_IRQHandler,Default_Handler

  .weak      TIM1_BRK_UP_TRG_COM_IRQHandler
  .thumb_set TIM1_BRK_UP_TRG_COM_IRQHandler,Default_Handler

  .weak      TIM1_CC_IRQHandler
  .thumb_set TIM1_CC_IRQHandler,Default_Handler

  .weak      TIM2_IRQHandler
  .thumb_set TIM2_IRQHandler,Default_Handler

  .weak      TIM3_IRQHandler
  .thumb_set TIM3_IRQHandler,Default_Handler

  .weak      TIM6_LPTIM1_DAC_IRQHandler
  .thumb_set TIM6_LPTIM1_DAC_IRQHandler,Default_Handler

  .weak      TIM7_IRQHandler
  .thumb_set TIM7_IRQHandler,Default_Handler

  .weak      TIM14_IRQHandler
  .thumb_set TIM14_IRQHandler,Default_Handler

  .weak      TIM15_IRQHandler
  .thumb_set TIM15_IRQHandler,Default_Handler

  .weak      TIM16_IRQHandler
  .thumb_set TIM16_IRQHandler,Default_Handler

  .weak      TIM17_IRQHandler
  .thumb_set TIM17_IRQHandler,Default_Handler

  .weak      I2C1_IRQHandler
  .thumb_set I2C1_IRQHandler,Default_Handler

  .weak      I2C2_IRQHandler
  .thumb_set I2C2_IRQHandler,Default_Handler

  .weak      SPI1_IRQHandler
  .thumb_set SPI1_IRQHandler,Default_Handler

  .weak      SPI2_IRQHandler
  .thumb_set SPI2_IRQHandler,Default_Handler

  .weak      USART1_IRQHandler
  .thumb_set USART1_IRQHandler,Default_Handler

  .weak      USART2_IRQHandler
  .thumb_set USART2_IRQHandler,Default_Handler

  .weak      USART3_4_IRQHandler
  .thumb_set USART3_4_IRQHandler,Default_Handler

  .weak      CAN_IRQHandler
  .thumb_set CAN_IRQHandler,Default_Handler

  .weak      USB_IRQHandler
  .thumb_set USB_IRQHandler,Default_Handler
