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
#include "hardfault.h"
#include "nrf.h"
#include "compiler_abstraction.h"
#include "nordic_common.h"
#ifdef SOFTDEVICE_PRESENT
#include "nrf_soc.h"
#endif

#if defined(DEBUG_NRF)
/**
 * @brief Pointer to the last received stack pointer.
 *
 * This pointer is set in the debug version of the HardFault handler.
 * It helps to debug HardFault reasons.
 */
volatile HardFault_stack_t *HardFault_p_stack;
#endif

/*lint -save -e14 */
__WEAK void HardFault_process(HardFault_stack_t *p_stack)
{
    // Restart the system by default
    NVIC_SystemReset();
}
/*lint -restore */

void HardFault_c_handler( uint32_t *p_stack_address )
{
#if defined(DEBUG_NRF)
    HardFault_p_stack = (HardFault_stack_t*)p_stack_address;
    /* Generate breakpoint if debugger is connected */
    __BKPT(0);
#endif

    HardFault_process((HardFault_stack_t*)p_stack_address);
}
