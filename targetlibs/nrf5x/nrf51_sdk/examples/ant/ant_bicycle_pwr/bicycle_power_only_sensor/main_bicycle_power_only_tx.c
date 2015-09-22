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

#include "main_bicycle_power_only_tx.h"
#include "bicycle_power_only_tx.h"
#include "defines.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "nrf_soc.h"
#include "app_error.h"
#include "app_timer.h"
#include "app_button.h"
#include "boards.h"
#include "nordic_common.h"
#include "app_util.h"

#define ANTPLUS_NETWORK_NUMBER        0                                         /**< Network number. */
#define ANT_EVENT_MSG_BUFFER_MIN_SIZE 32u                                       /**< Minimum size of ANT event message buffer. */

#define BP_TX_CHANNEL_TYPE            CHANNEL_TYPE_MASTER                       /**< Master Channel. */
#define BP_TX_TRANS_TYPE              5u                                        /**< Transmission Type. */
#define BP_TX_DEVICE_NUMBER           1u                                        /**< Device Number. */
#define BP_EXT_ASSIGN                 0                                         /**< ANT Ext Assign. */
#define BP_DEVICE_TYPE                0x0Bu                                     /**< Bike Power Device Type. */
#define BP_RF_FREQ                    0x39u                                     /**< RF Channel 57 (2457 MHz). */
#define BP_MSG_PERIOD                 0x1FF6u                                   /**< Decimal 8182 (~4.00Hz). */
#define BP_NETWORK_KEY                {0, 0, 0, 0, 0, 0, 0, 0}                  /**< The network key used. */

#define CALIBRATION_RESPONSE_CODE     true                                      /**< Calibration response message status code. */
#define CALIBRATION_DATA              0x55AAu                                   /**< Calibration response message calibration data value. */

#define APP_TIMER_PRESCALER           0                                         /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS          1u                                        /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE       2u                                        /**< Size of timer operation queues. */

#define BUTTON_DETECTION_DELAY        APP_TIMER_TICKS(50u, APP_TIMER_PRESCALER) /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define EVT_ANT_STACK                 (1u << 0)                                 /**< Event for: ANT stack interrupt received. */
#define EVT_BUTTON_0                  (1u << 1u)                                /**< Event for: button 0 pressed. */
#define EVT_BUTTON_1                  (1u << 2u)                                /**< Event for: button 1 pressed. */

static volatile uint32_t m_event_flags   = 0;                                   /**< Event flags used for deferred event processing in application main loop context. */
static const uint8_t     m_network_key[] = BP_NETWORK_KEY;                      /**< ANT network key. */


/**@brief Function for configuration of the ANT channel.
 */
static __INLINE void bicycle_power_channel_open(void)
{
    uint32_t err_code;

    // Set Network Address.
    err_code = sd_ant_network_address_set(ANTPLUS_NETWORK_NUMBER, (uint8_t*)m_network_key);
    APP_ERROR_CHECK(err_code);

    // Set Channel Number.
    err_code = sd_ant_channel_assign(BP_TX_ANT_CHANNEL,
                                     BP_TX_CHANNEL_TYPE,
                                     ANTPLUS_NETWORK_NUMBER,
                                     BP_EXT_ASSIGN);
    APP_ERROR_CHECK(err_code);

    // Set Channel ID.
    err_code = sd_ant_channel_id_set(BP_TX_ANT_CHANNEL,
                                     BP_TX_DEVICE_NUMBER,
                                     BP_DEVICE_TYPE,
                                     BP_TX_TRANS_TYPE);
    APP_ERROR_CHECK(err_code);

    // Set Channel RF frequency.
    err_code = sd_ant_channel_radio_freq_set(BP_TX_ANT_CHANNEL, BP_RF_FREQ);
    APP_ERROR_CHECK(err_code);

    // Set Channel period.
    err_code = sd_ant_channel_period_set(BP_TX_ANT_CHANNEL, BP_MSG_PERIOD);
    APP_ERROR_CHECK(err_code);

    // Open Channel.
    err_code = sd_ant_channel_open(BP_TX_ANT_CHANNEL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for checking for possible pending profile event and process it if received.
 *
 * @param[in] antplus_event Profile event to be processed.
 */
static __INLINE void profile_event_handle(antplus_event_return_t antplus_event)
{
    uint32_t err_code;

    if (antplus_event.event == ANTPLUS_EVENT_CALIBRATION_REQUEST)
    {
        if (antplus_event.param1 == BP_CID_170)
        {
            err_code = bp_only_tx_calib_resp_transmit(CALIBRATION_RESPONSE_CODE,
                                                      CALIBRATION_DATA);
            APP_ERROR_CHECK(err_code);
        }
    }
}


/**@brief Function for Stack Interrupt handling.
 */
void SD_EVT_IRQHandler(void)
{
    m_event_flags |= EVT_ANT_STACK;
}


/**@brief Function for button event handling.
 *
 * @param[in] pin_no The pin number of the button pressed.
 */
void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    if (button_action == APP_BUTTON_PUSH)
    {
        switch (pin_no)
        {
            case BSP_BUTTON_0:
                m_event_flags |= EVT_BUTTON_0;
                break;

            case BSP_BUTTON_1:
                m_event_flags |= EVT_BUTTON_1;
                break;

            default:
                APP_ERROR_HANDLER(pin_no);
                break;
        }
    }
}


void bp_only_tx_main_loop_run(void)
{
    uint32_t err_code;

    // Open the ANT channel and initialize the profile module.
    bicycle_power_channel_open();
    err_code = bp_only_tx_initialize();
    APP_ERROR_CHECK(err_code);

    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, NULL);

    // Initialize and enable button handler module.
    static app_button_cfg_t buttons[] =
    {
        {BSP_BUTTON_0, false, BUTTON_PULL, button_event_handler},
        {BSP_BUTTON_1, false, BUTTON_PULL, button_event_handler}
    };

    APP_ERROR_CHECK(app_button_init(buttons,
                                    sizeof(buttons) / sizeof(buttons[0]),
                                    BUTTON_DETECTION_DELAY));

    APP_ERROR_CHECK(app_button_enable());

    uint8_t                ant_channel;
    uint8_t                event_message_buffer[ANT_EVENT_MSG_BUFFER_MIN_SIZE];
    uint32_t               local_flags;
    antplus_event_return_t antplus_event;

    uint8_t event = NO_EVENT;
    for (;;)
    {
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);

        // @note Use local copy of event flags instead of the original one in order to allow the
        // compiler more freedom to optimize the code as non-volatile variables do not need to be
        // fetched from memory before each access.
        local_flags = m_event_flags;
        do
        {
            if (local_flags & EVT_ANT_STACK)
            {
                m_event_flags &= ~EVT_ANT_STACK;

                // Extract and process all pending ANT stack events.
                while (sd_ant_event_get(&ant_channel, &event, event_message_buffer) == NRF_SUCCESS)
                {
                    err_code = bp_only_tx_channel_event_handle(event,
                                                               event_message_buffer,
                                                               &antplus_event);
                    APP_ERROR_CHECK(err_code);

                    profile_event_handle(antplus_event);
                }
            }
            if (local_flags & EVT_BUTTON_0)
            {
                m_event_flags &= ~EVT_BUTTON_0;

                err_code = bp_only_tx_power_decrement();
                APP_ERROR_CHECK(err_code);
            }
            if (local_flags & EVT_BUTTON_1)
            {
                m_event_flags &= ~EVT_BUTTON_1;

                err_code = bp_only_tx_power_increment();
                APP_ERROR_CHECK(err_code);
            }

            local_flags = m_event_flags;
        }
        while (local_flags != 0);
    }
}

/**
 *@}
 **/
