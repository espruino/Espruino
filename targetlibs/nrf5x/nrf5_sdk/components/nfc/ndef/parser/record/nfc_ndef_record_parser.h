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

#ifndef NFC_NDEF_RECORD_PARSER_H__
#define NFC_NDEF_RECORD_PARSER_H__

#include <stdint.h>
#include "sdk_errors.h"
#include "nfc_ndef_record.h"

/**@file
 *
 * @defgroup nfc_ndef_record_parser Parser for NDEF records
 * @{
 * @ingroup  nfc_ndef_parser
 *
 * @brief    Parser for NFC NDEF records.
 *
 */


/**
 * @brief Function for parsing NDEF records.
 *
 * This parsing implementation uses the binary payload descriptor (@ref nfc_ndef_bin_payload_desc_t) to describe the payload for the record.
 *
 * @param[out]    p_bin_pay_desc     Pointer to the binary payload descriptor that will be filled and referenced by the record descriptor.
 * @param[out]    p_rec_desc         Pointer to the record descriptor that will be filled with parsed data.
 * @param[out]    p_record_location  Pointer to the record location.
 * @param[in]     p_nfc_data         Pointer to the raw data to be parsed.
 * @param[in,out] p_nfc_data_len     As input: size of the NFC data in the @p p_nfc_data buffer. As output: size of the parsed record.
 *
 * @retval NRF_SUCCESS               If the function completed successfully.
 * @retval NRF_ERROR_INVALID_LENGTH  If the expected record length is bigger than the provided input data amount.
 */
ret_code_t ndef_record_parser(nfc_ndef_bin_payload_desc_t * p_bin_pay_desc,
                              nfc_ndef_record_desc_t      * p_rec_desc,
                              nfc_ndef_record_location_t  * p_record_location,
                              uint8_t const               * p_nfc_data,
                              uint32_t                    * p_nfc_data_len);

/**
 * @brief Function for printing the parsed contents of the NDEF record.
 *
 * @param[in] num        Sequence number of the record within the NDEF message.
 * @param[in] p_rec_desc Pointer to the descriptor of the record that should be printed.
 *
 */
void ndef_record_printout(uint32_t num, nfc_ndef_record_desc_t * const p_rec_desc);

/**
 * @}
 */

#endif // NFC_NDEF_RECORD_PARSER_H__
