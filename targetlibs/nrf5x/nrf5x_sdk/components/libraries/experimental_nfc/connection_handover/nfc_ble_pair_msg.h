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

#ifndef NFC_BLE_PAIR_MSG_H__
#define NFC_BLE_PAIR_MSG_H__

/**@file
 *
 * @defgroup nfc_modules NFC modules
 * @ingroup nfc_api
 * @brief NFC modules.
 *
 * @defgroup nfc_ble_pair_msg BLE pairing message
 * @{
 * @ingroup  nfc_modules
 *
 * @brief    Generation of NFC NDEF messages used for BLE pairing over NFC.
 *
 */

#include <stdint.h>
#include "ble_advdata.h"

/**
 * @defgroup nfc_ndef_ble_pair_msg_types NFC NDEF message types for BLE pairing
 *
 * @brief NFC NDEF message types available for BLE pairing over NFC.
 * @{ */

/**
 * @brief NFC NDEF message for BLE pairing - record type application/vnd.bluetooth.le.oob and
 *        simplified message format.
 *
 * Simplified message with record type application/vnd.bluetooth.le.oob implemented according to
 * "Bluetooth Secure Simple Pairing Using NFC Application Document" (denotation
 * "NFCForum-AD-BTSSP_1_1" published on 2014-01-09) chapters 3.3, 3.4, 4.3.2, and "Supplement to the
 * Bluetooth Core Specification" (Version 5, adoption date: Dec 02 2014).
 */
#define NFC_BLE_PAIR_MSG_BLUETOOTH_LE_SHORT    0
/**
 * @brief NFC NDEF message for BLE pairing - record type application/vnd.bluetooth.le.oob and
 *        full message format.
 *
 * Full message with record type application/vnd.bluetooth.le.oob implemented according to
 * "Bluetooth Secure Simple Pairing Using NFC Application Document" (denotation
 * "NFCForum-AD-BTSSP_1_1" published on 2014-01-09) chapters 3.3, 3.4, 4.1.2, and "Supplement to the
 * Bluetooth Core Specification" (Version 5, adoption date: Dec 02 2014).
 */
#define NFC_BLE_PAIR_MSG_BLUETOOTH_LE_FULL     1
/**
 * @brief Non-standardized NFC NDEF message for BLE pairing - record type
 *        application/vnd.bluetooth.ep.oob and simplified message format.
 *
 * Simplified non-standardized message with record type application/vnd.bluetooth.ep.oob
 * implemented partially according to "Bluetooth Secure Simple Pairing Using NFC Application
 * Document" (denotation "NFCForum-AD-BTSSP_1_1" published on 2014-01-09) chapters 3.1, 3.2, 4.3.1,
 * and according to "Supplement to the Bluetooth Core Specification" (Version 5, adoption date:
 * Dec 02 2014).
 */
#define NFC_BLE_PAIR_MSG_BLUETOOTH_EP_SHORT    2
/**
 * @brief Non-standardized NFC NDEF message for BLE pairing - record type
 *        application/vnd.bluetooth.ep.oob and full message format.
 *
 * Full non-standardized message with record type application/vnd.bluetooth.ep.oob
 * implemented partially according to "Bluetooth Secure Simple Pairing Using NFC Application
 * Document" (denotation "NFCForum-AD-BTSSP_1_1" published on 2014-01-09) chapters 3.1, 3.2, 4.1.1,
 * and according to "Supplement to the Bluetooth Core Specification" (Version 5, adoption date:
 * Dec 02 2014).
 */
#define NFC_BLE_PAIR_MSG_BLUETOOTH_EP_FULL     3
/** @} */

/**@brief Function for generating an NFC NDEF message with a record for BLE pairing over NFC.
 *
 * @details The module allocates a static buffer that stores a generated message. The length of the
 *          static buffer and the message type can be configured in @c nfc_ble_pair_msg_config.h.
 *
 *          For message types @ref NFC_BLE_PAIR_MSG_BLUETOOTH_LE_SHORT and
 *          @ref NFC_BLE_PAIR_MSG_BLUETOOTH_LE_FULL AD, the AD structures 'LE Bluetooth Device Address' and
 *          'LE Role' are required, and the AD structures 'Security Manager TK value', 'Appearance',
 *          'Local Name', and 'Flags' are optional.
 *
 *          For message types @ref NFC_BLE_PAIR_MSG_BLUETOOTH_EP_SHORT and
 *          @ref NFC_BLE_PAIR_MSG_BLUETOOTH_EP_FULL, the AD structure 'Security Manager Out Of Band
 *          Flags' is required, and the AD structures 'Security Manager TK value', 'Appearance', and
 *          'Local Name' are optional.
 *
 * @note To be able to generate the message, a SoftDevice must be enabled and configured.
 *
 * @param[in]      p_ble_advdata       Pointer to the structure containing the content of the message
 *                                 to generate.
 * @param[out]     pp_encoded_data Pointer to store the pointer to the generated NFC message.
 * @param[out]     p_len           Pointer to store the length of the generated NFC message.
 *
 * @retval NRF_SUCCESS             If the operation was successful.
 * @retval NRF_ERROR_INVALID_PARAM If the operation failed because a wrong parameter was provided in
 *                                 \p p_ble_advdata.
 * @retval NRF_ERROR_DATA_SIZE     If the operation failed because not all the requested data could
 *                                 fit into the provided buffer or an encoded AD structure is too
 *                                 long and its length cannot be encoded with one octet.
 */
uint32_t nfc_ble_pair_msg_create(ble_advdata_t const *  const p_ble_advdata,
                                 uint8_t             ** const pp_encoded_data,
                                 uint16_t            *  const p_len);

/** @} */
#endif // NFC_BLE_PAIR_MSG_H__
