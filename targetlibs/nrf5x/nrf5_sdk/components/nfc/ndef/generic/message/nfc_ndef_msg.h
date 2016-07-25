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

#ifndef NFC_NDEF_MSG_H__
#define NFC_NDEF_MSG_H__

#include "nfc_ndef_record.h"
/**@file
 *
 * @defgroup nfc_ndef_msg Custom NDEF messages
 * @{
 * @ingroup  nfc_modules
 *
 * @brief    Generation of NFC NDEF messages for the NFC Type 2 Tag.
 *
 */

 /**
  * @brief NDEF message descriptor.
  */
 typedef struct {
     nfc_ndef_record_desc_t ** pp_record;        ///< Pointer to an array of pointers to NDEF record descriptors.
     uint32_t                  max_record_count; ///< Number of elements in the allocated pp_record array, which defines the maximum number of records within the NDEF message.
     uint32_t                  record_count;     ///< Number of records in the NDEF message.
 } nfc_ndef_msg_desc_t;
 
 /**
  * @brief  Function for encoding an NDEF message.
  *
  * This function encodes an NDEF message according to the provided message descriptor.
  *
  * @param[in] p_ndef_msg_desc  Pointer to the message descriptor.
  * @param[out] p_msg_buffer    Pointer to the message destination.
  * @param[in,out] p_msg_len    Size of the available memory for the message as input. Size of the generated
  *                                 message as output.
  * 
  * @return  Return value from @ref nfc_ndef_record_encode.
  */
ret_code_t nfc_ndef_msg_encode( nfc_ndef_msg_desc_t const * p_ndef_msg_desc,
                                uint8_t                   * p_msg_buffer,
                                uint32_t                  * const  p_msg_len);

/**
 * @brief Function for clearing an NDEF message.
 *
 * This function clears an NDEF message descriptor, thus empties the NDEF message.
 * 
 * @param[in,out] p_msg Pointer to the message descriptor.
 */
void nfc_ndef_msg_clear( nfc_ndef_msg_desc_t * p_msg);

/**
 * @brief Function for adding a record to an NDEF message.
 *
 * @param[in] p_record  Pointer to the record descriptor.
 * @param[in,out] p_msg Pointer to the message descriptor.
 *
 * @retval NRF_SUCCESS      If the record was added successfully.
 * @retval NRF_ERROR_NO_MEM If the message already contains the maximum number of records and the operation is not allowed.
 */
ret_code_t nfc_ndef_msg_record_add( nfc_ndef_msg_desc_t    * const p_msg,
                                    nfc_ndef_record_desc_t * const p_record);

                              
/**@brief Macro for creating and initializing an NFC NDEF message descriptor.
 *
 * This macro creates and initializes a static instance of type @ref nfc_ndef_msg_desc_t
 * and a static array of pointers to record descriptors (@ref nfc_ndef_record_desc_t) used
 * by the message. 
 *
 * Use the macro @ref NFC_NDEF_MSG to access the NDEF message descriptor instance.
 *
 * @param[in]  NAME                 Name of the related instance.
 * @param[in]  MAX_RECORD_CNT       Maximal count of records in the message.
 */
#define NFC_NDEF_MSG_DEF(NAME, MAX_RECORD_CNT)                                             \
    static nfc_ndef_record_desc_t   * NAME##_nfc_ndef_p_record_desc_array[MAX_RECORD_CNT]; \
    static nfc_ndef_msg_desc_t  NAME##_nfc_ndef_msg_desc =                                 \
        {                                                                                  \
            .pp_record = NAME##_nfc_ndef_p_record_desc_array,                              \
            .record_count = 0,                                                             \
            .max_record_count = MAX_RECORD_CNT                                             \
        }

/** @brief Macro for accessing the NFC NDEF message descriptor instance 
 *  that you created with @ref NFC_NDEF_MSG_DEF.
 */
#define NFC_NDEF_MSG(NAME) (NAME##_nfc_ndef_msg_desc)

/**
 * @brief Macro for creating and initializing an NFC NDEF record descriptor with an encapsulated NDEF message.

 * This macro creates and initializes a static instance of type 
 * @ref nfc_ndef_record_desc_t that contains an encapsulated NDEF message as 
 * payload. @ref nfc_ndef_msg_encode is used as payload constructor to encode 
 * the message. The encoded message is then used as payload for the record.
 *
 * Use the macro @ref NFC_NDEF_NESTED_NDEF_MSG_RECORD to access the NDEF record descriptor instance.
 *
 * @param[in] NAME             Name of the created record descriptor instance.
 * @param[in] TNF              Type Name Format (TNF) value for the record.
 * @param[in] P_ID             Pointer to the ID string.
 * @param[in] ID_LEN           Length of the ID string.
 * @param[in] P_TYPE           Pointer to the type string.
 * @param[in] TYPE_LEN         Length of the type string.
 * @param[in] P_NESTED_MESSAGE Pointer to the message descriptor to encapsulate
 *                             as the record's payload.
 */
#define NFC_NDEF_NESTED_NDEF_MSG_RECORD_DEF( NAME,                             \
                                             TNF,                              \
                                             P_ID,                             \
                                             ID_LEN,                           \
                                             P_TYPE,                           \
                                             TYPE_LEN,                         \
                                             P_NESTED_MESSAGE )                \
    static nfc_ndef_record_desc_t NAME##_ndef_record_nested_desc =             \
    {                                                                          \
        .tnf = TNF,                                                            \
                                                                               \
        .id_length = ID_LEN,                                                   \
        .p_id =  P_ID,                                                         \
                                                                               \
        .type_length = TYPE_LEN,                                               \
        .p_type = P_TYPE,                                                      \
                                                                               \
        .payload_constructor = (p_payload_constructor_t)(nfc_ndef_msg_encode), \
        .p_payload_descriptor = (void*) (P_NESTED_MESSAGE)                     \
    }

/** @brief Macro for accessing the NFC NDEF record descriptor instance
 *  that you created with @ref NFC_NDEF_NESTED_NDEF_MSG_RECORD_DEF.
 */
#define NFC_NDEF_NESTED_NDEF_MSG_RECORD(NAME) (NAME##_ndef_record_nested_desc)

/**
 * @}
 */
 #endif

