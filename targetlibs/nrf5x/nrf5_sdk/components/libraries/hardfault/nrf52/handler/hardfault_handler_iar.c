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
    "   ldr.n r3, 103f                          \n"
    "   tst   lr, #4                            \n"

    /* PSP is quite simple and does not require additional handler */
    "   itt   ne                                \n"
    "   mrsne r0, psp                           \n"
    /* Jump to the handler, do not store LR - returning from handler just exits exception */
    "   bxne  r3                                \n"

    /* Processing MSP requires stack checking */
    "   mrs r0, msp                             \n"

    "   ldr.n r1, 101f                          \n"
    "   ldr.n r2, 102f                          \n"

    /* MSP is in the range of <__StackTop, __StackLimit) */
    "   cmp   r0, r1                            \n"
    "   bhi.n 1f                                \n"
    "   cmp   r0, r2                            \n"
    "   bhi.n 2f                                \n"

    "1:                                         \n"
    "   mov   sp, r1                            \n"
    "   mov   r0, #0                            \n"

    "2:                                         \n"
    "   bx r3                                   \n"
    /* Data alignment if required */
    "   nop                                     \n"

    "101:                                       \n"
    "   DC32 %c0                                \n"
    "102:                                       \n"
    "   DC32 %c1                                \n"
    "103:                                       \n"
    "   DC32 %c2                                \n"
    : /* Outputs */
    : /* Inputs */
    "i"(__section_end("CSTACK")),
    "i"(__section_begin("CSTACK")),
    "i"(&HardFault_c_handler)
    );
}
