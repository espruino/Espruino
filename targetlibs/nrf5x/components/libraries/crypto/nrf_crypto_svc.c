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

#include "nrf_svc_function.h"
#include "nrf_error.h"
#include "sdk_common.h"

/* use direct calls */
#define SVC_INTERFACE_CALL_AS_NORMAL_FUNCTION
#include "nrf_crypto.h"

#ifndef NRF51

SVC_REGISTER_FUNCTION(const nrf_svc_func_reg_t nrf_crypto_svci_init) =
{
    .svc_num = NRF_SVCI_SVC_NUM,
    .svci_num = NRF_CRYPTO_SVCI_INIT,
    .func_ptr = (nrf_svc_func_t)&nrf_crypto_init
};

SVC_REGISTER_FUNCTION(const nrf_svc_func_reg_t nrf_crypto_svci_public_key_compute) =
{
    .svc_num = NRF_SVCI_SVC_NUM,
    .svci_num = NRF_CRYPTO_SVCI_PUBLIC_KEY_COMPUTE,
    .func_ptr = (nrf_svc_func_t)&nrf_crypto_public_key_compute
};

SVC_REGISTER_FUNCTION(const nrf_svc_func_reg_t nrf_crypto_svci_shared_secret_compute) =
{
    .svc_num = NRF_SVCI_SVC_NUM,
    .svci_num = NRF_CRYPTO_SVCI_SHARED_SECRET_COMPUTE,
    .func_ptr = (nrf_svc_func_t)&nrf_crypto_shared_secret_compute
};

SVC_REGISTER_FUNCTION(const nrf_svc_func_reg_t nrf_crypto_svci_sign) =
{
    .svc_num = NRF_SVCI_SVC_NUM,
    .svci_num = NRF_CRYPTO_SVCI_SIGN,
    .func_ptr = (nrf_svc_func_t)&nrf_crypto_sign
};

#endif

SVC_REGISTER_FUNCTION(const nrf_svc_func_reg_t nrf_crypto_svci_verify) =
{
    .svc_num = NRF_SVCI_SVC_NUM,
    .svci_num = NRF_CRYPTO_SVCI_VERIFY,
    .func_ptr = (nrf_svc_func_t)&nrf_crypto_verify
};

SVC_REGISTER_FUNCTION(const nrf_svc_func_reg_t nrf_crypto_svci_hash_compute) =
{
    .svc_num = NRF_SVCI_SVC_NUM,
    .svci_num = NRF_CRYPTO_SVCI_HASH_COMPUTE,
    .func_ptr = &nrf_crypto_hash_compute
};
