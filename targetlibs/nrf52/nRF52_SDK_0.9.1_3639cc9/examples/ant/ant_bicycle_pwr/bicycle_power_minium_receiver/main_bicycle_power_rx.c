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

/**@file
 * @brief ANT bicycle power minimum receiver application configuration, setup, and main processing loop.
 * @defgroup ant_bicycle_power_minimum_receiver ANT bicycle power minimum receiver example
 * @{
 * @ingroup nrf_ant_bicycle_power
 *
 */

#include "main_bicycle_power_rx.h"
#if defined(TRACE_UART)
    #include <stdio.h>
#endif // defined(TRACE_UART)
#include "bicycle_power_rx.h"
#include "defines.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "app_error.h"
#include "nrf_soc.h"
#include "app_timer.h"
#include "app_button.h"
#include "app_util.h"
#include "bsp.h"
#include "nordic_common.h"

#define ANTPLUS_NETWORK_NUMBER           0                            /**< Network number. */
#define ANT_EVENT_MSG_BUFFER_MIN_SIZE    32u                          /**< Minimum size of ANT event message buffer. */

#define BP_RX_CHANNEL_TYPE               CHANNEL_TYPE_SLAVE           /**< Slave. */
#define BP_RX_TRANS_TYPE                 0                            /**< Transmission Type. */
#define BP_RX_DEVICE_NUMBER              0                            /**< Device Number. */
#define BP_EXT_ASSIGN                    0                            /**< ANT Ext Assign. */
#define BP_DEVICE_TYPE                   0x0Bu                        /**< Bike Power Device Type. */
#define BP_RF_FREQ                       0x39u                        /**< RF Channel 57 (2457 MHz). */
#define BP_MSG_PERIOD                    0x1FF6u                      /**< Decimal 8182 (~4.00Hz). */
#define BP_NETWORK_KEY                   {0, 0, 0, 0, 0, 0, 0, 0}     /**< The network key used. */

#define APP_TIMER_MAX_TIMERS             (2u + BSP_APP_TIMERS_NUMBER) /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE          2u                           /**< Size of timer operation queues. */

#define EVT_BUTTON_0                     (1u << 0)                    /**< Event for: button 0 pressed. */
#define EVT_CALIBRATION_RESPONSE_TIMEOUT (1u << 1u)                   /**< Event for: calibration response timeout. */
#define EVT_ANT_STACK                    (1u << 2u)                   /**< Event for: ANT stack interrupt received. */

static const uint8_t     m_network_key[] = BP_NETWORK_KEY;            /**< ANT network key. */
static volatile uint32_t m_event_flags   = 0;                         /**< Event flags used for deferred event processing in application main loop context. */
static volatile bool     m_calibration_not_active;

static void calibration_process_cb_handle(calibration_notif_event_t event);


/**@brief Function for handling user button presses.
 *
 * This function starts the calibration process that is triggered when a user presses a button.
 * Disable the app_button module until the calibration process has been finished.
 */
static __INLINE void button_press_handle(void)
{
#if defined(TRACE_CALIBRATION)
    printf("start calibration process\n");
#endif // defined(TRACE_CALIBRATION)

    m_calibration_not_active = false;

    // Start the calibration process.
    bp_rx_calibration_start();
}


/**@brief Function for configuring the ANT channel.
 */
static __INLINE void bicycle_power_channel_open(void)
{
    uint32_t err_code;

    // Set network address.
    err_code = sd_ant_network_address_set(ANTPLUS_NETWORK_NUMBER, (uint8_t*)m_network_key);
    APP_ERROR_CHECK(err_code);

    // Set channel number.
    err_code = sd_ant_channel_assign(BP_RX_ANT_CHANNEL,
                                     CHANNEL_TYPE_SLAVE,
                                     ANTPLUS_NETWORK_NUMBER,
                                     BP_EXT_ASSIGN);
    APP_ERROR_CHECK(err_code);

    // Set channel ID.
    err_code = sd_ant_channel_id_set(BP_RX_ANT_CHANNEL,
                                     BP_RX_DEVICE_NUMBER,
                                     BP_DEVICE_TYPE,
                                     BP_RX_TRANS_TYPE);
    APP_ERROR_CHECK(err_code);

    // Set channel RF frequency.
    err_code = sd_ant_channel_radio_freq_set(BP_RX_ANT_CHANNEL, BP_RF_FREQ);
    APP_ERROR_CHECK(err_code);

    // Set channel period.
    err_code = sd_ant_channel_period_set(BP_RX_ANT_CHANNEL, BP_MSG_PERIOD);
    APP_ERROR_CHECK(err_code);

    // Open channel.
    err_code = sd_ant_channel_open(BP_RX_ANT_CHANNEL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for processing received profile page events.
 *
 * @param[in] p_event_return Profile page event to be processed.
 */
static __INLINE void profile_event_page_handle(const antplus_event_return_t * const p_event_return)
{
#if defined(TRACE_DATA_PAGE)
    const bp_page16_data_t * p_page16_data;
    const bp_page17_data_t * p_page17_data;
    const bp_page18_data_t * p_page18_data;
    const bp_page32_data_t * p_page32_data;
    const page80_data_t    * p_page80_data;
    const page81_data_t    * p_page81_data;
#endif // defined(TRACE_DATA_PAGE)
#if defined(TRACE_CALIBRATION)
    const bp_page1_response_data_t * p_page1_general_response_data;
#endif // defined(TRACE_CALIBRATION)

    switch (p_event_return->param1)
    {
        case BP_PAGE_1:
            // Calibration message main data page.
#if defined(TRACE_CALIBRATION)
            p_page1_general_response_data = bp_rx_data_page1_response_get();

            printf("page1:calibration_id %#x\n",
                   (uint8_t)p_page1_general_response_data->calibration_id);
            printf("page1:auto_zero_status %u\n",
                   (uint8_t)p_page1_general_response_data->auto_zero_status);
            printf("page1:calibration_data %i\n",
                   (uint16_t)p_page1_general_response_data->calibration_data);
#endif // defined(TRACE_CALIBRATION)
            break;

        case BP_PAGE_16:
            // Standard power only page.
#if defined(TRACE_DATA_PAGE)
            p_page16_data = bp_rx_data_page16_get();

            printf("Page16:event count %u\n", (unsigned int)p_page16_data->event_count);
            printf("Page16:pedal power %u\n", (unsigned int)p_page16_data->pedal_power);
            printf("Page16:instantaneous cadence %u\n", (unsigned int)p_page16_data->instantaneous_cadence);
            printf("Page16:accumulated power %u\n", (unsigned int)p_page16_data->accumulated_power);
            printf("Page16:instantaneous power %u\n", (unsigned int)p_page16_data->instantaneous_power);
#endif // defined(TRACE_DATA_PAGE)     
            break;

        case BP_PAGE_17:
            // Wheel Torque (WT) main data page.
#if defined(TRACE_DATA_PAGE)
            p_page17_data = bp_rx_data_page17_get();

            printf("Page17:update_event_counter %u\n", (unsigned int)p_page17_data->update_event_counter);
            printf("Page17:wheel_ticks %u\n", (unsigned int)p_page17_data->wheel_ticks);
            printf("Page17:instantaneous_cadence %u\n", (unsigned int)p_page17_data->instantaneous_cadence);
            printf("Page17:accumulated wheel_period %u\n", (unsigned int)p_page17_data->wheel_period);
            printf("Page17:accumulated_torgue %u\n", (unsigned int)p_page17_data->accumulated_torgue);
#endif // defined(TRACE_DATA_PAGE)                     
            break;

        case BP_PAGE_18:
            // Standard Crank Torque (CT) main data page.
#if defined(TRACE_DATA_PAGE)
            p_page18_data = bp_rx_data_page18_get();

            printf("Page18:update_event_counter %u\n", (unsigned int)p_page18_data->update_event_counter);
            printf("Page18:crank_ticks %u\n", (unsigned int)p_page18_data->crank_ticks);
            printf("Page18:instantaneous_cadence %u\n", (unsigned int)p_page18_data->instantaneous_cadence);
            printf("Page18:accumulated crank_period %u\n", (unsigned int)p_page18_data->crank_period);
            printf("Page18:accumulated_torgue %u\n", (unsigned int)p_page18_data->accumulated_torgue);
#endif // defined(TRACE_DATA_PAGE)
            break;

        case BP_PAGE_32:
            // Standard Crank Torque Frequency (CTF) main data page.
#if defined(TRACE_DATA_PAGE)
            p_page32_data = bp_rx_data_page32_get();

            printf("Page32:event_counter %u\n", (unsigned int)p_page32_data->update_event_counter);
            printf("Page32:slope %u\n", (unsigned int)p_page32_data->slope);
            printf("Page32:time_stamp %u\n", (unsigned int)p_page32_data->time_stamp);
            printf("Page32:torque_ticks_stamp %u\n", (unsigned int)p_page32_data->torque_ticks_stamp);
            printf("Page32:average_cadence %u\n", (unsigned int)p_page32_data->average_cadence);
#endif // defined(TRACE_DATA_PAGE)
            break;

        case COMMON_PAGE_80:
            // Manufacturer's identification common data page.
#if defined(TRACE_DATA_PAGE)
            p_page80_data = bp_rx_data_page80_get();

            printf("Page80:hw_revision %u\n", (unsigned int)p_page80_data->hw_revision);
            printf("Page80:manufacturing_id %u\n", (unsigned int)p_page80_data->manufacturing_id);
            printf("Page80:model_number %u\n", (unsigned int)p_page80_data->model_number);
#endif // defined(TRACE_DATA_PAGE)
            break;

        case COMMON_PAGE_81:
            // Product information common data page.
#if defined(TRACE_DATA_PAGE)
            p_page81_data = bp_rx_data_page81_get();

            printf("Page81:sw_revision %u\n", (unsigned int)p_page81_data->sw_revision);
            printf("Page81:serial_number %u\n", (unsigned int)p_page81_data->serial_number);
#endif // defined(TRACE_DATA_PAGE)
            break;

        default:
            APP_ERROR_HANDLER(p_event_return->param1);
            break;
    }
}


/**@brief Function for processing received profile events.
 *
 * @param[in] p_event_return Profile event to be processed.
 */
static __INLINE void profile_event_handle(const antplus_event_return_t * const p_event_return)
{
    switch (p_event_return->event)
    {
        case ANTPLUS_EVENT_PAGE:
            profile_event_page_handle(p_event_return);
            break;

        default:
            APP_ERROR_HANDLER(p_event_return->event);
            break;
    }
}

/**@brief Function for handling button events.
 *
 * @param[in]   event   Event generated by button pressed.
 */
static void button_event_handler(bsp_event_t event)
{
    switch (event)
    {
        case BSP_EVENT_KEY_0:
            if (m_calibration_not_active)
            {
                m_event_flags |= EVT_BUTTON_0;
            }
            break;
        default:
            break;
    }
}

void bicycle_power_rx_main_loop_run(void)
{
    uint32_t err_code;

    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, NULL);

    err_code = bsp_init(BSP_INIT_BUTTONS, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), button_event_handler);
    APP_ERROR_CHECK(err_code);

    m_calibration_not_active = true;

    // Open ANT channel.
    bicycle_power_channel_open();

    bicycle_power_rx_init();

    bp_rx_calibration_cb_register(calibration_process_cb_handle);

    uint8_t                ant_channel;
    uint8_t                event_message_buffer[ANT_EVENT_MSG_BUFFER_MIN_SIZE];
    uint32_t               events;
    antplus_event_return_t antplus_event;

    // @note: When it is passed as a parameter to methods, the 32 bit type is used for events
    //        instead of the API defined 8 bit type due to performance reasons.
    uint32_t event = NO_EVENT;
    for (;;)
    {
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);

        // @note: Use local copy of event flags instead of the original one in order to allow the
        //        compiler more freedom to optimize the code as non-volatile variables do not need
        //        to be fetched from memory before each access.
        events = m_event_flags;
        while (events != 0)
        {
            // Check for button state event and execute handling if needed.
            if (events & EVT_BUTTON_0)
            {
                m_event_flags &= ~EVT_BUTTON_0;

                button_press_handle();
            }
            // Check for ANT stack IRQ event and execute handling if needed.
            if (events & EVT_ANT_STACK)
            {
                m_event_flags &= ~EVT_ANT_STACK;

                while (sd_ant_event_get(&ant_channel, (uint8_t*)&event, event_message_buffer)
                       == NRF_SUCCESS)
                {
                    if (bp_rx_channel_event_handle(event, event_message_buffer, &antplus_event))
                    {
                        // We have a pending profile -> application event to be processed.
                        profile_event_handle(&antplus_event);
                    }
                }
            }
            // Check for calibration response timeout event and execute handling if needed.
            if (events & EVT_CALIBRATION_RESPONSE_TIMEOUT)
            {
                m_event_flags &= ~EVT_CALIBRATION_RESPONSE_TIMEOUT;
                bp_rx_calibration_tout_handle();
            }
            // Take a fresh snapshot of possible new events, which require processing.
            events = m_event_flags;
        }
    }
}


/**@brief Function for handling stack interrupts.
 */
void SD_EVT_IRQHandler(void)
{
    m_event_flags |= EVT_ANT_STACK;
}


/**@brief Function for handling calibration process statemachine callback events.
 *
 * @param[in] event The event to be handled.
 */
static void calibration_process_cb_handle(calibration_notif_event_t event)
{
    switch (event)
    {
        case CALIBRATION_NOT_ACTIVE_STATE_ENTER:
            m_calibration_not_active = true;
#if defined(TRACE_CALIBRATION)
            printf("calibration process event: not active enter\n");
#endif // defined(TRACE_CALIBRATION)
            break;

        case CALIBRATION_RESPONSE_TIMEOUT:
            // Enable handling of calibration response timeout
            // within the main processing loop context.
            m_event_flags |= EVT_CALIBRATION_RESPONSE_TIMEOUT;
#if defined(TRACE_CALIBRATION)
            printf("calibration response timeout\n");
#endif // defined(TRACE_CALIBRATION)
            break;

        default:
            APP_ERROR_HANDLER(event);
            break;
    }
}


/**
 *@}
 **/
