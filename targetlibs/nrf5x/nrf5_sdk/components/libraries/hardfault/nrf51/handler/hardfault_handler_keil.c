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

//lint -save -e27 -e10 -e19 -e40
extern char STACK$$Base;

/* This variable should be static but then it cannot be used in assembly code below.
 * The problem here is that the address of the section can be archived by $$ operator
 * that is not allowed in assembly code. */
char const * HardFault_Handler_stack_bottom = &STACK$$Base;
//lint -restore

__asm void HardFault_Handler(void)
{
    PRESERVE8
    EXTERN HardFault_c_handler
    EXTERN __initial_sp
    EXTERN HardFault_Handler_stack_bottom

    ldr   r0, =0xFFFFFFFD
    cmp   r0, lr
    bne   HardFault_Handler_ChooseMSP
    /* Reading PSP into R0 */
    mrs   r0, PSP
    b     HardFault_Handler_Continue
HardFault_Handler_ChooseMSP
    /* Reading MSP into R0 */
    mrs   r0, MSP
    /* -----------------------------------------------------------------
     * If we have selected MSP, check if we may use stack safely.
     * If not - reset the stack to the initial value. */
    ldr   r1, =__initial_sp
    ldr   r2, =HardFault_Handler_stack_bottom
    ldr   r2, [r2]

    /* MSP is in the range of <__StackTop, __StackLimit) */
    cmp   r0, r1
    bhi   HardFault_MoveSP
    cmp   r0, r2
    bhi   HardFault_Handler_Continue
    /* ----------------------------------------------------------------- */
HardFault_MoveSP
    mov   SP, r1
    movs  r0, #0

HardFault_Handler_Continue
    ldr r3, =HardFault_c_handler
    bx r3

    ALIGN
}
