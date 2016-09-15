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

#include "nfc_ac_rec.h"
#include <string.h>
#include "nrf_error.h"
#include "nrf.h"

#define AC_REC_CPS_BYTE_SIZE            1 ///< Size of the field with CPS data.
#define AC_REC_DATA_REF_LEN_SIZE        1 ///< Size of the Data Reference Length field.
#define AC_REC_AUX_DATA_REF_COUNT_SIZE  1 ///< Size of the Data Reference Length field.

const uint8_t nfc_ac_rec_type_field[] = {'a', 'c'}; ///< Alternative Carrier Record type.

/**
 * @brief Function for calculating the payload length of the NFC NDEF Alternative Carrier record.
 */
static uint32_t nfc_ac_rec_payload_size_get(nfc_ac_rec_payload_desc_t const * p_ac_rec_payload_desc)
{
    int32_t i = 0;
    // Initialize with size of byte with CPS.
    uint32_t payload_size = AC_REC_CPS_BYTE_SIZE;
    
    // Add Carrier Data Reference size.
    payload_size +=  p_ac_rec_payload_desc->carrier_data_ref.length + AC_REC_DATA_REF_LEN_SIZE;

    // Add Auxiliary Data Reference Count size.
    payload_size += AC_REC_AUX_DATA_REF_COUNT_SIZE;

    for (i = 0; i < p_ac_rec_payload_desc->aux_data_ref_count; i++)
    {
        // Add Auxiliary Data Reference size.
        payload_size += p_ac_rec_payload_desc->p_aux_data_ref[i].length + AC_REC_DATA_REF_LEN_SIZE;
    }
   
    return payload_size;
}


ret_code_t nfc_ac_rec_payload_constructor(nfc_ac_rec_payload_desc_t * p_nfc_rec_ac_payload_desc,
                                          uint8_t                   * p_buff,
                                          uint32_t                  * p_len)
{
    int32_t  i = 0;
    uint32_t payload_size = nfc_ac_rec_payload_size_get(p_nfc_rec_ac_payload_desc);

    // Not enough space in the buffer, return an error.
    if (payload_size > *p_len)
    {
        return NRF_ERROR_NO_MEM;
    }

    // Invalid CPS value.
    if ( p_nfc_rec_ac_payload_desc->cps & ~NFC_AC_CPS_MASK )
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Copy CPS.
    *p_buff = p_nfc_rec_ac_payload_desc->cps;
    p_buff += AC_REC_CPS_BYTE_SIZE;

    // Copy Carrier Data Reference.
    *p_buff = p_nfc_rec_ac_payload_desc->carrier_data_ref.length;
    p_buff += AC_REC_DATA_REF_LEN_SIZE;

    memcpy( p_buff,
            p_nfc_rec_ac_payload_desc->carrier_data_ref.p_data,
            p_nfc_rec_ac_payload_desc->carrier_data_ref.length );
    p_buff += p_nfc_rec_ac_payload_desc->carrier_data_ref.length;

    // Copy Auxiliary Data Reference.
    *p_buff = p_nfc_rec_ac_payload_desc->aux_data_ref_count;
    p_buff += AC_REC_AUX_DATA_REF_COUNT_SIZE;

    for (i = 0; i < p_nfc_rec_ac_payload_desc->aux_data_ref_count; i++)
    {
        *p_buff = p_nfc_rec_ac_payload_desc->p_aux_data_ref[i].length;
        p_buff += AC_REC_DATA_REF_LEN_SIZE;

        memcpy( p_buff,
                p_nfc_rec_ac_payload_desc->p_aux_data_ref[i].p_data,
                p_nfc_rec_ac_payload_desc->p_aux_data_ref[i].length );
        p_buff += p_nfc_rec_ac_payload_desc->p_aux_data_ref[i].length;
    }

    // Assign payload size to the return buffer.
    *p_len = payload_size;
    
    return NRF_SUCCESS;
}


void nfc_ac_rec_auxiliary_data_ref_clear(nfc_ndef_record_desc_t * p_ac_rec)
{
    nfc_ac_rec_payload_desc_t * p_ac_rec_payload =
            (nfc_ac_rec_payload_desc_t*)p_ac_rec->p_payload_descriptor;

    p_ac_rec_payload->aux_data_ref_count = 0;
}


ret_code_t nfc_ac_rec_auxiliary_data_ref_add(nfc_ndef_record_desc_t * p_ac_rec,
                                             uint8_t                * p_aux_data,
                                             uint8_t                  aux_length)
{
    nfc_ac_rec_payload_desc_t * p_ac_rec_payload =
            (nfc_ac_rec_payload_desc_t *)p_ac_rec->p_payload_descriptor;

    if (p_ac_rec_payload->aux_data_ref_count >= p_ac_rec_payload->max_aux_data_ref)
    {
        return NRF_ERROR_NO_MEM;
    }
    
    p_ac_rec_payload->p_aux_data_ref[p_ac_rec_payload->aux_data_ref_count].p_data = p_aux_data;
    p_ac_rec_payload->p_aux_data_ref[p_ac_rec_payload->aux_data_ref_count].length = aux_length;
    p_ac_rec_payload->aux_data_ref_count++;

    return NRF_SUCCESS;
}
