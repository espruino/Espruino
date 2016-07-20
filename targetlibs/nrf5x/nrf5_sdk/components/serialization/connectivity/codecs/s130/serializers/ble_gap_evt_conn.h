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
#ifndef BLE_GAP_EVT_CONN_H__
#define BLE_GAP_EVT_CONN_H__

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
 * @defgroup ble_gap_evt_conn GAP Connectivity event encoders
 * @{
 * @ingroup  ser_conn_s130_codecs
 *
 * @brief    GAP Connectivity event encoders.
 */
#include "ble.h"

/**
 * @brief Encodes ble_gap_evt_auth_key_request event.
 *
 * @sa @ref nrf51_evt_auth_key_request_encoding for packet format.
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
uint32_t ble_gap_evt_auth_key_request_enc(ble_evt_t const * const p_event,
                                          uint32_t                event_len,
                                          uint8_t * const         p_buf,
                                          uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_auth_status event.
 *
 * @sa @ref nrf51_evt_auth_status_encoding for packet format.
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
uint32_t ble_gap_evt_auth_status_enc(ble_evt_t const * const p_event,
                                     uint32_t                event_len,
                                     uint8_t * const         p_buf,
                                     uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_conn_param_update event.
 *
 * @sa @ref nrf51_evt_conn_param_update_encoding for packet format.
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
uint32_t ble_gap_evt_conn_param_update_enc(ble_evt_t const * const p_event,
                                           uint32_t                event_len,
                                           uint8_t * const         p_buf,
                                           uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_conn_sec_update event.
 *
 * @sa @ref nrf51_evt_conn_sec_update_encoding for packet format.
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
uint32_t ble_gap_evt_conn_sec_update_enc(ble_evt_t const * const p_event,
                                         uint32_t                event_len,
                                         uint8_t * const         p_buf,
                                         uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_connected event.
 *
 * @sa @ref nrf51_evt_connected_encoding for packet format.
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
uint32_t ble_gap_evt_connected_enc(ble_evt_t const * const p_event,
                                   uint32_t                event_len,
                                   uint8_t * const         p_buf,
                                   uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_disconnected event.
 *
 * @sa @ref nrf51_evt_disconnected_encoding for packet format.
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
uint32_t ble_gap_evt_disconnected_enc(ble_evt_t const * const p_event,
                                      uint32_t                event_len,
                                      uint8_t * const         p_buf,
                                      uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_passkey_display event.
 *
 * @sa @ref nrf51_evt_passkey_display for packet format.
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
uint32_t ble_gap_evt_passkey_display_enc(ble_evt_t const * const p_event,
                                         uint32_t                event_len,
                                         uint8_t * const         p_buf,
                                         uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_rssi_changed event.
 *
 * @sa @ref nrf51_evt_rssi_changed_encoding for packet format.
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
uint32_t ble_gap_evt_rssi_changed_enc(ble_evt_t const * const p_event,
                                      uint32_t                event_len,
                                      uint8_t * const         p_buf,
                                      uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_sec_info_request event.
 *
 * @sa @ref nrf51_evt_sec_info_request_encoding for packet format.
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
uint32_t ble_gap_evt_sec_info_request_enc(ble_evt_t const * const p_event,
                                          uint32_t                event_len,
                                          uint8_t * const         p_buf,
                                          uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_sec_params_request event.
 *
 * @sa @ref nrf51_evt_sec_param_request_encoding for packet format.
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
uint32_t ble_gap_evt_sec_params_request_enc(ble_evt_t const * const p_event,
                                            uint32_t                event_len,
                                            uint8_t * const         p_buf,
                                            uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_timeout event.
 *
 * @sa @ref nrf51_evt_timeout_encoding for packet format.
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
uint32_t ble_gap_evt_timeout_enc(ble_evt_t const * const p_event,
                                 uint32_t                event_len,
                                 uint8_t * const         p_buf,
                                 uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_sec_request event.
 *
 * @sa @ref nrf51_evt_sec_request_encoding for packet format.
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
uint32_t ble_gap_evt_sec_request_enc(ble_evt_t const * const p_event,
                                     uint32_t                event_len,
                                     uint8_t * const         p_buf,
                                     uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_conn_param_update_request event.
 *
 * @sa @ref nrf51_evt_conn_param_update_request_encoding for packet format.
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
uint32_t ble_gap_evt_conn_param_update_request_enc(ble_evt_t const * const p_event,
                                                   uint32_t                event_len,
                                                   uint8_t * const         p_buf,
                                                   uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_adv_report event.
 *
 * @sa @ref nrf51_evt_adv_report_encoding for packet format.
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
uint32_t ble_gap_evt_adv_report_enc(ble_evt_t const * const p_event,
                                    uint32_t                event_len,
                                    uint8_t * const         p_buf,
                                    uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_scan_req_report event.
 *
 * @sa @ref nrf51_evt_scan_req_report_encoding for packet format.
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
uint32_t ble_gap_evt_scan_req_report_enc(ble_evt_t const * const p_event,
                                         uint32_t                event_len,
                                         uint8_t * const         p_buf,

                                         uint32_t * const        p_buf_len);
/**
 * @brief Encodes ble_gap_evt_key_pressed event.
 *
 * @sa @ref nrf51_evt_key_pressed_encoding for packet format.
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
uint32_t ble_gap_evt_key_pressed_enc(ble_evt_t const * const p_event,
                                   uint32_t                event_len,
                                   uint8_t * const         p_buf,
                                   uint32_t * const        p_buf_len);

/**
 * @brief Encodes ble_gap_evt_lesc_dhkey_request event.
 *
 * @sa @ref nrf51_evt_lesc_dhkey_request_encoding for packet format.
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
uint32_t ble_gap_evt_lesc_dhkey_request_enc(ble_evt_t const * const p_event,
                                            uint32_t                event_len,
                                            uint8_t * const         p_buf,
                                            uint32_t * const        p_buf_len);

/** @} */
#endif
