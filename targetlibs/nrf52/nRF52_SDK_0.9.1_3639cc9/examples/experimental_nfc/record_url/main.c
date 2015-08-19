/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup nfc_url_record_example_main main.c
 * @{
 * @ingroup nfc_url_record_example
 * @brief NFC URL record example application main file.
 *
 */

#include <nrf52.h>
#include <nrf52_bitfields.h>
#include <stdbool.h>
#include <stdint.h>
#include "nfc_lib.h"
#include "nfc_uri_msg.h"
#include "boards.h"
#include "nrf_error.h"
#include "app_error.h"

static char url[] =
    {0x6E, 0x6F, 0x72, 0x64, 0x69, 0x63, 0x73, 0x65, 0x6D, 0x69, 0x2E, 0x63, 0x6F, 0x6D}; //URL "nordicsemi.com"

/**
 * @brief Callback function for handling NFC events.
 */
void nfc_callback(void *context, NfcEvent event, const char *data, size_t dataLength)
{
    (void)context;

    switch (event)
    {
        case NFC_EVENT_FIELD_ON:
            LEDS_ON(BSP_LED_0_MASK);
            break;
        case NFC_EVENT_FIELD_OFF:
            LEDS_OFF(BSP_LED_0_MASK);
            break;
        default:
            break;
    }
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{
    uint32_t  err_code;
    uint8_t * p_nfc_msg;
    uint16_t  nfc_msg_len;
    NfcRetval ret_val;

    /* Configure LED-pins as outputs */
    LEDS_CONFIGURE(BSP_LED_0_MASK);
    LEDS_OFF(BSP_LED_0_MASK);

    /* Set up NFC */
    ret_val = nfcSetup(nfc_callback, NULL);
    if (ret_val != NFC_RETVAL_OK)
    {
        APP_ERROR_CHECK((uint32_t) ret_val);
    }

    /* Create NFC message */
    nfc_uri_id_t id_www = NFC_URI_HTTP_WWW;         /* Choose protocol for the URL */

    err_code = nfc_uri_msg_create(id_www,
                                  (uint8_t *) url,
                                  sizeof(url),
                                  &p_nfc_msg,
                                  &nfc_msg_len);
    APP_ERROR_CHECK(err_code);

    /* Set created message as the NFC payload */
    ret_val = nfcSetPayload((char *) p_nfc_msg, nfc_msg_len);
    if (ret_val != NFC_RETVAL_OK)
    {
        APP_ERROR_CHECK((uint32_t) ret_val);
    }

    /* Start sensing NFC field */
    ret_val = nfcStartEmulation();
    if (ret_val != NFC_RETVAL_OK)
    {
        APP_ERROR_CHECK((uint32_t) ret_val);
    }

    while(1){}
}

/** @} */
