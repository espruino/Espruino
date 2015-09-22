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
#include "nfc_ble_pair_msg_config.h"
#include "nrf_error.h"
#include "ble_gap.h"
#include "app_util.h"

/* Common NFC OOB definitions */
#define NFC_BLE_PAIR_MSG_BLE_DEVICE_ADDR_LEN            AD_TYPE_BLE_DEVICE_ADDR_SIZE
#define NFC_BLE_PAIR_MSG_LE_ROLE_LEN                    AD_TYPE_LE_ROLE_SIZE
#define NFC_BLE_PAIR_MSG_LOCAL_NAME_LEN                 (ADV_LENGTH_FIELD_SIZE  + \
                                                         ADV_AD_TYPE_FIELD_SIZE + \
                                                         NFC_BLE_PAIR_MSG_CONFIG_LOCAL_NAME_MAX_LEN)
#define NFC_BLE_PAIR_MSG_BLUETOOTH_EP_FLAGS_LEN         AD_TYPE_OOB_FLAGS_SIZE
#define NFC_BLE_PAIR_MSG_BLUETOOTH_EP_ADDR_LEN          BLE_GAP_ADDR_LEN
#define NFC_BLE_PAIR_MSG_BLUETOOTH_OOB_DATA_LEN_SIZE    2UL

#if NFC_BLE_PAIR_MSG_CONFIG_TK_VALUE_PRESENT
    #define NFC_BLE_PAIR_MSG_TK_VALUE_LEN               AD_TYPE_TK_VALUE_SIZE
#else
    #define NFC_BLE_PAIR_MSG_TK_VALUE_LEN               0UL
#endif

#if NFC_BLE_PAIR_MSG_CONFIG_APPEARANCE_PRESENT
    #define NFC_BLE_PAIR_MSG_APPEARANCE_LEN             AD_TYPE_APPEARANCE_SIZE
#else
    #define NFC_BLE_PAIR_MSG_APPEARANCE_LEN             0UL
#endif

#if NFC_BLE_PAIR_MSG_CONFIG_FLAGS_PRESENT
    #define NFC_BLE_PAIR_MSG_FLAGS_LEN                  AD_TYPE_FLAGS_SIZE
#else
    #define NFC_BLE_PAIR_MSG_FLAGS_LEN                  0UL
#endif


/* Specific NFC OOB definitions depending on the chosen NDEF message type */
/* Full Handover Select Message (Bluetooth LE) */
#if (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_LE_FULL)

#define NFC_BLE_PAIR_MSG_PAYLOAD_LEN           (NFC_BLE_PAIR_MSG_BLE_DEVICE_ADDR_LEN         + \
                                                NFC_BLE_PAIR_MSG_LE_ROLE_LEN                 + \
                                                NFC_BLE_PAIR_MSG_TK_VALUE_LEN                + \
                                                NFC_BLE_PAIR_MSG_APPEARANCE_LEN              + \
                                                NFC_BLE_PAIR_MSG_FLAGS_LEN                   + \
                                                NFC_BLE_PAIR_MSG_LOCAL_NAME_LEN)
#define NFC_BLE_PAIR_MSG_HEADER_LEN            52UL
#define NFC_BLE_PAIR_MSG_LEN                   (NFC_BLE_PAIR_MSG_HEADER_LEN + \
                                                NFC_BLE_PAIR_MSG_PAYLOAD_LEN)
#define NFC_BLE_PAIR_MSG_PAYLOAD_LEN_OFFSET    17UL

/* Full Handover Select Message (Bluetooth EP) */
#elif (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_EP_FULL)

#define NFC_BLE_PAIR_MSG_PAYLOAD_LEN           (NFC_BLE_PAIR_MSG_TK_VALUE_LEN                + \
                                                NFC_BLE_PAIR_MSG_APPEARANCE_LEN              + \
                                                NFC_BLE_PAIR_MSG_FLAGS_LEN                   + \
                                                NFC_BLE_PAIR_MSG_LOCAL_NAME_LEN              + \
                                                NFC_BLE_PAIR_MSG_BLUETOOTH_EP_FLAGS_LEN      + \
                                                NFC_BLE_PAIR_MSG_BLUETOOTH_EP_ADDR_LEN       + \
                                                NFC_BLE_PAIR_MSG_BLUETOOTH_OOB_DATA_LEN_SIZE)
#define NFC_BLE_PAIR_MSG_HEADER_LEN            52UL
#define NFC_BLE_PAIR_MSG_LEN                   (NFC_BLE_PAIR_MSG_HEADER_LEN + \
                                                NFC_BLE_PAIR_MSG_PAYLOAD_LEN)
#define NFC_BLE_PAIR_MSG_PAYLOAD_LEN_OFFSET    17UL

/* Simplified Tag format (Bluetooth LE) */
#elif (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_LE_SHORT)

#define NFC_BLE_PAIR_MSG_PAYLOAD_LEN           (NFC_BLE_PAIR_MSG_BLE_DEVICE_ADDR_LEN    + \
                                                NFC_BLE_PAIR_MSG_LE_ROLE_LEN            + \
                                                NFC_BLE_PAIR_MSG_TK_VALUE_LEN           + \
                                                NFC_BLE_PAIR_MSG_APPEARANCE_LEN         + \
                                                NFC_BLE_PAIR_MSG_FLAGS_LEN              + \
                                                NFC_BLE_PAIR_MSG_LOCAL_NAME_LEN)
#define NFC_BLE_PAIR_MSG_HEADER_LEN            35UL
#define NFC_BLE_PAIR_MSG_LEN                   (NFC_BLE_PAIR_MSG_HEADER_LEN + \
                                                NFC_BLE_PAIR_MSG_PAYLOAD_LEN)
#define NFC_BLE_PAIR_MSG_PAYLOAD_LEN_OFFSET    2UL

/* Simplified Tag format (Bluetooth EP) */
#elif (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_EP_SHORT)

#define NFC_BLE_PAIR_MSG_PAYLOAD_LEN           (NFC_BLE_PAIR_MSG_TK_VALUE_LEN                + \
                                                NFC_BLE_PAIR_MSG_APPEARANCE_LEN              + \
                                                NFC_BLE_PAIR_MSG_FLAGS_LEN                   + \
                                                NFC_BLE_PAIR_MSG_LOCAL_NAME_LEN              + \
                                                NFC_BLE_PAIR_MSG_BLUETOOTH_EP_FLAGS_LEN      + \
                                                NFC_BLE_PAIR_MSG_BLUETOOTH_EP_ADDR_LEN       + \
                                                NFC_BLE_PAIR_MSG_BLUETOOTH_OOB_DATA_LEN_SIZE)
#define NFC_BLE_PAIR_MSG_HEADER_LEN            35UL
#define NFC_BLE_PAIR_MSG_LEN                   (NFC_BLE_PAIR_MSG_HEADER_LEN + \
                                                NFC_BLE_PAIR_MSG_PAYLOAD_LEN)
#define NFC_BLE_PAIR_MSG_PAYLOAD_LEN_OFFSET    2UL

#else
    #error "Error. Type of NFC NDEF message used for BLE pairing over NFC not specified."
#endif /* NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE */

#if (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_EP_FULL)
/* BR/EDR Handover Select Message
 * A Static Handover can be used in cases where the Handover Selector device is equipped with
 * an NFC Forum Tag only. Therefore, it cannot actively reply to a Handover Request Message.
 * A Handover Requester device detects this message during the NFC discovery phase and will then
 * be able to read data from the NFC Forum Tag. If the data that is read embodies a Handover Select
 * Message, the Handover Requester can use this information to choose one of the indicated
 * alternative carriers and try to establish a secondary connection.
 * This corresponds to example message from section 4.2.1 of [NFC BTSSP]*/
static uint8_t ndef_msg[NFC_BLE_PAIR_MSG_LEN] =
{
    0x91,       //NDEF record header - TNF + Flags: MB=1b ME=0b CF=0b SR=1b IL=0b TNF=001b (Well-Known)
    0x02,       //NDEF record header - Record Type Length = 2 octets
    0x0A,       //NDEF record header - Payload Length = 10 octets (1 byte in size because SR=1b)
                //NDEF record header - ID Length missing since it is optional (IL=0b)
    0x48, 0x73, //NDEF record header - Record(Payload) Type = ‘Hs’ (Handover Select Record)
                //NDEF record header - Payload ID missing since it is optional (IL=0b)
    0x12,       //NDEF record payload - Connection Handover specification version = 1.2
                //NDEF record payload - "ac" Records:
    0xD1,           //NDEF record header - TNF + Flags: MB=1b ME=1b CF=0b SR=1b IL=0b TNF=001b (Well-Known)
    0x02,           //NDEF record header - Record Type Length = 2 octets
    0x04,           //NDEF record header - Payload Length = 4 octets (1 byte in size because SR=1b)
                    //NDEF record header - ID Length missing since it is optional (IL=0b)
    0x61, 0x63,     //NDEF record header - Record(Payload) Type = ‘ac’ (Alternative Carrier Record)
                    //NDEF record header - Payload ID missing since it is optional (IL=0b)
    0x03,           //NDEF record payload - Carrier Power State = 3 (unknown)
    0x01,           //NDEF record payload - Carrier Data Reference Length = 1 octet
    0x30,           //NDEF record payload - Carrier Data Reference = ‘0’
    0x00,           //NDEF record payload - Auxiliary Data Reference Count: 0
    0x5A,       //NDEF record header - TNF + Flags: MB=0b ME=1b CF=0b SR=1b IL=1b TNF=010b (MIME media-type)
    0x20,       //NDEF record header - Record Type Length = 32 octets
    0x00,       //MUST BE CHANGED! - NDEF record header - Payload Length = x octets (1 byte in size because SR=1b)
    0x01,       //NDEF record header - Payload ID Length = 1 octet
                //NDEF record header - Record(Payload) Type = ‘application/vnd.bluetooth.ep.oob’:
    0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74,
    0x69, 0x6F, 0x6E, 0x2F, 0x76, 0x6E, 0x64, 0x2E,
    0x62, 0x6C, 0x75, 0x65, 0x74, 0x6F, 0x6F, 0x74,
    0x68, 0x2E, 0x65, 0x70, 0x2E, 0x6F, 0x6F, 0x62,
    0x30        //NDEF record header - Payload ID = ‘0’
                //NDEF record payload - add here Bluetooth AD types
};

#elif (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_LE_FULL)
/* Bluetooth LE Handover Select Message
 * A Static Handover can be used in cases where the Handover Selector device is equipped with
 * an NFC Forum Tag only. Therefore, it cannot actively reply to a Handover Request Message.
 * A Handover Requester device detects this message during the NFC discovery phase and will then
 * be able to read data from the NFC Forum Tag. If the data that is read embodies a Handover Select
 * Message, the Handover Requester can use this information to choose one of the indicated
 * alternative carriers and try to establish a secondary connection.
 * This corresponds to example message from section 4.2.2 of [NFC BTSSP]*/
static uint8_t ndef_msg[NFC_BLE_PAIR_MSG_LEN] =
{
    0x91,       //NDEF record header - TNF + Flags: MB=1b ME=0b CF=0b SR=1b IL=0b TNF=001b (Well-Known)
    0x02,       //NDEF record header - Record Type Length = 2 octets
    0x0A,       //NDEF record header - Payload Length = 10 octets (1 byte in size because SR=1b)
                //NDEF record header - ID Length missing since it is optional (IL=0b)
    0x48, 0x73, //NDEF record header - Record(Payload) Type = ‘Hs’ (Handover Select Record)
                //NDEF record header - Payload ID missing since it is optional (IL=0b)
    0x12,       //NDEF record payload - Connection Handover specification version = 1.2
                //NDEF record payload - "ac" Records:
    0xD1,           //NDEF record header - TNF + Flags: MB=1b ME=1b CF=0b SR=1b IL=0b TNF=001b (Well-Known)
    0x02,           //NDEF record header - Record Type Length = 2 octets
    0x04,           //NDEF record header - Payload Length = 4 octets (1 byte in size because SR=1b)
                    //NDEF record header - ID Length missing since it is optional (IL=0b)
    0x61, 0x63,     //NDEF record header - Record(Payload) Type = ‘ac’ (Alternative Carrier Record)
                    //NDEF record header - Payload ID missing since it is optional (IL=0b)
    0x01,           //NDEF record payload - Carrier Power State = 1 (active)
    0x01,           //NDEF record payload - Carrier Data Reference Length = 1 octet
    0x30,           //NDEF record payload - Carrier Data Reference = ‘0’
    0x00,           //NDEF record payload - Auxiliary Data Reference Count: 0
    0x5A,       //NDEF record header - TNF + Flags: MB=0b ME=1b CF=0b SR=1b IL=1b TNF=010b (MIME media-type)
    0x20,       //NDEF record header - Record Type Length = 32 octets
    0x00,       //MUST BE CHANGED! - NDEF record header - Payload Length = x octets (1 byte in size because SR=1b)
    0x01,       //NDEF record header - Payload ID Length = 1 octet
                //NDEF record header - Record(Payload) Type = ‘application/vnd.bluetooth.le.oob’:
    0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74,
    0x69, 0x6F, 0x6E, 0x2F, 0x76, 0x6E, 0x64, 0x2E,
    0x62, 0x6C, 0x75, 0x65, 0x74, 0x6F, 0x6F, 0x74,
    0x68, 0x2E, 0x6C, 0x65, 0x2E, 0x6F, 0x6F, 0x62,
    0x30        //NDEF record header - Payload ID = ‘0’
                //NDEF record payload - add here Bluetooth AD types
};

#elif (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_EP_SHORT)
/* BR/EDR OOB Message
 * In case a Handover Selector device would advertise only one alternative carrier
 * (i.e., a Bluetooth carrier), a simplified format without the Handover Select record may be used.
 * In this case, the NFC Forum Tag contains an NDEF message with only the Bluetooth OOB information.
 * This corresponds to example message from section 4.3.1 of [NFC BTSSP] */
static uint8_t ndef_msg[NFC_BLE_PAIR_MSG_LEN] =
{
    0xD2,       //NDEF record header - TNF + Flags: MB=1b ME=1b CF=0b SR=1b IL=0b TNF=010b (MIME media-type)
    0x20,       //NDEF record header - Record Type Length = 32 octets
    0x00,       //MUST BE CHANGED! - NDEF record header - Payload Length = x octets (1 byte in size because SR=1b)
                //NDEF record header - ID Length missing since it is optional (IL=0b)
                //NDEF record header - Record(Payload) Type = ‘application/vnd.bluetooth.ep.oob’:
    0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74,
    0x69, 0x6F, 0x6E, 0x2F, 0x76, 0x6E, 0x64, 0x2E,
    0x62, 0x6C, 0x75, 0x65, 0x74, 0x6F, 0x6F, 0x74,
    0x68, 0x2E, 0x65, 0x70, 0x2E, 0x6F, 0x6F, 0x62,
                //NDEF record header - Payload ID missing since it is optional (IL=0b)
                //NDEF record payload - add here Bluetooth AD types
};

#elif (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_LE_SHORT)
/* Bluetooth LE OOB Message
 * In case a Handover Selector device would advertise only one alternative carrier
 * (i.e., a Bluetooth carrier), a simplified format without the Handover Select record may be used.
 * In this case, the NFC Forum Tag contains an NDEF message with only the Bluetooth OOB information.
 * This corresponds to example message from section 4.3.2 of [NFC BTSSP] */
static uint8_t ndef_msg[NFC_BLE_PAIR_MSG_LEN] =
{
    0xD2,       //NDEF record header - TNF + Flags: MB=1b ME=1b CF=0b SR=1b IL=0b TNF=010b (MIME media-type)
    0x20,       //NDEF record header - Record Type Length = 32 octets
    0x00,       //MUST BE CHANGED! - NDEF record header - Payload Length = x octets (1 byte in size because SR=1b)
                //NDEF record header - ID Length missing since it is optional (IL=0b)
                //NDEF record header - Record(Payload) Type = ‘application/vnd.bluetooth.le.oob’:
    0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74,
    0x69, 0x6F, 0x6E, 0x2F, 0x76, 0x6E, 0x64, 0x2E,
    0x62, 0x6C, 0x75, 0x65, 0x74, 0x6F, 0x6F, 0x74,
    0x68, 0x2E, 0x6C, 0x65, 0x2E, 0x6F, 0x6F, 0x62
                //NDEF record header - Payload ID missing since it is optional (IL=0b)
                //NDEF record payload - add here Bluetooth AD types
};

#else
    #error "Error. Type of NFC NDEF message used for BLE pairing over NFC not specified."
#endif /* NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE */

#if ((NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_EP_FULL) || \
     (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_EP_SHORT))

static uint32_t nfc_ble_pair_msg_check_adv_data(ble_advdata_t const *  const p_ble_advdata)
{
    /* AD structures: LE data (BLE Device Address and LE Role) must not be included.
     * Security Manager OOB Flags field is required */
    if((true == p_ble_advdata->include_ble_device_addr)         ||
       (BLE_ADVDATA_ROLE_NOT_PRESENT != p_ble_advdata->le_role) ||
       (NULL == p_ble_advdata->p_sec_mgr_oob_flags))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    return NRF_SUCCESS;
}

#elif ((NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_LE_FULL) || \
       (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_LE_SHORT))

static uint32_t nfc_ble_pair_msg_check_adv_data(ble_advdata_t const *  const p_ble_advdata)
{
    /* AD structures: LE Bluetooth Device Address and LE Role are required.
     * Security Manager Out Of Band Flags structure must not be included. */
    if((false == p_ble_advdata->include_ble_device_addr)        ||
       (BLE_ADVDATA_ROLE_NOT_PRESENT == p_ble_advdata->le_role) ||
       (NULL != p_ble_advdata->p_sec_mgr_oob_flags))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    return NRF_SUCCESS;
}

#else
    #error "Error. Type of NFC NDEF message used for BLE pairing over NFC not specified."
#endif /* NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE */

static uint32_t nfc_ble_pair_msg_check_adv_data_payload_buf(ble_advdata_t const * const p_ble_advdata)
{
    /* Checking if the buffer to encode data is properly configured. */
    if(((!NFC_BLE_PAIR_MSG_CONFIG_TK_VALUE_PRESENT)
               && (NULL != p_ble_advdata->p_tk_value))         ||
       ((!NFC_BLE_PAIR_MSG_CONFIG_APPEARANCE_PRESENT)
               && (true == p_ble_advdata->include_appearance)) ||
       ((!NFC_BLE_PAIR_MSG_CONFIG_FLAGS_PRESENT)
               && (0 != p_ble_advdata->flags))                 ||
       ((0 == NFC_BLE_PAIR_MSG_CONFIG_LOCAL_NAME_MAX_LEN)
               && (BLE_ADVDATA_NO_NAME != p_ble_advdata->name_type)))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    /* If Flags AD structure is present, the BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED flag must be set. */
    if ((0 != p_ble_advdata->flags) &&
            ((p_ble_advdata->flags & BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED) == 0))
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    return NRF_SUCCESS;
}

#if ((NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_EP_FULL) || \
     (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_EP_SHORT))

/* This fuction is used to encode device address to Bluetooth EP NDEF OOB Handover message*/
static uint32_t nrf_oob_encode_bluetooth_ep_device_address(uint8_t  * const p_encoded_data,
                                                           uint16_t         max_len)
{
    uint32_t       err_code = NRF_SUCCESS;
    ble_gap_addr_t device_address;

    if (NFC_BLE_PAIR_MSG_BLUETOOTH_EP_ADDR_LEN > max_len)
    {
        return NRF_ERROR_DATA_SIZE;
    }

    // Get BLE address
    err_code = sd_ble_gap_address_get(&device_address);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Encode Bluetooth EP device address
    memcpy(p_encoded_data, device_address.addr, NFC_BLE_PAIR_MSG_BLUETOOTH_EP_ADDR_LEN);

    return NRF_SUCCESS;
}

#endif /* NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE */

#if ((NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_EP_FULL) || \
     (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_EP_SHORT))

uint32_t nfc_ble_pair_msg_create(ble_advdata_t const *  const p_ble_advdata,
                                 uint8_t             ** const pp_encoded_data,
                                 uint16_t            *  const p_len)
{
    uint32_t   err_code = NRF_SUCCESS;
    uint8_t *  p_payload;
    uint8_t *  p_ad_data;
    uint16_t   payload_len, ad_data_len;

    /* Check correctness of the configuration structure */
    err_code = nfc_ble_pair_msg_check_adv_data(p_ble_advdata);
    if(NRF_SUCCESS != err_code)
    {
        return err_code;
    }
    /* Check correctness of a static memory allocation */ 
    err_code = nfc_ble_pair_msg_check_adv_data_payload_buf(p_ble_advdata);
    if(NRF_SUCCESS != err_code)
    {
        return err_code;
    }

    /* Select NFC message payload to encode. */
    p_payload   = &ndef_msg[NFC_BLE_PAIR_MSG_HEADER_LEN];
    payload_len = NFC_BLE_PAIR_MSG_PAYLOAD_LEN;

    /* Encode AD structures into NFC message.
     * BT EP device address and payload length field must be inserted before the AD payload */
    p_ad_data   = (uint8_t *) (p_payload + NFC_BLE_PAIR_MSG_BLUETOOTH_EP_ADDR_LEN +
            NFC_BLE_PAIR_MSG_BLUETOOTH_OOB_DATA_LEN_SIZE);
    ad_data_len = payload_len - NFC_BLE_PAIR_MSG_BLUETOOTH_EP_ADDR_LEN -
            NFC_BLE_PAIR_MSG_BLUETOOTH_OOB_DATA_LEN_SIZE;
    err_code = adv_data_encode(p_ble_advdata, p_ad_data, &ad_data_len);
    if(NRF_SUCCESS != err_code)
    {
        return err_code;
    }

    /* Now as the final payload length is known OOB payload length field, and BT device address can
     * be encoded */
    payload_len = ad_data_len + NFC_BLE_PAIR_MSG_BLUETOOTH_EP_ADDR_LEN +
            NFC_BLE_PAIR_MSG_BLUETOOTH_OOB_DATA_LEN_SIZE;

    /* Encode OOB optional data length */
    p_payload += uint16_encode(payload_len, p_payload);

    /* Encode Bluetooth EP device address */
    err_code = nrf_oob_encode_bluetooth_ep_device_address(p_payload, payload_len);
    if(NRF_SUCCESS != err_code)
    {
        return err_code;
    }
    
    /* Update NFC message payload length field and point encoded message. */
    if(NRF_SUCCESS == err_code)
    {
        ndef_msg[NFC_BLE_PAIR_MSG_PAYLOAD_LEN_OFFSET] = (uint8_t) payload_len;
        *pp_encoded_data = &ndef_msg[0];
        *p_len = NFC_BLE_PAIR_MSG_HEADER_LEN + payload_len;
    }

    return err_code;
}

#elif ((NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_LE_FULL) || \
       (NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE == NFC_BLE_PAIR_MSG_BLUETOOTH_LE_SHORT))

uint32_t nfc_ble_pair_msg_create(ble_advdata_t const *  const p_ble_advdata,
                                 uint8_t             ** const pp_encoded_data,
                                 uint16_t            *  const p_len)
{
    uint32_t   err_code = NRF_SUCCESS;
    uint8_t  * p_payload;
    uint16_t   payload_len;

    /* Check correctness of the configuration structure */
    err_code = nfc_ble_pair_msg_check_adv_data(p_ble_advdata);
    if(NRF_SUCCESS != err_code)
    {
        return err_code;
    }
    /* Check correctness of a static memory allocation */ 
    err_code = nfc_ble_pair_msg_check_adv_data_payload_buf(p_ble_advdata);
    if(NRF_SUCCESS != err_code)
    {
        return err_code;
    }

    /* Select NFC message payload to encode. */
    p_payload   = &ndef_msg[NFC_BLE_PAIR_MSG_HEADER_LEN];
    payload_len = NFC_BLE_PAIR_MSG_PAYLOAD_LEN;

    /* Encode AD structures into NFC message. */
    err_code = adv_data_encode(p_ble_advdata, p_payload, &payload_len);
    /* Update NFC message payload length field and point encoded message. */
    if(NRF_SUCCESS == err_code)
    {
        ndef_msg[NFC_BLE_PAIR_MSG_PAYLOAD_LEN_OFFSET] = (uint8_t) payload_len;
        *pp_encoded_data = &ndef_msg[0];
        *p_len = NFC_BLE_PAIR_MSG_HEADER_LEN + payload_len;
    }

    return err_code;
}

#else
    #error "Error. Type of NFC NDEF message used for BLE pairing over NFC not specified."
#endif /* NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE */

