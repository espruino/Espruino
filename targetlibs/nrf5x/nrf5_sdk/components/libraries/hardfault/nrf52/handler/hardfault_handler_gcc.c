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
#include <stdint.h>

void HardFault_Handler(void) __attribute__(( naked ));

void HardFault_Handler(void)
{
    __asm volatile(
    "   ldr r3, =HardFault_c_handler            \n"
    "   tst lr, #4                              \n"

    /* PSP is quite simple and does not require additional handler */
    "   itt ne                                  \n"
    "   mrsne r0, psp                           \n"
    /* Jump to the handler, do not store LR - returning from handler just exits exception */
    "   bxne  r3                                \n"

    /* Processing MSP requires stack checking */
    "   mrs r0, msp                             \n"

    "   ldr   r1, =__StackTop                   \n"
    "   ldr   r2, =__StackLimit                 \n"

    /* MSP is in the range of <__StackTop, __StackLimit) */
    "   cmp   r0, r1                            \n"
    "   bhi   HardFault_MoveSP                  \n"
    "   cmp   r0, r2                            \n"
    "   bhi   HardFault_Handler_Continue        \n"

    "HardFault_MoveSP:                          \n"
    "   mov   sp, r1                            \n"
    "   mov   r0, #0                            \n"

    "HardFault_Handler_Continue:                \n"
    "   bx r3                                   \n"

    "   .align                                  \n"
    );
}
