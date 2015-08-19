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

#include "bicycle_power_rx.h"
#include <stdio.h>
#include "bp_pages.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "app_error.h"
#include "app_timer.h"
#include "nordic_common.h"
#include "app_util.h"

#define BUFFER_INDEX_MESG_ID               1u                                                  /**< Index for Message ID. */
#define BUFFER_INDEX_MESG_DATA             3u                                                  /**< Index for Data. */

#define TX_BUFFER_SIZE                     8u                                                  /**< Size of the calibration request transmit buffer. */

#define CALIBRATION_TIMEOUT_TICKS          APP_TIMER_TICKS(5000u, APP_TIMER_PRESCALER)         /**< Calibration response timeout value in units of timer ticks. */

#define CTF_AVERAGE_CADENCE_SHIFT_MAGIC    6u                                                  /**< CTF average cadence calculation bit shift magic number. */
#define CTF_AVERAGE_CADENCE_MULTIPLY_MAGIC 1875u                                               /**< CTF average cadence calculation multiply magic number. */
#define CTF_AVERAGE_CADENCE_ZERO_THRESHOLD 12u                                                 /**< Threshold, which when reached, will set the average cadence to 0. */

// Calibration process statemachine state definitons.
typedef enum
{
    CALIBRATION_NOT_ACTIVE, /**< Calibration process not running. */
    CALIBRATION_ACTIVE      /**< Calibration process running. */
} calibration_process_state_t;

static app_timer_id_t m_timer_id;                                                              /**< Timer ID used with the timer module. */

static bp_page16_data_t         m_page16_data;                                                 /**< Standard Power Only data page. */
static bp_page17_data_t         m_page17_data;                                                 /**< Wheel Torque (WT) main data page. */
static bp_page18_data_t         m_page18_data;                                                 /**< Standard Crank Torque (CT) main data page. */
static bp_page32_data_t         m_page32_data;                                                 /**< Standard Crank Torque Frequency (CTF) main data page. */
static bp_page1_response_data_t m_page1_general_response_data;                                 /**< Page 1 general calibration response data structure. */

static page80_data_t m_page80_data;                                                            /**< Manufacturer's identification common data page. */
static page81_data_t m_page81_data;                                                            /**< Product information common data page. */

static calibration_process_state_t    m_calibration_process_state    = CALIBRATION_NOT_ACTIVE; /**< Calibration process statemachine state variable. */
static calibration_process_callback_t m_calibration_process_callback = NULL;                   /**< Calibration process statemachine callback client. */


/**@brief Function for handling a timer callback event.
 *
 * The calibration handling process has timed out, meaning a calibration response message from the
 * sensor has not been received. The event is forwarded to the application for further processing.
 *
 * @param[in] p_context The callback context.
 */
void timer_callback_handle(void * p_context)
{
    APP_ERROR_CHECK_BOOL(m_calibration_process_callback != NULL);
    m_calibration_process_callback(CALIBRATION_RESPONSE_TIMEOUT);
}


void bicycle_power_rx_init(void)
{
    const uint32_t err_code = app_timer_create(&m_timer_id,
                                               APP_TIMER_MODE_SINGLE_SHOT,
                                               timer_callback_handle);
    APP_ERROR_CHECK(err_code);
}


void bp_rx_calibration_cb_register(calibration_process_callback_t callback)
{
    m_calibration_process_callback = callback;
}


/** @brief Function for executing method for entering calibration not active state.
 *
 * Execute method for entering calibration not active state, meaning notifying the application of
 * the state entry event. Additionally, if the timeout handler is running, it will be stopped.
 */
static __INLINE void calibration_not_active_execute(void)
{
    const uint32_t err_code = app_timer_stop(m_timer_id);

    APP_ERROR_CHECK(err_code);

    APP_ERROR_CHECK_BOOL(m_calibration_process_callback != NULL);
    m_calibration_process_callback(CALIBRATION_NOT_ACTIVE_STATE_ENTER);
}


/**@brief Function for transitioning to the next calibration state and executing its entry method.
 *
 * @param[in] new_state The next calibration process state transit to.
 */
static void calibration_state_change(calibration_process_state_t new_state)
{
    m_calibration_process_state = new_state;

    switch (m_calibration_process_state)
    {
        case CALIBRATION_NOT_ACTIVE:
            calibration_not_active_execute();
            break;

        case CALIBRATION_ACTIVE:
        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for decoding a received calibration data page from the ANT channel.
 *
 * This function decodes the received calibration data page from the ANT channel.
 * Only Calibration IDs 0xAC and 0xAF are supported by the implementation.
 *
 * @param[in] p_event_message_buffer ANT event message buffer.
 *
 * @return true if message contained valid data, false otherwise.
 */
static __INLINE bool calibration_data_page_decode(uint8_t const * const p_event_message_buffer)
{
    bool return_code = false;

    // We only process calibration messages when in correct state.
    if (m_calibration_process_state == CALIBRATION_ACTIVE)
    {
        bp_page1_response_data_t * p_page1_general_response_data;

        const uint32_t calibration_id = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 1u];

        switch (calibration_id)
        {
            case BP_CID_172:
                // Calibration ID 0xAC = calibration response manual zero success.
            case BP_CID_175:
                // Calibration ID 0xAF = calibration response failure.
                p_page1_general_response_data = bp_rx_data_page1_response_get();

                p_page1_general_response_data->calibration_id = calibration_id;
                p_page1_general_response_data->auto_zero_status
                    = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 2u];
                // Reset to zero prior bit-masking.
                p_page1_general_response_data->calibration_data = 0;
                // LSB.
                p_page1_general_response_data->calibration_data
                    = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 6u];
                // MSB.
                p_page1_general_response_data->calibration_data
                    |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 7u]) << 8u;

                return_code = true;
                break;

            default:
                // No implementation needed.
                break;
        }

        calibration_state_change(CALIBRATION_NOT_ACTIVE);
    }

    return return_code;
}


/**@brief Function for decoding received data page from the ANT channel.
 *
 * @param[in] p_event_message_buffer ANT event message buffer.
 * @param[out] p_event_return        Output data from the profile.
 *
 * @return true if p_event_return contains valid data, false otherwise.
 */
static __INLINE bool rx_data_page_decode(uint8_t const * const    p_event_message_buffer,
                                         antplus_event_return_t * p_event_return)
{
    // Variables required by the CTF cadence calculation.
    static uint32_t ctf_previous_event_count    = 0;
    static uint32_t ctf_previous_time_stamp     = 0;
    static uint32_t ctf_zero_speed_counter      = 0;
    static bool     ctf_first_page_check_active = true;
    uint32_t        ctf_event_count_difference;
    uint32_t        ctf_time_stamp_difference;

    // References to data pages.
    bp_page16_data_t * p_page16_data;
    bp_page17_data_t * p_page17_data;
    bp_page18_data_t * p_page18_data;
    bp_page32_data_t * p_page32_data;
    page80_data_t    * p_page80_data;
    page81_data_t    * p_page81_data;

    bool           return_code = true;
    const uint32_t page_id     = p_event_message_buffer[BUFFER_INDEX_MESG_DATA];

    p_event_return->event  = ANTPLUS_EVENT_PAGE;
    p_event_return->param1 = page_id;
    switch (page_id)
    {
        case BP_PAGE_1:
            // Calibration message main data page.
            return_code = calibration_data_page_decode(p_event_message_buffer);
            break;

        case BP_PAGE_16:
            // Standard power only page.
            p_page16_data = bp_rx_data_page16_get();

            p_page16_data->event_count = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 1u];
            p_page16_data->pedal_power = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 2u];
            p_page16_data->instantaneous_cadence
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 3u];
            // Reset to zero prior bit-masking.
            p_page16_data->accumulated_power = 0;
            // LSB.
            p_page16_data->accumulated_power
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 4u];
            // MSB.
            p_page16_data->accumulated_power
                |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 5u]) << 8u;
            // Reset to zero prior bit-masking.
            p_page16_data->instantaneous_power = 0;
            // LSB.
            p_page16_data->instantaneous_power
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 6u];
            // MSB.
            p_page16_data->instantaneous_power
                |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 7u]) << 8u;
            break;

        case BP_PAGE_17:
            // Wheel Torque (WT) main data page.
            p_page17_data = bp_rx_data_page17_get();

            p_page17_data->update_event_counter
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 1u];
            p_page17_data->wheel_ticks
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 2u];
            p_page17_data->instantaneous_cadence
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 3u];
            // Reset to zero prior bit-masking.
            p_page17_data->wheel_period = 0;
            // LSB.
            p_page17_data->wheel_period
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 4u];
            // MSB.
            p_page17_data->wheel_period
                |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 5u]) << 8u;
            // LSB.
            p_page17_data->accumulated_torgue
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 6u];
            // MSB.
            p_page17_data->accumulated_torgue
                |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 7u]) << 8u;
            break;

        case BP_PAGE_18:
            // Standard Crank Torque (CT) main data page.
            p_page18_data = bp_rx_data_page18_get();

            p_page18_data->update_event_counter
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 1u];
            p_page18_data->crank_ticks
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 2u];
            p_page18_data->instantaneous_cadence
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 3u];
            // Reset to zero prior bit-masking.
            p_page18_data->crank_period = 0;
            // LSB.
            p_page18_data->crank_period
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 4u];
            // MSB.
            p_page18_data->crank_period
                |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 5u]) << 8u;
            // LSB.
            p_page18_data->accumulated_torgue
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 6u];
            // MSB.
            p_page18_data->accumulated_torgue
                |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 7u]) << 8u;
            break;

        case BP_PAGE_32:
            // Standard Crank Torque Frequency (CTF) main data page.
            // @note: CTF messages are big endian. The byte order for these messages is reversed
            //        with respect to standard ANT+ messages. 

            p_page32_data = bp_rx_data_page32_get();

            p_page32_data->update_event_counter
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 1u];
            // Reset to zero prior bit-masking.
            p_page32_data->slope = 0;
            // MSB.
            p_page32_data->slope
                = (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 2u]) << 8u;
            // LSB.
            p_page32_data->slope
                |= p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 3u];
            // Reset to zero prior bit-masking.
            p_page32_data->time_stamp = 0;
            // MSB.
            p_page32_data->time_stamp
                = (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 4u]) << 8u;
            // LSB.
            p_page32_data->time_stamp
                |= p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 5u];
            // Reset to zero prior bit-masking.
            p_page32_data->torque_ticks_stamp = 0;
            // MSB.
            p_page32_data->torque_ticks_stamp
                = (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 6u]) << 8u;

            p_page32_data->torque_ticks_stamp
                |= p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 7u];

            // Calculate average cadence based on received data, if feasible.

            // Store N - (N-1) difference, where N is the current received event count.
            ctf_event_count_difference
                = (p_page32_data->update_event_counter - ctf_previous_event_count)
                  & UINT8_MAX;
            // Store N - (N-1) difference, where N is time of most recent received rotation event.
            ctf_time_stamp_difference
                = (p_page32_data->time_stamp - ctf_previous_time_stamp) & UINT16_MAX;

            if (!ctf_first_page_check_active)  // This is NOT the 1st received CTF page.
            {
                if (ctf_event_count_difference == 0)  // No new cadence event exist.
                {
                    ++ctf_zero_speed_counter;
                    if (ctf_zero_speed_counter == CTF_AVERAGE_CADENCE_ZERO_THRESHOLD)
                    {
                        p_page32_data->average_cadence = 0; // Zero-speed threshold reached.
                    }
                }
                else                                    // New cadence event exists.
                {
                    ctf_zero_speed_counter = 0;         // Reset zero-speed check counter.
                    if (ctf_time_stamp_difference != 0) // Time stamp difference exists
                    {
                        // Calculate new average cadence based on the received data.
                        p_page32_data->average_cadence =
                            ((ctf_event_count_difference << CTF_AVERAGE_CADENCE_SHIFT_MAGIC)
                             * CTF_AVERAGE_CADENCE_MULTIPLY_MAGIC) / ctf_time_stamp_difference;

                        // Update previous value counters to be ready for the next received message.
                        ctf_previous_event_count = p_page32_data->update_event_counter;
                        ctf_previous_time_stamp  = p_page32_data->time_stamp;
                    }
                    else // Time stamp difference does not exist.
                    {
                        // Reaching this execution path would imply a broken sensor implementation
                        // as having event count difference without time stamp difference. This
                        // will be silently discarded.
                    }
                }
            }
            else
            // This is the 1st received CTF page - no calculations are possible.
            {
                // Disable 1st received CTF page check.
                ctf_first_page_check_active = false;

                // Update previous value counters to be ready for the next received message.
                ctf_previous_event_count = p_page32_data->update_event_counter;
                ctf_previous_time_stamp  = p_page32_data->time_stamp;
            }
            break;

        case COMMON_PAGE_80:
            // Manufacturer's identification common data page.
            p_page80_data = bp_rx_data_page80_get();

            p_page80_data->hw_revision
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 3u];
            // LSB.
            p_page80_data->manufacturing_id
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 4u];
            // MSB.
            p_page80_data->manufacturing_id
                |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 5u]) << 8u;
            // LSB.
            p_page80_data->model_number
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 6u];
            // MSB.
            p_page80_data->model_number
                |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 7u]) << 8u;
            break;

        case COMMON_PAGE_81:
            // Product information common data page.
            p_page81_data = bp_rx_data_page81_get();

            p_page81_data->sw_revision
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 3u];
            // Bits 0 - 7.
            p_page81_data->serial_number
                = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 4u];
            // Bits 8 - 15.
            p_page81_data->serial_number
                |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 5u]) << 8u;
            // Bits 16 - 23.
            p_page81_data->serial_number
                |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 6u]) << 16u;
            // Bits 24 - 31.
            p_page81_data->serial_number
                |= (p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 7u]) << 24u;
            break;

        default:
            return_code = false;
            break;
    }

    return return_code;
}


/**@brief Function for processing received data (for example broadcast, acknowledge, burst) from
 * the ANT channel.
 *
 * @param[in] p_event_message_buffer ANT event message buffer.
 * @param[out] p_event_return        Output data from the profile.
 *
 * @return true if p_event_return contains valid data, false otherwise.
 */
static __INLINE bool rx_data_handle(uint8_t const * const    p_event_message_buffer,
                                    antplus_event_return_t * p_event_return)
{
    bool           return_code = false;
    const uint32_t message_id  = p_event_message_buffer[BUFFER_INDEX_MESG_ID];

    switch (message_id)
    {
        // Handle both BROADCAST, ACKNOWLEDGED and BURST data the same.
        case MESG_BROADCAST_DATA_ID:
        case MESG_ACKNOWLEDGED_DATA_ID:
        case MESG_BURST_DATA_ID:
            return_code = rx_data_page_decode(p_event_message_buffer, p_event_return);
            break;

        default:
            APP_ERROR_HANDLER(message_id);
            break;
    }

    return return_code;
}


/** @brief Function for transmitting calibration request message.
 *
 * @return The ant_acknowledge_message_tx method return code.
 */
static uint32_t calibration_request_transmit(void)
{
    static uint8_t tx_buffer[TX_BUFFER_SIZE];

    // Format the manual calibration request.
    tx_buffer[0] = BP_PAGE_1;
    tx_buffer[1] = BP_CID_170;
    tx_buffer[2] = BP_PAGE_RESERVE_BYTE;
    tx_buffer[3] = BP_PAGE_RESERVE_BYTE;
    tx_buffer[4] = BP_PAGE_RESERVE_BYTE;
    tx_buffer[5] = BP_PAGE_RESERVE_BYTE;
    tx_buffer[6] = BP_PAGE_RESERVE_BYTE;
    tx_buffer[7] = BP_PAGE_RESERVE_BYTE;

    return sd_ant_acknowledge_message_tx(BP_RX_ANT_CHANNEL, sizeof(tx_buffer), tx_buffer);
}


bool bp_rx_channel_event_handle(uint32_t                 event,
                                uint8_t const * const    p_event_message_buffer,
                                antplus_event_return_t * p_event_return)
{
    bool return_code = false;

    switch (event)
    {
        case EVENT_RX:
            return_code = rx_data_handle(p_event_message_buffer, p_event_return);
            break;

        case EVENT_TRANSFER_TX_FAILED:
            // Failed to receive acknowledgement to the calibration request message.

            // Retry only when in correct state.
            if (m_calibration_process_state == CALIBRATION_ACTIVE)
            {
                // @note: We will eventually exit the current state due to timeout, and we will
                //        always do a retransmit on EVENT_TRANSFER_TX_FAILED. Therefore, the return
                //        value of the calibration_request_transmit function is irrelevant.
                UNUSED_VARIABLE(calibration_request_transmit());
            }
            break;

        default:
            // No implementation needed.
            break;

    }

    return return_code;
}


void bp_rx_calibration_start(void)
{
    // @note: If this method is called when the calibration process is running, the end result will
    //        be undefined.

    if (m_calibration_process_state == CALIBRATION_ACTIVE)
    {
        APP_ERROR_HANDLER(m_calibration_process_state);
    }

    if (calibration_request_transmit() == NRF_SUCCESS)
    {
        // Calibration request transmit success, start the response timer and transit to next state.
        const uint32_t err_code = app_timer_start(m_timer_id, CALIBRATION_TIMEOUT_TICKS, NULL);
        APP_ERROR_CHECK(err_code);

        calibration_state_change(CALIBRATION_ACTIVE);
    }
    else
    {
        // Calibration request transmit failure, self transition.
        calibration_state_change(CALIBRATION_NOT_ACTIVE);
    }
}


void bp_rx_calibration_tout_handle(void)
{
    switch (m_calibration_process_state)
    {
        case CALIBRATION_ACTIVE:
            calibration_state_change(CALIBRATION_NOT_ACTIVE);
            break;

        case CALIBRATION_NOT_ACTIVE:
            // It is possible, that we receive this event after transiting to CALIBRATION_NOT_ACTIVE
            // state, thus we will silently discard it.
            break;

        default:
            APP_ERROR_HANDLER(m_calibration_process_state); // This should not happen.
            break;

    }
}


bp_page16_data_t * bp_rx_data_page16_get(void)
{
    return &(m_page16_data);
}


bp_page17_data_t * bp_rx_data_page17_get(void)
{
    return &(m_page17_data);
}


bp_page18_data_t * bp_rx_data_page18_get(void)
{
    return &(m_page18_data);
}


bp_page32_data_t * bp_rx_data_page32_get(void)
{
    return &(m_page32_data);
}


bp_page1_response_data_t * bp_rx_data_page1_response_get(void)
{
    return (&m_page1_general_response_data);
}


page80_data_t * bp_rx_data_page80_get(void)
{
    return &(m_page80_data);
}


page81_data_t * bp_rx_data_page81_get(void)
{
    return &(m_page81_data);
}

