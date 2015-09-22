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
 * @defgroup ant_sdm_tx_example main.c
 * @{
 * @ingroup ant_sdm_tx
 *
 * @brief The Stride and Distance Monitor minimal example TX (Master).
 */

#include "sdm_tx.h"
#include "sdm_tx_if.h"
#include "sdm_error.h"
#include "timer.h"
#include "nrf.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "app_error.h"
#include "ant_stack_config.h"

// Channel configuration.
#define CHANNEL_0                    0x00                     /**< ANT Channel 0. */
#define CHANNEL_0_TX_CHANNEL_PERIOD  8134u                    /**< Channel period 4.03 Hz. */
#define CHANNEL_0_ANT_EXT_ASSIGN     0x00                     /**< ANT Ext Assign. */
#define CHANNEL_0_FREQUENCY          57u                      /**< Channel Frequency. */
#define NETWORK_0                    0x00                     /**< ANT Network 0. */
#define NETWORK_0_KEY                {0, 0, 0, 0, 0, 0, 0, 0} /**< Public ANT network. */

// Channel ID configuration.
#define CHANNEL_0_CHAN_ID_DEV_TYPE   0x7Cu                    /**< Device type 124. */
#define CHANNEL_0_CHAN_ID_DEV_NUM    0xAAAAu                  /**< Device number. */
#define CHANNEL_0_CHAN_ID_TRANS_TYPE 0x05u                    /**< Transmission type. */

// Miscellaneous defines.
#define ANT_EVENT_MSG_BUFFER_MIN_SIZE 32u                     /**< Minimum size of ANT event message buffer. */
#define BROADCAST_DATA_BUFFER_SIZE    8u                      /**< Size of the broadcast data buffer. */

// Static variables and buffers.
static uint8_t m_broadcast_data[BROADCAST_DATA_BUFFER_SIZE]; /**< Primary data transmit buffer. */
static uint8_t m_network_0_key[] = NETWORK_0_KEY;            /**< ANT network key for SDM example. */


/**@brief Function for Stack Interrupt handling.
 */
void SD_EVT_IRQHandler(void)
{

}


#if defined(TRACE_UART)
/**@brief Callback function for asserts in the SoftDevice.
 *
 * param[in] line_num       Line number where the assert occurred.
 * param[in] p_file_name    File name where asserted.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    for (;;)
    {
        // No implementation needed.
    }
}


#endif // defined(TRACE_UART)


/**@brief Function for handling SoftDevice asserts.
 *
 * @param[in] pc            The value of the program counter.
 * @param[in] line_num      Line number where the error occurred.
 * @param[in] p_file_name   Pointer to the file name.
 */
void softdevice_assert_callback(uint32_t pc, uint16_t line_num, const uint8_t * p_file_name)
{
    for (;;)
    {
        // No implementation needed.
    }
}


/**@brief Function for handling an error.
 *
 * @param[in] error_code    Error code supplied to the handler.
 * @param[in] line_num      Line number where the handler is called.
 * @param[in] p_file_name   Pointer to the file name.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    for (;;)
    {
        // No implementation needed.
    }
}


/**@brief Function for setting up ANT module to be ready for TX broadcast.
 *
 * Issues the following commands:
 * - set network key
 * - assign channel
 * - set channel ID
 * - set channel frequency
 * - set channel period
 * - open channel
 */
static void ant_channel_sdm_tx_setup(void)
{
    uint32_t err_code;

    // Set network key.
    err_code = sd_ant_network_address_set(NETWORK_0, m_network_0_key);
    APP_ERROR_CHECK(err_code);

    // Set Channel Number.
    err_code = sd_ant_channel_assign(CHANNEL_0,
                                     CHANNEL_TYPE_MASTER,
                                     NETWORK_0,
                                     CHANNEL_0_ANT_EXT_ASSIGN);
    APP_ERROR_CHECK(err_code);

    // Set Channel ID.
    err_code = sd_ant_channel_id_set(CHANNEL_0,
                                     CHANNEL_0_CHAN_ID_DEV_NUM,
                                     CHANNEL_0_CHAN_ID_DEV_TYPE,
                                     CHANNEL_0_CHAN_ID_TRANS_TYPE);
    APP_ERROR_CHECK(err_code);

    // Set channel radio frequency.
    err_code = sd_ant_channel_radio_freq_set(CHANNEL_0, CHANNEL_0_FREQUENCY);
    APP_ERROR_CHECK(err_code);

    // Set channel period.
    err_code = sd_ant_channel_period_set(CHANNEL_0, CHANNEL_0_TX_CHANNEL_PERIOD);
    APP_ERROR_CHECK(err_code);

    // Open channel.
    err_code = sd_ant_channel_open(CHANNEL_0);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for Stride and Distance Monitor TX example main loop.
 *
 * @details The main loop will try to sleep as much as possible.
 * Every time a protocol event occours, the application will be woken up,
 * polling the ANT stack event queue. When finished processing all events,
 * the application will go back to sleep.
 */
void sdm_main_loop(void)
{
    uint8_t  event_id;
    uint8_t  ant_channel;
    uint8_t  next_page;
    uint32_t err_code;

    // Create ANT event message buffer.
    static uint8_t event_message_buffer[ANT_EVENT_MSG_BUFFER_MIN_SIZE];

    // Main loop.
    for (;;)
    {
        // Put CPU in sleep if possible.
        err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);

        // Extract and process all pending ANT events as
        // long as there are any left.
        do
        {
            // Fetch the event.
            err_code = sd_ant_event_get(&ant_channel, &event_id, event_message_buffer);
            if (err_code == NRF_SUCCESS)
            {
                // Handle event.
                switch (event_id)
                {
                    case EVENT_TX:
                        err_code = sdm_tx_next_page_num_get(&next_page);
                        APP_ERROR_CHECK(err_code);

                        err_code = sdm_tx_broadcast_data_set(next_page, m_broadcast_data);
                        APP_ERROR_CHECK(err_code);

                        err_code = sd_ant_broadcast_message_tx(CHANNEL_0,
                                                               BROADCAST_DATA_BUFFER_SIZE,
                                                               m_broadcast_data);
                        APP_ERROR_CHECK(err_code);

                        err_code = sdm_tx_log(m_broadcast_data);
                        APP_ERROR_CHECK(err_code);
                        break;

                    default:
                        break;
                }
            }
        }
        while (err_code == NRF_SUCCESS);
    }
}


/**@brief Function for callback for timer to invoke.
 *
 * @details This function will call sdm module to update its stride and distance data
 * according to what features SDM TX module has implemented.
 */
void sdm_data_timer_callback(void)
{
    uint32_t err_code = sdm_tx_sensor_data_update();
    APP_ERROR_CHECK(err_code);
}


/**@Function for application main entry, does not return.
 *
 * @details The main function will do all necessary initalization. This includes
 * sdm tx module, timer and SoftDevice.
 */
int main(void)
{
    uint32_t err_code;

    // In case of logging enabled, we enable sdm_tx module first.
    err_code = sdm_tx_init();
    APP_ERROR_CHECK(err_code);

    // Enable SoftDevice.
    err_code = sd_softdevice_enable(NRF_CLOCK_LFCLKSRC_XTAL_50_PPM,
                                    softdevice_assert_callback);
    APP_ERROR_CHECK(err_code);

    // Set application IRQ to lowest priority.
    err_code = sd_nvic_SetPriority(SD_EVT_IRQn,
                                   NRF_APP_PRIORITY_LOW);
    APP_ERROR_CHECK(err_code);

    // Enable application IRQ (triggered from protocol).
    err_code = sd_nvic_EnableIRQ(SD_EVT_IRQn);
    APP_ERROR_CHECK(err_code);

    err_code = ant_stack_static_config();
    APP_ERROR_CHECK(err_code);

    // Set up timer to update data.
    timer_initialize();
    timer_register(sdm_data_timer_callback);

    // Setup Channel_0 as a TX Master Only.
    ant_channel_sdm_tx_setup();

    // Start the timer after channel is set up.
    timer_start();

    // Initiate the broadcast loop by sending a packet on air,
    // then start waiting for an event on this broadcast message.
    err_code = sd_ant_broadcast_message_tx(CHANNEL_0, BROADCAST_DATA_BUFFER_SIZE, m_broadcast_data);
    APP_ERROR_CHECK(err_code);

    sdm_main_loop();
}


/**
 *@}
 **/
