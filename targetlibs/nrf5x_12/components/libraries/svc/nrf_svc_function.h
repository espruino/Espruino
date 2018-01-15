/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

#ifndef NRF_SVC_FUNCTION_H__
#define NRF_SVC_FUNCTION_H__

#include <stdint.h>
#include "section_vars.h"
#include "app_util.h"
#include "nrf_svci.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @brief Function to be called from an SVC handler.
 *
 * @warning The function prototype must be limited to a maximum of four arguments, due to the nature of SVC calls.
 */
typedef uint32_t (*nrf_svc_func_t)();

/** @brief Type holding the SVC number and the pointer to the corresponding function.
 *
 * Not that the function that is pointed to must not change version.
 */
typedef struct
{
    uint32_t            svc_num;        /**< Supervisor call number (actually 8-bit, padded for alignment). */
    uint32_t            svci_num;       /**< Supervisor call indirect number. */
    nrf_svc_func_t      func_ptr;
} nrf_svc_func_reg_t;


// Verify that the size of nrf_svc_func_t is aligned.
STATIC_ASSERT(sizeof(nrf_svc_func_reg_t) % 4 == 0);


/** @brief  Macro for registering a structure holding SVC number and function pointer.
 *
 * @details     This macro places the variable in a section named "svc_data" that
                the SVC handler uses during regular operation.
 */
#define SVC_REGISTER_FUNCTION(svc_var) NRF_SECTION_VARS_ADD(svc_data, svc_var)


#ifdef __cplusplus
}
#endif

#endif // NRF_SVC_FUNCTION_H__
