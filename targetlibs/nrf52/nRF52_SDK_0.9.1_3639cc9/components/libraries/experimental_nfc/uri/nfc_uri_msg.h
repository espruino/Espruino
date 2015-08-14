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

#ifndef NFC_URI_MSG_H__
#define NFC_URI_MSG_H__

/**@file
 *
 * @defgroup uri URI message
 * @{
 * @ingroup  nfc_modules
 *
 * @brief    Generation of NDEF URI messages for the NFC tag.
 *
 */

#include <stdint.h>

/**
 * @enum nfc_uri_id_t
 * @brief URI identifier codes according to "URI Record Type Definition"
 * (denotation "NFCForum-TS-RTD_URI_1.0" published on 2006-07-24) chapter 3.2.2.
 */
typedef enum
{
    NFC_URI_NONE          = 0x00,  /**< No prepending is done. */
    NFC_URI_HTTP_WWW      = 0x01,  /**< "http://www." */
    NFC_URI_HTTPS_WWW     = 0x02,  /**< "https://www." */
    NFC_URI_HTTP          = 0x03,  /**< "http:" */
    NFC_URI_HTTPS         = 0x04,  /**< "https:" */
    NFC_URI_TEL           = 0x05,  /**< "tel:" */
    NFC_URI_MAILTO        = 0x06,  /**< "mailto:" */
    NFC_URI_FTP_ANONYMOUS = 0x07,  /**< "ftp://anonymous:anonymous@" */
    NFC_URI_FTP_FTP       = 0x08,  /**< "ftp://ftp." */
    NFC_URI_FTPS          = 0x09,  /**< "ftps://" */
    NFC_URI_SFTP          = 0x0A,  /**< "sftp://" */
    NFC_URI_SMB           = 0x0B,  /**< "smb://" */
    NFC_URI_NFS           = 0x0C,  /**< "nfs://" */
    NFC_URI_FTP           = 0x0D,  /**< "ftp://" */
    NFC_URI_DAV           = 0x0E,  /**< "dav://" */
    NFC_URI_NEWS          = 0x0F,  /**< "news:" */
    NFC_URI_TELNET        = 0x10,  /**< "telnet://" */
    NFC_URI_IMAP          = 0x11,  /**< "imap:" */
    NFC_URI_RTSP          = 0x12,  /**< "rtsp://" */
    NFC_URI_URN           = 0x13,  /**< "urn:" */
    NFC_URI_POP           = 0x14,  /**< "pop:" */
    NFC_URI_SIP           = 0x15,  /**< "sip:" */
    NFC_URI_SIPS          = 0x16,  /**< "sips:" */
    NFC_URI_TFTP          = 0x17,  /**< "tftp:" */
    NFC_URI_BTSPP         = 0x18,  /**< "btspp://" */
    NFC_URI_BTL2CAP       = 0x19,  /**< "btl2cap://" */
    NFC_URI_BTGOEP        = 0x1A,  /**< "btgoep://" */
    NFC_URI_TCPOBEX       = 0x1B,  /**< "tcpobex://" */
    NFC_URI_IRDAOBEX      = 0x1C,  /**< "irdaobex://" */
    NFC_URI_FILE          = 0x1D,  /**< "file://" */
    NFC_URI_URN_EPC_ID    = 0x1E,  /**< "urn:epc:id:" */
    NFC_URI_URN_EPC_TAG   = 0x1F,  /**< "urn:epc:tag:" */
    NFC_URI_URN_EPC_PAT   = 0x20,  /**< "urn:epc:pat:" */
    NFC_URI_URN_EPC_RAW   = 0x21,  /**< "urn:epc:raw:" */
    NFC_URI_URN_EPC       = 0x22,  /**< "urn:epc:" */
    NFC_URI_URN_NFC       = 0x23,  /**< "urn:nfc:" */
    NFC_URI_RFU           = 0xFF   /**< No prepending is done. Reserved for future use. */
} nfc_uri_id_t;

/** @brief Function for generating an NFC NDEF message with a URI record.
 *
 * @details The module allocates a static buffer that stores a generated message. The length of the
 *          static buffer can be configured in @c nfc_uri_msg_config.h.
 *
 * @param[in]  uri_id_code          URI identifier code that defines the protocol field of the URI.
 * @param[in]  p_uri_data           Pointer to the URI string.
 *                                  It should not contain the protocol field if the protocol
 *                                  was chosen using \p uri_id_code
 * @param[in]  uri_data_len         Length of the URI string.
 * @param[out] pp_encoded_data      Pointer to store the pointer to the generated NFC message.
 * @param[out] p_len                Pointer to store the length of the generated NFC message.
 *
 * @retval     NRF_SUCCESS          If the message was successfully generated.
 * @retval     NRF_ERROR_DATA_SIZE  If the URI string is too long.
 */
uint32_t nfc_uri_msg_create(nfc_uri_id_t           uri_id_code,
                            uint8_t const *  const p_uri_data,
                            uint8_t                uri_data_len,
                            uint8_t       ** const pp_encoded_data,
                            uint16_t      *  const p_len);

/** @} */
#endif // NFC_URI_MSG_H__
