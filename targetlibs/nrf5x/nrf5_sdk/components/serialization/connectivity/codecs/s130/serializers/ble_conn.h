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
#ifndef BLE_CONN_H__
#define BLE_CONN_H__

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
 * @defgroup ble_conn Connectivity command request decoders and command response encoders
 * @{
 * @ingroup  ser_conn_s130_codecs
 *
 * @brief    Connectivity command request decoders and command response encoders.
 */
#include "ble.h"

/**@brief Decodes @ref sd_ble_tx_packet_count_get command request.
 *
 * @sa @ref nrf51_tx_packet_count_get_encoding for packet format,
 *     @ref ble_tx_packet_count_get_rsp_enc for response encoding.
 *
 * @param[in] p_buf           Pointer to beginning of command request packet.
 * @param[in] packet_len      Length (in bytes) of response packet.
 * @param[out] p_conn_handle  Pointer to connection handle.
 * @param[out] pp_count       Pointer to pointer to location for count.
 *
 * @retval NRF_SUCCESS                Decoding success.
 * @retval NRF_ERROR_NULL             Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Decoding failure. Invalid operation type.
 */
uint32_t ble_tx_packet_count_get_req_dec(uint8_t const * const p_buf,
                                         uint16_t              packet_len,
                                         uint16_t      * const p_conn_handle,
                                         uint8_t * * const     pp_count);

/**@brief Encodes @ref sd_ble_tx_packet_count_get command response.
 *
 * @sa @ref nrf51_tx_packet_count_get_encoding for packet format.
 *     @ref ble_tx_packet_count_get_req_dec for request decoding.
 *
 * @param[in] return_code         Return code indicating if command was successful or not.
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 * @param[in] p_count             Pointer to count value.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_tx_packet_count_get_rsp_enc(uint32_t              return_code,
                                         uint8_t * const       p_buf,
                                         uint32_t * const      p_buf_len,
                                         uint8_t const * const p_count);

/**@brief Event encoding dispatcher.
 *
 * The event encoding dispatcher will route the event packet to the correct encoder which in turn
 * encodes the contents of the event and updates the \p p_buf buffer.
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
 * @retval NRF_ERROR_NOT_SUPPORTED   Event encoder is not implemented.
 */
uint32_t ble_event_enc(ble_evt_t const * const p_event,
                       uint32_t                event_len,
                       uint8_t * const         p_buf,
                       uint32_t * const        p_buf_len);

/**@brief Decodes @ref sd_ble_version_get command request.
 *
 * @sa @ref nrf51_version_get_encoding for packet format,
 *     @ref ble_version_get_rsp_enc for response encoding.
 *
 * @param[in] p_buf           Pointer to beginning of command request packet.
 * @param[in] packet_len      Length (in bytes) of response packet.
 * @param[out] pp_version     Pointer to pointer to @ref ble_version_t address.
 *
 * @retval NRF_SUCCESS                Decoding success.
 * @retval NRF_ERROR_NULL             Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Decoding failure. Invalid operation type.
 */
uint32_t ble_version_get_req_dec(uint8_t const * const   p_buf,
                                 uint16_t                packet_len,
                                 ble_version_t * * const pp_version);

/**@brief Encodes @ref sd_ble_version_get command response.
 *
 * @sa @ref nrf51_version_get_encoding for packet format.
 *     @ref ble_version_get_req_dec for request decoding.
 *
 * @param[in] return_code         Return code indicating if command was successful or not.
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 * @param[in] p_version           Pointer to @ref ble_version_t address.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_version_get_rsp_enc(uint32_t                    return_code,
                                 uint8_t * const             p_buf,
                                 uint32_t * const            p_buf_len,
                                 ble_version_t const * const p_version);


/**@brief Decodes @ref sd_ble_opt_get command request.
 *
 * @sa @ref nrf51_opt_get_encoding for packet format,
 *     @ref ble_opt_get_rsp_enc for response encoding.
 *
 * @param[in] p_buf           Pointer to beginning of command request packet.
 * @param[in] packet_len      Length (in bytes) of response packet.
 * @param[out] p_opt_id       Pointer to pointer to @ref ble_version_t address.
 * @param[out] pp_opt         Pointer to pointer to @ref ble_opt_t address.
 *
 * @retval NRF_SUCCESS                Decoding success.
 * @retval NRF_ERROR_NULL             Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Decoding failure. Invalid operation type.
 */
uint32_t ble_opt_get_req_dec(uint8_t const * const   p_buf,
                             uint16_t                packet_len,
                             uint32_t *  const       p_opt_id,
                             ble_opt_t **const       pp_opt );


/**@brief Encodes @ref sd_ble_opt_get command response.
 *
 * @sa @ref nrf51_opt_get_encoding for packet format.
 *     @ref ble_opt_get_req_dec for request decoding.
 *
 * @param[in] return_code         Return code indicating if command was successful or not.
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 * @param[in] opt_id              identifies type of ble_opt_t union
 * @param[in] p_opt               Pointer to @ref ble_opt_t union.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */

uint32_t ble_opt_get_rsp_enc(uint32_t                return_code,
                             uint8_t * const         p_buf,
                             uint32_t * const        p_buf_len,
                             uint32_t                opt_id,
                             ble_opt_t const * const p_opt);

                             
/**@brief Decodes @ref sd_ble_opt_set command request.
 *
 * @sa @ref nrf51_opt_set_encoding for packet format,
 *     @ref ble_opt_set_rsp_enc for response encoding.
 *
 * @param[in] p_buf           Pointer to beginning of command request packet.
 * @param[in] packet_len      Length (in bytes) of response packet.
 * @param[out] p_opt_id       Pointer to @ref ble_opt_t union type identifier.
 * @param[out] pp_opt         Pointer to pointer to @ref ble_opt_t union.
 *
 * @retval NRF_SUCCESS                Decoding success.
 * @retval NRF_ERROR_NULL             Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Decoding failure. Invalid operation type.
 */
uint32_t ble_opt_set_req_dec(uint8_t const * const   p_buf,
                             uint16_t                packet_len,
                             uint32_t *  const       p_opt_id,
                             ble_opt_t **const       pp_opt );


/**@brief Encodes @ref sd_ble_opt_set command response.
 *
 * @sa @ref nrf51_opt_set_encoding for packet format.
 *     @ref ble_opt_set_req_dec for request decoding.
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

uint32_t ble_opt_set_rsp_enc(uint32_t                return_code,
                             uint8_t * const         p_buf,
                             uint32_t * const        p_buf_len);

                             
/**@brief Decodes @ref sd_ble_uuid_encode command request.
 *
 * @sa @ref nrf51_uuid_encode_encoding for packet format,
 *     @ref ble_uuid_encode_rsp_enc for response encoding.
 *
 * @param[in] p_buf           Pointer to beginning of command request packet.
 * @param[in] packet_len      Length (in bytes) of response packet.
 * @param[out] pp_uuid        Pointer to pointer to @ref ble_uuid_t structure.
 * @param[out] pp_uuid_le_len Pointer to pointer to the length of encoded UUID.
 * @param[out] pp_uuid_le     Pointer to pointer to buffer where encoded UUID will be stored.
 *
 * @retval NRF_SUCCESS                Decoding success.
 * @retval NRF_ERROR_NULL             Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Decoding failure. Invalid operation type.
 */
uint32_t ble_uuid_encode_req_dec(uint8_t const * const p_buf,
                                 uint16_t              packet_len,
                                 ble_uuid_t * * const  pp_uuid,
                                 uint8_t * * const     pp_uuid_le_len,
                                 uint8_t * * const     pp_uuid_le);

/**@brief Encodes @ref sd_ble_uuid_encode command response.
 *
 * @sa @ref nrf51_uuid_encode_encoding for packet format.
 *     @ref ble_uuid_encode_req_dec for request decoding.
 *
 * @param[in] return_code         Return code indicating if command was successful or not.
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 * @param[in] uuid_le_len         Length of the encoded UUID.
 * @param[in] p_uuid_le          Pointer to the buffer with encoded UUID.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_uuid_encode_rsp_enc(uint32_t              return_code,
                                 uint8_t * const       p_buf,
                                 uint32_t * const      p_buf_len,
                                 uint8_t               uuid_le_len,
                                 uint8_t const * const p_uuid_le);

/**@brief Decodes @ref sd_ble_uuid_decode command request.
 *
 * @sa @ref nrf51_uuid_decode_encoding for packet format,
 *     @ref ble_uuid_decode_rsp_enc for response encoding.
 *
 * @param[in]  p_buf           Pointer to beginning of command request packet.
 * @param[in]  buf_len      Length (in bytes) of response packet.
 * @param[out] p_uuid_le_len   Pointer to the length of encoded UUID.
 * @param[out] pp_uuid_le      Pointer to pointer to buffer where encoded UUID will be stored.
 * @param[out] pp_uuid         Pointer to pointer to @ref ble_uuid_t structure.
 *                             \c It will be set to NULL if p_uuid is not present in the packet.
 *
 * @retval NRF_SUCCESS                Decoding success.
 * @retval NRF_ERROR_NULL             Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Decoding failure. Incorrect buffer length.
 */
uint32_t ble_uuid_decode_req_dec(uint8_t const * const p_buf,
                                 uint32_t const        buf_len,
                                 uint8_t *             p_uuid_le_len,
                                 uint8_t * * const     pp_uuid_le,
                                 ble_uuid_t * * const  pp_uuid);

/**@brief Encodes @ref sd_ble_uuid_decode command response.
 *
 * @sa @ref nrf51_uuid_decode_encoding for packet format.
 *     @ref ble_uuid_decode_req_dec for request decoding.
 *
 * @param[in] return_code         Return code indicating if command was successful or not.
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 * @param[in] p_uuid              Pointer to the buffer with encoded UUID.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_uuid_decode_rsp_enc(uint32_t                 return_code,
                                 uint8_t * const          p_buf,
                                 uint32_t * const         p_buf_len,
                                 ble_uuid_t const * const p_uuid);

/**@brief Decodes @ref sd_ble_uuid_vs_add command request.
 *
 * @sa @ref nrf51_uuid_vs_add_encoding for packet format,
 *     @ref ble_uuid_vs_add_rsp_enc for response encoding.
 *
 * @param[in]  p_buf        Pointer to beginning of command request packet.
 * @param[in]  buf_len      Length (in bytes) of response packet.
 * @param[out] pp_uuid      Pointer to pointer to UUID.
 *                          \c It will be set to NULL if p_uuid is not present in the packet.
 * @param[out] pp_uuid_type Pointer to pointer to UUID type.
 *                          \c It will be set to NULL if p_uuid_type is not present in the packet.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t ble_uuid_vs_add_req_dec(uint8_t const * const   p_buf,
                                 uint16_t                buf_len,
                                 ble_uuid128_t * * const pp_uuid,
                                 uint8_t * * const       pp_uuid_type);

/**@brief Encodes @ref sd_ble_uuid_vs_add command response.
 *
 * @sa @ref nrf51_uuid_vs_add_encoding for packet format.
 *     @ref ble_uuid_vs_add_req_dec for request decoding.
 *
 * @param[in] return_code         Return code indicating if command was successful or not.
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 * @param[in] p_uuid_type              Pointer to the UUID type.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_uuid_vs_add_rsp_enc(uint32_t              return_code,
                                 uint8_t * const       p_buf,
                                 uint32_t * const      p_buf_len,
                                 uint8_t const * const p_uuid_type);
                                 
/**@brief Decodes @ref sd_ble_enable command request.
 *
 * @sa @ref nrf51_enable_encoding for packet format,
 *     @ref ble_enable_rsp_enc for response encoding.
 *
 * @param[in]  p_buf                Pointer to beginning of command request packet.
 * @param[in]  packet_len              Length (in bytes) of response packet.
 * @param[out] pp_ble_enable_params Pointer to pointer to ble_enable_params_t.
 *                                  \c It will be set to NULL if p_ble_enable_params is not present in the packet.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t ble_enable_req_dec(uint8_t const * const         p_buf,
                            uint32_t                      packet_len,
                            ble_enable_params_t * * const pp_ble_enable_params);

/**@brief Encodes @ref sd_ble_enable command response.
 *
 * @sa @ref nrf51_enable_encoding for packet format.
 *     @ref ble_enable_req_dec for request decoding.
 *
 * @param[in]  return_code        Return code indicating if command was successful or not.
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_enable_rsp_enc(uint32_t         return_code,
                            uint8_t * const  p_buf,
                            uint32_t * const p_buf_len);

/**@brief Pre-decodes opt_id of @ref ble_opt_t for middleware.
 *
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] packet_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 * @param[in,out] p_opt_id        Pointer to opt_id which identifies type of @ref ble_opt_t union.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_opt_id_pre_dec(uint8_t const * const   p_buf,
                            uint16_t                packet_len,
                            uint32_t *  const       p_opt_id);

/**@brief Decodes @ref sd_ble_user_mem_reply command request.
 *
 * @sa @ref nrf51_user_mem_reply_encoding for packet format,
 *     @ref ble_user_mem_reply_rsp_enc for response encoding.
 *
 * @param[in]  p_buf                Pointer to beginning of command request packet.
 * @param[in]  packet_len           Length (in bytes) of response packet.
 * @param[in]  p_conn_handle        Pointer to Connection Handle.
 * @param[in,out] pp_block          Pointer to pointer to ble_user_mem_block_t.
 *                                  \c It will be set to NULL if p_block is not present in the packet.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t ble_user_mem_reply_req_dec(uint8_t const * const          p_buf,
                                    uint32_t                       packet_len,
                                    uint16_t * const               p_conn_handle,
                                    ble_user_mem_block_t * * const pp_block);

/**@brief Encodes @ref sd_ble_user_mem_reply command response.
 *
 * @sa @ref nrf51_user_mem_reply_encoding for packet format.
 *     @ref ble_user_mem_reply_req_dec for request decoding.
 *
 * @param[in]  return_code        Return code indicating if command was successful or not.
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_user_mem_reply_rsp_enc(uint32_t         return_code,
                                    uint8_t * const  p_buf,
                                    uint32_t * const p_buf_len);

/** @} */

#endif
