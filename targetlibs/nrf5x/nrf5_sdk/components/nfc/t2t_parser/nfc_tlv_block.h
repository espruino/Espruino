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

#ifndef NFC_TLV_BLOCK_H__
#define NFC_TLV_BLOCK_H__

#include <stdint.h>

/**@file
 *
 * @defgroup nfc_type_2_tag_tlv_block Type 2 Tag TLV blocks
 * @{
 * @ingroup  nfc_type_2_tag_parser
 *
 * @brief Descriptor for a Type 2 Tag TLV block.
 *
 */

/**
 * @brief Tag field values.
 *
 * Possible values for the tag field in a TLV block.
 */
typedef enum
{
    TLV_NULL            = 0x00, ///< Might be used for padding of memory areas.
    TLV_LOCK_CONTROL    = 0x01, ///< Defines details of the lock bits.
    TLV_MEMORY_CONTROL  = 0x02, ///< Identifies reserved memory areas.
    TLV_NDEF_MESSAGE    = 0x03, ///< Contains an NDEF message.
    TLV_PROPRIETARY     = 0xFD, ///< Tag proprietary information.
    TLV_TERMINATOR      = 0xFE  ///< Last TLV block in the data area.
} tlv_block_types_t;

/**
 * @brief TLV block descriptor.
 */
typedef struct
{
    uint8_t    tag;             ///< Type of the TLV block.
    uint16_t   length;          ///< Length of the value field.
    uint8_t  * p_value;         ///< Pointer to the value field (NULL if no value field is present in the block).
} tlv_block_t;

#define TLV_T_LENGTH        1           ///< Length of a tag field.

#define TLV_L_SHORT_LENGTH  1           ///< Length of a short length field.
#define TLV_L_LONG_LENGTH   3           ///< Length of an extended length field.
#define TLV_L_FORMAT_FLAG   0xFF        ///< Value indicating the use of an extended length field.

#define TLV_NULL_TERMINATOR_LEN     0   ///< Predefined length of the NULL and TERMINATOR TLV blocks.
#define TLV_LOCK_MEMORY_CTRL_LEN    3   ///< Predefined length of the LOCK CONTROL and MEMORY CONTROL blocks.

/**
 * @}
 */

#endif /* NFC_TLV_BLOCK_H__ */
