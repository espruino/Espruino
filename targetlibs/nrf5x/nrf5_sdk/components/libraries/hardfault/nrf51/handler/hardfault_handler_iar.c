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

#pragma section = "CSTACK"
extern void HardFault_c_handler( uint32_t * );

__stackless void HardFault_Handler(void);

__stackless void HardFault_Handler(void)
{
    __asm volatile(
    "   ldr   r0, 100f                         \n"
    "   cmp   r0, lr                           \n"
    "   bne   1f                               \n"
    /* Reading PSP into R0 */
    "   mrs   r0, PSP                          \n"
    "   b     3f                               \n"
    "1:                                        \n"
    /* Reading MSP into R0 */
    "   mrs   r0, MSP                          \n"
    /* -----------------------------------------------------------------
     * If we have selected MSP check if we may use stack safetly.
     * If not - reset the stack to the initial value. */
    "   ldr   r1, 101f                         \n"
    "   ldr   r2, 102f                         \n"

    /* MSP is in the range of <__StackTop, __StackLimit) */
    "   cmp   r0, r1                           \n"
    "   bhi   2f                               \n"
    "   cmp   r0, r2                           \n"
    "   bhi   3f                               \n"
    /* ----------------------------------------------------------------- */
    "2:                                        \n"
    "   mov   SP, r1                           \n"
    "   movs  r0, #0                           \n"

    "3:                                        \n"
    "   ldr r3, 103f                           \n"
    "   bx r3                                  \n"

    "100:                                      \n"
    "   DC32 0xFFFFFFFD                        \n"
    "101:                                      \n"
    "   DC32 %c0                               \n"
    "102:                                      \n"
    "   DC32 %c1                               \n"
    "103:                                      \n"
    "   DC32 %c2                               \n"
    : /* Outputs */
    : /* Inputs */
    "i"(__section_end("CSTACK")),
    "i"(__section_begin("CSTACK")),
    "i"(&HardFault_c_handler)
    );
}
