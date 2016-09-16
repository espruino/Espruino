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
#include <stdint.h>
#include "compiler_abstraction.h"


//lint -save -e27

__ASM void HardFault_Handler(void)
{
    PRESERVE8
    EXTERN HardFault_c_handler
    EXTERN |STACK$$Base|
    EXTERN |STACK$$Limit|

    ldr r3, =HardFault_c_handler
    tst lr, #4

    /* PSP is quite simple and does not require additional handler */
    itt ne
    mrsne r0, psp
    /* Jump to the handler, do not store LR - returning from handler just exits exception */
    bxne  r3

    /* Processing MSP requires stack checking */
    mrs r0, msp

    ldr   r1, =|STACK$$Limit|
    ldr   r2, =|STACK$$Base|

    /* MSP is in the range of the stack area */
    cmp   r0, r1
    bhi   HardFault_MoveSP
    cmp   r0, r2
    bhi   HardFault_Handler_Continue

HardFault_MoveSP
    mov   sp, r1
    mov   r0, #0

HardFault_Handler_Continue
    bx r3

    ALIGN
}

//lint -restore
#endif //HARDFAULT_HANDLER_ENABLED
