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
#endif // TRACE_UART 

#include <stdint.h>
#include "bsp.h"
#include "sdm_rx_if.h"
#include "sdm_rx.h"
#include "nrf_error.h"
#include "nordic_common.h"
#include "app_error.h"
#if defined(TRACE_UART)
// Page 1 data.
static uint8_t  m_page_1_stride_count      = 0; /**< Peer device stride count. */
// Page 2 data.
static uint8_t  m_page_2_cadence           = 0; /**< Peer device cadence. */
static uint8_t  m_page_2_status            = 0; /**< Peer device status. */
// Page 80 data.
static uint8_t  m_page_80_hw_revision      = 0; /**< Peer device hardware revision. */
static uint16_t m_page_80_manufacturer_id  = 0; /**< Peer device manufacturer ID. */
static uint16_t m_page_80_model_number     = 0; /**< Peer device model number. */
// Page 81 data.
static uint8_t  m_page_81_sw_revision      = 0; /**< Peer device software revision. */
static uint32_t m_page_81_serial_number    = 0; /**< Peer device serial number. */
// Miscellaneous data.
static uint32_t m_accumulated_stride_count = 0; /**< Peer device accumulated stride count. */


/**@brief Function for decoding the content of a memory location into a uint16_t.
 *
 * @param[in] p_source The memory address to start reading 2 bytes from.
 *
 * @return uint16_t    The decoded value.
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


uint32_t sdm_rx_init(void)
{
    return NRF_SUCCESS;
}


uint32_t sdm_rx_data_process(uint8_t * p_page_data)
{
#if defined(TRACE_UART)
    switch (p_page_data[SDM_PAGE_INDEX_PAGE_NUM])
    {
        case SDM_PAGE_1:
            // Accumulate stride count before overwriting previous value.

            m_accumulated_stride_count += (
                (p_page_data[SDM_PAGE_1_INDEX_STRIDE_COUNT] - m_page_1_stride_count) & UINT8_MAX);
            m_page_1_stride_count = p_page_data[SDM_PAGE_1_INDEX_STRIDE_COUNT];
            break;

        case SDM_PAGE_2:
            m_page_2_cadence = p_page_data[SDM_PAGE_2_INDEX_CADENCE];
            m_page_2_status  = p_page_data[SDM_PAGE_2_INDEX_STATUS];
            break;

        case SDM_PAGE_80:
            m_page_80_hw_revision     = p_page_data[SDM_PAGE_80_INDEX_HW_REVISION];
            m_page_80_manufacturer_id = sdm_uint16_decode(
                &p_page_data[SDM_PAGE_80_INDEX_MANUFACTURER_ID]);
            m_page_80_model_number = sdm_uint16_decode(
                &p_page_data[SDM_PAGE_80_INDEX_MODEL_NUMBER]);
            break;

        case SDM_PAGE_81:
            m_page_81_sw_revision   = p_page_data[SDM_PAGE_81_INDEX_SW_REVISION];
            m_page_81_serial_number = sdm_uint32_decode(
                &p_page_data[SDM_PAGE_81_INDEX_SERIAL_NUMBER]);
            break;

        default:
            break;
    }
#endif // TRACE_UART

    return NRF_SUCCESS;
}


uint32_t sdm_rx_log(uint8_t page)
{
#if defined(TRACE_UART)
    static const char * p_location[4] = {"Laces", "Midsole", "Other", "Ankle"};
    static const char * p_battery[4]  = {"New", "Good", "OK", "Low"};
    static const char * p_health[4]   = {"OK", "Error", "Warning", ""};
    static const char * p_state[4]    = {"Inactive", "Active", "", ""};

    switch (page)
    {
        case SDM_PAGE_1:
            printf("page 1: stride count = %u, accumulated stride count = %u\n",
                   (unsigned int)m_page_1_stride_count,
                   (unsigned int)m_accumulated_stride_count);
            break;

        case SDM_PAGE_2:
            printf("page 2: location = %s, battery = %s, health = %s, state = %s, cadence = %u\n", 
                p_location[(m_page_2_status & SDM_STATUS_LOCATION_MASK) >> SDM_STATUS_LOCATION_POS], 
                p_battery[(m_page_2_status & SDM_STATUS_BATTERY_MASK) >> SDM_STATUS_BATTERY_POS], 
                p_health[(m_page_2_status & SDM_STATUS_HEALTH_MASK) >> SDM_STATUS_HEALTH_POS], 
                p_state[(m_page_2_status & SDM_STATUS_USE_STATE_MASK) >> SDM_STATUS_USE_STATE_POS],
                m_page_2_cadence);
            break;

        case SDM_PAGE_80:
            printf("page 80: HW revision = %u, manufacturer ID = %u, model number = %u\n",
                   m_page_80_hw_revision,
                   m_page_80_manufacturer_id,
                   m_page_80_model_number);
            break;

        case SDM_PAGE_81:
            printf("page 81: SW revision = %u, serial number = %u\n",
                   (unsigned int)m_page_81_sw_revision,
                   (unsigned int)m_page_81_serial_number);
            break;

        case SDM_PAGE_EVENT_RX_FAIL:
            printf("Event RX fail\n");
            break;

        default:
            break;
    }
#endif // TRACE_UART
#if defined(TRACE_GPIO)
    uint32_t err_code;
    switch (page)
    {
        case SDM_PAGE_1:
        case SDM_PAGE_2:
            err_code = bsp_indication_set(BSP_INDICATE_USER_STATE_0);
            APP_ERROR_CHECK(err_code);
            break;

        case SDM_PAGE_80:
        case SDM_PAGE_81:
            err_code = bsp_indication_set(BSP_INDICATE_USER_STATE_1);
            APP_ERROR_CHECK(err_code);
            break;

        case SDM_PAGE_EVENT_RX_FAIL:
            err_code = bsp_indication_set(BSP_INDICATE_USER_STATE_2);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
#endif // TRACE_GPIO
    return NRF_SUCCESS;
}
