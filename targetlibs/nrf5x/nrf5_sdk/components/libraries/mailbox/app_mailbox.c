/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
#include <string.h>
#include "app_mailbox.h"
#include "nrf_error.h"
#include "app_util_platform.h"
#include "app_util.h"
#include "nrf_assert.h"

ret_code_t app_mailbox_create(const app_mailbox_t * queue_def)
{
    queue_def->p_cb->r_idx    = 0;
    queue_def->p_cb->w_idx    = 0;
    queue_def->p_cb->len      = 0;
    queue_def->p_cb->mode     = APP_MAILBOX_MODE_NO_OVERFLOW;

    return NRF_SUCCESS;
}

static __INLINE void dequeue(app_mailbox_cb_t * p_cb, uint8_t queue_sz)
{
    uint8_t  r_idx = p_cb->r_idx + 1;
    p_cb->len--;

    //Handle index wrapping.
    p_cb->r_idx = (r_idx == queue_sz) ? 0 : r_idx;
}

static __INLINE void enqueue(app_mailbox_cb_t * p_cb, uint8_t queue_sz)
{
    uint8_t  w_idx = p_cb->w_idx + 1;
    p_cb->len++;

    //Handle index wrapping.
    p_cb->w_idx = (w_idx == queue_sz) ? 0 : w_idx;
}

ret_code_t app_mailbox_put(const app_mailbox_t * p_mailbox, void * p_item)
{
    ASSERT(p_mailbox);
    return app_mailbox_sized_put(p_mailbox, p_item, p_mailbox->item_sz);
}

ret_code_t app_mailbox_sized_put(const app_mailbox_t * p_mailbox, void * p_item, uint16_t size)
{
    ASSERT((uint32_t)p_item>0);
    ASSERT(p_mailbox);
    uint32_t           err_code  = NRF_ERROR_INTERNAL;
    uint32_t *         p_dst     = p_mailbox->p_pool;
    bool               do_put    = true;
    uint8_t            queue_sz  = p_mailbox->queue_sz;
    uint16_t           item_sz   = p_mailbox->item_sz;
    app_mailbox_cb_t * p_cb      = p_mailbox->p_cb;

    CRITICAL_REGION_ENTER();

    if (p_cb->len == queue_sz)
    {
        if (p_cb->mode == APP_MAILBOX_MODE_NO_OVERFLOW)
        {
            do_put = false;
        }
        else
        {
            // Remove the oldest element.
            dequeue(p_cb, queue_sz);
        }
        err_code = NRF_ERROR_NO_MEM;
    }
    else
    {
        err_code = NRF_SUCCESS;
    }

    if (do_put)
    {
        p_dst = (uint32_t *)((uint32_t)p_dst + (p_cb->w_idx * (item_sz + sizeof(uint32_t))));
        enqueue(p_cb, queue_sz);

        //Put data in mailbox.
        *p_dst = (uint32_t)size;
        p_dst++;
        memcpy(p_dst, p_item, size);
    }

    CRITICAL_REGION_EXIT();

    return err_code;
}

ret_code_t app_mailbox_get(const app_mailbox_t * p_mailbox, void * p_item)
{
    uint16_t size;
    return app_mailbox_sized_get(p_mailbox, p_item, &size);
}

ret_code_t app_mailbox_sized_get(const app_mailbox_t * p_mailbox, void * p_item, uint16_t * p_size)
{
    ASSERT(p_mailbox);
    ASSERT((uint32_t)p_item>0);
    uint32_t *         p_src     = p_mailbox->p_pool;
    ret_code_t         err_code  = NRF_SUCCESS;
    uint16_t           item_sz   = p_mailbox->item_sz;
    uint8_t            queue_sz  = p_mailbox->queue_sz;
    app_mailbox_cb_t * p_cb      = p_mailbox->p_cb;

    CRITICAL_REGION_ENTER();

    if (p_cb->len == 0)
    {
        err_code = NRF_ERROR_NO_MEM;
    }
    else
    {
        p_src = (void *)((uint32_t)p_src + (p_cb->r_idx * (item_sz + sizeof(uint32_t))));
        dequeue(p_cb, queue_sz);
    }

    if (err_code == NRF_SUCCESS)
    {
        uint16_t size = (uint16_t)*p_src;
        *p_size = size;
        p_src++;
        memcpy(p_item, p_src, size);
    }

    CRITICAL_REGION_EXIT();

    return err_code;
}

uint32_t app_mailbox_length_get (const app_mailbox_t * p_mailbox)
{
    ASSERT(p_mailbox);
    return p_mailbox->p_cb->len;
}

void app_mailbox_mode_set(const app_mailbox_t * p_mailbox, app_mailbox_overflow_mode_t mode)
{
    ASSERT(p_mailbox);
    p_mailbox->p_cb->mode = mode;
}
