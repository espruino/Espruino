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

#include "nfc_ep_oob_rec.h"
#include "sdk_errors.h"
#include "ble_gap.h"
#include "app_util.h"

/* NFC OOB EP definitions */
#define NFC_EP_OOB_REC_GAP_ADDR_LEN          BLE_GAP_ADDR_LEN
#define NFC_EP_OOB_REC_OOB_DATA_LEN_SIZE     2UL
#define NFC_EP_OOB_REC_PAYLOAD_PREFIX_LEN    (NFC_EP_OOB_REC_GAP_ADDR_LEN + \
                                             NFC_EP_OOB_REC_OOB_DATA_LEN_SIZE)

/* Record Payload Type for Bluetooth Carrier Configuration EP record */
static const uint8_t ep_oob_rec_type_field[] =
{
    'a', 'p', 'p', 'l', 'i', 'c', 'a', 't', 'i', 'o', 'n', '/', 'v', 'n', 'd', '.',
    'b', 'l', 'u', 'e', 't', 'o', 'o', 't', 'h', '.', 'e', 'p', '.', 'o', 'o', 'b'
};

/**
 * @brief Function for validating AD structure content for a Bluetooth Carrier Configuration EP record.
 *
 * This function validates AD structure content. LE Bluetooth Device Address and LE Role
 * fields must not be included. Security Manager OOB Flags structure is required.
 *
 * @param[in]       p_ble_advdata   Pointer to the description of the payload.
 *
 * @retval NRF_SUCCESS                     If the validation was successful.
 * @retval NRF_ERROR_INVALID_PARAM         Otherwise.
 */
static ret_code_t nfc_ep_oob_adv_data_check(ble_advdata_t const *  const p_ble_advdata)
{
    if((true == p_ble_advdata->include_ble_device_addr)         ||
       (BLE_ADVDATA_ROLE_NOT_PRESENT != p_ble_advdata->le_role) ||
       (NULL == p_ble_advdata->p_sec_mgr_oob_flags))
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
 * @brief Function for encoding device address to Bluetooth Carrier Configuration EP record.
 *
 * This fuction is used to encode device address to Bluetooth Carrier Configuration EP record.
 *
 * @param[in]       p_encoded_data      Pointer to the buffer where encoded data will be returned.
 * @param[in]       max_len             Available memory in the buffer.
 *
 * @retval NRF_SUCCESS                  If the encoding was successful.
 * @retval NRF_ERROR_NO_MEM             If available memory was not enough.
 * @retval NRF_ERROR_xxx                If any other error occured.
 */
static ret_code_t nfc_ep_oob_bluetooth_device_address_encode(uint8_t  * const p_encoded_data,
                                                             uint16_t         max_len)
{
    ret_code_t      err_code = NRF_SUCCESS;
    ble_gap_addr_t  device_address;

    if (NFC_EP_OOB_REC_GAP_ADDR_LEN > max_len)
    {
        return NRF_ERROR_NO_MEM;
    }

    /* Get BLE address */
    err_code = sd_ble_gap_address_get(&device_address);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    /* Encode Bluetooth EP device address */
    memcpy(p_encoded_data, device_address.addr, NFC_EP_OOB_REC_GAP_ADDR_LEN);

    return NRF_SUCCESS;
}

/**
 * @brief Function for constructing the payload for a Bluetooth Carrier Configuration EP record.
 *
 * This function encodes the record payload according to the BLE AD structure. It implements
 * an API compatible with @ref p_payload_constructor_t.
 *
 * @param[in]       p_ble_advdata   Pointer to the description of the payload.
 * @param[out]      p_buff          Pointer to payload destination.
 *
 * @param[in,out]   p_len           Size of available memory to write as input. Size of generated
 *                                  payload as output.
 *
 * @retval NRF_SUCCESS          If the record payload was encoded successfully.
 * @retval NRF_ERROR_NO_MEM     If available memory was not enough for record payload to be encoded.
 * @retval Other                If any other error occurred during record payload encoding.
 */
static ret_code_t nfc_ep_oob_payload_constructor(ble_advdata_t * p_ble_advdata, 
                                                 uint8_t       * p_buff, 
                                                 uint32_t      * p_len)
{
    ret_code_t  err_code = NRF_SUCCESS;
    uint8_t   * p_ad_data;
    uint16_t    payload_len, ad_data_len;

    /* Check correctness of the configuration structure */
    err_code = nfc_ep_oob_adv_data_check(p_ble_advdata);
    if(NRF_SUCCESS != err_code)
    {
        return err_code;
    }

    /* Validate if there is enough memory for OOB payload length field and BLE device address */
    if(NFC_EP_OOB_REC_PAYLOAD_PREFIX_LEN > *p_len)
    {
        return NRF_ERROR_NO_MEM;
    }

    /* Set proper memory offset in payload buffer for AD structure and count available memory.
     * Bluetooth EP device address and OOB payload length field must be inserted before the AD payload */
    p_ad_data   = (uint8_t *) (p_buff + NFC_EP_OOB_REC_PAYLOAD_PREFIX_LEN);
    ad_data_len = *p_len - NFC_EP_OOB_REC_PAYLOAD_PREFIX_LEN;
    if( *p_len - NFC_EP_OOB_REC_PAYLOAD_PREFIX_LEN > UINT16_MAX )
    {
        ad_data_len = UINT16_MAX;
    }

    /* Encode AD structures into NFC record payload */
    err_code = adv_data_encode(p_ble_advdata, p_ad_data, &ad_data_len);
    if(NRF_SUCCESS != err_code)
    {
        return err_code;
    }

    /* Now as the final payload length is known OOB payload length field, and Bluetooth device
     * address can be encoded */
    payload_len  = ad_data_len + NFC_EP_OOB_REC_PAYLOAD_PREFIX_LEN;
    p_buff      += uint16_encode(payload_len, p_buff);
    err_code     = nfc_ep_oob_bluetooth_device_address_encode(p_buff, p_ad_data - p_buff);
    if(NRF_SUCCESS != err_code)
    {
        return err_code;
    }

    /* Update total payload length */
    *p_len = payload_len;

    return err_code;
}


nfc_ndef_record_desc_t * nfc_ep_oob_rec_declare(uint8_t                        rec_payload_id,
                                                ble_advdata_t    const * const p_ble_advdata)
{
    static uint8_t payload_id = 0;
        
    NFC_NDEF_GENERIC_RECORD_DESC_DEF( nfc_ep_oob_rec,
                                      TNF_MEDIA_TYPE,
                                      &payload_id,   // memory for possible ID value
                                      0,             // no ID by default
                                      (ep_oob_rec_type_field),
                                      sizeof(ep_oob_rec_type_field),
                                      nfc_ep_oob_payload_constructor,
                                      NULL);

    nfc_ndef_record_desc_t * nfc_ep_oob_rec = &NFC_NDEF_GENERIC_RECORD_DESC( nfc_ep_oob_rec);

    /* Update record descriptor */
    nfc_ep_oob_rec->p_payload_descriptor = (void *) p_ble_advdata;

    /* Handle record ID configuration */
    payload_id                = rec_payload_id;
    nfc_ep_oob_rec->id_length = (rec_payload_id != 0) ? 1 : 0;

    return nfc_ep_oob_rec;
}
