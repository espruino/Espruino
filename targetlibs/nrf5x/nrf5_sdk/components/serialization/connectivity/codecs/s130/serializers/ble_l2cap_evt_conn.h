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
#ifndef BLE_L2CAP_EVT_CONN_H__
#define BLE_L2CAP_EVT_CONN_H__

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */

/**
 * @addtogroup ser_conn_s130_codecs Connectivity s130 codecs
 * @ingroup ser_codecs
 */

/**@file
 *
 * @defgroup ble_l2cap_evt_conn L2CAP Connectivity event encoders
 * @{
 * @ingroup  ser_conn_s130_codecs
 *
 * @brief    L2CAP Connectivity event encoders.
 */
#include "ble.h"

/**
 * @brief Encodes ble_l2cap_evt_rx event.
 *
 * @sa @ref nrf51_l2cap_evt_rx_encoding for packet format.
 *
 * @param[in] p_event          Pointer to the \ref ble_evt_t buffer that shall be encoded.
 * @param[in] event_len        Size (in bytes) of \p p_event buffer.
 * @param[out] p_buf           Pointer to the beginning of a buffer for encoded event packet.
 * @param[in,out] p_buf_len    \c in: Size (in bytes) of \p p_buf buffer.
 *                             \c out: Length of encoded contents in \p p_buf.
 *
 * @retval NRF_SUCCESS               Encoding success.
 * @retval NRF_ERROR_NULL            Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Encoding failure. Incorrect buffer length.
 */
uint32_t ble_l2cap_evt_rx_enc(ble_evt_t const * const p_event,
                              uint32_t                event_len,
                              uint8_t * const         p_buf,
                              uint32_t * const        p_buf_len);

/** @} */
#endif
