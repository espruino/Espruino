/* Copyright (c) 2011 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */

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
 * @defgroup ble_l2cap_conn L2CAP Connectivity command request decoders and command response encoders
 * @{
 * @ingroup  ser_conn_s130_codecs
 *
 * @brief    L2CAP Connectivity command request decoders and command response encoders.
 */

#ifndef BLE_L2CAP_CONN_H__
#define BLE_L2CAP_CON__H__

#include "ble.h"
#include "ble_types.h"
#include "ble_ranges.h"
#include "ble_err.h"
#include "ble_l2cap.h"

/**@brief Decodes @ref sd_ble_l2cap_cid_register command request.
 *
 * @sa @ref nrf51_ble_l2cap_cid_register for packet format,
 *     @ref ble_l2cap_cid_register_rsp_enc for response encoding.
 *
 * @param[in] p_buf           Pointer to beginning of command request packet.
 * @param[in] buf_len      Length (in bytes) of response packet.
 * @param[in] p_cid           Pointer to L2CAP CID.
 *
 * @retval NRF_SUCCESS                Decoding success.
 * @retval NRF_ERROR_NULL             Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Decoding failure. Invalid operation type.
 */
uint32_t ble_l2cap_cid_register_req_dec(uint8_t const * const p_buf,
                                        uint32_t              buf_len,
                                        uint16_t *            p_cid);

/**@brief Encodes @ref sd_ble_l2cap_cid_register command response.
 *
 * @sa @ref nrf51_ble_l2cap_cid_register for packet format.
 *     @ref ble_l2cap_cid_register_req_dec for request decoding.
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
uint32_t ble_l2cap_cid_register_rsp_enc(uint32_t         return_code,
                                        uint8_t * const  p_buf,
                                        uint32_t * const p_buf_len);

/**@brief Decodes @ref sd_ble_l2cap_cid_unregister command request.
 *
 * @sa @ref nrf51_ble_l2cap_cid_unregister for packet format,
 *     @ref ble_l2cap_cid_unregister_rsp_enc for response encoding.
 *
 * @param[in] p_buf           Pointer to beginning of command request packet.
 * @param[in] buf_len      Length (in bytes) of response packet.
 * @param[in] p_cid           Pointer to L2CAP CID.
 *
 * @retval NRF_SUCCESS                Decoding success.
 * @retval NRF_ERROR_NULL             Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Decoding failure. Invalid operation type.
 */
uint32_t ble_l2cap_cid_unregister_req_dec(uint8_t const * const p_buf,
                                          uint32_t              buf_len,
                                          uint16_t *            p_cid);

/**@brief Encodes @ref sd_ble_l2cap_cid_unregister command response.
 *
 * @sa @ref nrf51_ble_l2cap_cid_unregister for packet format.
 *     @ref ble_l2cap_cid_unregister_req_dec for request decoding.
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
uint32_t ble_l2cap_cid_unregister_rsp_enc(uint32_t         return_code,
                                          uint8_t * const  p_buf,
                                          uint32_t * const p_buf_len);

/**@brief Decodes @ref sd_ble_l2cap_tx command request.
 *
 * @sa @ref nrf51_ble_l2cap_tx for packet format,
 *     @ref ble_l2cap_tx_rsp_enc for response encoding.
 *
 * @param[in] p_buf           Pointer to beginning of command request packet.
 * @param[in] buf_len      Length (in bytes) of response packet.
 * @param[in] p_conn_handle   Pointer to connection handle.
 * @param[in] pp_l2cap_header Pointer to pointer to L2CAP header.
 * @param[in] pp_data         Pointer to pointer L2CAP data.
 *
 * @retval NRF_SUCCESS                Decoding success.
 * @retval NRF_ERROR_NULL             Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Decoding failure. Invalid operation type.
 */
uint32_t ble_l2cap_tx_req_dec(uint8_t const * const        p_buf,
                              uint32_t const               buf_len,
                              uint16_t *                   p_conn_handle,
                              ble_l2cap_header_t * * const pp_l2cap_header,
                              uint8_t const * *            pp_data);

/**@brief Encodes @ref sd_ble_l2cap_tx command response.
 *
 * @sa @ref nrf51_ble_l2cap_tx for packet format.
 *     @ref ble_l2cap_tx_req_dec for request decoding.
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
uint32_t ble_l2cap_tx_rsp_enc(uint32_t         return_code,
                              uint8_t * const  p_buf,
                              uint32_t * const p_buf_len);

#endif //BLE_L2CAP_CONN_H__

/**
   @}
 */
