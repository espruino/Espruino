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
#ifndef NRF_SOC_CONN_H__
#define NRF_SOC_CONN_H__

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
 * @defgroup soc_conn SOC Connectivity command request decoders and command response encoders
 * @{
 * @ingroup  ser_conn_s130_codecs
 *
 * @brief    SOC Connectivity command request decoders and command response encoderss.
 */
#include "nrf_soc.h"

/**@brief Decodes @ref sd_power_system_off command request.
 *
 * @sa @ref nrf51_sd_power_off for packet format.
 *
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in] packet_len     Length (in bytes) of request packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_PARAM    Encoding failure. Incorrect parameter.
 */
uint32_t power_system_off_req_dec(uint8_t const * const p_buf, uint16_t packet_len);


/**@brief Decodes @ref sd_temp_get command request.
 *
 * @sa @ref nrf51_sd_temp_get for packet format.
 *     @ref temp_get_rsp_enc for response encoding.
 *
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in] buf_len     Length (in bytes) of request packet.
 * @param[out] pp_temp       Pointer to pointer to result of temperature measurement.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_PARAM    Encoding failure. Incorrect parameter.
 */
uint32_t temp_get_req_dec(uint8_t const * const p_buf,
                          uint32_t              buf_len,
                          int32_t * * const     pp_temp);

/**@brief Encodes @ref sd_temp_get command response.
 *
 * @sa @ref nrf51_sd_temp_get for packet format.
 *     @ref temp_get_req_dec for request decoding.
 *
 * @param[in] return_code         Return code indicating if command was successful or not.
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 * @param[in] p_temp              Pointer to result of temperature measurement.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t temp_get_rsp_enc(uint32_t         return_code,
                          uint8_t * const  p_buf,
                          uint32_t * const p_buf_len,
                          int32_t * const  p_temp);

/**@brief Decodes @ref sd_ecb_block_encrypt command request.
 *
 * @sa @ref nrf51_sd_ecb_block_encrypt for packet format.
 *     @ref ecb_block_encrypt_rsp_enc for response encoding.
 *
 * @param[in] p_buf          Pointer to buffer where encoded data command will be returned.
 * @param[in] buf_len        Length (in bytes) of request packet.
 * @param[out] pp_ecb_data   Pointer to pointer to ECB data.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied
 * @retval NRF_ERROR_INVALID_PARAM    Encoding failure. Incorrect parameter.
 */
uint32_t ecb_block_encrypt_req_dec(uint8_t const * const            p_buf,
                                   uint32_t                         buf_len,
                                   nrf_ecb_hal_data_t * * const     pp_ecb_data);

/**@brief Encodes @ref sd_ecb_block_encrypt command response.
 *
 * @sa @ref nrf51_sd_ecb_block_encrypt for packet format.
 *     @ref ecb_block_encrypt_req_dec for request decoding.
 *
 * @param[in] return_code         Return code indicating if command was successful or not.
 * @param[out] p_buf              Pointer to buffer where encoded data command response will be
 *                                returned.
 * @param[in,out] p_buf_len       \c in: size of \p p_buf buffer.
 *                                \c out: Length of encoded command response packet.
 * @param[in] p_ecb_data          Pointer to ECB data.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ecb_block_encrypt_rsp_enc(uint32_t         return_code,
                                   uint8_t * const  p_buf,
                                   uint32_t * const p_buf_len,
                                   nrf_ecb_hal_data_t * const  p_ecb_data);

/** @} */
#endif

