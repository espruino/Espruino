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

#ifndef NFC_TEXT_REC_H__
#define NFC_TEXT_REC_H__

/**@file
 *
 * @defgroup nfc_text_rec Text records
 * @{
 * @ingroup  nfc_ndef_messages
 *
 * @brief    Generation of NFC NDEF Text record descriptions.
 *
 */

#include "nfc_ndef_record.h"

/**
 * @brief Text record payload descriptor.
 */
typedef struct
{
    enum{
        UTF_8   = 0,
        UTF_16  = 1,
    }               utf;            ///< Type of the Unicode Transformation Format.

    uint8_t const * p_lang_code;    ///< Pointer to the IANA language code.
    uint8_t         lang_code_len;  ///< Length of the IANA language code.

    uint8_t const * p_data;         ///< Pointer to the user text.
    uint32_t        data_len;       ///< Length of the user text.
} nfc_text_rec_payload_desc_t;

/**
 * @brief Constructor for an NFC NDEF Text record payload.
 *
 * @param[in]       p_nfc_rec_text_payload_desc Pointer to the Text record description.
 * @param[out]      p_buff                      Pointer to the payload destination.
 *
 * @param[in,out]   p_len                       Size of the available memory to write as input. 
 *                                              Size of the generated record payload as output.
 */
ret_code_t nfc_text_rec_payload_constructor(nfc_text_rec_payload_desc_t * p_nfc_rec_text_payload_desc,
                                            uint8_t                     * p_buff,
                                            uint32_t                    * p_len);

/**
 * @brief External reference to the type field of the Text record, defined in the
 * file @c nfc_text_rec.c. It is used in the @ref NFC_NDEF_TEXT_RECORD_DESC_DEF macro.
 */
extern const uint8_t nfc_text_rec_type_field[];

/**
 * @brief Size of the type field of the Text record, defined in the
 * file @c nfc_text_rec.c. It is used in the @ref NFC_NDEF_TEXT_RECORD_DESC_DEF macro.
 */
#define NFC_TEXT_REC_TYPE_LENGTH 1

/**
 *@brief Macro for creating and initializing an NFC NDEF record descriptor for an Text record.
 *
 * This macro creates and initializes a static instance of type @ref nfc_ndef_record_desc_t and 
 * a static instance of type @ref nfc_text_rec_payload_desc_t, which together constitute
 * an instance of an Text record.
 *
 * Use the macro @ref NFC_NDEF_TEXT_RECORD_DESC to access the NDEF Text record descriptor instance.
 *
 * @param[in] NAME              Name of the created record descriptor instance.
 * @param[in] UTF               Unicode Transformation Format.
 * @param[in] P_LANG_CODE       Pointer to the IANA language code.
 * @param[in] LANG_CODE_LEN     Length of the IANA language code.
 * @param[in] P_DATA            Pointer to the user text.
 * @param[in] DATA_LEN          Length of the user text.
 */
#define NFC_NDEF_TEXT_RECORD_DESC_DEF(NAME,                                 \
                                      UTF,                                  \
                                      P_LANG_CODE,                          \
                                      LANG_CODE_LEN,                        \
                                      P_DATA,                               \
                                      DATA_LEN)                             \
    static nfc_text_rec_payload_desc_t NAME##_nfc_text_rec_payload_desc;    \
    NAME##_nfc_text_rec_payload_desc = (nfc_text_rec_payload_desc_t)        \
    {                                                                       \
        .utf            = UTF,                                              \
        .p_lang_code    = P_LANG_CODE,                                      \
        .lang_code_len  = LANG_CODE_LEN,                                    \
        .p_data         = P_DATA,                                           \
        .data_len       = DATA_LEN,                                         \
    };                                                                      \
    NFC_NDEF_GENERIC_RECORD_DESC_DEF(NAME,                                  \
                                     TNF_WELL_KNOWN,                        \
                                     0,                                     \
                                     0,                                     \
                                     nfc_text_rec_type_field,               \
                                     NFC_TEXT_REC_TYPE_LENGTH,              \
                                     nfc_text_rec_payload_constructor,      \
                                     &(NAME##_nfc_text_rec_payload_desc))

/**
 * @brief Macro for accessing the NFC NDEF Text record descriptor 
 * instance that was created with @ref NFC_NDEF_TEXT_RECORD_DESC_DEF.
 */
#define NFC_NDEF_TEXT_RECORD_DESC(NAME) NFC_NDEF_GENERIC_RECORD_DESC(NAME)

/** @} */
#endif // NFC_TEXT_REC_H__
