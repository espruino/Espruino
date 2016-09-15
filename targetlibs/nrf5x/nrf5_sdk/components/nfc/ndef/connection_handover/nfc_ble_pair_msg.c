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

#include "nfc_ble_pair_msg.h"
#include "nfc_hs_rec.h"
#include "nfc_ac_rec.h"
#include "nfc_le_oob_rec.h"
#include "nfc_ep_oob_rec.h"
#include "nfc_ndef_msg.h"

/* Default value for Security Manager Out Of Band Flags field in BLE AD structure */
/* which is used for EP OOB Record payload */
static const uint8_t sec_mgr_oob_flags = 
    (AD_TYPE_SEC_MGR_OOB_FLAG_SET   << AD_TYPE_SEC_MGR_OOB_FLAG_OOB_DATA_PRESENT_POS) |
    (AD_TYPE_SEC_MGR_OOB_FLAG_SET   << AD_TYPE_SEC_MGR_OOB_FLAG_OOB_LE_SUPPORTED_POS) |
    (AD_TYPE_SEC_MGR_OOB_FLAG_CLEAR << AD_TYPE_SEC_MGR_OOB_FLAG_SIM_LE_AND_EP_POS)    |
    (AD_TYPE_SEC_MGR_OOB_ADDRESS_TYPE_RANDOM << AD_TYPE_SEC_MGR_OOB_FLAG_ADDRESS_TYPE_POS);

/** @brief Function for generating a description of a simplified LE OOB message according to the BLE 
 *         AD structure.
 *
 * This function declares and initializes a static instance of a simplified LE OOB message
 * with Bluetooth Carrier Configuration LE record. Payload of this record can be configured
 * via AD structure.
 *
 * @param[in]   p_le_advdata          Pointer to the AD for LE OOB record.
 * @param[out]  pp_le_oob_msg_desc    Pointer to pointer to the NDEF message instance.
 *
 * @retval      NRF_SUCCESS           If the function completed successfully.
 * @retval      NRF_ERROR_xxx         If an error occurred.
 */
static ret_code_t nfc_ble_simplified_le_oob_msg_declare( ble_advdata_t       const * const p_le_advdata,
                                                         nfc_ndef_msg_desc_t      **       pp_le_oob_msg_desc)
{
    ret_code_t err_code;
    nfc_ndef_record_desc_t * p_nfc_le_oob_record;

    /* Create NFC NDEF message description, capacity - 1 record */
    NFC_NDEF_MSG_DEF(nfc_le_oob_msg, 1);
    
    /* The message description is static, therefore */
    /* you must clear the message (needed for supporting multiple calls) */
    nfc_ndef_msg_clear(&NFC_NDEF_MSG(nfc_le_oob_msg));

    if(p_le_advdata != NULL)
    {
        /* Create NFC NDEF LE OOB Record description without record ID field */
        p_nfc_le_oob_record = nfc_le_oob_rec_declare(0 , p_le_advdata);

        /* Add LE OOB Record as lone record to message */
        err_code = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_le_oob_msg), p_nfc_le_oob_record);

        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    *pp_le_oob_msg_desc = &NFC_NDEF_MSG(nfc_le_oob_msg);

    return NRF_SUCCESS;
}

/** @brief Function for generating a description of a simplified EP OOB message according to the BLE 
 *         AD structure.
 *
 * This function declares and initializes a static instance of a simplified EP OOB message
 * with Bluetooth Carrier Configuration EP record. Payload of this record can be configured
 * via AD structure.
 *
 * @param[in]   p_ep_advdata          Pointer to the AD structure for EP OOB record.
 * @param[out]  pp_ep_oob_msg_desc    Pointer to pointer to the NDEF message instance.
 *
 * @retval      NRF_SUCCESS           If the function completed successfully.
 * @retval      NRF_ERROR_xxx         If an error occurred.
 */
static ret_code_t nfc_ble_simplified_ep_oob_msg_declare( ble_advdata_t       const * const p_ep_advdata,
                                                         nfc_ndef_msg_desc_t      **       pp_ep_oob_msg_desc)
{
    ret_code_t err_code;
    nfc_ndef_record_desc_t * p_nfc_ep_oob_record;

    /* Create NFC NDEF message description, capacity - 1 record */
    NFC_NDEF_MSG_DEF(nfc_ep_oob_msg, 1);
    
    /* The message description is static, therefore */
    /* you must clear the message (needed for supporting multiple calls) */
    nfc_ndef_msg_clear(&NFC_NDEF_MSG(nfc_ep_oob_msg));

    if(p_ep_advdata != NULL)
    {
        /* Create NFC NDEF EP OOB Record description without record ID field */
        p_nfc_ep_oob_record = nfc_ep_oob_rec_declare(0 , p_ep_advdata);

        /* Add EP OOB Record as lone record to message */
        err_code = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_ep_oob_msg), p_nfc_ep_oob_record);

        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    *pp_ep_oob_msg_desc = &NFC_NDEF_MSG(nfc_ep_oob_msg);

    return NRF_SUCCESS;
}

/** @brief Function for generating a description of a Handover Select NDEF message according to
 *         the BLE AD structures.
 *
 * This function declares and initializes a static instance of an NFC NDEF message description
 * of a Handover Select NDEF message with a Hs record and two OOB records (LE and EP with
 * modifications for Windows). Payload of these records can be configured via AD structures.
 *
 * @warning The order of LE and EP records cannot be changed. Android devices are able to pair
 *          correctly only when the LE record appears before the EP record.
 *
 * @param[in]   p_le_advdata       Pointer to the AD structure for LE OOB record.
 * @param[in]   p_ep_advdata       Pointer to the AD structure for EP OOB record.
 * @param[out]  pp_bt_oob_full_msg Pointer to a pointer to the NDEF message instance.
 *
 * @retval NRF_SUCCESS     If the function completed successfully.
 * @retval NRF_ERROR_xxx   If an error occurred.
 */
static ret_code_t nfc_ble_full_handover_select_msg_declare( ble_advdata_t    const * const p_le_advdata,
                                                            ble_advdata_t    const * const p_ep_advdata,
                                                            nfc_ndef_msg_desc_t   **       pp_bt_oob_full_msg)
{
    ret_code_t err_code = NRF_SUCCESS;
    
    // Carrier reference buffers for ac records.
    static uint8_t carrier_le_reference = '0';
    static uint8_t carrier_ep_reference = '1';
    
    // Create ac records for both message types.
    NFC_NDEF_AC_RECORD_DESC_DEF(ac_rec_le, NFC_AC_CPS_ACTIVE, 1, &carrier_le_reference, 1);
    NFC_NDEF_AC_RECORD_DESC_DEF(ac_rec_ep, NFC_AC_CPS_ACTIVE, 1, &carrier_ep_reference, 1);
    
    // Create a Hs record and assign existing ac records to it.
    NFC_NDEF_HS_RECORD_DESC_DEF(hs_rec, 1, 3, 2);

    nfc_ndef_record_desc_t * p_nfc_hs_record = &NFC_NDEF_HS_RECORD_DESC(hs_rec);

    // Clear the record before assigning local records to it (in case this function has already been called).
    nfc_hs_rec_local_record_clear(p_nfc_hs_record);

    err_code = nfc_hs_rec_local_record_add(p_nfc_hs_record, &NFC_NDEF_AC_RECORD_DESC(ac_rec_le));
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = nfc_hs_rec_local_record_add(p_nfc_hs_record, &NFC_NDEF_AC_RECORD_DESC(ac_rec_ep));
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Create le and ep records.
    nfc_ndef_record_desc_t * p_nfc_le_oob_record =
            nfc_le_oob_rec_declare(carrier_le_reference , p_le_advdata);

    nfc_ndef_record_desc_t * p_nfc_ep_oob_record =
            nfc_ep_oob_rec_declare(carrier_ep_reference , p_ep_advdata);
    
    // Create full NDEF Handover Select message for Connection Handover and assign Hs, le and ep records to it.
    NFC_NDEF_MSG_DEF(hs_full_msg, 3);

    // Clear the message before assigning records to it (in case this function has already been called).
    nfc_ndef_msg_clear(&NFC_NDEF_MSG(hs_full_msg));

    err_code = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(hs_full_msg), p_nfc_hs_record);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(hs_full_msg), p_nfc_le_oob_record);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(hs_full_msg), p_nfc_ep_oob_record);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    *pp_bt_oob_full_msg = &NFC_NDEF_MSG(hs_full_msg);
    
    return err_code;
}

/** @brief Function for creating an AD structure with common configuration for EP and LE OOB records.
 *
 * This function creates an AD structure and initializes its fields with default content. Only 
 * fields that are common for both EP and LE OOB records are filled.
 *
 * @param[in]       p_tk_value          Pointer to the authentication Temporary Key (TK). If NULL,
 *                                      TK field of the returned AD structure is empty.
 * @param[out]      p_adv_data          Pointer to BLE AD structure with common configuration for EP and 
 *                                      LE OOB records.
 */
static void common_adv_data_create( ble_advdata_tk_value_t * const p_tk_value,
                                    ble_advdata_t          * const p_adv_data)
{
    memset((uint8_t *) p_adv_data, 0, sizeof(ble_advdata_t));

    /* Set common configuration of AD structure for both Bluetooth EP and LE record */
    p_adv_data->include_appearance = true;
    p_adv_data->name_type          = BLE_ADVDATA_FULL_NAME;
    p_adv_data->p_tk_value         = NULL;
    if(p_tk_value != NULL)
    {
        p_adv_data->p_tk_value = p_tk_value;
    }
}

/** @brief Function for creating an AD structure with default configuration for an LE OOB record.
 *
 * This function creates an AD structure and initializes its fields with default content for
 * LE OOB record payload.
 *
 * @param[in]       p_tk_value          Pointer to the authentication Temporary Key (TK). If NULL,
 *                                      TK field of the returned AD structure is empty.
 * @param[out]      p_le_adv_data       Pointer to BLE AD structure with default configuration 
 *                                      for LE OOB record.
 */
static void le_oob_specific_adv_data_create( ble_advdata_tk_value_t * const p_tk_value,
                                             ble_advdata_t          * const p_le_adv_data)
{
    /* Create default configuration which is common for both EP and LE OOB Records */
    common_adv_data_create(p_tk_value, p_le_adv_data);

    /* LE specific configuration */
    p_le_adv_data->include_ble_device_addr = true;
    p_le_adv_data->le_role                 = BLE_ADVDATA_ROLE_ONLY_PERIPH;
    p_le_adv_data->flags                   = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
}

/** @brief Function for creating an AD structure with default configuration for an EP OOB record.
 *
 * This function creates an AD structure and initializes its fields with default content for
 * EP OOB record payload.
 *
 * @param[in]       p_tk_value          Pointer to the authentication Temporary Key (TK). If NULL,
 *                                      TK field of the returned AD structure is empty.
 * @param[out]      p_ep_adv_data       Pointer to BLE AD structure with default configuration
 *                                      for EP OOB record.
 */
static void ep_oob_specific_adv_data_create( ble_advdata_tk_value_t * const p_tk_value,
                                             ble_advdata_t          * const p_ep_adv_data)
{
    /* Create default configuration which is common for both EP and LE OOB Records */
    common_adv_data_create(p_tk_value, p_ep_adv_data);

    /* EP specific configuration */
    p_ep_adv_data->p_sec_mgr_oob_flags = (uint8_t *) &sec_mgr_oob_flags;
}

ret_code_t nfc_ble_simplified_le_oob_msg_encode( ble_advdata_t     const * const p_le_advdata,
                                                 uint8_t                 *       p_buf,
                                                 uint32_t                *       p_len)
{
    nfc_ndef_msg_desc_t * p_le_oob_msg_desc;
    ret_code_t            err_code;

    err_code = nfc_ble_simplified_le_oob_msg_declare(p_le_advdata, &p_le_oob_msg_desc);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    /* Encode whole message into buffer */
    err_code = nfc_ndef_msg_encode(p_le_oob_msg_desc,
                                   p_buf,
                                   p_len);

    return err_code;
}

ret_code_t nfc_ble_simplified_ep_oob_msg_encode( ble_advdata_t     const * const p_ep_advdata,
                                                 uint8_t                 *       p_buf,
                                                 uint32_t                *       p_len)
{
    nfc_ndef_msg_desc_t * p_ep_oob_msg_desc;
    ret_code_t            err_code;

    err_code = nfc_ble_simplified_ep_oob_msg_declare(p_ep_advdata, &p_ep_oob_msg_desc);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    /* Encode whole message into buffer */
    err_code = nfc_ndef_msg_encode(p_ep_oob_msg_desc,
                                   p_buf,
                                   p_len);

    return err_code;
}

ret_code_t nfc_ble_full_handover_select_msg_encode( ble_advdata_t    const * const p_le_advdata,
                                                    ble_advdata_t    const * const p_ep_advdata,
                                                    uint8_t                *       p_buf,
                                                    uint32_t               *       p_len)
{
    nfc_ndef_msg_desc_t * p_full_hs_msg_desc;
    ret_code_t            err_code;

    err_code = nfc_ble_full_handover_select_msg_declare(p_le_advdata,
                                                        p_ep_advdata,
                                                        &p_full_hs_msg_desc);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    /* Encode whole message into buffer */
    err_code = nfc_ndef_msg_encode(p_full_hs_msg_desc,
                                   p_buf,
                                   p_len);

    return err_code;
}

ret_code_t nfc_ble_pair_default_msg_encode( nfc_ble_pair_type_t            nfc_ble_pair_type,
                                            ble_advdata_tk_value_t * const p_tk_value,
                                            uint8_t                *       p_buf,
                                            uint32_t               *       p_len)
{
    ble_advdata_t le_adv_data;
    ble_advdata_t ep_adv_data;
    ret_code_t    err_code = NRF_SUCCESS;

    switch(nfc_ble_pair_type)
    {

        case NFC_BLE_PAIR_MSG_BLUETOOTH_LE_SHORT:
            le_oob_specific_adv_data_create(p_tk_value, &le_adv_data);
            err_code = nfc_ble_simplified_le_oob_msg_encode(&le_adv_data, p_buf, p_len);
            break;

        case NFC_BLE_PAIR_MSG_BLUETOOTH_EP_SHORT:
            ep_oob_specific_adv_data_create(p_tk_value, &ep_adv_data);
            err_code = nfc_ble_simplified_ep_oob_msg_encode(&ep_adv_data, p_buf, p_len);
            break;

        case NFC_BLE_PAIR_MSG_FULL:
            le_oob_specific_adv_data_create(p_tk_value, &le_adv_data);
            ep_oob_specific_adv_data_create(p_tk_value, &ep_adv_data);
            err_code = nfc_ble_full_handover_select_msg_encode(&le_adv_data,
                                                               &ep_adv_data,
                                                               p_buf,
                                                               p_len);
            break;

    }

    return err_code;
}
