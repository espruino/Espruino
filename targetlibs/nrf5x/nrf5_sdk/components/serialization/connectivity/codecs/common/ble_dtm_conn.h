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

#ifndef BLE_DTM_CONN_H__
#define BLE_DTM_CONN_H__

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */

/**
 * @addtogroup ser_conn_common_codecs Connectivity common codecs
 * @ingroup ser_codecs
 */

/**@file
 *
 * @defgroup ble_dtm_conn DTM Connectivity command request decoder and command response encoder
 * @{
 * @ingroup  ser_conn_common_codecs
 *
 * @brief   DTM Connectivity command request decoder and command response encoder
 */

#include "dtm_uart.h"

/**@brief Decodes @ref ble_dtm_init command request.
 *
 * @sa @ref nrf51_dtm_init_encoding_sec for packet format,
 *     @ref ble_dtm_init_rsp_enc for response encoding.
 *
 * @param[in] p_buf               Pointer to beginning of command request packet.
 * @param[in] packet_len             Length (in bytes) of request packet.
 * @param[in] p_comm_params       Pointer to the structure with DTM Uart configuration.

 *
 * @retval NRF_SUCCESS                Decoding success.
 * @retval NRF_ERROR_NULL             Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Decoding failure. Incorrect buffer length.
 */
uint32_t ble_dtm_init_req_dec(uint8_t const * const           p_buf,
                              uint16_t                        packet_len,
                              app_uart_stream_comm_params_t * p_comm_params);


/**@brief Encodes @ref ble_dtm_init command response.
 *
 * @sa @ref nrf51_dtm_init_encoding_sec for packet format.
 *     @ref ble_dtm_init_req_dec for request decoding.
 *
 * @param[in] return_code         Return code indicating if command was successful or not.
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_dtm_init_rsp_enc(uint32_t         return_code,
                              uint8_t * const  p_buf,
                              uint32_t * const p_buf_len);

/** @} */
#endif // BLE_DTM_CONN_H__

