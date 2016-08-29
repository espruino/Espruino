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

#ifndef NFC_HS_REC_H__
#define NFC_HS_REC_H__

/**@file
 *
 * @defgroup nfc_hs_rec Hs (Handover Select) records
 * @{
 * @ingroup  nfc_ble_pair_msg
 *
 * @brief Generation of NFC NDEF Handover Select records for NDEF messages.
 *
 */

#include <stdint.h>
#include "nfc_ndef_record.h"
#include "nfc_ndef_msg.h"

/**
 * @brief Handover Select record payload descriptor.
 */
typedef struct
{
    uint8_t                 major_version;      ///< Major version number of the supported Connection Handover specification.
    uint8_t                 minor_version;      ///< Minor version number of the supported Connection Handover specification.
    nfc_ndef_msg_desc_t   * p_local_records;    ///< Pointer to a message encapsulating local records.
} nfc_hs_rec_payload_desc_t;


/**
 * @brief Constructor for an NFC NDEF Handover Select record payload.
 *
 * This function encodes the payload of a Handover Select record as specified in the Connection 
 * Handover standard. It implements an API compatible with @ref p_payload_constructor_t.
 */

ret_code_t nfc_hs_rec_payload_constructor(nfc_hs_rec_payload_desc_t * p_nfc_hs_rec_payload_desc,
                                          uint8_t                   * p_buff,
                                          uint32_t                  * p_len);

/**
 * @brief An external reference to the type field of the Handover Select record, defined in the
 * file @c nfc_hs_rec.c. It is used in the @ref NFC_NDEF_HS_RECORD_DESC_DEF macro.
 */
extern const uint8_t nfc_hs_rec_type_field[];

/**
 * @brief Size of the type field of the Handover Select record, defined in the
 * file @c nfc_hs_rec.c. It is used in the @ref NFC_NDEF_HS_RECORD_DESC_DEF macro.
 */
#define NFC_HS_REC_TYPE_LENGTH 2

/**
 * @brief Macro for creating and initializing an NFC NDEF record descriptor for a Handover Select record.
 *
 * This macro creates and initializes a static instance of type @ref nfc_ndef_record_desc_t and 
 * a static instance of type @ref nfc_hs_rec_payload_desc_t, which together constitute an instance of a Handover Select record.
 *
 * Use the macro @ref NFC_NDEF_HS_RECORD_DESC to access the NDEF Handover Select record descriptor instance.
 *
 * @param[in] NAME          Name of the created record descriptor instance.
 * @param[in] MAJOR_VERSION Major version number of the supported Connection Handover specification.
 * @param[in] MINOR_VERSION Minor version number of the supported Connection Handover specification.
 * @param[in] MAX_RECORDS   Maximum number of local records (ac records plus optional err record).
 */
#define NFC_NDEF_HS_RECORD_DESC_DEF(NAME,                                   \
                                    MAJOR_VERSION,                          \
                                    MINOR_VERSION,                          \
                                    MAX_RECORDS)                            \
    NFC_NDEF_MSG_DEF(NAME, MAX_RECORDS);                                    \
    static nfc_hs_rec_payload_desc_t NAME##_nfc_hs_rec_payload_desc =       \
    {                                                                       \
        .major_version = MAJOR_VERSION,                                     \
        .minor_version = MINOR_VERSION,                                     \
        .p_local_records = &NFC_NDEF_MSG(NAME)                              \
    };                                                                      \
    NFC_NDEF_GENERIC_RECORD_DESC_DEF(NAME,                                  \
                                     TNF_WELL_KNOWN,                        \
                                     0,                                     \
                                     0,                                     \
                                     nfc_hs_rec_type_field  ,               \
                                     NFC_HS_REC_TYPE_LENGTH,                \
                                     nfc_hs_rec_payload_constructor,        \
                                     &(NAME##_nfc_hs_rec_payload_desc))

/**
 * @brief Macro for accessing the NFC NDEF Handover Select record descriptor
 * instance that was created with @ref NFC_NDEF_HS_RECORD_DESC_DEF.
 */
#define NFC_NDEF_HS_RECORD_DESC(NAME) NFC_NDEF_GENERIC_RECORD_DESC(NAME)

/**
 * @brief Function for clearing local records in the NFC NDEF Handover Select record.
 *
 * This function clears local records from the Handover Select record.
 *
 * @param[in, out] p_hs_rec Pointer to the Handover Select record descriptor.
 */
void nfc_hs_rec_local_record_clear(nfc_ndef_record_desc_t * p_hs_rec);

/**
 * @brief Function for adding a local record to an NFC NDEF Handover Select record.
 *
 * @param[in, out] p_hs_rec Pointer to a Handover Select record.
 * @param[in] p_local_rec   Pointer to a local record to add.
 *
 * @retval NRF_SUCCESS      If the local record was added successfully.
 * @retval NRF_ERROR_NO_MEM If the Handover Select record already contains the maximum number of local records.
 */
ret_code_t nfc_hs_rec_local_record_add(nfc_ndef_record_desc_t * p_hs_rec,
                                       nfc_ndef_record_desc_t * p_local_rec);

/** @} */
#endif // NFC_HS_REC_H__
