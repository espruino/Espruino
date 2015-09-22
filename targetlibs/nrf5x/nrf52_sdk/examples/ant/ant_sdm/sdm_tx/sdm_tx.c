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

#if defined(TRACE_UART)
  #include <stdio.h>
  #include "app_uart.h"
  #include "app_error.h"
#endif // TRACE_UART

#define UART_TX_BUF_SIZE 256u                                           /**< UART Tx buffer size. */
#define UART_RX_BUF_SIZE 1u                                             /**< UART Rx buffer size. */

#include <string.h>
#include "bsp.h"
#include "nrf_gpio.h"
#include "sdm_tx_if.h"
#include "sdm_tx.h"
#include "sdm_error.h"
#include "nrf_error.h"
#include "nordic_common.h"

#if LEDS_NUMBER < 2
#error "Not enough resources on  board."
#endif
static uint8_t m_message_num          = 0;                              /**< Counter to keep track of message number. */
static uint8_t m_common_page_num      = 0;                              /**< Counter to keep track of common page number. */

static uint8_t m_sdm_data_page_1[]    = {1, 0, 0, 0, 0, 0, 0, 0};       /**< Data page 1 default/unused values. */
static uint8_t m_sdm_data_page_2[]    = {2, 255, 255, 0, 0, 0, 255, 0}; /**< Data page 2 default/unused values. */
static uint8_t m_sdm_common_page_80[] = {80, 255, 255, 0, 0, 0, 0, 0};  /**< Common page 80 default/unused values. */
static uint8_t m_sdm_common_page_81[] = {81, 255, 255, 0, 0, 0, 0, 0};  /**< Common page 81 default/unused values. */

static uint8_t m_page_1_stride_count  = 0;                              /**< Current measured stride count. */
static uint8_t m_page_2_status        = 0;                              /**< Current device status. */

#if defined(TRACE_UART)
/**@brief Function for decoding content of a memory location into a uint16_t.
 *
 * @return uint16_t    The decoded value
 */
static __INLINE uint16_t sdm_uint16_decode(const uint8_t * p_source)
{
    return ((((uint16_t)((uint8_t *)p_source)[0])) |
            (((uint16_t)((uint8_t *)p_source)[1]) << 8u));
}


/**@brief Function for decoding content of a memory location into a uint32_t.
 *
 * @return uint32_t    The decoded value.
 */
static __INLINE uint32_t sdm_uint32_decode(const uint8_t * p_source)
{
    return ((((uint32_t)((uint8_t *)p_source)[0]) << 0)  |
            (((uint32_t)((uint8_t *)p_source)[1]) << 8u) |
            (((uint32_t)((uint8_t *)p_source)[2]) << 16u)|
            (((uint32_t)((uint8_t *)p_source)[3]) << 24u));
}
#endif // TRACE_UART


/**@brief Function for encoding a uint16_t value into 2 bytes.
 *
 * @param[out] p_target The memory address to write bytes to.
 * @param[in]  source   The value to encode.
 */
static __INLINE void sdm_uint16_encode(uint8_t * p_target, uint16_t source)
{
    p_target[0] = (uint8_t) ((source & 0x00FFu) >> 0);
    p_target[1] = (uint8_t) ((source & 0xFF00u) >> 8u);
}


/**@brief Function for encoding a uint32_t value into 4 bytes.
 *
 * @param[out] p_target The memory address to write bytes to.
 * @param[in]  source   The value to encode.
 */
static __INLINE void sdm_uint32_encode(uint8_t * p_target, uint32_t source)
{
    p_target[0] = (uint8_t) ((source & 0x000000FFu) >> 0);
    p_target[1] = (uint8_t) ((source & 0x0000FF00u) >> 8u);
    p_target[2] = (uint8_t) ((source & 0x00FF0000u) >> 16u);
    p_target[3] = (uint8_t) ((source & 0xFF000000u) >> 24u);
}


#if defined(TRACE_UART)
/**@brief Funtion for UART error handling, which is called when an error has occurred.
 *
 * @param[in] p_event Event supplied to the handler.
 */
void uart_error_handle(app_uart_evt_t * p_event)
{
    if ((p_event->evt_type == APP_UART_FIFO_ERROR) ||
        (p_event->evt_type == APP_UART_COMMUNICATION_ERROR))
    {
        // Copying parameters to static variables because parameters are not accessible in debugger.
        static volatile app_uart_evt_t uart_event;

        uart_event.evt_type = p_event->evt_type;
        uart_event.data     = p_event->data;
        UNUSED_VARIABLE(uart_event);

        for (;;)
        {
            // No implementation needed.
        }
    }
}


#endif // TRACE_UART


uint32_t sdm_tx_init(void)
{
    m_message_num     = 0;
    m_common_page_num = 0;

#if defined(TRACE_UART)
    // Configure and make UART ready for usage.
    app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud38400
    };

    uint32_t err_code;
    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_error_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    APP_ERROR_CHECK(err_code);
#endif // TRACE_UART
#if defined(TRACE_GPIO)
    // Configure pins LED_0 and LED_1 as outputs.
    nrf_gpio_cfg_output(BSP_LED_0);
    nrf_gpio_cfg_output(BSP_LED_1);

    // Reset the pins if they already is in a high state.
    nrf_gpio_pin_clear(BSP_LED_0);
    nrf_gpio_pin_clear(BSP_LED_1);
#endif // TRACE_GPIO
    // Initialize data page 1.
    m_page_1_stride_count = 0;

    // Initialize data page 2.
    m_page_2_status = SDM_STATUS_USE_STATE_ACTIVE << SDM_STATUS_USE_STATE_POS;

    // Initialize common page 80.
    m_sdm_common_page_80[SDM_PAGE_80_INDEX_HW_REVISION] = SDM_PAGE_80_HW_REVISION;
    sdm_uint16_encode(&m_sdm_common_page_80[SDM_PAGE_80_INDEX_MANUFACTURER_ID],
                      SDM_PAGE_80_MANUFACTURER_ID);
    sdm_uint16_encode(&m_sdm_common_page_80[SDM_PAGE_80_INDEX_MODEL_NUMBER],
                      SDM_PAGE_80_MODEL_NUMBER);

    // Initialize common page 81.
    m_sdm_common_page_81[SDM_PAGE_81_INDEX_SW_REVISION] = SDM_PAGE_81_SW_REVISION;
    sdm_uint32_encode(&m_sdm_common_page_81[SDM_PAGE_81_INDEX_SERIAL_NUMBER],
                      SDM_PAGE_81_SERIAL_NUMBER);

    return NRF_SUCCESS;
}


uint32_t sdm_tx_next_page_num_get(uint8_t * p_next_page)
{
    // Toggle between page 1 and page 2.
    // As each page will be transmitted twice, thus we use a bit mask of 0x2.
    // Message num will increment 1 for each message, and we will match 0 two times,
    // and 1 two times. When hitting the message_num four it will overflow, and we 
    // will have the two last bits 0 again for two messages. And so on.
    if (m_message_num & 0x2u)
    {
        (*p_next_page) = SDM_PAGE_2;
    }
    else
    {
        (*p_next_page) = SDM_PAGE_1;
    }

    // As message_num starts as 0 (post increment), we count up to the 64th before
    // reach a state where we shall send a common page. We will use a seperate
    // counter "m_common_page_num" to switch between the two common pages.
    if (m_message_num >= 64u)
    {
        // If previous common page was the 4th common page sent, this indicates we
        // have sent both our common pages twice. We have already set the next page
        // return value above, so it is safe to clear the common_page_num.
        // This will restart our common page toggle.
        if (m_common_page_num & 0x4u)
        {
            m_common_page_num = 0;
        }

        // Common_page_num will use a bitmask of 0x2 to send each common page twice.
        if (m_common_page_num & 0x2u)
        {
            (*p_next_page) = SDM_PAGE_81;
        }
        else
        {
            (*p_next_page) = SDM_PAGE_80;
        }

        // If common page_num is 1, this is the second time we shall transmit
        // this common page. When hitting this, we will reset our message_page_num
        // counter to start a new sequence of 64 messages followed by a common page.
        if (m_common_page_num & 0x1u)
        {
            m_message_num = 0;
        }

        m_common_page_num++;
    }
    // If not a common page, this is a data page.
    else
    {
        m_message_num++;
    }

    return NRF_SUCCESS;
}


uint32_t sdm_tx_broadcast_data_set(uint8_t page_number, uint8_t * p_message)
{
    uint32_t err_code;

    // If more than stride count value is updated, a race condition might occour.
    // A timer interrupt on higher level might update other variables in the page
    // buffer in the middle of copying values.
    // Disabling interrupts might be a solution for this.
    switch (page_number)
    {
        case SDM_PAGE_1:
            memcpy(p_message, m_sdm_data_page_1, sizeof(m_sdm_data_page_1));
            p_message[SDM_PAGE_1_INDEX_STRIDE_COUNT] = m_page_1_stride_count;
            err_code                                 = NRF_SUCCESS;
            break;

        case SDM_PAGE_2:
            memcpy(p_message, m_sdm_data_page_2, sizeof(m_sdm_data_page_2));
            p_message[SDM_PAGE_2_INDEX_STATUS] = m_page_2_status;
            err_code                           = NRF_SUCCESS;
            break;

        case SDM_PAGE_80:
            memcpy(p_message, m_sdm_common_page_80, sizeof(m_sdm_common_page_80));
            err_code = NRF_SUCCESS;
            break;

        case SDM_PAGE_81:
            memcpy(p_message, m_sdm_common_page_81, sizeof(m_sdm_common_page_81));
            err_code = NRF_SUCCESS;
            break;

        default:
            err_code = SDM_ERROR_INVALID_PAGE_NUMBER;
            break;
    }

    return err_code; 
}


uint32_t sdm_tx_sensor_data_update(void)
{
    m_page_1_stride_count++;
    return NRF_SUCCESS;
}


uint32_t sdm_tx_log(uint8_t * p_message)
{
#if defined(TRACE_UART)
    static const char * p_location[4] = {"Laces", "Midsole", "Other", "Ankle"};
    static const char * p_battery[4]  = {"New", "Good", "OK", "Low"};
    static const char * p_health[4]   = {"OK", "Error", "Warning", ""};
    static const char * p_state[4]    = {"Inactive", "Active", "", ""};

    switch (p_message[SDM_PAGE_INDEX_PAGE_NUM])
    {
        case SDM_PAGE_1:
            printf("page 1: stride count = %u\n", p_message[SDM_PAGE_1_INDEX_STRIDE_COUNT]);
            break;

        case SDM_PAGE_2:
            printf("page 2: location = %s, battery = %s, health = %s, state = %s\n",
                   p_location[(p_message[SDM_PAGE_2_INDEX_STATUS] & SDM_STATUS_LOCATION_MASK) >>
                              SDM_STATUS_LOCATION_POS],
                   p_battery[(p_message[SDM_PAGE_2_INDEX_STATUS] & SDM_STATUS_BATTERY_MASK) >>
                             SDM_STATUS_BATTERY_POS],
                   p_health[(p_message[SDM_PAGE_2_INDEX_STATUS] & SDM_STATUS_HEALTH_MASK) >>
                            SDM_STATUS_HEALTH_POS],
                   p_state[(p_message[SDM_PAGE_2_INDEX_STATUS] & SDM_STATUS_USE_STATE_MASK) >>
                           SDM_STATUS_USE_STATE_POS]);
            break;

        case SDM_PAGE_80:
            printf("page 80: HW revision = %u, manufacturer ID = %u, model number = %u\n",
                   p_message[SDM_PAGE_80_INDEX_HW_REVISION],
                   sdm_uint16_decode(&p_message[SDM_PAGE_80_INDEX_MANUFACTURER_ID]),
                   sdm_uint16_decode(&p_message[SDM_PAGE_80_INDEX_MODEL_NUMBER]));
            break;

        case SDM_PAGE_81:
            printf("page 81: SW revision = %u, serial number = %u\n",
                   (unsigned int)p_message[SDM_PAGE_81_INDEX_SW_REVISION],
                   (unsigned int)sdm_uint32_decode(&p_message[SDM_PAGE_81_INDEX_SERIAL_NUMBER]));
            break;

        default:
            break;
    }
#endif // TRACE_UART
#if defined(TRACE_GPIO)
    switch (p_message[SDM_PAGE_INDEX_PAGE_NUM])
    {
        case SDM_PAGE_1:
        case SDM_PAGE_2:
            nrf_gpio_pin_toggle(BSP_LED_0);
            break;

        case SDM_PAGE_80:
        case SDM_PAGE_81:
            nrf_gpio_pin_toggle(BSP_LED_1);
            break;

        default:
            break;
    }
#endif // TRACE_GPIO

    return NRF_SUCCESS;
}
