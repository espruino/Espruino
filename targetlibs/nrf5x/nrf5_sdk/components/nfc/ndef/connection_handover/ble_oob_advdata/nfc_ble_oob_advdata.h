/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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

/** @file
 *
 * @defgroup nfc_ble_oob_advdata Advertising and Scan Response Data Encoder for NFC OOB pairing
 * @{
 * @ingroup nfc_ble_pair_msg
 * @brief Function for encoding data in the Advertising and Scan Response Data format, which
 *        can be used to create payload of NFC message intended for initiating the Out-of-Band
 *        pairing.
 */

#ifndef NFC_BLE_OOB_ADVDATA_H__
#define NFC_BLE_OOB_ADVDATA_H__

#include <stdint.h>
#include "ble_advdata.h"
#include "app_util.h"
#include "sdk_errors.h"


#define AD_TYPE_LE_ROLE_DATA_SIZE          1UL                                 /**< Data size (in octets) of the LE Bluetooth Device Address AD type. */
#define AD_TYPE_LE_ROLE_SIZE               (ADV_AD_DATA_OFFSET + \
                                            AD_TYPE_LE_ROLE_DATA_SIZE)         /**< Size (in octets) of the LE Bluetooth Device Address AD type. */
#define AD_TYPE_TK_VALUE_DATA_SIZE         (sizeof(ble_advdata_tk_value_t))    /**< Data size (in octets) of the Security Manager TK value AD type. */
#define AD_TYPE_TK_VALUE_SIZE              (ADV_AD_DATA_OFFSET + \
                                            AD_TYPE_TK_VALUE_DATA_SIZE)        /**< Size (in octets) of the Security Manager TK value AD type. */
#define AD_TYPE_OOB_FLAGS_DATA_SIZE        1UL                                 /**< Data size (in octets) of the Security Manager OOB Flags AD type. */
#define AD_TYPE_OOB_FLAGS_SIZE             (ADV_AD_DATA_OFFSET + \
                                            AD_TYPE_OOB_FLAGS_DATA_SIZE)       /**< Size (in octets) of the Security Manager OOB Flags AD type. */

#define AD_TYPE_SEC_MGR_OOB_FLAG_SET                   1U                      /**< Security Manager OOB Flag set. Flag selection is done using _POS defines */
#define AD_TYPE_SEC_MGR_OOB_FLAG_CLEAR                 0U                      /**< Security Manager OOB Flag clear. Flag selection is done using _POS defines */
#define AD_TYPE_SEC_MGR_OOB_FLAG_OOB_DATA_PRESENT_POS  0UL                     /**< Security Manager OOB Data Present Flag position. */
#define AD_TYPE_SEC_MGR_OOB_FLAG_OOB_LE_SUPPORTED_POS  1UL                     /**< Security Manager OOB Low Energy Supported Flag position. */
#define AD_TYPE_SEC_MGR_OOB_FLAG_SIM_LE_AND_EP_POS     2UL                     /**< Security Manager OOB Simultaneous LE and BR/EDR to Same Device Capable Flag position. */
#define AD_TYPE_SEC_MGR_OOB_ADDRESS_TYPE_PUBLIC        0UL                     /**< Security Manager OOB Public Address type. */
#define AD_TYPE_SEC_MGR_OOB_ADDRESS_TYPE_RANDOM        1UL                     /**< Security Manager OOB Random Address type. */
#define AD_TYPE_SEC_MGR_OOB_FLAG_ADDRESS_TYPE_POS      3UL                     /**< Security Manager OOB Address type Flag (0 = Public Address, 1 = Random Address) position. */

/**@brief Payload field values of LE Role BLE GAP AD Type. Corresponds with @ref ble_advdata_le_role_t enum. */
typedef enum
{
    NFC_BLE_ADVDATA_ROLE_ENCODED_ONLY_PERIPH = 0,                      /**< Only Peripheral Role supported. */
    NFC_BLE_ADVDATA_ROLE_ENCODED_ONLY_CENTRAL,                         /**< Only Central Role supported. */
    NFC_BLE_ADVDATA_ROLE_ENCODED_BOTH_PERIPH_PREFERRED,                /**< Peripheral and Central Role supported. Peripheral Role preferred for connection establishment. */
    NFC_BLE_ADVDATA_ROLE_ENCODED_BOTH_CENTRAL_PREFERRED                /**< Peripheral and Central Role supported. Central Role preferred for connection establishment */
} nfc_ble_advdata_le_role_encoded_t;

/**@brief Function for encoding data in the Advertising and Scan Response data format, which
 *        is used for NFC OOB pairing.
 *
 *
 * @details This function encodes data into the Advertising and Scan Response data format (AD structures).
 *          Encoding is based on the selections in the supplied structures. This function uses
 *          @ref adv_data_encode to encode regular data and adds addtional AD Structures which
 *          are specific for NFC OOB pairing: Security Manager TK Value, OOB Flags and LE Role.
 *
 * @param[in]      p_advdata       Pointer to the structure for specifying the content of encoded data.
 * @param[out]     p_encoded_data  Pointer to the buffer where encoded data will be returned.
 * @param[in,out]  p_len           \c in: Size of \p p_encoded_data buffer.
 *                                 \c out: Length of encoded data.
 *
 * @retval NRF_SUCCESS             If the operation was successful.
 * @retval NRF_ERROR_INVALID_PARAM If the operation failed because a wrong parameter was provided in \p p_advdata.
 * @retval NRF_ERROR_DATA_SIZE     If the operation failed because not all the requested data could fit into the
 *                                 provided buffer or some encoded AD structure is too long and its
 *                                 length cannot be encoded with one octet.
 */
ret_code_t nfc_ble_oob_adv_data_encode(ble_advdata_t const * const p_advdata,
                                       uint8_t             * const p_encoded_data,
                                       uint16_t            * const p_len);

#endif // NFC_BLE_OOB_ADVDATA_H__

/** @} */
