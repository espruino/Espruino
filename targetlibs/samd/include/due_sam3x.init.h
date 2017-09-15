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
