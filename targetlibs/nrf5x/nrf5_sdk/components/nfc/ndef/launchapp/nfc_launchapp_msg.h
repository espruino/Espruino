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

#ifndef NFC_LAUNCHAPP_MSG_H__
#define NFC_LAUNCHAPP_MSG_H__

/** @file
 *
 * @defgroup nfc_launch_app_msg Launch app messages
 * @{
 * @ingroup nfc_ndef_messages
 *
 * @brief Generation of NFC NDEF messages that can be used to launch apps.
 *
 */

#include <stdint.h>
#include "nfc_ndef_msg.h"


/** @brief Function for encoding an NFC NDEF launch app message.
 *
 * This function encodes an NFC NDEF message into a buffer.
 *
 * @param[in]  p_android_package_name       Pointer to the Android package name string.
 *                                          If NULL, the Android Application Record will be skipped.
 * @param[in]  android_package_name_length  Length of the Android package name.
 * @param[in]  p_win_app_id                 Pointer to the Windows application ID string (GUID).
 *                                          If NULL, the Windows LaunchApp record will be skipped.
 * @param[in]  win_app_id_length            Length of the Windows application ID.
 * @param[out] p_buf                        Pointer to the buffer for the message.
 * @param[in,out] p_len                     Size of the available memory for the message as input.
 *                                          Size of the generated message as output.
 *
 * @retval NRF_SUCCESS              If the description was successfully created.
 * @retval NRF_ERROR_INVALID_PARAM  If both p_android_package_name and windows_application_id were
 *                                  invalid (equal to NULL).
 * @retval NRF_ERROR_NO_MEM         If the predicted message size is bigger than the provided
 *                                  buffer space.
 * @retval Other                    Other codes might be returned depending on
 *                                  the function @ref nfc_ndef_msg_encode
 */
ret_code_t nfc_launchapp_msg_encode(uint8_t const * p_android_package_name,
                                    uint8_t         android_package_name_length,
                                    uint8_t const * p_win_app_id,
                                    uint8_t         win_app_id_length,
                                    uint8_t       * p_buf,
                                    uint32_t      * p_len);

/**
 * @}
 */
 #endif // NFC_LAUNCHAPP_MSG_H__


