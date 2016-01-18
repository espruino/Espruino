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

#include "nfc_ndef_msg.h"
#include "nrf.h"

/**
 * @brief Resolve the value of record location flags of the NFC NDEF record within a NFC NDEF message.
 */
__STATIC_INLINE nfc_ndef_record_location_t record_location_get(uint32_t index,
                                                               uint32_t record_count)
{
    nfc_ndef_record_location_t record_location;

    if (index == 0)
    {
        if (record_count == 1)
        {
            record_location = NDEF_LONE_RECORD;
        }
        else
        {
            record_location = NDEF_FIRST_RECORD;
        }
    }
    else if (record_count == index + 1)
    {
        record_location = NDEF_LAST_RECORD;
    }
    else
    {
        record_location = NDEF_MIDDLE_RECORD;
    }

    return record_location;
}


ret_code_t nfc_ndef_msg_encode(nfc_ndef_msg_desc_t const * p_ndef_msg_desc,
                               uint8_t                   * p_msg_buffer,
                               uint32_t * const            p_msg_len)
{
    nfc_ndef_record_location_t record_location;
    uint32_t                   temp_len;
    uint32_t                   i;
    uint32_t                   err_code;

    uint32_t sum_of_len = 0;

    nfc_ndef_record_desc_t * * pp_record_rec_desc = p_ndef_msg_desc->pp_record;

    for (i = 0; i < p_ndef_msg_desc->record_count; i++)
    {
        record_location = record_location_get(i, p_ndef_msg_desc->record_count);

        temp_len = *p_msg_len - sum_of_len;

        err_code = nfc_ndef_record_encode(*pp_record_rec_desc,
                                          record_location,
                                          p_msg_buffer,
                                          &temp_len);

        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }

        sum_of_len   += temp_len;
        p_msg_buffer += temp_len;

        /* next record */
        pp_record_rec_desc++;
    }

    *p_msg_len = sum_of_len;

    return NRF_SUCCESS;
}


void nfc_ndef_msg_clear(nfc_ndef_msg_desc_t * p_msg)
{
    p_msg->record_count = 0;
}


ret_code_t nfc_ndef_msg_record_add(nfc_ndef_msg_desc_t * const    p_msg,
                                   nfc_ndef_record_desc_t * const p_record)
{
    if (p_msg->record_count >= p_msg->max_record_count)
    {
        return NRF_ERROR_NO_MEM;
    }
    
    p_msg->pp_record[p_msg->record_count] = p_record;
    p_msg->record_count++;

    return NRF_SUCCESS;
}


