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

#include <string.h>
#include "nrf_error.h"
#include "app_util.h"
#include "nfc_ndef_record.h"

/**
 * @brief Type of description of payload of Windows LaunchApp record.
 */
typedef struct
{
    const uint8_t * platform;
    uint8_t         platform_length;
    const uint8_t * app_id;
    uint8_t         app_id_length;
} win_launchapp_payload_desc_t;

/** @brief Description of payload of Windows LaunchApp record. */
static win_launchapp_payload_desc_t win_launchapp_dsc;

static const uint8_t launchapp_type_str[] = {'a', 'n', 'd', 'r', 'o', 'i', 'd', '.', 'c', 'o', 'm',
                                             ':', 'p', 'k', 'g'};

static const uint8_t win_launchapp_type_str[] = {'w', 'i', 'n', 'd', 'o', 'w', 's', '.', 'c', 'o',
                                                 'm', '/', 'L', 'a', 'u', 'n', 'c', 'h', 'A', 'p',
                                                 'p'};

static const uint8_t win_phone_str[] = {'W', 'i', 'n', 'd', 'o', 'w', 's', 'P', 'h', 'o', 'n', 'e'};

nfc_ndef_record_desc_t * nfc_android_application_rec_declare(uint8_t const * p_package_name,
                                                             uint8_t         package_name_length)
{
    NFC_NDEF_RECORD_BIN_DATA_DEF(android_app_rec,
                                 TNF_EXTERNAL_TYPE,    // tnf <- external
                                 NULL, // no id
                                 0,    // no id
                                 launchapp_type_str,
                                 sizeof(launchapp_type_str),
                                 NULL,
                                 0);

    NFC_NDEF_BIN_PAYLOAD_DESC(android_app_rec).p_payload      = p_package_name;
    NFC_NDEF_BIN_PAYLOAD_DESC(android_app_rec).payload_length = package_name_length;

    return &NFC_NDEF_RECORD_BIN_DATA(android_app_rec);
}

#define WIN_LUNCHAPP_EMPTY_PARAMETER 0x20 ///< The empty parameter value for the Wndows LaunchApp Record.

/**
 * @brief Function for constructing the payload for a Windows LaunchApp Record.
 *
 * This function encodes the payload according to the LaunchApp record definition. It implements an API
 * compatible with p_payload_constructor_t.
 *
 * @param[in] p_input    Pointer to the description of the payload.
 * @param[out] p_buff    Pointer to payload destination. If NULL, the function will calculate
 *                       the expected size of the write based on the description.
 *
 * @param[in,out] p_len  Size of available memory to write as input. Size of generated
 *                       payload as output.
 *
 * @retval NRF_SUCCESS    Always success.
 */
static ret_code_t nfc_win_launchapp_payload_constructor(win_launchapp_payload_desc_t * p_input,
                                                        uint8_t                      * p_buff,
                                                        uint32_t                     * p_len)
{

    win_launchapp_payload_desc_t * launch_desc = (win_launchapp_payload_desc_t *) p_input;

    uint32_t temp_len = (uint32_t)launch_desc->platform_length + launch_desc->app_id_length + 7;
    
    if (temp_len > *p_len)
    {
        return NRF_ERROR_NO_MEM;
    }
    
    *p_len = temp_len;

    *p_buff++ = 0x00; // platform count: 1
    *p_buff++ = 0x01; // -||-

    *p_buff++ = launch_desc->platform_length;
    memcpy(p_buff, launch_desc->platform, launch_desc->platform_length); // platform
    p_buff += launch_desc->platform_length;


    *p_buff++ = launch_desc->app_id_length;
    memcpy(p_buff, launch_desc->app_id, launch_desc->app_id_length);
    p_buff += launch_desc->app_id_length;

    *p_buff++ = 0x00; // parameters length 1B
    *p_buff++ = 0x01; // -||-
    *p_buff++ = WIN_LUNCHAPP_EMPTY_PARAMETER; // empty parameter

    return NRF_SUCCESS;
}


nfc_ndef_record_desc_t * nfc_windows_launchapp_rec_declare(const uint8_t * p_win_app_id,
                                                           uint8_t         win_app_id_length)
{

    NFC_NDEF_GENERIC_RECORD_DESC_DEF(win_launchapp,
                                     TNF_ABSOLUTE_URI,
                                     NULL,
                                     0,
                                     win_launchapp_type_str,
                                     sizeof(win_launchapp_type_str),
                                     nfc_win_launchapp_payload_constructor,
                                     &win_launchapp_dsc);

    win_launchapp_dsc.platform        = win_phone_str;
    win_launchapp_dsc.platform_length = sizeof(win_phone_str);

    win_launchapp_dsc.app_id        = p_win_app_id;
    win_launchapp_dsc.app_id_length = win_app_id_length;

    return &NFC_NDEF_GENERIC_RECORD_DESC(win_launchapp);
}


