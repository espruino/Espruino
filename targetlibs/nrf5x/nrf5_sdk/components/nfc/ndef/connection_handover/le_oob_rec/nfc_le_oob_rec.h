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

#ifndef NFC_LE_OOB_REC_H__
#define NFC_LE_OOB_REC_H__

/**@file
 *
 * @defgroup nfc_le_oob_rec LE OOB records
 * @{
 * @ingroup  nfc_ble_pair_msg
 *
 * @brief    Generation of NFC NDEF LE OOB records for NDEF messages.
 *
 */

#include <stdint.h>
#include "nfc_ndef_record.h"
#include "ble_advdata.h"

/** @brief Function for generating a description of an NFC NDEF Bluetooth Carrier Configuration LE Record.
 *
 * This function declares and initializes a static instance of an NFC NDEF record description
 * for a Bluetooth Carrier Configuration LE record.
 *
 * @note The record payload data (@p p_ble_advdata) should be declared as static. If it is 
 *       declared as automatic, the NDEF message encoding (see @ref nfc_ble_simplified_le_oob_msg_encode) 
 *       must be done in the same variable scope.
 *
 * @param[in]  rec_payload_id       NDEF record header Payload ID field (Limited to one byte). 
 *                                  If 0, no ID is present in the record description.
 * @param[in]  p_ble_advdata        Pointer to the encoded BLE advertising data structure. This
 *                                  data is used to create the record payload.
 *
 * @return Pointer to the description of the record.
 */
nfc_ndef_record_desc_t * nfc_le_oob_rec_declare(uint8_t                        rec_payload_id,
                                                ble_advdata_t    const * const p_ble_advdata);

/** @} */
#endif // NFC_LE_OOB_REC_H__
