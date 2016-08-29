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
 
#ifndef BLE_DTM_APP_H__
#define BLE_DTM_APP_H__

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */

/**
 * @addtogroup ser_app_common_codecs Application common codecs
 * @ingroup ser_codecs
 */

/**@file
 *
 * @defgroup ble_dtm_app DTM Application command request encoders and command response decoders
 * @{
 * @ingroup  ser_app_common_codecs
 *
 * @brief   DTM Application command request encoders and command response decoders.
 */

#include "dtm_uart_params.h"

/**@brief Encodes @ref ble_dtm_init command request.
 *
 * @sa @ref nrf51_dtm_init_encoding_sec for packet format,
 *     @ref ble_dtm_init_rsp_dec for command response decoder.
 *
 * @param[in] p_uart_comm_params    Pointer to UART configuration parameters.
 * @param[in] p_buf                 Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len         \c in: Size of \p p_buf buffer.
 *                                  \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_dtm_init_req_enc(app_uart_stream_comm_params_t const * const p_uart_comm_params,
                              uint8_t                             * const p_buf,
                              uint32_t                            * const p_buf_len);


/**@brief Decodes response @ref ble_dtm_init command.
 *
 * @sa @ref nrf51_dtm_init_encoding_sec for packet format,
 *     @ref ble_dtm_init_req_enc for command request encoder.
 *
 * @param[in] p_buf             Pointer to beginning of command response packet.
 * @param[in] packet_len        Length (in bytes) of response packet.
 * @param[out] p_result_code    Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 */
uint32_t ble_dtm_init_rsp_dec(uint8_t const * const p_buf, 
                              uint32_t              packet_len,
                              uint32_t * const      p_result_code);


/**@brief Function to initializing DTM mode.
 *
 * @param[in] p_uart_comm_params    Pointer to the DTM UART configuration parameters.
 *
 * @retval NRF_SUCCESS          Encoding success.
 * @retval NRF_ERROR_NULL       Encoding failure. NULL pointer supplied.
 */
uint32_t ble_dtm_init(app_uart_stream_comm_params_t * p_uart_comm_params);

/** @} */
#endif // BLE_DTM_APP_H__
