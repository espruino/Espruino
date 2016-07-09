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
    "   .syntax unified                        \n"

    "   ldr   r0, =0xFFFFFFFD                  \n"
    "   cmp   r0, lr                           \n"
    "   bne   HardFault_Handler_ChooseMSP      \n"
    /* Reading PSP into R0 */
    "   mrs   r0, PSP                          \n"
    "   b     HardFault_Handler_Continue       \n"
    "HardFault_Handler_ChooseMSP:              \n"
    /* Reading MSP into R0 */
    "   mrs   r0, MSP                          \n"
    /* -----------------------------------------------------------------
     * If we have selected MSP check if we may use stack safetly.
     * If not - reset the stack to the initial value. */
    "   ldr   r1, =__StackTop                  \n"
    "   ldr   r2, =__StackLimit                \n"

    /* MSP is in the range of <__StackTop, __StackLimit) */
    "   cmp   r0, r1                           \n"
    "   bhi   HardFault_MoveSP                 \n"
    "   cmp   r0, r2                           \n"
    "   bhi   HardFault_Handler_Continue       \n"
    /* ----------------------------------------------------------------- */
    "HardFault_MoveSP:                         \n"
    "   mov   SP, r1                           \n"
    "   movs  r0, #0                           \n"

    "HardFault_Handler_Continue:               \n"
    "   ldr r3, =HardFault_c_handler           \n"
    "   bx r3                                  \n"

    "   .align                                 \n"
    );
}
