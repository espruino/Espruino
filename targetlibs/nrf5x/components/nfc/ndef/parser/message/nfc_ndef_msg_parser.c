
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

#include "sdk_config.h"
#if NFC_NDEF_MSG_PARSER_ENABLED

#include "nfc_ndef_msg_parser.h"
#include "nrf_delay.h"

#define NRF_LOG_MODULE_NAME "NFC_NDEF_MSG_PARSER"
#if NFC_NDEF_MSG_PARSER_LOG_ENABLED
#define NRF_LOG_LEVEL       NFC_NDEF_MSG_PARSER_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  NFC_NDEF_MSG_PARSER_INFO_COLOR
#else // NFC_NDEF_MSG_PARSER_LOG_ENABLED
#define NRF_LOG_LEVEL       0
#endif // NFC_NDEF_MSG_PARSER_LOG_ENABLED
#include "nrf_log.h"

ret_code_t ndef_msg_parser(uint8_t  * const p_result_buf,
                           uint32_t * const p_result_buf_len,
                           uint8_t  * const p_nfc_data,
                           uint32_t * const p_nfc_data_len)
{
    ret_code_t                  ret_code;
    nfc_ndef_parser_memo_desc_t parser_memory_helper;

    ret_code = ndef_parser_memo_resolve(p_result_buf,
                                        p_result_buf_len,
                                        &parser_memory_helper);

    if (ret_code != NRF_SUCCESS)
    {
        return ret_code;
    }

    ret_code = internal_ndef_msg_parser(&parser_memory_helper,
                                        p_nfc_data,
                                        p_nfc_data_len);

    return ret_code;
}


void ndef_msg_printout(nfc_ndef_msg_desc_t * const p_msg_desc)
{
    uint32_t i;

    nrf_delay_ms(100);
    NRF_LOG_INFO("NDEF message contains %d record(s)\r\n\r\n", p_msg_desc->record_count);

    for (i = 0; i < p_msg_desc->record_count; i++)
    {
        ndef_record_printout(i, p_msg_desc->pp_record[i]);
    }
}

#endif // NFC_NDEF_MSG_PARSER_ENABLED
