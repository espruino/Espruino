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
#ifndef HARFAULT_H__
#define HARFAULT_H__
#include <stdint.h>
#include <stddef.h>
/**
 * @defgroup hardfault_default HardFault exception
 * @{
 * @brief Default HardFault exception implementation.
 * @ingroup app_common
 */

/**
 * @brief Contents of the stack.
 *
 * This structure is used to re-create the stack layout after a HardFault exception was raised.
 */
typedef struct HardFault_stack
{
    uint32_t r0;  ///< R0 register.
    uint32_t r1;  ///< R1 register.
    uint32_t r2;  ///< R2 register.
    uint32_t r3;  ///< R3 register.
    uint32_t r12; ///< R12 register.
    uint32_t lr;  ///< Link register.
    uint32_t pc;  ///< Program counter.
    uint32_t psr; ///< Program status register.
}HardFault_stack_t;

/**
 * @brief Function for processing HardFault exceptions.
 *
 * An application that needs to process HardFault exceptions should provide an implementation of this function.
 * It will be called from the HardFault handler.
 * If no implementation is provided, the library uses a default one, which just restarts the MCU.
 *
 * @note If the DEBUG_NRF macro is defined, the software breakpoint is set just before the call 
 *       to this function.
 *
 * @param p_stack Pointer to the stack bottom.
 *                This pointer might be NULL if the HardFault was called when the main stack was
 *                the active stack and a stack overrun is detected.
 *                In such a situation, the stack pointer is reinitialized to the default position,
 *                and the stack content is lost.
 */
void HardFault_process(HardFault_stack_t *p_stack);

/** @} */
#endif /* HARFAULT_H__ */

