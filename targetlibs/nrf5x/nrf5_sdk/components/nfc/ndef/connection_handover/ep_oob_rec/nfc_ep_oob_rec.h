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

#ifndef NFC_EP_OOB_REC_H__
#define NFC_EP_OOB_REC_H__

/**@file
 *
 * @defgroup nfc_ep_oob_rec EP OOB records
 * @{
 * @ingroup  nfc_ble_pair_msg
 *
 * @brief    Generation of NFC NDEF EP OOB records for NDEF messages.
 *
 */

#include <stdint.h>
#include "nfc_ndef_record.h"
#include "ble_advdata.h"

/** @brief Function for generating a description of an NFC NDEF Bluetooth Carrier Configuration EP record.
 *
 * This function declares and initializes a static instance of an NFC NDEF record description
 * for a Bluetooth Carrier Configuration EP record.
 *
 * @note The record payload data (@p p_ble_advdata) should be declared as static. If it is 
 *       declared as automatic, the NDEF message encoding (see @ref nfc_ble_simplified_ep_oob_msg_encode) 
 *       must be done in the same variable scope.
 *
 * @param[in]  rec_payload_id       NDEF record header Payload ID field (limited to one byte). 
 *                                  If 0, no ID is present in the record description.
 * @param[in]  p_ble_advdata        Pointer to the encoded BLE advertising data structure. This
 *                                  data is used to create the record payload.
 *
 * @return Pointer to the description of the record.
 */
nfc_ndef_record_desc_t * nfc_ep_oob_rec_declare(uint8_t                        rec_payload_id,
                                                ble_advdata_t    const * const p_ble_advdata);

/** @} */
#endif // NFC_EP_OOB_REC_H__
