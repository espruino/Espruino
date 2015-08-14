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

#include <string.h>
#include "nfc_uri_msg.h"
#include "nfc_uri_msg_config.h"
#include "nrf_error.h"
#include "app_util.h"

/**@brief Common NFC URI definitions. */
#define NFC_URI_ID_LEN                      (1UL)
#define NFC_URI_NDEF_MSG_HEADER_LEN         (4UL)
#define NFC_URI_PAYLOAD_LEN                 (NFC_URI_ID_LEN + NFC_URI_MAX_LEN)
#define NFC_URI_NDEF_MSG_LEN                (NFC_URI_NDEF_MSG_HEADER_LEN + NFC_URI_PAYLOAD_LEN)
#define NFC_URI_NDEF_MSG_PAYLOAD_LEN_OFFSET (2UL)

/**@brief NDEF URI message. 
 * When a NFC Poller device reads this message it should try to open the resource
 * pointed to by the URI. The protocol field of the URI is identified by
 * the URI Identifier Code (one octet).
 * The following data corresponds to the example message described in "URI Record Type Definition"
 * (denotation "NFCForum-TS-RTD_URI_1.0" published on 2006-07-24) chapter 4.A1 */
static uint8_t ndef_msg[NFC_URI_NDEF_MSG_LEN] = 
{
    0xD1,       //NDEF record header - TNF + Flags: MB=1b ME=1b CF=0b SR=1b IL=0b TNF=001b (Well-Known)
    0x01,       //NDEF record header - Record Type Length = 2 octets
    0x00,       //MUST BE CHANGED! - NDEF record header - Payload Length = x octets (1 byte in size because SR=1b)
                //NDEF record header - ID Length missing since it is optional (IL=0b)
    0x55,       //NDEF record header - Record(Payload) Type = ‘U’ (URI Record)
                //NDEF record header - Payload ID missing since it is optional (IL=0b)
};

uint32_t nfc_uri_msg_create(nfc_uri_id_t           uri_id_code,
                            uint8_t const *  const p_uri_data,
                            uint8_t                uri_data_len,
                            uint8_t       ** const pp_encoded_data,
                            uint16_t      *  const p_len)
{
    uint8_t *  p_payload;
    uint8_t    payload_index = 0;

    /* Check length of URI data */
    if (uri_data_len > NFC_URI_MAX_LEN)
    {
        return NRF_ERROR_DATA_SIZE;
    }

    /* Select NFC message payload to encode. */
    p_payload   = &ndef_msg[NFC_URI_NDEF_MSG_HEADER_LEN];

    /* Encode URI ID code into the NFC message */
    p_payload[payload_index] = (uint8_t) uri_id_code;
    payload_index           += NFC_URI_ID_LEN;

    /* Encode URI data into the NFC message. */
    memcpy(&p_payload[payload_index], p_uri_data, uri_data_len);
    payload_index += uri_data_len;
    
    /* Update NFC message payload length field and point encoded message. */
    ndef_msg[NFC_URI_NDEF_MSG_PAYLOAD_LEN_OFFSET] = (uint8_t) payload_index;
    *pp_encoded_data                              = &ndef_msg[0];
    *p_len                                        = NFC_URI_NDEF_MSG_HEADER_LEN + payload_index;

    return NRF_SUCCESS;
}
