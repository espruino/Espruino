/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#include "dfu_ble_svc.h"
#include <string.h>
#include "nrf_error.h"
#include "crc16.h"

#if defined ( __CC_ARM )
static dfu_ble_peer_data_t m_peer_data __attribute__((section("NoInit"), zero_init));            /**< This variable should be placed in a non initialized RAM section in order to be valid upon soft reset from application into bootloader. */
static uint16_t            m_peer_data_crc __attribute__((section("NoInit"), zero_init));        /**< CRC variable to ensure the integrity of the peer data provided. */
#elif defined ( __GNUC__ )
__attribute__((section(".noinit"))) static dfu_ble_peer_data_t m_peer_data;                      /**< This variable should be placed in a non initialized RAM section in order to be valid upon soft reset from application into bootloader. */
__attribute__((section(".noinit"))) static uint16_t            m_peer_data_crc;                  /**< CRC variable to ensure the integrity of the peer data provided. */
#elif defined ( __ICCARM__ )
__no_init static dfu_ble_peer_data_t m_peer_data     @ 0x20003F80;                               /**< This variable should be placed in a non initialized RAM section in order to be valid upon soft reset from application into bootloader. */
__no_init static uint16_t            m_peer_data_crc @ 0x20003F80 + sizeof(dfu_ble_peer_data_t); /**< CRC variable to ensure the integrity of the peer data provided. */
#endif


/**@brief Function for setting the peer data from application in bootloader before reset.
 *
 * @param[in] p_peer_data  Pointer to the peer data containing keys for the connection.
 *
 * @retval NRF_SUCCES      The data was set succesfully.
 * @retval NRF_ERROR_NULL  If a null pointer was passed as argument.
 */
static uint32_t dfu_ble_peer_data_set(dfu_ble_peer_data_t * p_peer_data)
{
    if (p_peer_data == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t src = (uint32_t)p_peer_data;
    uint32_t dst = (uint32_t)&m_peer_data;
    // Calculating length in order to check if destination is residing inside source.
    // Source inside the the destination (calculation underflow) is safe a source is read before 
    // written to destination so that when destination grows into source, the source data is no 
    // longer needed.
    uint32_t len = dst - src;

    if (src == dst)
    {
        // Do nothing as source and destination are identical, just calculate crc below.
    }
    else if (len < sizeof(dfu_ble_peer_data_t))
    {
        uint32_t i = 0;

        dst += sizeof(dfu_ble_peer_data_t);
        src += sizeof(dfu_ble_peer_data_t);

        // Copy byte wise backwards when facing overlapping structures.
        while (i++ <= sizeof(dfu_ble_peer_data_t))
        {
            *((uint8_t *)dst--) = *((uint8_t *)src--);
        }
    }
    else
    {
        memcpy((void *)dst, (void *)src, sizeof(dfu_ble_peer_data_t));
    }

    m_peer_data_crc = crc16_compute((uint8_t *)&m_peer_data, sizeof(m_peer_data), NULL);

    return NRF_SUCCESS;
}


/**@brief   Function for handling second stage of SuperVisor Calls (SVC).
 *
 * @details The function will use svc_num to call the corresponding SVC function.
 *
 * @param[in] svc_num    SVC number for function to be executed
 * @param[in] p_svc_args Argument list for the SVC.
 *
 * @return This function returns the error value of the SVC return. For further details, please
 *         refer to the details of the SVC implementation itself.
 *         @ref NRF_ERROR_SVC_HANDLER_MISSING is returned if no SVC handler is implemented for the
 *         provided svc_num.
 */
void C_SVC_Handler(uint8_t svc_num, uint32_t * p_svc_args)
{
    switch (svc_num)
    {
        case DFU_BLE_SVC_PEER_DATA_SET:
            p_svc_args[0] = dfu_ble_peer_data_set((dfu_ble_peer_data_t *)p_svc_args[0]);
            break;

        default:
            p_svc_args[0] = NRF_ERROR_SVC_HANDLER_MISSING;
            break;
    }
}


/**@brief   Function for handling the first stage of SuperVisor Calls (SVC) in assembly.
 *
 * @details The function will use the link register (LR) to determine the stack (PSP or MSP) to be
 *          used and then decode the SVC number afterwards. After decoding the SVC number then
 *          @ref C_SVC_Handler is called for further processing of the SVC.
 */
#if defined ( __CC_ARM )
__asm void SVC_Handler(void)
{
EXC_RETURN_CMD_PSP  EQU 0xFFFFFFFD  ; EXC_RETURN using PSP for ARM Cortex. If Link register contains this value it indicates the PSP was used before the SVC, otherwise the MSP was used.

    IMPORT C_SVC_Handler
    LDR   R0, =EXC_RETURN_CMD_PSP   ; Load the EXC_RETURN into R0 to be able to compare against LR to determine stack pointer used. 
    CMP   R0, LR                    ; Compare the link register with R0. If equal then PSP was used, otherwise MSP was used before SVC.
    BNE   UseMSP                    ; Branch to code fetching SVC arguments using MSP.
    MRS   R1, PSP                   ; Move PSP into R1.
    B     Call_C_SVC_Handler        ; Branch to Call_C_SVC_Handler below.
UseMSP
    MRS   R1, MSP                   ; MSP was used, therefore Move MSP into R1.
Call_C_SVC_Handler
    LDR   R0, [R1, #24]             ; The arguments for the SVC was stacked. R1 contains Stack Pointer, the values stacked before SVC are R0, R1, R2, R3, R12, LR, PC (Return address), xPSR. 
                                    ; R1 contains current SP so the PC of the stacked frame is at SP + 6 words (24 bytes). We load the PC into R0.
    SUBS  R0, #2                    ; The PC before the SVC is in R0. We subtract 2 to get the address prior to the instruction executed where the SVC number is located.
    LDRB  R0, [R0]                  ; SVC instruction low octet: Load the byte at the address before the PC to fetch the SVC number.
    LDR   R2, =C_SVC_Handler        ; Load address of C implementation of SVC handler.
    BX    R2                        ; Branch to C implementation of SVC handler. R0 is now the SVC number, R1 is the StackPointer where the arguments (R0-R3) of the original SVC are located.
    ALIGN
}
#elif defined ( __GNUC__ )
void __attribute__ (( naked )) SVC_Handler(void)
{
    const uint32_t exc_return = 0xFFFFFFFD;      // EXC_RETURN using PSP for ARM Cortex. If Link register contains this value it indicates the PSP was used before the SVC, otherwise the MSP was used.
    
    __asm volatile(
        "cmp   lr, %0\t\n"                       // Compare the link register with argument 0 (%0), which is exc_return. If equal then PSP was used, otherwise MSP was used before SVC.
        "bne   UseMSP\t\n"                       // Branch to code fetching SVC arguments using MSP.
        "mrs   r1, psp\t\n"                      // Move PSP into R1.
        "b     Call_C_SVC_Handler\t\n"           // Branch to Call_C_SVC_Handler below.
        "UseMSP:  \t\n"                          //
        "mrs   r1, msp\t\n"                      // MSP was used, therefore Move MSP into R1.
        "Call_C_SVC_Handler:  \t\n"              //
        "ldr   r0, [r1, #24]\t\n"                // The arguments for the SVC was stacked. R1 contains Stack Pointer, the values stacked before SVC are R0, R1, R2, R3, R12, LR, PC (Return address), xPSR. 
                                                 // R1 contains current SP so the PC of the stacked frame is at SP + 6 words (24 bytes). We load the PC into R0.
        "sub   r0, r0, #2\t\n"                   // The PC before the SVC is in R0. We subtract 2 to get the address prior to the instruction executed where the SVC number is located.
        "ldrb  r0, [r0]\t\n"                     // SVC instruction low octet: Load the byte at the address before the PC to fetch the SVC number.
        "bx    %1\t\n"                           // Branch to C implementation of SVC handler, argument 1 (%1). R0 is now the SVC number, R1 is the StackPointer where the arguments (R0-R3) of the original SVC are located.
        ".align\t\n"
        :: "r" (exc_return), "r" (C_SVC_Handler) // Argument list for the gcc assembly. exc_return is %0, C_SVC_Handler is %1.
        : "r0", "r1"                             // List of register maintained manually.
    );
}
#elif defined ( __ICCARM__ )
void SVC_Handler(void)
{
    asm("movs  r0, #0x02\n"                    // Load 0x02 into R6 to prepare for exec return test.
        "mvns  r0, r0\n"                       // Invert R0 to obtain exec return code using PSP for ARM Cortex.
        "cmp   lr, r0\n"                       // Compare the link register with argument 0 (%0), which is exc_return. If equal then PSP was used, otherwise MSP was used before SVC.
        "bne.n UseMSP\n"                       // Branch to code fetching SVC arguments using MSP.
        "mrs   r1, psp\n"                      // Move PSP into R1.
        "b.n   Call_C_SVC_Handler\t\n"         // Branch to Call_C_SVC_Handler below.
        "UseMSP:  \n"                          //
        "mrs   r1, msp\n"                      // MSP was used, therefore Move MSP into R1.
        "Call_C_SVC_Handler:  \n"              //
        "ldr   r0, [r1, #24]\n"                // The arguments for the SVC was stacked. R1 contains Stack Pointer, the values stacked before SVC are R0, R1, R2, R3, R12, LR, PC (Return address), xPSR. 
                                               // R1 contains current SP so the PC of the stacked frame is at SP + 6 words (24 bytes). We load the PC into R0.
        "subs  r0, #0x02\n"                    // The PC before the SVC is in R0. We subtract 2 to get the address prior to the instruction executed where the SVC number is located.
        "ldrb  r0, [r0]\n"                     // SVC instruction low octet: Load the byte at the address before the PC to fetch the SVC number.
        "bx    %0\n"                           // Branch to C implementation of SVC handler, argument 1 (%1). R0 is now the SVC number, R1 is the StackPointer where the arguments (R0-R3) of the original SVC are located.
        :: "r" (C_SVC_Handler)                 // Argument list for the gcc assembly. C_SVC_Handler is %0.
        : "r0", "r1"                           // List of register maintained manually.
    );
}
#else
#error Compiler not supported.
#endif


uint32_t dfu_ble_peer_data_get(dfu_ble_peer_data_t * p_peer_data)
{
    uint16_t crc;

    if (p_peer_data == NULL)
    {
        return NRF_ERROR_NULL;
    }

    crc = crc16_compute((uint8_t *)&m_peer_data, sizeof(m_peer_data), NULL);
    if (crc != m_peer_data_crc)
    {
        return NRF_ERROR_INVALID_DATA;
    }

    *p_peer_data = m_peer_data;

    // corrupt CRC to invalidate shared information.
    m_peer_data_crc++;

    return NRF_SUCCESS;
}
