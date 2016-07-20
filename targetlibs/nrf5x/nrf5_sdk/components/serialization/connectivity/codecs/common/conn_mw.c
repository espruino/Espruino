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
#include <stddef.h>

#include "ble_serialization.h"
#include "nrf_soc.h"
#include "ble.h"
#include "ble_l2cap.h"
#include "ble_gap.h"
#include "ble_gattc.h"
#include "ble_gatts.h"

/**@brief Connectivity middleware handler type. */
typedef uint32_t (*conn_mw_handler_t)(uint8_t const * const p_rx_buf,
                                      uint32_t              rx_buf_len,
                                      uint8_t * const       p_tx_buf,
                                      uint32_t * const      p_tx_buf_len);

/**@brief Connectivity middleware item. */
typedef struct
{
    uint8_t           opcode;     /**< Opcode by which specific codec is identified */
    conn_mw_handler_t fp_handler; /**< Function pointer to handler associated with given opcode */
} conn_mw_item_t;

/* Include handlers for given softdevice */
#include "conn_mw_items.c"

/**@brief Number of registered connectivity middleware handlers. */
static const uint32_t conn_mw_item_len = sizeof (conn_mw_item) / sizeof (conn_mw_item[0]);

/**@brief Local function for finding connectivity middleware handler in the table.. */
static conn_mw_handler_t conn_mw_handler_get(uint8_t opcode)
{
    conn_mw_handler_t fp_handler = NULL;
    uint32_t          i;

    for (i = 0; i < conn_mw_item_len; i++)
    {
        if (opcode == conn_mw_item[i].opcode)
        {
            fp_handler = conn_mw_item[i].fp_handler;
            break;
        }
    }

    return fp_handler;
}

uint32_t conn_mw_handler(uint8_t const * const p_rx_buf,
                         uint32_t              rx_buf_len,
                         uint8_t * const       p_tx_buf,
                         uint32_t * const      p_tx_buf_len)
{
    SER_ASSERT_NOT_NULL(p_rx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf);
    SER_ASSERT_NOT_NULL(p_tx_buf_len);

    conn_mw_handler_t fp_handler;
    uint32_t          err_code = NRF_SUCCESS;
    uint8_t           opcode   = p_rx_buf[SER_CMD_OP_CODE_POS];

    fp_handler = conn_mw_handler_get(opcode);

    if (fp_handler)
    {
        err_code = fp_handler(p_rx_buf, rx_buf_len, p_tx_buf, p_tx_buf_len);
    }
    else
    {
        err_code = NRF_ERROR_NOT_SUPPORTED;
    }

    return err_code;
}
