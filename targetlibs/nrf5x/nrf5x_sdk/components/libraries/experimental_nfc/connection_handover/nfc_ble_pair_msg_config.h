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

/**@file
 *
 * @defgroup nfc_ble_pair_msg_config BLE pairing message configuration
 * @{
 * @ingroup  nfc_modules
 *
 * @brief    Configuration of NFC NDEF messages used for BLE pairing over NFC.
 *
 */

#ifndef NFC_BLE_PAIR_MSG_CONFIG_H__
#define NFC_BLE_PAIR_MSG_CONFIG_H__

#include "nfc_ble_pair_msg.h"

/**
 * @name  Configuration of NFC NDEF messages for BLE pairing
 *
 * @brief For message types @ref NFC_BLE_PAIR_MSG_BLUETOOTH_LE_SHORT and
 *        @ref NFC_BLE_PAIR_MSG_BLUETOOTH_LE_FULL, the AD structures 'LE Bluetooth Device Address' and
 *        'LE Role' are required, so their presence is not configurable.
 *
 *        For message types @ref NFC_BLE_PAIR_MSG_BLUETOOTH_EP_SHORT and
 *        @ref NFC_BLE_PAIR_MSG_BLUETOOTH_EP_FULL, the AD structure 'Security Manager Out Of Band
 *        Flags' is required, so its presence is not configurable.
 * @{ */

/**
 * @brief Type of NFC NDEF message for BLE pairing. 
 * @details The NFC NDEF message type should be one of the types defined in
 *        @ref nfc_ndef_ble_pair_msg_types.
 */
#define NFC_BLE_PAIR_MSG_CONFIG_MSG_TYPE                     NFC_BLE_PAIR_MSG_BLUETOOTH_LE_SHORT

/**
 * @brief Configures a presence of the AD structure of type 'Security Manager TK value'.
 *
 * This is an optional parameter for all types of NFC NDEF messages for BLE pairing. It should be
 * present for Out-of-Band pairing over NFC. It is not needed for Just Works pairing over NFC.
 *
 * If the define is different than 0, static memory buffers that store a generated message are
 * extended to hold the 'Security Manager TK value' structure.
 */
#define NFC_BLE_PAIR_MSG_CONFIG_TK_VALUE_PRESENT             1
/**
 * @brief Configures a presence of the AD structure of type 'Appearance'.
 *
 * This is an optional parameter for all types of NFC NDEF messages for BLE pairing.
 *
 * If the define is different than 0, static memory buffers that store a generated message are
 * extended to hold the 'Appearance' structure.
 */
#define NFC_BLE_PAIR_MSG_CONFIG_APPEARANCE_PRESENT           1
/**
 * @brief Configures the maximum length of Bluetooth Local Name, which is part of the AD structure of
 *        type 'Local Name'.
 *
 * This is an optional parameter for all types of NFC NDEF messages for BLE pairing.
 *
 * If the define is different than 0, static memory buffers that store a generated message are
 * extended to hold the 'Local Name' structure.
 */
#define NFC_BLE_PAIR_MSG_CONFIG_LOCAL_NAME_MAX_LEN           16UL
/**
 * @brief Configures a presence of the AD structure of type 'Flags'.
 *
 * This is an optional parameter for message types @ref NFC_BLE_PAIR_MSG_BLUETOOTH_LE_SHORT and
 * @ref NFC_BLE_PAIR_MSG_BLUETOOTH_LE_FULL.
 *
 * If the define is different than 0, static memory buffers that store a generated message are
 * extended to hold the 'Flags' structure.
 */
#define NFC_BLE_PAIR_MSG_CONFIG_FLAGS_PRESENT                1

/** @} */

#endif // NFC_BLE_PAIR_MSG_CONFIG_H__

/** @} */
