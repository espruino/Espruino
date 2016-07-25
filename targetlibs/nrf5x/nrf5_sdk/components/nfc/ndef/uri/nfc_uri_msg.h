/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

#ifndef NFC_URI_MSG_H__
#define NFC_URI_MSG_H__

/**@file
 *
 * @defgroup nfc_uri_msg URI messages
 * @{
 * @ingroup  nfc_ndef_messages
 *
 * @brief    Generation of NFC NDEF messages with a URI record.
 *
 */

#include "nfc_ndef_msg.h"
#include "nfc_uri_rec.h"
#include "nrf_error.h"

/** @brief Function for encoding an NFC NDEF URI message.
 *
 * This function encodes an NFC NDEF message into a buffer.
 *
 * @param[in]       uri_id_code         URI identifier code that defines the protocol field of the URI.
 * @param[in]       p_uri_data          Pointer to the URI string.
 *                                      The string should not contain the protocol field if the protocol
 *                                      was specified in @p uri_id_code.
 * @param[in]       uri_data_len        Length of the URI string.
 * @param[out]      p_buf               Pointer to the buffer for the message.
 * @param[in,out]   p_len               Size of the available memory for the message as input.
 *                                      Size of the generated message as output.
 *
 * @retval NRF_SUCCESS                  If the description was successfully created.
 * @retval NRF_ERROR_INVALID_PARAM      If the URI string was invalid (equal to NULL).
 * @retval NRF_ERROR_NO_MEM             If the predicted message size is bigger than the provided
 *                                      buffer space.
 * @retval Other                        Other codes might be returned depending on
 *                                      the function @ref nfc_ndef_msg_encode.
 */
ret_code_t nfc_uri_msg_encode( nfc_uri_id_t           uri_id_code,
                               uint8_t const *  const p_uri_data,
                               uint8_t                uri_data_len,
                               uint8_t       *        p_buf,
                               uint32_t      *        p_len);

/**
 * @}
 */
#endif // NFC_URI_MSG_H__
