/**
 * Include or paste this file only in your main file (before your main function).
 *
 * It defines the basics to run the SAM3 controller:
 *
 *  - Interrupt handlers (like interrupt service routines (ISR))
 *
 *  - The system default system tick ISR is used with a 32 bit counter.
 *    Modify SysTick_Handler as you like it.
 *
 *  - Provides the "init_controller()" function, that simply forwards
 *    to SAM SystemInit(), ensures that the libc is initialized the system
 *    tick interrupt is set to one millisecond, and that the watchdog is
 *    disabled (and dies not throw an NMI after a bit of time).
 *
 *  - That's pretty much it - now you have the plain controller and full
 *    control over it.
 *
 * @file due_am3x.init.h
 * @author stfwi
 */
#include "due_sam3x.h"
#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************************
 * Device interrupt vector. Functions are overwritable (weak aliases).
 ********************************************************************************/
static void __phantom_handler(void) { while(1); }

void NMI_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void HardFault_Handler (void) __attribute__ ((weak, alias("__phantom_handler")));
void MemManage_Handler (void) __attribute__ ((weak, alias("__phantom_handler")));
void BusFault_Handler  (void) __attribute__ ((weak, alias("__phantom_handler")));
void UsageFault_Handler(void) __attribute__ ((weak, alias("__phantom_handler")));
void DebugMon_Handler  (void) __attribute__ ((weak, alias("__phantom_handler")));
void SVC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void PendSV_Handler    (void) __attribute__ ((weak, alias("__phantom_handler")));
void SysTick_Handler(void) { TimeTick_Increment(); }
void SUPC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void RSTC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void RTC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void RTT_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void WDT_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void PMC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void EFC0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void EFC1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void UART_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_SMC_INSTANCE_
void SMC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_SDRAMC_INSTANCE_
void SDRAMC_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void PIOA_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void PIOB_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_PIOC_INSTANCE_
void PIOC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOD_INSTANCE_
void PIOD_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOE_INSTANCE_
void PIOE_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOF_INSTANCE_
void PIOF_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void USART0_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void USART1_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void USART2_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_USART3_INSTANCE_
void USART3_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void HSMCI_Handler      (void) __attribute__ ((weak, alias("__phantom_handler")));
void TWI0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void TWI1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void SPI0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_SPI1_INSTANCE_
void SPI1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void SSC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC0_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC1_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC2_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC3_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC4_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC5_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_TC2_INSTANCE_
void TC6_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC7_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC8_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void PWM_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void ADC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void DACC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void DMAC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void UOTGHS_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void TRNG_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_EMAC_INSTANCE_
void EMAC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void CAN0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void CAN1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));


/********************************************************************************
 * Controller initialisation. Call this function in main(), which is itself called
 * from Reset_Handler()
 ********************************************************************************/
void __libc_init_array(void);

void init_controller(void)
{
  /*
   * SAM System init: Initializes the PLL / clock.
   * Defined in CMSIS/ATMEL/sam3xa/source/system_sam3xa.c
   */
  SystemInit();
  /*
   * Config systick interrupt timing, core clock is in microseconds --> 1ms
   * Defined in CMSIS/CMSIS/include/core_cm3.h
   */
  if (SysTick_Config(SystemCoreClock / 1000)) while (1);

  /*
   * No watchdog now
   *
   */
  WDT_Disable(WDT);

  /*
   * GCC libc init, also done in Reset_Handler()
   */
   __libc_init_array();
}


/********************************************************************************
 * C++ catching pure virtual methods ( virtual myMethod = 0)
 ********************************************************************************/

void __cxa_pure_virtual(void)
{ while(1); }


#ifdef	__cplusplus
}
#endif

/********************************************************************************
 * EOF
 ********************************************************************************/
