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

#include "ser_phy.h"
#include "app_error.h"


__weak uint32_t ser_phy_open(ser_phy_events_handler_t events_handler)
{
    /* A function stub. Function should be implemented according to ser_phy.h API. */
    APP_ERROR_CHECK_BOOL(false);

    return NRF_SUCCESS;
}

__weak uint32_t ser_phy_tx_pkt_send(const uint8_t * p_buffer, uint16_t num_of_bytes)
{
    /* A function stub. Function should be implemented according to ser_phy.h API. */
    APP_ERROR_CHECK_BOOL(false);

    return NRF_SUCCESS;
}

__weak uint32_t ser_phy_rx_buf_set(uint8_t * p_buffer)
{
    /* A function stub. Function should be implemented according to ser_phy.h API. */
    APP_ERROR_CHECK_BOOL(false);

    return NRF_SUCCESS;
}

__weak void ser_phy_close(void)
{
    /* A function stub. Function should be implemented according to ser_phy.h API. */
    APP_ERROR_CHECK_BOOL(false);
}


__weak void ser_phy_interrupts_enable(void)
{
    /* A function stub. Function should be implemented according to ser_phy.h API. */
    APP_ERROR_CHECK_BOOL(false);
}


__weak void ser_phy_interrupts_disable(void)
{
    /* A function stub. Function should be implemented according to ser_phy.h API. */
    APP_ERROR_CHECK_BOOL(false);
}


