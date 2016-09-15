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
 * @defgroup soc_app SOC Application command request encoders and command response decoders
 * @{
 * @ingroup  ser_app_s130_codecs
 *
 * @brief    SOC Application command request encoders and command response decoders.
 */
 
#ifndef NRF_SOC_APP_H__
#define NRF_SOC_APP_H__

#include <stdint.h>
#include "nrf_soc.h"
/**@brief Encodes @ref sd_power_system_off command request.
 *
 * @sa @ref nrf51_sd_power_off for packet format.
 *
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: size of p_buf buffer. \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t power_system_off_req_enc(uint8_t * const p_buf, uint32_t * const p_buf_len);


/**@brief Encodes @ref sd_temp_get command request.
 *
 * @sa @ref nrf51_sd_temp_get for packet format.
       @ref temp_get_rsp_dec for command response decoder.
 *
 * @param[in] p_temp         Pointer to result of temperature measurement.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: size of p_buf buffer. \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t temp_get_req_enc(int32_t const * const p_temp,
                          uint8_t * const       p_buf,
                          uint32_t * const      p_buf_len);

/**@brief Decodes response to @ref sd_temp_get command.
 *
 * @sa @ref nrf51_sd_temp_get for packet format,
 *     @ref temp_get_req_enc for command request encoder.
 *
 * @param[in] p_buf        Pointer to beginning of command response packet.
 * @param[in] packet_len   Length (in bytes) of response packet.
 * @param[out] p_result_code     Command result code.
 * @param[out] p_temp      Pointer to result of temperature measurement.
 *
 * @return NRF_SUCCESS              Version information stored successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Length of \p p_event is too small to
 *                                  hold decoded event.
 */
uint32_t temp_get_rsp_dec(uint8_t const * const p_buf,
                          uint32_t              packet_len,
                          uint32_t * const      p_result_code,
                          int32_t * const       p_temp);

/**@brief Encodes @ref sd_ecb_block_encrypt command request.
 *
 * @sa @ref nrf51_sd_ecb_block_encrypt for packet format.
       @ref ecb_block_encrypt_rsp_dec for command response decoder.
 *
 * @param[in] p_ecb_data     Pointer to ECB data.
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in,out] p_buf_len  \c in: size of p_buf buffer. \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ecb_block_encrypt_req_enc(nrf_ecb_hal_data_t * p_ecb_data,
                                     uint8_t * const              p_buf,
                                     uint32_t * const             p_buf_len);

/**@brief Decodes response to @ref sd_ecb_block_encrypt command.
 *
 * @sa @ref nrf51_sd_ecb_block_encrypt for packet format,
 *     @ref ecb_block_encrypt_req_enc for command request encoder.
 *
 * @param[in] p_buf            Pointer to beginning of command response packet.
 * @param[in] packet_len       Length (in bytes) of response packet.
 * @param[out] p_ecb_data      Pointer to ECB data.
 * @param[out] p_result_code   Command result code.
 *
 * @return NRF_SUCCESS              Version information stored successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE      Decoding failure. Length of \p p_event is too small to
 *                                  hold decoded event.
 */
uint32_t ecb_block_encrypt_rsp_dec(uint8_t const * const  p_buf,
                                   uint32_t               packet_len,
                                   nrf_ecb_hal_data_t *   p_ecb_data,
                                   uint32_t * const       p_result_code);
/** @} */

#endif // NRF_SOC_APP_H__
