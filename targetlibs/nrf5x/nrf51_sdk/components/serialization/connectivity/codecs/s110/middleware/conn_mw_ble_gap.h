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
#ifndef _CONN_MW_BLE_GAP_H
#define _CONN_MW_BLE_GAP_H

#include <stdint.h>

/**@brief Handles @ref sd_ble_gap_address_set command request and prepares response.
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
 */
uint32_t conn_mw_ble_gap_address_set(uint8_t const * const p_rx_buf,
                                     uint32_t rx_buf_len,
                                     uint8_t * const p_tx_buf,
                                     uint32_t * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_address_get command request and prepares response.
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
 */
uint32_t conn_mw_ble_gap_address_get(uint8_t const * const p_rx_buf,
                                     uint32_t rx_buf_len,
                                     uint8_t * const p_tx_buf,
                                     uint32_t * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_adv_data_set command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_adv_data_set(uint8_t const * const p_rx_buf,
                                      uint32_t              rx_buf_len,
                                      uint8_t       * const p_tx_buf,
                                      uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_adv_start command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_adv_start(uint8_t const * const p_rx_buf,
                                   uint32_t              rx_buf_len,
                                   uint8_t       * const p_tx_buf,
                                   uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_adv_stop command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_adv_stop(uint8_t const * const p_rx_buf,
                                  uint32_t              rx_buf_len,
                                  uint8_t       * const p_tx_buf,
                                  uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_conn_param_update command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_conn_param_update(uint8_t const * const p_rx_buf,
                                           uint32_t              rx_buf_len,
                                           uint8_t       * const p_tx_buf,
                                           uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_disconnect command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_disconnect(uint8_t const * const p_rx_buf,
                                    uint32_t              rx_buf_len,
                                    uint8_t       * const p_tx_buf,
                                    uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_tx_power_set command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_tx_power_set(uint8_t const * const p_rx_buf,
                                      uint32_t              rx_buf_len,
                                      uint8_t       * const p_tx_buf,
                                      uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_appearance_set command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_appearance_set(uint8_t const * const p_rx_buf,
                                        uint32_t              rx_buf_len,
                                        uint8_t       * const p_tx_buf,
                                        uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_appearance_get command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_appearance_get(uint8_t const * const p_rx_buf,
                                        uint32_t              rx_buf_len,
                                        uint8_t       * const p_tx_buf,
                                        uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_ppcp_set command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_ppcp_set(uint8_t const * const p_rx_buf,
                                  uint32_t              rx_buf_len,
                                  uint8_t       * const p_tx_buf,
                                  uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_ppcp_get command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_ppcp_get(uint8_t const * const p_rx_buf,
                                  uint32_t              rx_buf_len,
                                  uint8_t       * const p_tx_buf,
                                  uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_device_name_get command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_device_name_get(uint8_t const * const p_rx_buf,
                                         uint32_t              rx_buf_len,
                                         uint8_t       * const p_tx_buf,
                                         uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_device_name_set command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_device_name_set(uint8_t const * const p_rx_buf,
                                         uint32_t              rx_buf_len,
                                         uint8_t       * const p_tx_buf,
                                         uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_authenticate command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_authenticate(uint8_t const * const p_rx_buf,
                                      uint32_t              rx_buf_len,
                                      uint8_t       * const p_tx_buf,
                                      uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_sec_params_reply command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_sec_params_reply(uint8_t const * const p_rx_buf,
                                          uint32_t              rx_buf_len,
                                          uint8_t       * const p_tx_buf,
                                          uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_auth_key_reply command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_auth_key_reply(uint8_t const * const p_rx_buf,
                                        uint32_t              rx_buf_len,
                                        uint8_t       * const p_tx_buf,
                                        uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_sec_info_reply command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_sec_info_reply(uint8_t const * const p_rx_buf,
                                        uint32_t              rx_buf_len,
                                        uint8_t       * const p_tx_buf,
                                        uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_conn_sec_get command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_conn_sec_get(uint8_t const * const p_rx_buf,
                                      uint32_t              rx_buf_len,
                                      uint8_t       * const p_tx_buf,
                                      uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_rssi_start command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_rssi_start(uint8_t const * const p_rx_buf,
                                    uint32_t              rx_buf_len,
                                    uint8_t       * const p_tx_buf,
                                    uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_rssi_stop command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_rssi_stop(uint8_t const * const p_rx_buf,
                                   uint32_t              rx_buf_len,
                                   uint8_t       * const p_tx_buf,
                                   uint32_t      * const p_tx_buf_len);
 
/**@brief Handles @ref sd_ble_gap_rssi_get command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_rssi_get(uint8_t const * const p_rx_buf,
                                  uint32_t              rx_buf_len,
                                  uint8_t       * const p_tx_buf,
                                  uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_connect command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_connect(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t * const       p_tx_buf,
                                 uint32_t * const      p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_connect_cancel command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_connect_cancel(uint8_t const * const p_rx_buf,
                                        uint32_t              rx_buf_len,
                                        uint8_t * const       p_tx_buf,
                                        uint32_t * const      p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_encrypt command and prepares response.
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
 */

uint32_t conn_mw_ble_gap_encrypt(uint8_t const * const p_rx_buf,
                                 uint32_t              rx_buf_len,
                                 uint8_t       * const p_tx_buf,
                                 uint32_t      * const p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_scan_start command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_scan_start(uint8_t const * const p_rx_buf,
                                    uint32_t              rx_buf_len,
                                    uint8_t * const       p_tx_buf,
                                    uint32_t * const      p_tx_buf_len);

/**@brief Handles @ref sd_ble_gap_scan_stop command and prepares response.
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
 */
uint32_t conn_mw_ble_gap_scan_stop(uint8_t const * const p_rx_buf,
                                    uint32_t              rx_buf_len,
                                    uint8_t * const       p_tx_buf,
                                    uint32_t * const      p_tx_buf_len);

#endif //_CONN_MW_BLE_GAP_H
