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

#ifndef NFC_LAUNCHAPP_REC_H__
#define NFC_LAUNCHAPP_REC_H__

/**@file
 *
 * @defgroup nfc_launch_app_rec Launch app records
 * @{
 * @ingroup  nfc_launch_app_msg
 *
 * @brief    Generation of NFC NDEF record descriptions that launch apps.
 *
 */

#include <stdint.h>
#include "nfc_ndef_record.h"

/** @brief Function for generating a description of an NFC NDEF Android Application Record (AAR).
 *
 * This function declares and initializes a static instance of an NFC NDEF record description
 * of an Android Application Record (AAR).
 *
 * @note The record payload data (@p p_package_name) should be declared as 
 *       static. If it is declared as automatic, the NDEF message encoding 
 *       (see @ref nfc_ndef_msg_encode) must be done in the same variable 
 *       scope.
 *
 * @param[in]  p_package_name       Pointer to the Android package name string.
 * @param[in]  package_name_length  Length of the Android package name.
 *
 * @return Pointer to the description of the record.
 */
nfc_ndef_record_desc_t * nfc_android_application_rec_declare(uint8_t const * p_package_name,
                                                             uint8_t         package_name_length);

/** @brief Function for generating a description of an NFC NDEF Windows LaunchApp record.
 *
 * This function declares and initializes a static instance of an NFC NDEF record description
 * of a Windows LaunchApp record.
 *
 * @note The record payload data (@p p_win_app_id) should be declared as 
 *       static. If it is declared as automatic, the NDEF message encoding 
 *       (see @ref nfc_ndef_msg_encode) must be done in the same variable 
 *       scope.
 *
 * @param[in]  p_win_app_id         Pointer to the Windows application ID string (GUID).
 * @param[in]  win_app_id_length    Length of the Windows application ID.
 *
 * @return Pointer to the description of the record.
 */
nfc_ndef_record_desc_t * nfc_windows_launchapp_rec_declare(const uint8_t * p_win_app_id,
                                                           uint8_t         win_app_id_length);
/** @} */
#endif // NFC_LAUNCHAPP_REC
