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
#ifndef CONN_MW_NRF_SOC_H__
#define CONN_MW_NRF_SOC_H__

#include <stdint.h>

/**@brief Handles @ref sd_power_system_off command request.
 *
 * @param[in]     p_rx_buf            Pointer to input buffer.
 * @param[in]     rx_buf_len          Size of p_rx_buf.
 * @param[out]    p_tx_buf            Pointer to output buffer.
 * @param[in,out] p_tx_buf_len        \c in: size of \p p_tx_buf buffer.
 *                                    \c out: Length of valid data in \p p_tx_buf.
 *
 * @retval NRF_SUCCESS                Handler success.
 * @retval NRF_ERROR_NULL             Handler failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Handler failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Handler failure. Invalid operation type.
 * @retval NRF_ERROR_NOT_SUPPORTED    Handler failure. Opcode not supported.
 */
uint32_t conn_mw_power_system_off(uint8_t const * const p_rx_buf,
                                  uint32_t rx_buf_len,
                                  uint8_t * const p_tx_buf,
                                  uint32_t * const p_tx_buf_len);

/**@brief Handles @ref sd_temp_get command request and prepares response.
 *
 * @param[in]     p_rx_buf            Pointer to input buffer.
 * @param[in]     rx_buf_len          Size of p_rx_buf.
 * @param[out]    p_tx_buf            Pointer to output buffer.
 * @param[in,out] p_tx_buf_len        \c in: size of \p p_tx_buf buffer.
 *                                    \c out: Length of valid data in \p p_tx_buf.
 *
 * @retval NRF_SUCCESS                Handler success.
 * @retval NRF_ERROR_NULL             Handler failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Handler failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Handler failure. Invalid operation type.
 * @retval NRF_ERROR_NOT_SUPPORTED    Handler failure. Opcode not supported.
 */
uint32_t conn_mw_temp_get(uint8_t const * const p_rx_buf,
                          uint32_t rx_buf_len,
                          uint8_t * const p_tx_buf,
                          uint32_t * const p_tx_buf_len);

/**@brief Handles @ref sd_ecb_block_encrypt command request and prepares response.
 *
 * @param[in]     p_rx_buf            Pointer to input buffer.
 * @param[in]     rx_buf_len          Size of p_rx_buf.
 * @param[out]    p_tx_buf            Pointer to output buffer.
 * @param[in,out] p_tx_buf_len        \c in: size of \p p_tx_buf buffer.
 *                                    \c out: Length of valid data in \p p_tx_buf.
 *
 * @retval NRF_SUCCESS                Handler success.
 * @retval NRF_ERROR_NULL             Handler failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Handler failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Handler failure. Invalid operation type.
 * @retval NRF_ERROR_NOT_SUPPORTED    Handler failure. Opcode not supported.
 */
uint32_t conn_mw_ecb_block_encrypt(uint8_t const * const p_rx_buf,
                                   uint32_t              rx_buf_len,
                                   uint8_t * const       p_tx_buf,
                                   uint32_t * const      p_tx_buf_len);

#endif
