/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

#include "nfc_ndef_msg_parser_local.h"


ret_code_t internal_ndef_msg_parser(nfc_ndef_parser_memo_desc_t * const p_parser_memo_desc,
                                    uint8_t                     const * p_nfc_data,
                                    uint32_t                    * const p_nfc_data_len)
{
    nfc_ndef_record_location_t record_location;

    ret_code_t ret_code;

    uint32_t nfc_data_left     = *p_nfc_data_len;
    uint32_t temp_nfc_data_len = 0;

    // want to modify -> use local copy
    nfc_ndef_bin_payload_desc_t * p_bin_pay_desc = p_parser_memo_desc->p_bin_pay_desc;
    nfc_ndef_record_desc_t      * p_rec_desc     = p_parser_memo_desc->p_rec_desc;


    while (nfc_data_left > 0)
    {
        temp_nfc_data_len = nfc_data_left;

        ret_code = ndef_record_parser(p_bin_pay_desc,
                                      p_rec_desc,
                                      &record_location,
                                      p_nfc_data,
                                      &temp_nfc_data_len);

        if (ret_code != NRF_SUCCESS)
        {
            return ret_code;
        }

        // verify the records location flags
        if (p_parser_memo_desc->p_msg_desc->record_count == 0)
        {
            if ((record_location != NDEF_FIRST_RECORD) && (record_location != NDEF_LONE_RECORD))
            {
                return NRF_ERROR_INVALID_DATA;
            }
        }
        else
        {
            if ((record_location != NDEF_MIDDLE_RECORD) && (record_location != NDEF_LAST_RECORD))
            {
                return NRF_ERROR_INVALID_DATA;
            }
        }

        ret_code = nfc_ndef_msg_record_add(p_parser_memo_desc->p_msg_desc, p_rec_desc);

        if (ret_code != NRF_SUCCESS)
        {
            return ret_code;
        }

        nfc_data_left -= temp_nfc_data_len;

        if ((record_location == NDEF_LAST_RECORD) || (record_location == NDEF_LONE_RECORD))
        {
            *p_nfc_data_len = *p_nfc_data_len - nfc_data_left;
            return NRF_SUCCESS;
        }
        else
        {
            if (p_parser_memo_desc->p_msg_desc->record_count ==
                p_parser_memo_desc->p_msg_desc->max_record_count)
            {
                return NRF_ERROR_NO_MEM;
            }

            p_nfc_data += temp_nfc_data_len;
            p_bin_pay_desc++;
            p_rec_desc++;
        }
    }

    return NRF_ERROR_INVALID_DATA;

}


ret_code_t ndef_parser_memo_resolve(uint8_t                     * const p_result_buf,
                                    uint32_t                    * const p_result_buf_len,
                                    nfc_ndef_parser_memo_desc_t * const p_parser_memo_desc)
{

    uint32_t                   max_rec_num;
    uint32_t                   memory_last;
    uint8_t                  * p_end;
    nfc_ndef_record_desc_t * * pp_record_desc_array;

    if (*p_result_buf_len < sizeof(parsed_ndef_msg_1_t))
    {
        return NRF_ERROR_NO_MEM;
    }

    memory_last = (*p_result_buf_len) - sizeof(parsed_ndef_msg_1_t);
    max_rec_num = (memory_last / (NFC_PARSER_M_DELTA)) + 1;

    p_parser_memo_desc->p_msg_desc = (nfc_ndef_msg_desc_t *) p_result_buf;
    pp_record_desc_array           =
        (nfc_ndef_record_desc_t * *) &p_parser_memo_desc->p_msg_desc[1];
    p_parser_memo_desc->p_bin_pay_desc =
        (nfc_ndef_bin_payload_desc_t *) &pp_record_desc_array[max_rec_num];
    p_parser_memo_desc->p_rec_desc =
        (nfc_ndef_record_desc_t *) &p_parser_memo_desc->p_bin_pay_desc[max_rec_num];

    // initialize message description
    p_parser_memo_desc->p_msg_desc->pp_record        = pp_record_desc_array;
    p_parser_memo_desc->p_msg_desc->max_record_count = max_rec_num;
    p_parser_memo_desc->p_msg_desc->record_count     = 0;

    p_end = (uint8_t *) &p_parser_memo_desc->p_rec_desc[max_rec_num];

    *p_result_buf_len = p_end - p_result_buf;

    return NRF_SUCCESS;
}


