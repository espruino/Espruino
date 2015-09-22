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

#include <string.h>
#include "nrf_sec.h"
#include "sha256.h"
#include "ecc.h"

#define MAX_LENGTH_KEY       8 /**< Maximum key length in words. For the NIST-256 the key is 256 bits (8 words). Usedd only internally to test signing locally on the nRF51/nRF52. */
#define MAX_LENGTH_POINT     8 /**< Maximum length of a point on the curve in words. For the NIST-256 curve a point is 256 bits (8 words). */
#define SHA256_DIGEST_LENGTH 8 /**< Length of digest output from SHA256 in words. 8 words equals to 256 bit. */

/**@brief Structure containg key to be used for signing.
 *
 * @details Currently used for local test purposes.
 */
// Question: Should we add keytype, so that ECDSA, RSA, DSA can be specifiedm as to what algo is in use ?
typedef struct
{
    uint32_t length;               /**< Length of key. */
    uint32_t key[MAX_LENGTH_KEY];  /**< Key. */
} nrf_sec_key_t;


/**@brief Structure with length and array containing either the X or Y part of an ECC coordinate or r or s of the signature.
 */
typedef struct
{
    uint32_t length;                    /**< Length of point in words. For a point on a NIST-256 curve, the point length is 8 words (256 bits). */
    uint32_t point[MAX_LENGTH_POINT];   /**< Array containing the point value */
} nrf_sec_ecc_data_t;

static void convert_point(uint8_t * p_in, uint32_t len, nrf_sec_ecc_data_t * p_out)
{
    uint32_t i;
    
    uint8_t * p_tmp = (uint8_t *)p_out->point;
    
    p_out->length = len >> 2;

    for(i = 0; i < len; i++)
    {
        p_tmp[len-i-1] = p_in[i];
    }
}

static void byte_reorder(uint8_t * bytes, uint32_t length)
{
    uint8_t   tmp;
    
    uint32_t i;
    uint32_t loop = length >> 1;
    
    for(i = 0; i<loop; i++)
    {
        tmp               = bytes[i];
        bytes[i]          = bytes[length-i-1];
        bytes[length-i-1] = tmp;
    }
}

static uint32_t nrf_sec_hash(nrf_sec_data_t * p_data,
                uint8_t                     * p_digest,
                nrf_sec_hash_func_t           hash_func)
{
    uint32_t            err_code;
    sha256_context_t    context;

    if(hash_func != NRF_SEC_SHA256)
    {
        return NRF_ERROR_NOT_SUPPORTED;
    }

    err_code = sha256_init(&context);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = sha256_update(&context, p_data->p_data, p_data->length);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = sha256_final(&context, p_digest);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    return NRF_SUCCESS;
}

static uint32_t nrf_sec_verify(nrf_sec_data_t           * p_data,
                               nrf_sec_ecc_point_t      * p_Q,
                               nrf_sec_ecc_signature_t  * p_signature,      
                               nrf_sec_algo_t             algorithm)
{
    int                 ecc_err;
    uint32_t            err_code;
    nrf_sec_ecc_data_t  q_x;
    nrf_sec_ecc_data_t  q_y;
    nrf_sec_ecc_data_t  sig_r;
    nrf_sec_ecc_data_t  sig_s;
    uint32_t            digest[SHA256_DIGEST_LENGTH + 1] = {SHA256_DIGEST_LENGTH, };

    if(algorithm != NRF_SEC_NIST256_SHA256)
    {
        return NRF_ERROR_NOT_SUPPORTED;
    }

    if(p_Q->x_len > sizeof(q_x.point))
    {
        return NRF_ERROR_INVALID_LENGTH;
    }
    
    // handle reordering
    convert_point(p_Q->p_x, p_Q->x_len, &q_x);
    convert_point(p_Q->p_y, p_Q->y_len, &q_y);
    convert_point(p_signature->p_r, p_signature->r_len, &sig_r);
    convert_point(p_signature->p_s, p_signature->s_len, &sig_s);
    
    err_code = nrf_sec_hash(p_data, (uint8_t *)&digest[1], NRF_SEC_SHA256);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    byte_reorder((uint8_t *)&digest[1], 32);
    
    ecc_err = ecc_ecdsa_validate((uint32_t *)q_x.point, 
                                 (uint32_t *)&q_y.point, 
                                 &digest[1], 
                                 (uint32_t *)&sig_r.point, 
                                 (uint32_t *)&sig_s.point);
    if (ecc_err != 0)
    {
        return NRF_ERROR_INVALID_DATA;
    }
    return NRF_SUCCESS;
}

void C_SVC_Handler(uint8_t svc_num, uint32_t * p_svc_args)
{
    switch (svc_num)
    {
        case NRF_SEC_SVC_HASH:
            p_svc_args[0] = nrf_sec_hash((nrf_sec_data_t *)     p_svc_args[0],
                                         (uint8_t *)            p_svc_args[1],
                                         (nrf_sec_hash_func_t)  p_svc_args[2]);
            break;

        case NRF_SEC_SVC_VERIFY:
            p_svc_args[0] = nrf_sec_verify((nrf_sec_data_t *)           p_svc_args[0],
                                           (nrf_sec_ecc_point_t *)      p_svc_args[1],
                                           (nrf_sec_ecc_signature_t *)  p_svc_args[2],
                                           (nrf_sec_algo_t)             p_svc_args[3]);
            break;
        
        default:
            p_svc_args[0] = NRF_ERROR_SVC_HANDLER_MISSING;
            break;
    }
}

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
#else
#error Compiler not supported.
#endif

int main(void)
{
    while(1); // Main should never be called - only to be used for test purposes.
}
