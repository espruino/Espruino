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

#include "nfc_hs_rec.h"
#include "nfc_ac_rec.h"
#include "nrf_error.h"

#define HS_REC_VERSION_SIZE     1

const uint8_t nfc_hs_rec_type_field[] = {'H', 's'}; ///< Handover Select record type.


ret_code_t nfc_hs_rec_payload_constructor(nfc_hs_rec_payload_desc_t * p_nfc_hs_rec_payload_desc,
                                          uint8_t                   * p_buff,
                                          uint32_t                  * p_len)
{  
    ret_code_t err_code = NRF_SUCCESS;
    
    // There must be at least 1 free byte in buffer for version byte.
    if (*p_len < HS_REC_VERSION_SIZE)
    {    
        return NRF_ERROR_NO_MEM;
    }

    // Major/minor version byte.
    *p_buff = ( (p_nfc_hs_rec_payload_desc->major_version << 4) & 0xF0) |
              (  p_nfc_hs_rec_payload_desc->minor_version       & 0x0F);
    p_buff += HS_REC_VERSION_SIZE;
    
    // Decrement remaining buffer size.
    *p_len -= HS_REC_VERSION_SIZE;

    // Encode local records encapsulated in a message.
    err_code = nfc_ndef_msg_encode(p_nfc_hs_rec_payload_desc->p_local_records, p_buff, p_len);
    if (err_code!= NRF_SUCCESS)
    {
        return err_code;
    }

    // Add version byte to the total record size.
    *p_len += HS_REC_VERSION_SIZE;
        
    return NRF_SUCCESS;
}


void nfc_hs_rec_local_record_clear(nfc_ndef_record_desc_t * p_hs_rec)
{
    nfc_hs_rec_payload_desc_t* p_hs_payload =
            (nfc_hs_rec_payload_desc_t*)p_hs_rec->p_payload_descriptor;

    nfc_ndef_msg_clear(p_hs_payload->p_local_records);
}


ret_code_t nfc_hs_rec_local_record_add(nfc_ndef_record_desc_t * p_hs_rec,
                                       nfc_ndef_record_desc_t * p_local_rec)
{
    nfc_hs_rec_payload_desc_t* p_hs_payload =
            (nfc_hs_rec_payload_desc_t*)p_hs_rec->p_payload_descriptor;

    return nfc_ndef_msg_record_add(p_hs_payload->p_local_records, p_local_rec);
}
