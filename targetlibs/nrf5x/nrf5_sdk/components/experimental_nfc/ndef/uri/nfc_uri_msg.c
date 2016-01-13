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

#include <string.h>
#include "nfc_uri_msg.h"

/** @brief Function for generating a description of an NFC NDEF URI message.
 *
 * This function declares and initializes a static instance of an NFC NDEF message description
 * and NFC NDEF records descriptions.
 *
 * @param[in]  uri_id_code          URI identifier code that defines the protocol field of the URI.
 * @param[in]  p_uri_data           Pointer to the URI string.
 *                                  This string should not contain the protocol field if the protocol
 *                                  was specified in @p uri_id_code
 * @param[in]  uri_data_len         Length of the URI string.
 * @param[out] pp_uri_msg_desc      Pointer to pointer to the NDEF message description.
 *
 * @retval NRF_SUCCESS              If the description was successfully created.
 * @retval NRF_ERROR_INVALID_PARAM  If the URI string was invalid (equal to NULL).
 */
static ret_code_t nfc_uri_msg_declare( nfc_uri_id_t           uri_id_code,
                                       uint8_t const *  const p_uri_data,
                                       uint8_t                uri_data_len,
                                       nfc_ndef_msg_desc_t ** pp_uri_msg_desc)
{
    ret_code_t               err_code;
    nfc_ndef_record_desc_t * p_uri_rec;

    /* Create NFC NDEF message description, capacity - 1 record */
    NFC_NDEF_MSG_DEF(nfc_uri_msg, 1);
    
    /* The message description is static, therefore */
    /* you must clear the message (needed for supporting multiple calls) */
    nfc_ndef_msg_clear(&NFC_NDEF_MSG(nfc_uri_msg));

    if(p_uri_data != NULL)
    {
        /* Create NFC NDEF URI Record description */
        p_uri_rec = nfc_uri_rec_declare(uri_id_code,
                                        p_uri_data,
                                        uri_data_len);

        /* Add URI record as lone record to message */
        err_code = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_uri_msg), p_uri_rec);

        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    *pp_uri_msg_desc = &NFC_NDEF_MSG(nfc_uri_msg);

    return NRF_SUCCESS;
}

ret_code_t nfc_uri_msg_encode( nfc_uri_id_t           uri_id_code,
                               uint8_t const *  const p_uri_data,
                               uint8_t                uri_data_len,
                               uint8_t       *        p_buf,
                               uint32_t      *        p_len)
{
    ret_code_t err_code;
    nfc_ndef_msg_desc_t * p_uri_msg_desc;

    /* Create NFC NDEF message description with URI record */
    err_code = nfc_uri_msg_declare( uri_id_code,
                                    p_uri_data,
                                    uri_data_len,
                                    &p_uri_msg_desc);

    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    /* Encode whole message into buffer */
    err_code = nfc_ndef_msg_encode(p_uri_msg_desc,
                                   p_buf,
                                   p_len);

    return err_code;
}
