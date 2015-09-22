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
* @defgroup spi_master_example_main main.c
* @{
* @ingroup spi_master_example
*
* @brief SPI Master Loopback Example Application main file.
*
* This file contains the source code for a sample application using SPI.
*
*/

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "spi_master.h"
#include "bsp.h"
#include "app_timer.h"
#include "nordic_common.h"


#define APP_TIMER_PRESCALER      0                      /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS     BSP_APP_TIMERS_NUMBER  /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE  2                      /**< Size of timer operation queues. */

#define DELAY_MS                 1000                   /**< Timer Delay in milli-seconds. */

/** @def  TX_RX_MSG_LENGTH
 * number of bytes to transmit and receive. This amount of bytes will also be tested to see that
 * the received bytes from slave are the same as the transmitted bytes from the master */
#define TX_RX_MSG_LENGTH         100

#if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)

typedef enum
{
    TEST_STATE_SPI0_LSB,    /**< Test SPI0, bits order LSB */
    TEST_STATE_SPI0_MSB,    /**< Test SPI0, bits order MSB */
    TEST_STATE_SPI1_LSB,    /**< Test SPI1, bits order LSB */
    TEST_STATE_SPI1_MSB     /**< Test SPI1, bits order MSB */
} spi_master_ex_state_t;

static uint8_t m_tx_data_spi[TX_RX_MSG_LENGTH]; /**< SPI master TX buffer. */
static uint8_t m_rx_data_spi[TX_RX_MSG_LENGTH]; /**< SPI master RX buffer. */

static volatile bool m_transfer_completed = true;

#ifdef SPI_MASTER_0_ENABLE
static spi_master_ex_state_t m_spi_master_ex_state = TEST_STATE_SPI0_LSB;
#else
static spi_master_ex_state_t m_spi_master_ex_state = TEST_STATE_SPI1_LSB;
#endif

/**@brief Function for error handling, which is called when an error has occurred. 
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name. 
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    UNUSED_VARIABLE(bsp_indication_set(BSP_INDICATE_FATAL_ERROR));

    for (;; )
    {
        // No implementation needed.
    }
}



/**@brief The function initializes TX buffer to values to be sent and clears RX buffer.
 *
 * @note Function initializes TX buffer to values from 0 to (len - 1).
 *       and clears RX buffer (fill by 0).
 *
 * @param[out] p_tx_data    A pointer to a buffer TX.
 * @param[out] p_rx_data    A pointer to a buffer RX.
 * @param[in] len           A length of the data buffers.
 */
static void init_buf(uint8_t * const p_tx_buf,
                     uint8_t * const p_rx_buf,
                     const uint16_t  len)
{
    uint16_t i;

    for (i = 0; i < len; i++)
    {
        p_tx_buf[i] = i;
        p_rx_buf[i] = 0;
    }
}


/**@brief Function for checking if buffers are equal.
 *
 * @note Function compares each element of p_tx_buf with p_rx_buf.
 *
 * @param[in] p_tx_data     A pointer to a buffer TX.
 * @param[in] p_rx_data     A pointer to a buffer RX.
 * @param[in] len           A length of the data buffers.
 *
 * @retval true     Buffers are equal.
 * @retval false    Buffers are different.
 */
static bool check_buf_equal(const uint8_t * const p_tx_buf,
                            const uint8_t * const p_rx_buf,
                            const uint16_t        len)
{
    uint16_t i;

    for (i = 0; i < len; i++)
    {
        if (p_tx_buf[i] != p_rx_buf[i])
        {
            return false;
        }
    }
    return true;
}


#ifdef SPI_MASTER_0_ENABLE
/**@brief Handler for SPI0 master events.
 *
 * @param[in] spi_master_evt    SPI master event.
 */
void spi_master_0_event_handler(spi_master_evt_t spi_master_evt)
{
    uint32_t err_code = NRF_SUCCESS;
    bool result = false;

    switch (spi_master_evt.evt_type)
    {
        case SPI_MASTER_EVT_TRANSFER_COMPLETED:
            // Check if received data is correct.
            result = check_buf_equal(m_tx_data_spi, m_rx_data_spi, TX_RX_MSG_LENGTH);
            APP_ERROR_CHECK_BOOL(result);

            // Close SPI master.
            spi_master_close(SPI_MASTER_0);

            err_code = bsp_indication_set(BSP_INDICATE_RCV_OK);
            APP_ERROR_CHECK(err_code);

            m_transfer_completed = true;
            break;

        default:
            // No implementation needed.
            break;
    }
}
#endif /* SPI_MASTER_0_ENABLE */


#ifdef SPI_MASTER_1_ENABLE
/**@brief Handler for SPI1 master events.
 *
 * @param[in] spi_master_evt    SPI master event.
 */
void spi_master_1_event_handler(spi_master_evt_t spi_master_evt)
{
    uint32_t err_code = NRF_SUCCESS;
    bool result = false;

    switch (spi_master_evt.evt_type)
    {
        case SPI_MASTER_EVT_TRANSFER_COMPLETED:
            // Check if received data is correct.
            result = check_buf_equal(m_tx_data_spi, m_rx_data_spi, TX_RX_MSG_LENGTH);
            APP_ERROR_CHECK_BOOL(result);

            // Close SPI master.
            spi_master_close(SPI_MASTER_1);

            err_code = bsp_indication_set(BSP_INDICATE_RCV_OK);
            APP_ERROR_CHECK(err_code);

            m_transfer_completed = true;
            break;

        default:
            // No implementation needed.
            break;
    }
}
#endif /* SPI_MASTER_1_ENABLE */


/**@brief Function for initializing a SPI master driver.
 *
 * @param[in] spi_master_instance       An instance of SPI master module.
 * @param[in] spi_master_event_handler  An event handler for SPI master events.
 * @param[in] lsb                       Bits order LSB if true, MSB if false.
 */
static void spi_master_init(spi_master_hw_instance_t   spi_master_instance,
                            spi_master_event_handler_t spi_master_event_handler,
                            const bool                 lsb)
{
    uint32_t err_code = NRF_SUCCESS;

    // Configure SPI master.
    spi_master_config_t spi_config = SPI_MASTER_INIT_DEFAULT;

    switch (spi_master_instance)
    {
        #ifdef SPI_MASTER_0_ENABLE
        case SPI_MASTER_0:
        {
            spi_config.SPI_Pin_SCK  = SPIM0_SCK_PIN;
            spi_config.SPI_Pin_MISO = SPIM0_MISO_PIN;
            spi_config.SPI_Pin_MOSI = SPIM0_MOSI_PIN;
            spi_config.SPI_Pin_SS   = SPIM0_SS_PIN;
        }
        break;
        #endif /* SPI_MASTER_0_ENABLE */

        #ifdef SPI_MASTER_1_ENABLE
        case SPI_MASTER_1:
        {
            spi_config.SPI_Pin_SCK  = SPIM1_SCK_PIN;
            spi_config.SPI_Pin_MISO = SPIM1_MISO_PIN;
            spi_config.SPI_Pin_MOSI = SPIM1_MOSI_PIN;
            spi_config.SPI_Pin_SS   = SPIM1_SS_PIN;
        }
        break;
        #endif /* SPI_MASTER_1_ENABLE */

        default:
            break;
    }

    spi_config.SPI_CONFIG_ORDER = (lsb ? SPI_CONFIG_ORDER_LsbFirst : SPI_CONFIG_ORDER_MsbFirst);

    err_code = spi_master_open(spi_master_instance, &spi_config);
    APP_ERROR_CHECK(err_code);

    // Register event handler for SPI master.
    spi_master_evt_handler_reg(spi_master_instance, spi_master_event_handler);
}


/**@brief Function for sending and receiving data.
 *
 * @param[in]   spi_master_hw_instance  SPI master instance.
 * @param[in]   p_tx_data               A pointer to a buffer TX.
 * @param[out]  p_rx_data               A pointer to a buffer RX.
 * @param[in]   len                     A length of the data buffers.
 */
static void spi_send_recv(const spi_master_hw_instance_t spi_master_hw_instance,
                          uint8_t * const                p_tx_data,
                          uint8_t * const                p_rx_data,
                          const uint16_t                 len)
{
    // Initalize buffers.
    init_buf(p_tx_data, p_rx_data, len);

    // Start transfer.
    uint32_t err_code =
        spi_master_send_recv(spi_master_hw_instance, p_tx_data, len, p_rx_data, len);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for executing and switching state.
 *
 */
static void switch_state(void)
{
    switch (m_spi_master_ex_state)
    {
        #ifdef SPI_MASTER_0_ENABLE
        case TEST_STATE_SPI0_LSB:
            spi_master_init(SPI_MASTER_0, spi_master_0_event_handler, true);

            spi_send_recv(SPI_MASTER_0, m_tx_data_spi, m_rx_data_spi, TX_RX_MSG_LENGTH);
            m_spi_master_ex_state = TEST_STATE_SPI0_MSB;

            break;

        case TEST_STATE_SPI0_MSB:
            spi_master_init(SPI_MASTER_0, spi_master_0_event_handler, false);

            spi_send_recv(SPI_MASTER_0, m_tx_data_spi, m_rx_data_spi, TX_RX_MSG_LENGTH);

            #ifdef SPI_MASTER_1_ENABLE
            m_spi_master_ex_state = TEST_STATE_SPI1_LSB;
            #else
            m_spi_master_ex_state = TEST_STATE_SPI0_LSB;
            #endif /* SPI_MASTER_1_ENABLE */

            break;
        #endif /* SPI_MASTER_0_ENABLE */

        #ifdef SPI_MASTER_1_ENABLE
        case TEST_STATE_SPI1_LSB:
            spi_master_init(SPI_MASTER_1, spi_master_1_event_handler, true);

            spi_send_recv(SPI_MASTER_1, m_tx_data_spi, m_rx_data_spi, TX_RX_MSG_LENGTH);
            m_spi_master_ex_state = TEST_STATE_SPI1_MSB;

            break;

        case TEST_STATE_SPI1_MSB:
            spi_master_init(SPI_MASTER_1, spi_master_1_event_handler, false);

            spi_send_recv(SPI_MASTER_1, m_tx_data_spi, m_rx_data_spi, TX_RX_MSG_LENGTH);

            #ifdef SPI_MASTER_0_ENABLE
            m_spi_master_ex_state = TEST_STATE_SPI0_LSB;
            #else
            m_spi_master_ex_state = TEST_STATE_SPI1_LSB;
            #endif /* SPI_MASTER_0_ENABLE */

            break;
        #endif /* SPI_MASTER_1_ENABLE */

        default:
            break;
    }

    nrf_delay_ms(DELAY_MS);
}


#endif /* defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE) */

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


/** @brief Function for main application entry.
 */
int main(void)
{
    // Setup bsp module.
    bsp_configuration();

    for (;; )
    {
        #if defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)
        if (m_transfer_completed)
        {
            m_transfer_completed = false;
            switch_state();
        }
        #endif // defined(SPI_MASTER_0_ENABLE) || defined(SPI_MASTER_1_ENABLE)
    }
}


/** @} */
