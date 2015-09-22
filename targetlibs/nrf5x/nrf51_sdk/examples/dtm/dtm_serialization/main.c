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
 * @defgroup ble_sdk_app_dtm_main main.c
 * @{
 * @ingroup ble_sdk_app_dtm
 * @brief Serialized DTM Sample Application main file.
 *
 * This file contains the source code for an DTM activation example.
 */

#include <stdbool.h>
#include <stdint.h>
#include "app_button.h"
#include "app_timer.h"
#include "ble_advdata.h"
#include "ble_dtm_app.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "bsp.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0 /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define DTM_INIT_BUTTON_ID 0  /**< Button to initializing DTM mode on connectivity chip. */
#define DTM_TX_PIN_NO      9  /**< Pin used by DTM to transmitting a data. */
#define DTM_RX_PIN_NO      11 /**< Pin used by DTM to receiving a data. */

#define DEVICE_NAME                "Nordic_DTM" /**< Name of device. Will be included in the advertising data. */
#define APP_ADV_INTERVAL           64           /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS 180          /**< The advertising timeout (in units of seconds). */

#define APP_BUTTON_DETECTION_DELAY 100 /**< Delay of detecting button events. */

#define APP_TIMER_PRESCALER     0                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS    (4 + BSP_APP_TIMERS_NUMBER) /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE 4                           /**< Size of timer operation queues. */

#define DEAD_BEEF 0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

static ble_gap_adv_params_t m_adv_params; /**< Parameters to be passed to the stack when starting advertising. */


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    uint32_t                err_code;
    ble_advdata_t           advdata;
    ble_gap_conn_sec_mode_t sec_mode;
    uint8_t                 flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    // Set GAP parameters
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code =
        sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_WATCH);
    APP_ERROR_CHECK(err_code);

    // Build and set advertising data
    memset(&advdata, 0, sizeof (advdata));

    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = false;
    advdata.flags              = flags;

    err_code = ble_advdata_set(&advdata, NULL);
    APP_ERROR_CHECK(err_code);

    // Initialize advertising parameters (used when starting advertising)
    memset(&m_adv_params, 0, sizeof (m_adv_params));

    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    m_adv_params.p_peer_addr = NULL; // Undirected advertisement
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = APP_ADV_INTERVAL;
    m_adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;
}


/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_start(&m_adv_params);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);

    // err_code = bsp_buttons_enable(1 << DTM_INIT_BUTTON_ID);
    // APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the DTM mode.
 */
static void dtm_init(void)
{
    uint32_t                      err_code;
    app_uart_stream_comm_params_t uart_params;

    uart_params.baud_rate = UART_BAUD_RATE_19200;
    uart_params.rx_pin_no = DTM_RX_PIN_NO;
    uart_params.tx_pin_no = DTM_TX_PIN_NO;

    err_code = ble_dtm_init(&uart_params);

    if (err_code == NRF_SUCCESS)
    {
        err_code = bsp_indication_set(BSP_INDICATE_USER_STATE_OFF);
        APP_ERROR_CHECK(err_code);

        // Close serialization transport layer.
        err_code = sd_softdevice_disable();
        APP_ERROR_CHECK(err_code);

        // Enter to infinite loop.
        for (;;)
        {
            err_code = sd_app_evt_wait();
            APP_ERROR_CHECK(err_code);
        }
    }
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
static void bsp_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_KEY_0:
            dtm_init();
            break;

        default:
            break;
    }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);

    // Enable BLE stack
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof (ble_enable_params));
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code = NRF_SUCCESS;

    // Initialize.
    ble_stack_init();
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, NULL);

    err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                        APP_TIMER_TICKS(APP_BUTTON_DETECTION_DELAY, APP_TIMER_PRESCALER),
                        bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    // Initialize advertising.
    advertising_init();

    // Start execution.
    advertising_start();

    // Enter main loop.
    for (;;)
    {
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);
    }
}


/**
 * @}
 */
