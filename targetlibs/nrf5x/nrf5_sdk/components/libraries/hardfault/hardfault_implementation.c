/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#include "sdk_config.h"
#if HARDFAULT_HANDLER_ENABLED
#include "hardfault.h"
#include "nrf.h"
#include "compiler_abstraction.h"
#include "app_util_platform.h"
#include "nordic_common.h"
#ifdef SOFTDEVICE_PRESENT
#include "nrf_soc.h"
#endif
#define NRF_LOG_MODULE_NAME "HARDFAULT"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#if defined(DEBUG_NRF)
/**
 * @brief Pointer to the last received stack pointer.
 *
 * This pointer is set in the debug version of the HardFault handler.
 * It helps to debug HardFault reasons.
 */
volatile HardFault_stack_t * HardFault_p_stack;
#endif

/*lint -save -e14 */
__WEAK void HardFault_process(HardFault_stack_t * p_stack)
{
    // Restart the system by default
    NVIC_SystemReset();
}
/*lint -restore */

void HardFault_c_handler(uint32_t * p_stack_address)
{
#if defined(DEBUG_NRF)
    HardFault_p_stack = (HardFault_stack_t *)p_stack_address;
    (void)HardFault_p_stack;

    // Debugger detection is only possible on NRF52 (Cortex-M4), on NRF51
    // (Cortex-M0) the processor has no access to CoreDebug registers.
    #if __CORTEX_M == 0x04
        // C_DEBUGEN == 1 -> Debugger Connected
        if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
        {
            /* Generate breakpoint if debugger is connected */
            NRF_BREAKPOINT;
        }
    #endif // __CORTEX_M == 0x04
#endif // DEBUG_NRF
    NRF_LOG_ERROR("Hardfault PC:%x\r\n", ((HardFault_stack_t *)p_stack_address)->pc);
    NRF_LOG_FINAL_FLUSH();
    HardFault_process((HardFault_stack_t *)p_stack_address);
}
#endif //HARDFAULT_HANDLER_ENABLED
