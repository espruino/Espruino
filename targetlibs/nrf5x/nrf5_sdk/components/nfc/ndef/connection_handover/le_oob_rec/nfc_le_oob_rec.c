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

#include "nfc_le_oob_rec.h"
#include "sdk_errors.h"
#include "ble_gap.h"

/* Record Payload Type for Bluetooth Carrier Configuration LE record */
static const uint8_t le_oob_rec_type_field[] =
{
    'a', 'p', 'p', 'l', 'i', 'c', 'a', 't', 'i', 'o', 'n', '/', 'v', 'n', 'd', '.',
    'b', 'l', 'u', 'e', 't', 'o', 'o', 't', 'h', '.', 'l', 'e', '.', 'o', 'o', 'b'
};

/**
 * @brief Function for validating AD structure content for a Bluetooth Carrier Configuration LE record.
 *
 * This function validates AD structure content. LE Bluetooth Device Address and LE Role
 * fields are required. Security Manager Out Of Band Flags structure must not be included.
 *
 * @param[in]       p_ble_advdata   Pointer to the description of the payload.
 *
 * @retval NRF_SUCCESS                     If the validation was successful.
 * @retval NRF_ERROR_INVALID_PARAM         Otherwise.
 */
static ret_code_t nfc_le_oob_adv_data_check(ble_advdata_t const *  const p_ble_advdata)
{
    if((false == p_ble_advdata->include_ble_device_addr)        ||
       (BLE_ADVDATA_ROLE_NOT_PRESENT == p_ble_advdata->le_role) ||
       (NULL != p_ble_advdata->p_sec_mgr_oob_flags))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    /* If Flags field in AD structure is present, the BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED flag 
       must be set. */
    if ((0 != p_ble_advdata->flags) &&
            ((p_ble_advdata->flags & BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED) == 0))
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    return NRF_SUCCESS;
}

/**
 * @brief Function for constructing the payload for a Bluetooth Carrier Configuration LE record.
 *
 * This function encodes the record payload according to the BLE AD structure. It implements
 * an API compatible with @ref p_payload_constructor_t
 *
 * @param[in]       p_ble_advdata   Pointer to the description of the payload.
 * @param[out]      p_buff          Pointer to payload destination.
 *
 * @param[in,out]   p_len           Size of available memory to write as input. Size of generated
 *                                  payload as output.
 *
 * @retval NRF_SUCCESS   If the record payload was encoded successfully.
 * @retval Other         If the record payload encoding failed.
 */
static ret_code_t nfc_le_oob_payload_constructor(ble_advdata_t * p_ble_advdata, 
                                                 uint8_t       * p_buff, 
                                                 uint32_t      * p_len)
{
    ret_code_t err_code = NRF_SUCCESS;

    /* Check correctness of the configuration structure */
    err_code = nfc_le_oob_adv_data_check(p_ble_advdata);
    if(NRF_SUCCESS != err_code)
    {
        return err_code;
    }

    /* Encode AD structures into NFC record payload */
    uint16_t buff_len = *p_len;
    if(*p_len > UINT16_MAX)
    {
        buff_len = UINT16_MAX;
    }
    err_code = adv_data_encode(p_ble_advdata, p_buff, &buff_len);

    /* Update total payload length */
    *p_len = (uint32_t) buff_len;
    
    return err_code;
}

nfc_ndef_record_desc_t * nfc_le_oob_rec_declare(uint8_t                        rec_payload_id,
                                                ble_advdata_t    const * const p_ble_advdata)
{
    static uint8_t payload_id = 0;
        
    NFC_NDEF_GENERIC_RECORD_DESC_DEF( nfc_le_oob_rec,
                                      TNF_MEDIA_TYPE,
                                      &payload_id,   // memory for possible ID value
                                      0,             // no ID by default
                                      (le_oob_rec_type_field),
                                      sizeof(le_oob_rec_type_field),
                                      nfc_le_oob_payload_constructor,
                                      NULL);

    nfc_ndef_record_desc_t * nfc_le_oob_rec = &NFC_NDEF_GENERIC_RECORD_DESC( nfc_le_oob_rec);

    /* Update record descriptor */
    nfc_le_oob_rec->p_payload_descriptor = (void *) p_ble_advdata;

    /* Handle record ID configuration */
    payload_id                = rec_payload_id;
    nfc_le_oob_rec->id_length = (rec_payload_id != 0) ? 1 : 0;

    return nfc_le_oob_rec;
}
