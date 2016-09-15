/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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
#ifndef BLE_APP_H__
#define BLE_APP_H__

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */

/**
 * @addtogroup ser_app_s130_codecs Application s130 codecs
 * @ingroup ser_codecs
 */

/**@file
 *
 * @defgroup ble_app Application command request encoders and command response decoders
 * @{
 * @ingroup  ser_app_s130_codecs
 *
 * @brief    Application command request encoders and command response decoders.
 */
#include "ble.h"
/**
 * @brief Encodes @ref sd_ble_tx_packet_count_get command request.
 *
 * @sa @ref nrf51_tx_packet_count_get_encoding for packet format,
 *     @ref ble_tx_packet_count_get_rsp_dec for command response decoder.
 *
 * @param[in] conn_handle    Connection handle.
 * @param[in] p_count        Pointer to count value to be filled.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @note  \p p_count  will not be updated by the command
 *        request encoder. Updated values are set by @ref ble_tx_packet_count_get_rsp_dec.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_tx_packet_count_get_req_enc(uint16_t              conn_handle,
                                         uint8_t const * const p_count,
                                         uint8_t * const       p_buf,
                                         uint32_t * const      p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_tx_packet_count_get command.
 *
 * @sa @ref nrf51_tx_packet_count_get_encoding for packet format,
 *     @ref ble_tx_packet_count_get_req_enc for command request encoder.
 *
 * @param[in] p_buf           Pointer to beginning of command response packet.
 * @param[in] packet_len      Length (in bytes) of response packet.
 * @param[out] pp_count       Pointer to pointer to count value.
 * @param[out] p_result_code  Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_tx_packet_count_get_rsp_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         uint8_t * * const     pp_count,
                                         uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_uuid_encode command request.
 *
 * @sa @ref nrf51_uuid_encode_encoding for packet format,
 *     @ref ble_uuid_encode_rsp_dec for command response decoder.
 *
 * @param[in] p_uuid         Pointer to a @ref ble_uuid_t structure that will be encoded into bytes.
 * @param[in] p_uuid_le_len  Size of \p p_uuid_le if \p p_uuid_le is not NULL
 * @param[in] p_uuid_le      Pointer to a buffer where the little endian raw UUID bytes(2 or 16)
 *                           will be stored. Can be NULL to calculate required size.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @note  \p p_uuid_le_len and \p p_uuid_le will not be updated by the command
 *        request encoder. Updated values are set by @ref ble_uuid_encode_rsp_dec.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_uuid_encode_req_enc(ble_uuid_t const * const p_uuid,
                                 uint8_t const * const    p_uuid_le_len,
                                 uint8_t const * const    p_uuid_le,
                                 uint8_t * const          p_buf,
                                 uint32_t * const         p_buf_len);

/**@brief Decodes response to @ref sd_ble_uuid_encode command.
 *
 * @sa @ref nrf51_uuid_encode_encoding for packet format,
 *     @ref ble_uuid_encode_req_enc for command request encoder.
 *
 * @param[in] p_buf              Pointer to beginning of command response packet.
 * @param[in] packet_len         Length (in bytes) of response packet.
 * @param[in,out] p_uuid_le_len  \c in: Size (in bytes) of \p p_uuid_le buffer.
 *                               \c out: Length of decoded contents of \p p_uuid_le.
 * @param[out] p_uuid_le         Pointer to a buffer where the encoded UUID will be stored.
 * @param[out] p_result_code     Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE       Length of \p p_uuid_le is too small to hold decoded
 *                                   value from response.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match expected
 *                                   operation code.
 */
uint32_t ble_uuid_encode_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 uint8_t * const       p_uuid_le_len,
                                 uint8_t * const       p_uuid_le,
                                 uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_uuid_decode command request.
 *
 * @sa @ref nrf51_uuid_decode_encoding for packet format,
 *     @ref ble_uuid_decode_rsp_dec for command response decoder.
 *
 * @param[in] uuid_le_len  Size of \p p_uuid_le if \p p_uuid_le is not NULL
 * @param[in] p_uuid_le      Pointer to a buffer where the little endian raw UUID bytes(2 or 16)
 *                           is stored.
 * @param[out] p_uuid        Pointer to a @ref ble_uuid_t structure were raw UUID will be decoded.
 * @param[in]  p_buf         Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @note  \p p_uuid will not be updated by the command request encoder.
 *           Updated values are set by @ref ble_uuid_decode_rsp_dec.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_uuid_decode_req_enc(uint8_t               uuid_le_len,
                                 uint8_t const * const p_uuid_le,
                                 ble_uuid_t * const    p_uuid,
                                 uint8_t * const       p_buf,
                                 uint32_t * const      p_buf_len);

/**@brief Decodes response to @ref sd_ble_uuid_decode command.
 *
 * @sa @ref nrf51_uuid_decode_encoding for packet format,
 *     @ref ble_uuid_decode_req_enc for command request encoder.
 *
 * @param[in] p_buf              Pointer to beginning of command response packet.
 * @param[in] packet_len         Length (in bytes) of response packet.
 * @param[out] p_uuid         Pointer to a buffer where the decoded UUID will be stored.
 * @param[out] p_result_code     Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match expected
 *                                   operation code.
 */
uint32_t ble_uuid_decode_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 ble_uuid_t * * const  p_uuid,
                                 uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_uuid_vs_add command request.
 *
 * @sa @ref nrf51_uuid_vs_add_encoding for packet format,
 *     @ref ble_uuid_vs_add_rsp_dec for command response decoder.
 *
 * @param[in] p_vs_uuid     Pointer to a @ref ble_uuid128_t structure.
 * @param[in] p_uuid_type   Pointer to uint8_t where UUID type will be returned.
 * @param[in] p_buf         Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len \c in: Size of \p p_buf buffer.
 *                          \c out: Length of encoded command packet.
 *
 * @note  \p p_uuid_type will not be updated by the command request encoder.
 *           Updated values are set by @ref ble_uuid_vs_add_rsp_dec.
 *
 * @retval NRF_SUCCESS              Encoding success.
 * @retval NRF_ERROR_NULL           Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH Encoding failure. Incorrect buffer length.
 */
uint32_t ble_uuid_vs_add_req_enc(ble_uuid128_t const * const p_vs_uuid,
                                 uint8_t * const             p_uuid_type,
                                 uint8_t * const             p_buf,
                                 uint32_t * const            p_buf_len);

/**@brief Decodes response to @ref sd_ble_uuid_vs_add command.
 *
 * @sa @ref nrf51_uuid_vs_add_encoding for packet format,
 *     @ref ble_uuid_vs_add_req_enc for command request encoder.
 *
 * @param[in] p_buf          Pointer to beginning of command response packet.
 * @param[in] buf_len     Length (in bytes) of response packet.
 * @param[out] pp_uuid_type  Pointer to a pointer to uint8_t where the decoded UUID type will be stored.
 * @param[out] p_result_code Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match expected
 *                                   operation code.
 */
uint32_t ble_uuid_vs_add_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint8_t * * const     pp_uuid_type,
                                 uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_version_get command request.
 *
 * @sa @ref nrf51_version_get_encoding for packet format,
 *     @ref ble_version_get_rsp_dec for command response decoder.
 *
 * @param[in] p_version      Pointer to a @ref ble_version_t structure to be filled by the response.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_version_get_req_enc(ble_version_t const * const p_version,
                                 uint8_t * const             p_buf,
                                 uint32_t * const            p_buf_len);

/**@brief Decodes response to @ref sd_ble_version_get command.
 *
 * @sa @ref nrf51_version_get_encoding for packet format,
 *     @ref ble_version_get_req_enc for command request encoder.
 *
 * @param[in] p_buf        Pointer to beginning of command response packet.
 * @param[in] packet_len   Length (in bytes) of response packet.
 * @param[out] p_version    Pointer to a @ref ble_version_t where decoded version will be stored.
 * @param[out] p_result_code     Command result code.
 *
 * @return NRF_SUCCESS              Version information stored successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Length of \p p_event is too small to
 *                                  hold decoded event.
 */
uint32_t ble_version_get_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 ble_version_t *       p_version,
                                 uint32_t * const      p_result_code);


/**@brief Encodes @ref sd_ble_opt_set command request.
 *
 * @sa @ref nrf51_opt_set_encoding for packet format,
 *     @ref ble_opt_set_rsp_dec for command response decoder.
 *
 * @param[in] opt_id         Identifies type of parameter in ble_opt_t union.
 * @param[in] p_opt          Pointer to ble_opt_t union.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Invalid opt id.
 */
uint32_t ble_opt_set_req_enc(uint32_t const          opt_id,
                             ble_opt_t const * const p_opt,
                             uint8_t * const         p_buf,
                             uint32_t * const        p_buf_len);

/**@brief Decodes response to @ref sd_ble_opt_set command.
 *
 * @sa @ref nrf51_opt_set_encoding for packet format,
 *     @ref ble_opt_set_req_enc for command request encoder.
 *
 * @param[in] p_buf              Pointer to beginning of command response packet.
 * @param[in] packet_len         Length (in bytes) of response packet.
 * @param[out] p_result_code     Command result code.
 *
 * @return NRF_SUCCESS              Version information stored successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t ble_opt_set_rsp_dec(uint8_t const * const p_buf,
                             uint32_t              packet_len,
                             uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_enable command request.
 *
 * @sa @ref nrf51_enable_encoding for packet format,
 *     @ref ble_enable_rsp_dec for command response decoder.
 *
 * @param[in] p_ble_enable_params       Pointer to a @ref ble_enable_params_t structure.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_enable_req_enc(ble_enable_params_t * p_ble_enable_params,
                            uint8_t * const       p_buf,
                            uint32_t * const      p_buf_len);

/**@brief Decodes response to @ref sd_ble_enable command.
 *
 * @sa @ref nrf51_enable_encoding for packet format,
 *     @ref ble_enable_req_enc for command request encoder.
 *
 * @param[in] p_buf          Pointer to beginning of command response packet.
 * @param[in] packet_len     Length (in bytes) of response packet.
 * @param[out] p_result_code Command result code.
 *
 * @return NRF_SUCCESS              Success.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Length of \p p_event is too small to
 *                                  hold decoded event.
 */
uint32_t ble_enable_rsp_dec(uint8_t const * const p_buf,
                            uint32_t              packet_len,
                            uint32_t * const      p_result_code);
                             
/**@brief Encodes @ref sd_ble_opt_get command request.
 *
 * @sa @ref nrf51_opt_get_encoding for packet format,
 *     @ref ble_opt_get_rsp_dec for command response decoder.
 *
 * @param[in] opt_id         Identifies type of parameter in ble_opt_t union.
 * @param[in] p_opt          Pointer to a @ref ble_opt_t union to be filled by the response.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Invalid opt id.
 */
uint32_t ble_opt_get_req_enc(uint32_t                opt_id,
                             ble_opt_t const * const p_opt,
                             uint8_t * const         p_buf,
                             uint32_t * const        p_buf_len);

/**@brief Decodes response to @ref sd_ble_opt_get command.
 *
 * @sa @ref nrf51_opt_get_encoding for packet format,
 *     @ref ble_opt_get_req_enc for command request encoder.
 *
 * @param[in] p_buf        Pointer to beginning of command response packet.
 * @param[in] packet_len   Length (in bytes) of response packet.
 * @param[out] p_opt_id    Pointer to a decoded opt_id
 * @param[out] p_opt       Pointer to a decoded @ref ble_opt_t union
 * @param[out] p_result_code     Command result code.
 *
 * @return NRF_SUCCESS              Opt stored successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Length of \p p_event is too small to
 *                                  hold decoded event.
 * @retval NRF_ERROR_INVALID_PARAM  Invalid opt id.
 */

uint32_t ble_opt_get_rsp_dec(uint8_t const * const p_buf,
                             uint32_t              packet_len,
                             uint32_t      * const p_opt_id,
                             ble_opt_t     * const p_opt,
                             uint32_t      * const p_result_code);

/**@brief Encodes @ref sd_ble_user_mem_reply command request.
 *
 * @sa @ref nrf51_user_mem_reply_encoding for packet format,
 *     @ref ble_user_mem_reply_rsp_dec for command response decoder.
 *
 * @param[in] conn_handle    Connection Handle.
 * @param[in] p_block        Pointer to a @ref ble_user_mem_block_t structure.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Invalid opt id.
 */
uint32_t ble_user_mem_reply_req_enc(uint16_t                     conn_handle,
                                    ble_user_mem_block_t const * p_block,
                                    uint8_t * const              p_buf,
                                    uint32_t * const             p_buf_len);

/**@brief Decodes response to @ref sd_ble_user_mem_reply command.
 *
 * @sa @ref nrf51_user_mem_reply_encoding for packet format,
 *     @ref ble_user_mem_reply_req_enc for command request encoder.
 *
 * @param[in] p_buf        Pointer to beginning of command response packet.
 * @param[in] packet_len   Length (in bytes) of response packet.
 * @param[out] p_result_code     Command result code.
 *
 * @return NRF_SUCCESS              Opt stored successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Length of \p p_event is too small to
 *                                  hold decoded event.
 * @retval NRF_ERROR_INVALID_PARAM  Invalid opt id.
 */
uint32_t ble_user_mem_reply_rsp_dec(uint8_t const * const p_buf,
                                    uint32_t              packet_len,
                                    uint32_t      * const p_result_code);

/**@brief Event decoding dispatcher.
 *
 * The event decoding dispatcher will route the event packet to the correct decoder which in turn
 * decodes the contents of the event and updates the \p p_event struct.
 *
 * If \p p_event is null, the required length of \p p_event is returned in \p p_event_len.
 *
 * @param[in] p_buf            Pointer to beginning of event packet.
 * @param[in] packet_len       Length (in bytes) of event packet.
 * @param[in,out] p_event      Pointer to a \ref ble_evt_t buffer where the decoded event will be
 *                             stored. If NULL, required length will be returned in \p p_event_len.
 * @param[in,out] p_event_len  \c in: Size (in bytes) of \p p_event buffer.
 *                             \c out: Length of decoded contents of \p p_event.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE       Decoding failure. Length of \p p_event is too small to
 *                                   hold decoded event.
 * @retval NRF_ERROR_NOT_FOUND       Decoding failure. No event decoder is available.
 */
uint32_t ble_event_dec(uint8_t const * const p_buf,
                       uint32_t              packet_len,
                       ble_evt_t * const     p_event,
                       uint32_t * const      p_event_len);

/** @} */
#endif
