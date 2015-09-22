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

/**@file
 * @defgroup spi_master_example_with_slave_main main.c
 * @{
 * @ingroup spi_master_example
 *
 * @brief SPI master example application to be used with the SPI slave example application.
 */

#include <stdio.h>
#include <stdbool.h>
#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "bsp.h"
#include "app_timer.h"
#include "nrf_drv_spi.h"
#include "nordic_common.h"

/*
 * This example uses only one instance of the SPI master.
 * Please make sure that only one instance of the SPI master is enabled in config file.
 */

#define APP_TIMER_PRESCALER      0                      /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS     BSP_APP_TIMERS_NUMBER  /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE  2                      /**< Size of timer operation queues. */

#define DELAY_MS                 1000                /**< Timer Delay in milli-seconds. */

#define TX_RX_BUF_LENGTH         16u                 /**< SPI transaction buffer length. */

#if (SPI0_ENABLED == 1)
    static const nrf_drv_spi_t m_spi_master = NRF_DRV_SPI_INSTANCE(0);
#elif (SPI1_ENABLED == 1)
    static const nrf_drv_spi_t m_spi_master = NRF_DRV_SPI_INSTANCE(1);
#elif (SPI2_ENABLED == 1)
    static const nrf_drv_spi_t m_spi_master = NRF_DRV_SPI_INSTANCE(2);
#else
    #error "No SPI enabled."
#endif

// Data buffers.
static uint8_t m_tx_data[TX_RX_BUF_LENGTH] = {0}; /**< A buffer with data to transfer. */
static uint8_t m_rx_data[TX_RX_BUF_LENGTH] = {0}; /**< A buffer for incoming data. */

static volatile bool m_transfer_completed = true; /**< A flag to inform about completed transfer. */


/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    UNUSED_VARIABLE(bsp_indication_set(BSP_INDICATE_FATAL_ERROR));

    for (;;)
    {
        // No implementation needed.
    }
}


/**@brief Function for checking if data coming from a SPI slave are valid.
 *
 * @param[in] p_buf     A pointer to a data buffer.
 * @param[in] len       A length of the data buffer.
 * 
 * @note Expected ASCII characters from: 'a' to: ('a' + len - 1).
 *
 * @retval true     Data are valid.
 * @retval false    Data are invalid.
 */
static __INLINE bool buf_check(uint8_t * p_buf, uint16_t len)
{
    uint16_t i;

    for (i = 0; i < len; i++)
    {
        if (p_buf[i] != (uint8_t)('a' + i))
        {
            return false;
        }
    }

    return true;
}


/**@brief Function for SPI master event callback.
 *
 * Upon receiving an SPI transaction complete event, checks if received data are valid.
 *
 * @param[in] spi_master_evt    SPI master driver event.
 */
static void spi_master_event_handler(nrf_drv_spi_event_t event)
{
    uint32_t err_code = NRF_SUCCESS;
    bool result = false;

    switch (event)
    {
        case NRF_DRV_SPI_EVENT_DONE:
            // Check if data are valid.
            result = buf_check(m_rx_data, TX_RX_BUF_LENGTH);
            APP_ERROR_CHECK_BOOL(result);

            err_code = bsp_indication_set(BSP_INDICATE_RCV_OK);
            APP_ERROR_CHECK(err_code);

            // Inform application that transfer is completed.
            m_transfer_completed = true;
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief The function initializes TX buffer to values to be sent and clears RX buffer.
 *
 * @note Function initializes TX buffer to values from 'A' to ('A' + len - 1)
 *       and clears RX buffer (fill by 0).
 *
 * @param[in] p_tx_data     A pointer to a buffer TX.
 * @param[in] p_rx_data     A pointer to a buffer RX.
 * @param[in] len           A length of the data buffers.
 */
static void init_buffers(uint8_t * const p_tx_data,
                         uint8_t * const p_rx_data,
                         const uint16_t  len)
{
    uint16_t i;

    for (i = 0; i < len; i++)
    {
        p_tx_data[i] = (uint8_t)('A' + i);
        p_rx_data[i] = 0;
    }
}


/**@brief Functions prepares buffers and starts data transfer.
 *
 * @param[in] p_tx_data     A pointer to a buffer TX.
 * @param[in] p_rx_data     A pointer to a buffer RX.
 * @param[in] len           A length of the data buffers.
 */
static void spi_send_recv(uint8_t * const p_tx_data,
                          uint8_t * const p_rx_data,
                          const uint16_t  len)
{
    // Initalize buffers.
    init_buffers(p_tx_data, p_rx_data, len);

    // Start transfer.
    uint32_t err_code = nrf_drv_spi_transfer(&m_spi_master,
        p_tx_data, len, p_rx_data, len);
    APP_ERROR_CHECK(err_code);
    nrf_delay_ms(DELAY_MS);
}


/**@brief Function for initializing bsp module.
 */
void bsp_configuration()
{
    uint32_t err_code = NRF_SUCCESS;

    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        // Do nothing.
    }

    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, NULL);
        
    err_code = bsp_init(BSP_INIT_LED, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for application main entry. Does not return. */
int main(void)
{
    // Setup bsp module.
    bsp_configuration();

    nrf_drv_spi_config_t const config =
    {
        #if (SPI0_ENABLED == 1)
            .sck_pin  = SPIM0_SCK_PIN,
            .mosi_pin = SPIM0_MOSI_PIN,
            .miso_pin = SPIM0_MISO_PIN,
            .ss_pin   = SPIM0_SS_PIN,
        #elif (SPI1_ENABLED == 1)
            .sck_pin  = SPIM1_SCK_PIN,
            .mosi_pin = SPIM1_MOSI_PIN,
            .miso_pin = SPIM1_MISO_PIN,
            .ss_pin   = SPIM1_SS_PIN,
        #elif (SPI2_ENABLED == 1)
            .sck_pin  = SPIM2_SCK_PIN,
            .mosi_pin = SPIM2_MOSI_PIN,
            .miso_pin = SPIM2_MISO_PIN,
            .ss_pin   = SPIM2_SS_PIN,
        #endif
        .irq_priority = APP_IRQ_PRIORITY_LOW,
        .orc          = 0xCC,
        .frequency    = NRF_DRV_SPI_FREQ_1M,
        .mode         = NRF_DRV_SPI_MODE_0,
        .bit_order    = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST,
    };
    ret_code_t err_code = nrf_drv_spi_init(&m_spi_master, &config, spi_master_event_handler);
    APP_ERROR_CHECK(err_code);

    for (;;)
    {
        if (m_transfer_completed)
        {
            m_transfer_completed = false;

            // Set buffers and start data transfer.
            spi_send_recv(m_tx_data, m_rx_data, TX_RX_BUF_LENGTH);
        }
    }
}


/** @} */
