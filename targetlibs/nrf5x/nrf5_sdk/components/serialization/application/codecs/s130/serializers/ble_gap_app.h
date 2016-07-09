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
#ifndef BLE_GAP_APP_H__
#define BLE_GAP_APP_H__

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
 * @defgroup ble_gap_app GAP Application command request encoders and command response decoders
 * @{
 * @ingroup  ser_app_s130_codecs
 *
 * @brief    GAP Application command request encoders and command response decoders.
 */
#include "ble.h"
#include "ble_gap.h"

/**
 * @brief Encodes @ref sd_ble_gap_address_get command request.
 *
 * @sa @ref nrf51_address_get_encoding for packet format,
 *     @ref ble_gap_address_get_rsp_dec for command response decoder.
 *
 * @param[in] p_address      Pointer to address.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @note  \p p_address  will not be updated by the command
 *        request encoder. Updated values are set by @ref ble_gap_address_get_rsp_dec.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_address_get_req_enc(ble_gap_addr_t const * const p_address,
                                     uint8_t * const              p_buf,
                                     uint32_t * const             p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_gap_address_get command.
 *
 * @sa @ref nrf51_address_get_encoding for packet format,
 *     @ref ble_gap_address_get_req_enc for command request encoder.
 *
 * @param[in] p_buf           Pointer to beginning of command response packet.
 * @param[in] packet_len      Length (in bytes) of response packet.
 * @param[out] p_address      Pointer to address.
 * @param[out] p_result_code  Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_gap_address_get_rsp_dec(uint8_t const * const  p_buf,
                                     uint32_t               packet_len,
                                     ble_gap_addr_t * const p_address,
                                     uint32_t * const       p_result_code);

/**
 * @brief Encodes @ref sd_ble_gap_address_set command request.
 *
 * @sa @ref nrf51_gap_address_set_encoding for packet format,
 *     @ref ble_gap_address_set_rsp_dec for command response decoder.
 *
 * @param[in]     addr_cycle_mode      Address cycle mode.
 * @param[in]     p_addr               Pointer to address structure.
 * @param[in,out] p_buf                Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len            \c in: size of \p p_buf buffer.
 *                                     \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS               Encoding success.
 * @retval NRF_ERROR_NULL            Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_address_set_req_enc(uint8_t                      addr_cycle_mode,
                                     ble_gap_addr_t const * const p_addr,
                                     uint8_t * const              p_buf,
                                     uint32_t * const             p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_gap_address_set command.
 *
 * @sa @ref nrf51_gap_address_set_encoding for packet format,
 *     @ref ble_gap_address_set_req_enc for command request encoder.
 *
 * @param[in]  p_buf          Pointer to beginning of command response packet.
 * @param[in]  packet_len     Length (in bytes) of response packet.
 * @param[out] p_result_code  Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_gap_address_set_rsp_dec(uint8_t const * const p_buf,
                                     uint32_t              packet_len,
                                     uint32_t * const      p_result_code);

/**
 * @brief Encodes @ref sd_ble_gap_adv_data_set command request.
 *
 * @sa @ref nrf51_adv_set_encoding for packet format,
 *     @ref ble_gap_adv_data_set_rsp_dec for command response decoder.
 *
 * @param[in] p_data         Raw data to be placed in advertisement packet. If NULL, no changes
 *                           are made to the current advertisement packet data.
 * @param[in] dlen           Data length for p_data. Max size: @ref BLE_GAP_ADV_MAX_SIZE octets.
 *                           Should be 0 if p_data is NULL, can be 0 if p_data is not NULL.
 * @param[in] p_sr_data      Raw data to be placed in scan response packet. If NULL,
 *                           no changes are made to the current scan response packet data.
 * @param[in] srdlen         Data length for p_sr_data. Max size: @ref BLE_GAP_ADV_MAX_SIZE octets.
 *                           Should be 0 if p_sr_data is NULL, can be 0 if p_data is not NULL.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_adv_data_set_req_enc(uint8_t const * const p_data,
                                      uint8_t               dlen,
                                      uint8_t const * const p_sr_data,
                                      uint8_t               srdlen,
                                      uint8_t * const       p_buf,
                                      uint32_t * const      p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_gap_adv_data_set command.
 *
 * @sa @ref nrf51_adv_set_encoding for packet format,
 *     @ref ble_gap_adv_data_set_req_enc for command request encoder.
 *
 * @param[in]  p_buf          Pointer to beginning of command response packet.
 * @param[in]  packet_len     Length (in bytes) of response packet.
 * @param[out] p_result_code  Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_gap_adv_data_set_rsp_dec(uint8_t const * const p_buf,
                                      uint32_t              packet_len,
                                      uint32_t * const      p_result_code);

/**
 * @brief Encodes @ref sd_ble_gap_adv_start command request.
 *
 * @sa @ref nrf51_adv_start_encoding for packet format,
 *     @ref ble_gap_adv_start_rsp_dec for command response decoder.
 *
 * @param[in] p_adv_params   Pointer to advertising parameters structure.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_adv_start_req_enc(ble_gap_adv_params_t const * const p_adv_params,
                                   uint8_t * const                    p_buf,
                                   uint32_t * const                   p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_gap_adv_start command.
 *
 * @sa @ref nrf51_adv_start_encoding for packet format,
 *     @ref ble_gap_adv_start_req_enc for command request encoder.
 *
 * @param[in]  p_buf          Pointer to beginning of command response packet.
 * @param[in]  packet_len     Length (in bytes) of response packet.
 * @param[out] p_result_code  Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_gap_adv_start_rsp_dec(uint8_t const * const p_buf,
                                   uint32_t              packet_len,
                                   uint32_t * const      p_result_code);

/**
 * @brief Encodes @ref sd_ble_gap_tx_power_set command request.
 *
 * @sa @ref nrf51_tx_power_set_encoding for packet format,
 *     @ref ble_gap_tx_power_set_rsp_dec for command response decoder.
 *
 * @param[in]     tx_power   Radio transmit power in dBm (accepted values are -40, -30, -20, -16, -12, -8, -4, 0, and 4 dBm).
 * @param[in]     p_buf      Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_tx_power_set_req_enc(int8_t           tx_power,
                                      uint8_t * const  p_buf,
                                      uint32_t * const p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_gap_tx_power_set command.
 *
 * @sa @ref nrf51_tx_power_set_encoding for packet format,
 *     @ref ble_gap_tx_power_set_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_gap_tx_power_set_rsp_dec(uint8_t const * const p_buf,
                                      uint32_t              packet_len,
                                      uint32_t * const      p_result_code);

/**
 * @brief Encodes @ref sd_ble_gap_appearance_get command request.
 *
 * @sa @ref nrf51_appearance_get_encoding for packet format,
 *     @ref ble_gap_appearance_get_rsp_dec for command response decoder.
 *
 * @param[in] p_appearance   Appearance (16-bit), see @ref BLE_APPEARANCES
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @note  \p p_appearance  will not be updated by the command
 *        request encoder. Updated values are set by @ref ble_gap_appearance_get_rsp_dec.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_appearance_get_req_enc(uint16_t const * const p_appearance,
                                        uint8_t * const        p_buf,
                                        uint32_t * const       p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_gap_appearance_get command.
 *
 * @sa @ref nrf51_appearance_get_encoding for packet format,
 *     @ref ble_gap_appearance_get_req_enc for command request encoder.
 *
 * @param[in] p_buf           Pointer to beginning of command response packet.
 * @param[in] packet_len      Length (in bytes) of response packet.
 * @param[out] p_appearance   Appearance (16-bit), see @ref BLE_APPEARANCES.
 * @param[out] p_result_code  Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_gap_appearance_get_rsp_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint16_t * const      p_appearance,
                                        uint32_t * const      p_result_code);

/**
 * @brief Encodes @ref sd_ble_gap_appearance_set command request.
 *
 * @sa @ref nrf51_appearance_set_encoding for packet format,
 *     @ref ble_gap_appearance_set_rsp_dec for command response decoder.
 *
 * @param[in] appearance   Appearance (16-bit), see @ref BLE_APPEARANCES.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_appearance_set_req_enc(uint16_t         appearance,
                                        uint8_t * const  p_buf,
                                        uint32_t * const p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_gap_appearance_set command.
 *
 * @sa @ref nrf51_appearance_set_encoding for packet format,
 *     @ref ble_gap_appearance_set_req_enc for command request encoder.
 *
 * @param[in] p_buf           Pointer to beginning of command response packet.
 * @param[in] packet_len      Length (in bytes) of response packet.
 * @param[out] p_result_code  Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_gap_appearance_set_rsp_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint32_t * const      p_result_code);

/**
 * @brief Encodes @ref sd_ble_gap_device_name_get command request.
 *
 * @sa @ref nrf51_device_name_get_encoding for packet format,
 *     @ref ble_gap_device_name_get_rsp_dec for command response decoder.
 *
 * @param[in] p_dev_name     Pointer to an empty buffer where the UTF-8 <b>non NULL-terminated</b>
 *                           string will be placed. Set to NULL to obtain the complete device
 *                           name length.
 * @param[in] p_dev_name_len          Length of the buffer pointed by p_dev_name.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @note  \p p_dev_name and \p  p_len will not be updated by the command
 *        request encoder. Updated values are set by @ref ble_gap_device_name_get_rsp_dec.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_device_name_get_req_enc(uint8_t const * const  p_dev_name,
                                         uint16_t const * const p_dev_name_len,
                                         uint8_t * const        p_buf,
                                         uint32_t * const       p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_gap_device_name_get command.
 *
 * @sa @ref nrf51_device_name_get_encoding for packet format,
 *     @ref ble_gap_device_name_get_req_enc for command request encoder.
 *
 * @param[in] p_buf              Pointer to beginning of command response packet.
 * @param[in] packet_len         Length (in bytes) of response packet.
 * @param[out] p_dev_name        Pointer to an empty buffer where the UTF-8
 *                               <b>non NULL-terminated</b> string will be placed.
 * @param[in,out] p_dev_name_len  Length of the buffer pointed by p_dev_name, complete device name
 *                               length on output.
 * @param[out] p_result_code     Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_gap_device_name_get_rsp_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         uint8_t * const       p_dev_name,
                                         uint16_t * const      p_dev_name_len,
                                         uint32_t * const      p_result_code);

/**
 * @brief Encodes @ref sd_ble_gap_device_name_set command request.
 *
 * @sa @ref nrf51_device_name_set_encoding for packet format,
 *     @ref ble_gap_device_name_set_rsp_dec for command response decoder.
 *
 * @param[in] p_write_perm   Write permissions for the Device Name characteristic see
 *                           @ref ble_gap_conn_sec_mode_t.
 * @param[in] p_dev_name     Pointer to a UTF-8 encoded, <b>non NULL-terminated</b> string.
 * @param[in] len            Length of the UTF-8, <b>non NULL-terminated</b> string pointed
 *                           to by p_dev_name in octets (must be smaller or equal
 *                           than @ref BLE_GAP_DEVNAME_MAX_LEN).
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_device_name_set_req_enc(ble_gap_conn_sec_mode_t const * const p_write_perm,
                                         uint8_t const * const                 p_dev_name,
                                         uint16_t                              len,
                                         uint8_t * const                       p_buf,
                                         uint32_t * const                      p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_gap_device_name_set command.
 *
 * @sa @ref nrf51_device_name_set_encoding for packet format,
 *     @ref ble_gap_device_name_set_req_enc for command request encoder.
 *
 * @param[in] p_buf              Pointer to beginning of command response packet.
 * @param[in] packet_len         Length (in bytes) of response packet.
 * @param[out] p_result_code     Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_gap_device_name_set_rsp_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         uint32_t * const      p_result_code);

/**
 * @brief Encodes @ref sd_ble_gap_ppcp_set command request.
 *
 * @sa @ref nrf51_ppcp_set_encoding for packet format,
 *     @ref ble_gap_ppcp_set_rsp_dec for command response decoder.
 *
 * @param[in] p_conn_params  Pointer to a @ref ble_gap_conn_params_t structure with the
 *                           desired parameters.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_ppcp_set_req_enc(ble_gap_conn_params_t const * const p_conn_params,
                                  uint8_t * const                     p_buf,
                                  uint32_t * const                    p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_gap_ppcp_set command.
 *
 * @sa @ref nrf51_ppcp_set_encoding for packet format,
 *     @ref ble_gap_ppcp_set_req_enc for command request encoder.
 *
 * @param[in] p_buf              Pointer to beginning of command response packet.
 * @param[in] packet_len         Length (in bytes) of response packet.
 * @param[out] p_result_code     Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_gap_ppcp_set_rsp_dec(uint8_t const * const p_buf,
                                  uint32_t              packet_len,
                                  uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_gap_conn_param_update command request.
 *
 * @sa @ref nrf51_gap_conn_param_update_encoding for packet format,
 *     @ref ble_gap_conn_param_update_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle     Connection handle of the connection.
 * @param[in]      p_conn_params   Pointer to desired connection parameters..
 * @param[in]      p_buf           Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                 \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_conn_param_update_req_enc(uint16_t                            conn_handle,
                                           ble_gap_conn_params_t const * const p_conn_params,
                                           uint8_t * const                     p_buf,
                                           uint32_t * const                    p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_conn_param_update command.
 *
 * @sa @ref nrf51_gap_conn_param_update_encoding for packet format,
 *     @ref ble_gap_conn_param_update_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_conn_param_update_rsp_dec(uint8_t const * const p_buf,
                                           uint32_t              packet_len,
                                           uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_gap_disconnect command request.
 *
 * @sa @ref nrf51_disconnect_encoding for packet format,
 *     @ref ble_gap_disconnect_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle       Connection handle of the connection.
 * @param[in]      hci_status_code   HCI status code, see @ref BLE_HCI_STATUS_CODES.
 * @param[in]      p_buf             Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len         \c in: size of \p p_buf buffer.
 *                                   \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_disconnect_req_enc(uint16_t         conn_handle,
                                    uint8_t          hci_status_code,
                                    uint8_t * const  p_buf,
                                    uint32_t * const p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_disconnect command.
 *
 * @sa @ref nrf51_disconnect_encoding for packet format,
 *     @ref ble_gap_disconnect_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_disconnect_rsp_dec(uint8_t const * const p_buf,
                                    uint32_t              packet_len,
                                    uint32_t * const      p_result_code);


/**@brief Encodes @ref sd_ble_gap_rssi_stop command request.
 *
 * @sa @ref nrf51_rssi_stop_encoding for packet format,
 *     @ref ble_gap_rssi_stop_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle       Connection handle of the connection.
 * @param[in]      p_buf             Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len         \c in: size of \p p_buf buffer.
 *                                   \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_rssi_stop_req_enc(uint16_t         conn_handle,
                                   uint8_t * const  p_buf,
                                   uint32_t * const p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_rssi_stop command.
 *
 * @sa @ref nrf51_rssi_stop_encoding for packet format,
 *     @ref ble_gap_rssi_stop_rsp_dec for command response decoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_rssi_stop_rsp_dec(uint8_t const * const p_buf,
                                   uint32_t              packet_len,
                                   uint32_t * const      p_result_code);




/**@brief Encodes @ref sd_ble_gap_ppcp_get command request.
 *
 * @sa @ref nrf51_gap_ppcp_get_encoding for packet format,
 *     @ref ble_gap_ppcp_get_rsp_dec for command response decoder.
 *
 * @param[in]      p_conn_params  Pointer to a @ref ble_gap_conn_params_t structure where the
 *                                parameters will be stored.
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @note  \p p_conn_params will not be updated by the command request encoder. Updated values are
 *        set by @ref ble_gap_ppcp_get_rsp_dec.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_ppcp_get_req_enc(ble_gap_conn_params_t const * const p_conn_params,
                                  uint8_t * const                     p_buf,
                                  uint32_t * const                    p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_ppcp_get command.
 *
 * @sa @ref nrf51_gap_ppcp_get_encoding for packet format,
 *     @ref ble_gap_ppcp_get_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_conn_params Pointer to a @ref ble_gap_conn_params_t structure where the parameters
 *                           will be stored.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_ppcp_get_rsp_dec(uint8_t const * const         p_buf,
                                  uint32_t                      packet_len,
                                  ble_gap_conn_params_t * const p_conn_params,
                                  uint32_t * const              p_result_code);

/**@brief Encodes @ref sd_ble_gap_auth_key_reply command request.
 *
 * @sa @ref nrf51_auth_key_reply_encoding for packet format,
 *     @ref ble_gap_auth_key_reply_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle    Connection handle of the connection.
 * @param[in]      key_type       Key type which defines length of key data as defined for
 *                                @ref sd_ble_gap_auth_key_reply .
 * @param[in]      p_key          Pointer to a buffer which contains key
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_PARAM    Encoding failure. Incorrect param provided (key_type).
 */
uint32_t ble_gap_auth_key_reply_req_enc(uint16_t              conn_handle,
                                        uint8_t               key_type,
                                        uint8_t const * const p_key,
                                        uint8_t * const       p_buf,
                                        uint32_t * const      p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_auth_key_reply command.
 *
 * @sa @ref nrf51_auth_key_reply_encoding for packet format,
 *     @ref ble_gap_auth_key_reply_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_auth_key_reply_rsp_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_gap_sec_info_reply command request.
 *
 * @sa @ref nrf51_gap_sec_info_reply_encoding for packet format,
 *     @ref ble_gap_sec_info_reply_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle    Connection handle of the connection.
 * @param[in]      p_enc_info     Pointer to a @ref ble_gap_enc_info_t encryption information
 *                                structure.
 * @param[in]      p_id_info      Pointer to a @ref ble_gap_irk_t id information
 *                                structure.
 * @param[in]      p_sign_info    Pointer to a @ref ble_gap_sign_info_t signing information
 *                                structure.
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_sec_info_reply_req_enc(uint16_t                    conn_handle,
                                        ble_gap_enc_info_t  const * p_enc_info,
                                        ble_gap_irk_t       const * p_id_info, 
                                        ble_gap_sign_info_t const * p_sign_info,
                                        uint8_t * const             p_buf,
                                        uint32_t * const            p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_sec_info_reply command.
 *
 * @sa @ref nrf51_gap_sec_info_reply_encoding for packet format,
 *     @ref ble_gap_sec_info_reply_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_sec_info_reply_rsp_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_gap_sec_params_reply command request.
 *
 * @sa @ref nrf51_sec_params_reply_encoding for packet format,
 *     @ref ble_gap_sec_params_reply_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle    Connection handle of the connection.
 * @param[in]      sec_status     Security status, see @ref BLE_GAP_SEC_STATUS.
 * @param[in]      p_sec_params   Pointer to @ref ble_gap_sec_params_t security parameters
 *                                structure.
 * @param[in]      p_sec_keyset   Pointer to @ref ble_gap_sec_keyset_t security keys
 *                                structure.
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_sec_params_reply_req_enc(uint16_t                           conn_handle,
                                          uint8_t                            sec_status,
                                          ble_gap_sec_params_t const * const p_sec_params,
                                          ble_gap_sec_keyset_t const * const p_sec_keyset,
                                          uint8_t * const                    p_buf,
                                          uint32_t * const                   p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_sec_params_reply command.
 *
 * @sa @ref nrf51_sec_params_reply_encoding for packet format,
 *     @ref ble_gap_sec_params_reply_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[in]  p_sec_keyset  Pointer to @ref ble_gap_sec_keyset_t security keys
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_sec_params_reply_rsp_dec(uint8_t const * const        p_buf,
                                          uint32_t                     packet_len,
                                          ble_gap_sec_keyset_t const * const p_sec_keyset,
                                          uint32_t * const             p_result_code);

/**@brief Encodes @ref sd_ble_gap_authenticate command request.
 *
 * @sa @ref nrf51_ble_gap_authenticate_encoding for packet format,
 *     @ref ble_gap_authenticate_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle    Connection handle of the connection.
 * @param[in]      p_sec_params   Pointer to a @ref ble_gap_sec_params_t security parameters
 *                                structure.
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_authenticate_req_enc(uint16_t                           conn_handle,
                                      ble_gap_sec_params_t const * const p_sec_params,
                                      uint8_t * const                    p_buf,
                                      uint32_t * const                   p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_authenticate command.
 *
 * @sa @ref nrf51_ble_gap_authenticate_encoding for packet format,
 *     @ref ble_gap_authenticate_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_authenticate_rsp_dec(uint8_t const * const p_buf,
                                      uint32_t              packet_len,
                                      uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_gap_adv_stop command request.
 *
 * @sa @ref nrf51_sd_ble_gap_adv_stop for packet format,
 *     @ref ble_gap_adv_stop_rsp_dec for command response decoder.
 *
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_adv_stop_req_enc(uint8_t * const p_buf, uint32_t * const p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_adv_stop command.
 *
 * @sa @ref nrf51_sd_ble_gap_adv_stop for packet format,
 *     @ref ble_gap_adv_stop_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_adv_stop_rsp_dec(uint8_t const * const p_buf,
                                  uint32_t              packet_len,
                                  uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_gap_conn_sec_get command request.
 *
 * @sa @ref nrf51_gap_conn_sec_get_encoding for packet format,
 *     @ref ble_gap_conn_sec_get_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle       Connection handle of the connection.
 * @param[in]      p_conn_sec        Pointer to \ref ble_gap_conn_sec_t which will be filled in
 *                                   response.
 * @param[in]      p_buf             Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len         \c in: size of \p p_buf buffer.
 *                                   \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_conn_sec_get_req_enc(uint16_t                         conn_handle,
                                      ble_gap_conn_sec_t const * const p_conn_sec,
                                      uint8_t * const                  p_buf,
                                      uint32_t * const                 p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_conn_sec_get command.
 *
 * @sa @ref nrf51_gap_conn_sec_get_encoding for packet format,
 *     @ref ble_gap_conn_sec_get_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] pp_conn_sec   Pointer to pointer to \ref ble_gap_conn_sec_t which will be filled by
 *                           the decoded data (if present).
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_conn_sec_get_rsp_dec(uint8_t const * const        p_buf,
                                      uint32_t                     packet_len,
                                      ble_gap_conn_sec_t * * const pp_conn_sec,
                                      uint32_t * const             p_result_code);

/**@brief Encodes @ref sd_ble_gap_rssi_start command request.
 *
 * @sa @ref nrf51_rssi_start_encoding for packet format,
 *     @ref ble_gap_rssi_start_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle       Connection handle of the connection.
 * @param[in]      threshold_dbm     Threshold in dBm.
 * @param[in]      skip_count        Sample skip count.
 * @param[in]      p_buf             Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len         \c in: size of \p p_buf buffer.
 *                                   \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_rssi_start_req_enc(uint16_t         conn_handle,
                                    uint8_t          threshold_dbm,
                                    uint8_t          skip_count,
                                    uint8_t * const  p_buf,
                                    uint32_t * const p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_rssi_start command.
 *
 * @sa @ref nrf51_rssi_start_encoding for packet format,
 *     @ref ble_gap_rssi_start_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_rssi_start_rsp_dec(uint8_t const * const p_buf,
                                    uint32_t              packet_len,
                                    uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_gap_scan_stop command request.
 *
 * @sa @ref nrf51_scan_stop_encoding for packet format,
 *     @ref ble_gap_scan_stop_rsp_dec for command response decoder.
 *
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_scan_stop_req_enc(uint8_t * const p_buf, uint32_t * const p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_scan_stop command.
 *
 * @sa @ref nrf51_scan_stop_encoding for packet format,
 *     @ref ble_gap_scan_stop_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_scan_stop_rsp_dec(uint8_t const * const p_buf,
                                   uint32_t              packet_len,
                                   uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_gap_scan_start command request.
 *
 * @sa @ref nrf51_scan_start_encoding for packet format,
 *     @ref ble_gap_scan_start_rsp_dec for command response decoder.
 *
 * @param[in]      p_scan_params  Pointer to scan params structure.
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_scan_start_req_enc(ble_gap_scan_params_t const *  p_scan_params,
                                    uint8_t * const                p_buf,
                                    uint32_t * const               p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_scan_start command.
 *
 * @sa @ref nrf51_scan_start_encoding for packet format,
 *     @ref ble_gap_scan_start_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_scan_start_rsp_dec(uint8_t const * const p_buf,
                                    uint32_t              packet_len,
                                    uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_gap_connect command request.
 *
 * @sa @ref nrf51_connect_encoding for packet format,
 *     @ref ble_gap_connect_rsp_dec for command response decoder.
 *
 * @param[in]      p_peer_addr    Pointer to peer address.
 * @param[in]      p_scan_params  Pointer to scan params structure.
 * @param[in]      p_conn_params  Pointer to desired connection parameters.
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_connect_req_enc(ble_gap_addr_t const * const        p_peer_addr,
                                 ble_gap_scan_params_t const * const p_scan_params,
                                 ble_gap_conn_params_t const * const p_conn_params,
                                 uint8_t * const                     p_buf,
                                 uint32_t * const                    p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_connect command.
 *
 * @sa @ref nrf51_connect_encoding for packet format,
 *     @ref ble_gap_connect_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_connect_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 uint32_t * const      p_result_code);

/**@brief Encodes @ref sd_ble_gap_connect_cancel command request.
 *
 * @sa @ref nrf51_connect_cancel_encoding for packet format,
 *     @ref ble_gap_connect_cancel_rsp_dec for command response decoder.
 *
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_connect_cancel_req_enc(uint8_t * const  p_buf,
                                        uint32_t * const p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_connect_cancel command.
 *
 * @sa @ref nrf51_connect_cancel_encoding for packet format,
 *     @ref ble_gap_connect_cancel_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_connect_cancel_rsp_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint32_t * const      p_result_code);


/**@brief Encodes @ref sd_ble_gap_encrypt command request.
 *
 * @sa @ref nrf51_gap_encrypt_encoding for packet format,
 *     @ref ble_gap_encrypt_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle    Connection handle.
 * @param[in]      p_master_id    Pointer to a master identification structure.
 * @param[in]      p_enc_info     Pointer to desired connection parameters.
 * @param[in]      p_buf          Pointer to a ble_gap_enc_info_t encryption information structure.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */

uint32_t ble_gap_encrypt_req_enc( uint16_t                          conn_handle,
                                  ble_gap_master_id_t const * const p_master_id,
                                  ble_gap_enc_info_t const  * const p_enc_info,
                                  uint8_t                   * const p_buf,
                                  uint32_t                  * const p_buf_len);


/**@brief Decodes response to @ref sd_ble_gap_encrypt command.
 *
 * @sa @ref nrf51_gap_encrypt_encoding for packet format,
 *     @ref ble_gap_encrypt_req_enc for command request encoder.
 *
 * @param[in]  p_buf         Pointer to beginning of command response packet.
 * @param[in]  packet_len    Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_encrypt_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 uint32_t      * const p_result_code);

/**@brief Encodes @ref sd_ble_gap_rssi_get command request.
 *
 * @sa @ref nrf51_rssi_get_encoding for packet format,
 *     @ref ble_gap_rssi_get_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle    Connection handle.
 * @param[in]      p_rssi         Pointer to the rssi value.
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_rssi_get_req_enc(uint16_t             conn_handle,
                                  int8_t const * const p_rssi,
                                  uint8_t  * const     p_buf,
                                  uint32_t * const     p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_rssi_get command.
 *
 * @sa @ref nrf51_rssi_get_encoding for packet format,
 *     @ref ble_gap_rssi_get_req_enc for command request encoder.
 *
 * @param[in]  p_buf           Pointer to beginning of command response packet.
 * @param[in]  packet_len      Length (in bytes) of response packet.
 * @param[out] p_rssi          Pointer to rssi value.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_rssi_get_rsp_dec(uint8_t const * const p_buf,
                                  uint32_t              packet_len,
                                  int8_t * const        p_rssi,
                                  uint32_t      * const p_result_code);

/**@brief Encodes @ref sd_ble_gap_keypress_notify command request.
 *
 * @sa @ref nrf51_keypress_notify_encoding for packet format,
 *     @ref ble_gap_keypress_notify_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle    Connection handle.
 * @param[in]      kp_not         See @ref sd_ble_gap_keypress_notify
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_keypress_notify_req_enc(uint16_t                           conn_handle,
                                          uint8_t                           kp_not,
                                          uint8_t * const                   p_buf,
                                          uint32_t * const                  p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_keypress_notify command.
 *
 * @sa @ref nrf51_keypress_notify_encoding for packet format,
 *     @ref ble_gap_keypress_notify_req_enc for command request encoder.
 *
 * @param[in]  p_buf           Pointer to beginning of command response packet.
 * @param[in]  packet_len      Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_keypress_notify_rsp_dec(uint8_t const * const              p_buf,
                                        uint32_t                           packet_len,
                                        uint32_t * const                   p_result_code);

/**@brief Encodes @ref sd_ble_gap_lesc_dhkey_reply command request.
 *
 * @sa @ref nrf51_lesc_dhkey_reply_encoding for packet format,
 *     @ref ble_gap_lesc_dhkey_reply_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle    Connection handle.
 * @param[in]      p_dhkey        See @ref sd_ble_gap_lesc_dhkey_reply
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_lesc_dhkey_reply_req_enc(uint16_t             conn_handle,
                                          ble_gap_lesc_dhkey_t const *p_dhkey,
                                          uint8_t * const      p_buf,
                                          uint32_t * const     p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_lesc_dhkey_reply command.
 *
 * @sa @ref nrf51_lesc_dhkey_reply_encoding for packet format,
 *     @ref ble_gap_lesc_dhkey_reply_req_enc for command request encoder.
 *
 * @param[in]  p_buf           Pointer to beginning of command response packet.
 * @param[in]  packet_len      Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_lesc_dhkey_reply_rsp_dec(uint8_t const * const              p_buf,
                                        uint32_t                           packet_len,
                                        uint32_t * const                   p_result_code);

/**@brief Encodes @ref sd_ble_gap_lesc_oob_data_set command request.
 *
 * @sa @ref nrf51_lesc_oob_data_set_encoding for packet format,
 *     @ref ble_gap_lesc_oob_data_set_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle    Connection handle.
 * @param[in]      p_oobd_own     See @ref sd_ble_gap_lesc_oob_data_set
 * @param[in]      p_oobd_peer    See @ref sd_ble_gap_lesc_oob_data_set
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_lesc_oob_data_set_req_enc(uint16_t             conn_handle,
                                           ble_gap_lesc_oob_data_t const *p_oobd_own,
                                           ble_gap_lesc_oob_data_t const *p_oobd_peer,
                                           uint8_t * const      p_buf,
                                           uint32_t * const     p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_lesc_oob_data_set command.
 *
 * @sa @ref nrf51_lesc_oob_data_set_encoding for packet format,
 *     @ref ble_gap_lesc_oob_data_set_req_enc for command request encoder.
 *
 * @param[in]  p_buf           Pointer to beginning of command response packet.
 * @param[in]  packet_len      Length (in bytes) of response packet.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_lesc_oob_data_set_rsp_dec(uint8_t const * const              p_buf,
                                           uint32_t                           packet_len,
                                           uint32_t * const                   p_result_code);

/**@brief Encodes @ref sd_ble_gap_lesc_oob_data_get command request.
 *
 * @sa @ref nrf51_lesc_oob_data_get_encoding for packet format,
 *     @ref ble_gap_lesc_oob_data_get_rsp_dec for command response decoder.
 *
 * @param[in]      conn_handle    Connection handle.
 * @param[in]      p_pk_own       See @ref sd_ble_gap_lesc_oob_data_get
 * @param[in]      p_oobd_own     See @ref sd_ble_gap_lesc_oob_data_get
 * @param[in]      p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in, out] p_buf_len      \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_gap_lesc_oob_data_get_req_enc(uint16_t                      conn_handle,
                                           ble_gap_lesc_p256_pk_t const *p_pk_own,
                                           ble_gap_lesc_oob_data_t      *p_oobd_own,
                                           uint8_t * const               p_buf,
                                           uint32_t * const              p_buf_len);

/**@brief Decodes response to @ref sd_ble_gap_lesc_oob_data_get command.
 *
 * @sa @ref nrf51_lesc_oob_data_get_encoding for packet format,
 *     @ref ble_gap_lesc_oob_data_get_req_enc for command request encoder.
 *
 * @param[in]  p_buf           Pointer to beginning of command response packet.
 * @param[in]  packet_len      Length (in bytes) of response packet.
 * @param[out] pp_oobd_own     Pointer to pointer to location where OOB data is decoded.
 * @param[out] p_result_code   Command response result code.
 *
 * @retval NRF_SUCCESS              Decoding success.
 * @retval NRF_ERROR_NULL           Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA   Decoding failure. Decoded operation code does not match expected
 *                                  operation code.
 */
uint32_t ble_gap_lesc_oob_data_get_rsp_dec(uint8_t const * const       p_buf,
                                           uint32_t                    packet_len,
                                           ble_gap_lesc_oob_data_t  * *pp_oobd_own,
                                           uint32_t * const            p_result_code);
/** @} */
#endif
