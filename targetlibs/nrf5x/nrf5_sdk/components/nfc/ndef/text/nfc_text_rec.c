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
#include "nfc_text_rec.h"
#include "nrf_error.h"

#define TEXT_REC_STATUS_SIZE          1     ///< Size of the status.
#define TEXT_REC_STATUS_UTF_POS       7     ///< Position of a character encoding type.
#define TEXT_REC_RESERVED_POS         6     ///< Reserved position.

const uint8_t nfc_text_rec_type_field[] = {'T'};


/**
 * @brief Function for calculating payload size.
 */
__STATIC_INLINE uint32_t nfc_text_rec_payload_size_get(nfc_text_rec_payload_desc_t * p_nfc_rec_text_payload_desc)
{
    return (TEXT_REC_STATUS_SIZE
          + p_nfc_rec_text_payload_desc->lang_code_len
          + p_nfc_rec_text_payload_desc->data_len);
}

ret_code_t nfc_text_rec_payload_constructor(nfc_text_rec_payload_desc_t * p_nfc_rec_text_payload_desc,
                                            uint8_t                     * p_buff,
                                            uint32_t                    * p_len)
{
    if ((p_nfc_rec_text_payload_desc->lang_code_len == 0)
     || (p_nfc_rec_text_payload_desc->lang_code_len & (1 << TEXT_REC_RESERVED_POS))
     || (p_nfc_rec_text_payload_desc->lang_code_len & (1 << TEXT_REC_STATUS_UTF_POS))
     || (p_nfc_rec_text_payload_desc->p_lang_code   == NULL)
     || (p_nfc_rec_text_payload_desc->data_len      == 0)
     || (p_nfc_rec_text_payload_desc->p_data        == NULL)
     || (p_buff                                     == NULL)
     || (p_len                                      == NULL))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    uint32_t payload_size = nfc_text_rec_payload_size_get(p_nfc_rec_text_payload_desc);

    if (payload_size > *p_len)
    {
        return NRF_ERROR_NO_MEM;
    }

    *p_buff = (p_nfc_rec_text_payload_desc->lang_code_len
            + (p_nfc_rec_text_payload_desc->utf << TEXT_REC_STATUS_UTF_POS));
    p_buff += TEXT_REC_STATUS_SIZE;

    memcpy(p_buff,
           p_nfc_rec_text_payload_desc->p_lang_code,
           p_nfc_rec_text_payload_desc->lang_code_len);
    p_buff += p_nfc_rec_text_payload_desc->lang_code_len;

    memcpy(p_buff,
           p_nfc_rec_text_payload_desc->p_data,
           p_nfc_rec_text_payload_desc->data_len);
    *p_len = payload_size;

    return NRF_SUCCESS;
}


