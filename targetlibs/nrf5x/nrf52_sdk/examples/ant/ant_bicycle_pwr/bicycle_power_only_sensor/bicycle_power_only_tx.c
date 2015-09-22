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

#include "bicycle_power_only_tx.h"
#include "compiler_abstraction.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "app_error.h"
#include "app_util.h"

#define HIGH_BYTE(word) (uint8_t)((word >> 8) & 0x00FFu) /**< Get high byte of a uint16_t. */
#define LOW_BYTE(word)  (uint8_t)(word & 0x00FFu)        /**< Get low byte of a uint16_t. */

#define BUFFER_INDEX_MESG_ID           1u                /**< Index for Message ID. */
#define BUFFER_INDEX_MESG_DATA         3u                /**< Index for Data. */

#define TX_BUFFER_SIZE                 8u                /**< Transmit buffer size. */
#define COMMON_PAGE_INTERLEAVE_COUNT   120u              /**< Interleave count for the common pages. */

#define PAGE_NUMBER_INDEX              0                 /**< Index of the data page number field.  */
#define EVENT_COUNT_INDEX              1u                /**< Index of the event count field in the power-only main data page. */
#define PEDAL_POWER_INDEX              2u                /**< Index of the pedal power field in the power-only main data page. */
#define INSTANT_CADENCE_INDEX          3u                /**< Index of the instantaneous cadence field in the power-only main data page. */
#define ACCUMMULATED_POWER_LSB_INDEX   4u                /**< Index of the accumulated power LSB field in the power-only main data page. */
#define ACCUMMULATED_POWER_MSB_INDEX   5u                /**< Index of the accumulated power MSB field in the power-only main data page. */
#define INSTANT_POWER_LSB_INDEX        6u                /**< Index of the instantaneous power LSB field in the power-only main data page. */
#define INSTANT_POWER_MSB_INDEX        7u                /**< Index of the instantaneous power MSB field in the power-only main data page. */

#define CALIBRATION_ID_INDEX           1u                /**< Index of the calibration ID field in the calibration response main data page. */
#define AUTO_ZERO_STATUS_INDEX         2u                /**< Index of the auto zero status field in the calibration response main data page. */
#define CALIBRATION_DATA_LSB_INDEX     6u                /**< Index of the calibration data LSB field in the calibration response main data page. */
#define CALIBRATION_DATA_MSB_INDEX     7u                /**< Index of the calibration data MSB field in the calibration response main data page. */

#define AUTO_ZERO_NOT_SUPPORTED        0xFFu             /**< Auto zero is not a supported value in the calibration response main data page. */

#define CALIBRATION_RESPONSE_SUCCESS   0xACu             /**< Calibration response success status code in the calibration response main data page. */
#define CALIBRATION_RESPONSE_FAILURE   0xAFu             /**< Calibration response failure status code in the calibration response main data page. */

#define INITIAL_INSTANT_POWER_VALUE    100u              /**< Initial instantaneous power field value. */
#define INSTANT_POWER_INCREMENT_AMOUNT 2u                /**< Amount to be added to current instantaneous power field as a response to user input. */
#define INSTANT_POWER_DECREMENT_AMOUNT 2u                /**< Amount to be subtracted from current instantaneous power field as a response to user input. */

#define HW_REVISION                    0x7Fu             /**< Hardware revision for manufacturer's identification common page. */
#define MANUFACTURER_ID                0xAAAAu           /**< Manufacturer ID for manufacturer's identification common page. */
#define MODEL_NUMBER                   0x5555u           /**< Model number for manufacturer's identification common page. */

#define SW_REVISION                    0xAAu             /**< Software revision number for product information common page. */
#define SERIAL_NUMBER                  0xAA55AA55u       /**< Serial number for product information common page. */


// Data page type identifer used to determine page type to be transmitted next.
typedef enum
{
    POWER_PAGE,                    /**< Power main data page. */
    PRODUCT_INFORMATION_PAGE,      /**< Product information common data page. */
    MANUFACTURERS_INFORMATION_PAGE /**< Manufacturers information common data page. */
} data_page_type_t;

static uint8_t  m_power_data_page_tx_buffer[TX_BUFFER_SIZE];                  /**< Power main data page transmit buffer. */
static uint8_t  m_calibration_response_tx_buffer[TX_BUFFER_SIZE];             /**< Calibration response transmit buffer. */
static uint32_t m_instantaneous_power          = INITIAL_INSTANT_POWER_VALUE; /**< Instantaneous power field value. */
static bool     m_calibration_response_pending = false;                       /**< Flag value to determine if there is a calibration response pending for transmission or not. */


/**@brief Function for transmitting a broadcast message.
 *
 * @param[in]   p_buffer                 Message to be transmitted.
 *
 * @return      ant_broadcast_message_tx API return code, NRF_SUCCESS for success.
 */
static __INLINE uint32_t broadcast_message_transmit(const uint8_t * p_buffer)
{
    return sd_ant_broadcast_message_tx(BP_TX_ANT_CHANNEL, TX_BUFFER_SIZE, (uint8_t*)p_buffer);
}


/**@brief Function for transmitting manufacturers information data page.
 *
 * @return ant_broadcast_message_tx API return code, NRF_SUCCESS for success.
 */
static __INLINE uint32_t manufacturers_information_data_page_transmit(void)
{
    static const uint8_t tx_buffer[TX_BUFFER_SIZE] =
    {
        COMMON_PAGE_80,
        BP_PAGE_RESERVE_BYTE,
        BP_PAGE_RESERVE_BYTE,
        HW_REVISION,
        LOW_BYTE(MANUFACTURER_ID),
        HIGH_BYTE(MANUFACTURER_ID),
        LOW_BYTE(MODEL_NUMBER),
        HIGH_BYTE(MODEL_NUMBER)
    };

    return broadcast_message_transmit(tx_buffer);
}


/**@brief Function for transmitting product information data page.
 *
 * @return ant_broadcast_message_tx API return code, NRF_SUCCESS for success.
 */
static __INLINE uint32_t product_information_data_page_transmit(void)
{
    static const uint8_t tx_buffer[TX_BUFFER_SIZE] =
    {
        COMMON_PAGE_81,
        BP_PAGE_RESERVE_BYTE,
        BP_PAGE_RESERVE_BYTE,
        SW_REVISION,
        (uint8_t)(SERIAL_NUMBER),
        (uint8_t)(SERIAL_NUMBER >> 8u),
        (uint8_t)(SERIAL_NUMBER >> 16u),
        (uint8_t)(SERIAL_NUMBER >> 24u)
    };

    return broadcast_message_transmit(tx_buffer);
}


/**@brief Function for updating and transmitting a power main data page. 
 *
 * @return ant_broadcast_message_tx API return code, NRF_SUCCESS for success.
 */
static __INLINE uint32_t power_main_data_page_transmit(void)
{
    static uint32_t accumulated_power = 0;

    ++(m_power_data_page_tx_buffer[EVENT_COUNT_INDEX]);
    accumulated_power += m_instantaneous_power;
    m_power_data_page_tx_buffer[ACCUMMULATED_POWER_LSB_INDEX] = LOW_BYTE(accumulated_power);
    m_power_data_page_tx_buffer[ACCUMMULATED_POWER_MSB_INDEX] = HIGH_BYTE(accumulated_power);
    m_power_data_page_tx_buffer[INSTANT_POWER_LSB_INDEX]      = LOW_BYTE(m_instantaneous_power);
    m_power_data_page_tx_buffer[INSTANT_POWER_MSB_INDEX]      = HIGH_BYTE(m_instantaneous_power);

    return broadcast_message_transmit(m_power_data_page_tx_buffer);
}


/**@brief Function for resolving the data page type to be transmitted.
 *
 * @return Type of the data page to be transmitted.
 */
static __INLINE data_page_type_t data_page_type_to_transmit_resolve(void)
{
    static uint8_t common_page_transmit_counter      = COMMON_PAGE_INTERLEAVE_COUNT;
    static bool    transmit_product_information_page = false;

    data_page_type_t data_page_type_to_transmit;

    if (((m_power_data_page_tx_buffer[EVENT_COUNT_INDEX] + 1u) & 0xFF) != common_page_transmit_counter)
    {
        // Power main data page transmission time.
        data_page_type_to_transmit = POWER_PAGE;
    }
    else
    {
        // Common page transmission time. Update the common page transmit counter to be ready for
        // the next round and resolve which of the supported common pages is to be transmitted.

        common_page_transmit_counter += COMMON_PAGE_INTERLEAVE_COUNT;

        if (transmit_product_information_page)
        {
            // Product information page transmisson time.
            // Adjust the boolean flag ready for the next round.
            transmit_product_information_page = false;
            data_page_type_to_transmit        = PRODUCT_INFORMATION_PAGE;
        }
        else
        {
            // Manufacturers information page transmisson time.
            // Adjust the boolean flag ready for the next round.
            transmit_product_information_page = true;
            data_page_type_to_transmit        = MANUFACTURERS_INFORMATION_PAGE;
        }
    }

    return data_page_type_to_transmit;
}


/**@brief Function for transmitting either a power main data page or a common data page.
 *
 * @return ant_broadcast_message_tx      API return code, NRF_SUCCESS for success.
 */
static __INLINE uint32_t data_page_transmit(void)
{
    uint32_t err_code;

    switch (data_page_type_to_transmit_resolve())
    {
        case POWER_PAGE:
            err_code = power_main_data_page_transmit();
            break;

        case PRODUCT_INFORMATION_PAGE:
            err_code = product_information_data_page_transmit();
            break;

        case MANUFACTURERS_INFORMATION_PAGE:
            err_code = manufacturers_information_data_page_transmit();
            break;

        default:
            err_code = NRF_ERROR_INTERNAL;
            APP_ERROR_HANDLER(err_code);
            break;
    }

    return err_code;
}


/**@brief Function for decoding the received ANT channel data.
 *
 * @param[in]   p_event_message_buffer   ANT event message buffer.
 * @param[out]  p_event_return           Output data from the profile.
 */
static __INLINE void rx_data_decode(uint8_t const * const    p_event_message_buffer,
                                    antplus_event_return_t * p_event_return)
{
    const uint32_t page_id = p_event_message_buffer[BUFFER_INDEX_MESG_DATA];

    switch (page_id)
    {
        case BP_PAGE_1:
            // Set application event regarding reception of calibration request.
            p_event_return->event  = ANTPLUS_EVENT_CALIBRATION_REQUEST;
            p_event_return->param1 = p_event_message_buffer[BUFFER_INDEX_MESG_DATA + 1u];
            break;

        default:
            break;
    }
}


/**@brief Function for processing the received data (for example broadcast, acknowledge, burst) from the ANT channel. 
 *
 * @param[in]   p_event_message_buffer   ANT event message buffer.
 * @param[out]  p_event_return           Output data from the profile.
 */
static __INLINE void rx_data_handle(uint8_t const * const    p_event_message_buffer,
                                    antplus_event_return_t * p_event_return)
{
    const uint32_t message_id = p_event_message_buffer[BUFFER_INDEX_MESG_ID];

    switch (message_id)
    {
        // Handle BROADCAST, ACKNOWLEDGED and BURST data the same.
        case MESG_BROADCAST_DATA_ID:
        case MESG_ACKNOWLEDGED_DATA_ID:
        case MESG_BURST_DATA_ID:
            rx_data_decode(p_event_message_buffer, p_event_return);
            break;

        default:
            APP_ERROR_HANDLER(message_id);
            break;
    }
}


uint32_t bp_only_tx_initialize(void)
{
    // Configure all the non volatile fields in the calibration response tx buffer.

    m_calibration_response_tx_buffer[PAGE_NUMBER_INDEX]      = BP_PAGE_1;
    m_calibration_response_tx_buffer[AUTO_ZERO_STATUS_INDEX] = AUTO_ZERO_NOT_SUPPORTED;
    m_calibration_response_tx_buffer[3]                      = BP_PAGE_RESERVE_BYTE;
    m_calibration_response_tx_buffer[4]                      = BP_PAGE_RESERVE_BYTE;
    m_calibration_response_tx_buffer[5]                      = BP_PAGE_RESERVE_BYTE;

    // Configure all the fields in the power data tx buffer
    // and send it to the stack for transmission.

    m_power_data_page_tx_buffer[PAGE_NUMBER_INDEX]            = BP_PAGE_16;
    m_power_data_page_tx_buffer[EVENT_COUNT_INDEX]            = 0;
    m_power_data_page_tx_buffer[PEDAL_POWER_INDEX]            = BP_PAGE_RESERVE_BYTE;
    m_power_data_page_tx_buffer[INSTANT_CADENCE_INDEX]        = BP_PAGE_RESERVE_BYTE;
    m_power_data_page_tx_buffer[ACCUMMULATED_POWER_LSB_INDEX] = LOW_BYTE(m_instantaneous_power);
    m_power_data_page_tx_buffer[ACCUMMULATED_POWER_MSB_INDEX] = HIGH_BYTE(m_instantaneous_power);
    m_power_data_page_tx_buffer[INSTANT_POWER_LSB_INDEX]      = LOW_BYTE(m_instantaneous_power);
    m_power_data_page_tx_buffer[INSTANT_POWER_MSB_INDEX]      = HIGH_BYTE(m_instantaneous_power);

    return broadcast_message_transmit(m_power_data_page_tx_buffer);
}


uint32_t bp_only_tx_channel_event_handle(uint32_t                 event,
                                         uint8_t const          * p_event_message_buffer,
                                         antplus_event_return_t * p_event_return)
{
    uint32_t err_code;
  
    p_event_return->event = ANTPLUS_EVENT_NONE;

    switch (event)
    {
        case EVENT_TX:

            // Broadcast tx message has been processed by the
            // ANT stack, send next message for it to transmit.

            if (!m_calibration_response_pending)
            {
                // No pending calibration response message exist, thus transmit a data page.
                err_code = data_page_transmit();
            }
            else
            {
                // Pending calibration response message exists, thus transmit it.
                m_calibration_response_pending = false;
                err_code                       = broadcast_message_transmit(
                                                     m_calibration_response_tx_buffer);
            }
            break;

        case EVENT_RX:
            rx_data_handle(p_event_message_buffer, p_event_return);
            // Fall through.
        default:
            err_code = NRF_SUCCESS;
            break;
    }

    return err_code;
}


uint32_t bp_only_tx_power_increment(void)
{
    m_instantaneous_power += INSTANT_POWER_INCREMENT_AMOUNT;
    return NRF_SUCCESS;
}


uint32_t bp_only_tx_power_decrement(void)
{
    m_instantaneous_power -= INSTANT_POWER_DECREMENT_AMOUNT;
    return NRF_SUCCESS;
}


uint32_t bp_only_tx_calib_resp_transmit(bool calibration_success, uint32_t calibration_data)
{
    m_calibration_response_pending = true;

    m_calibration_response_tx_buffer[CALIBRATION_ID_INDEX] =
        ((calibration_success) ? CALIBRATION_RESPONSE_SUCCESS : CALIBRATION_RESPONSE_FAILURE);
    m_calibration_response_tx_buffer[CALIBRATION_DATA_LSB_INDEX] = LOW_BYTE(calibration_data);
    m_calibration_response_tx_buffer[CALIBRATION_DATA_MSB_INDEX] = HIGH_BYTE(calibration_data);

    return NRF_SUCCESS;
}

