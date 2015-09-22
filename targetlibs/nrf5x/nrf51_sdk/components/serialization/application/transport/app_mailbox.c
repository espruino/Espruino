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
#include <string.h>
#include "app_mailbox.h"
#include "nrf_error.h"
#include "app_util_platform.h"

/**
 * @brief Mailbox handle used for managing a mailbox queue.
 */
typedef struct app_mailbox
{
    uint8_t  r_idx;    /**< Read index for the mailbox queue. */
    uint8_t  w_idx;    /**< Write index for the mailbox queue. */
    uint8_t  len;      /**< Number of elements currently in the mailbox queue. */
    uint8_t  queue_sz; /**< Capacity of the queue. */
    uint32_t item_sz;  /**< Size of single item. */
    void *   pool;     /**< Pointer to the pool where mailbox items are stored. */
} app_mailbox_t;

uint32_t app_mailbox_create(const app_mailbox_def_t * queue_def, app_mailbox_id_t * mailbox_id)
{
    app_mailbox_t * p_mailbox;
    uint32_t        err_code = NRF_SUCCESS;

    if (mailbox_id == NULL)
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else if (queue_def->queue_sz > 255)
    {
        *mailbox_id = (app_mailbox_id_t)NULL;
        err_code    = NRF_ERROR_INVALID_PARAM;
    }
    else
    {
        p_mailbox           = (app_mailbox_t *)queue_def->pool;
        p_mailbox->r_idx    = 0;
        p_mailbox->w_idx    = 0;
        p_mailbox->len      = 0;
        p_mailbox->queue_sz = (uint8_t)queue_def->queue_sz;
        p_mailbox->item_sz  = queue_def->item_sz;
        p_mailbox->pool     = (void *)((uintptr_t)queue_def->pool + sizeof (app_mailbox_t));
        *mailbox_id         = (app_mailbox_id_t)p_mailbox;
    }

    return err_code;
}

uint32_t app_mailbox_put(app_mailbox_id_t mailbox_id, void * mail)
{
    uint32_t        err_code  = NRF_ERROR_INTERNAL;
    app_mailbox_t * p_mailbox = (app_mailbox_t *)mailbox_id;
    void *          p_dst     = p_mailbox->pool;

    //Check if there is room left in mailbox.
    if (mail == NULL)
    {
        err_code = NRF_ERROR_NULL;
    }
    else
    {
        CRITICAL_REGION_ENTER();

        if (p_mailbox->len == p_mailbox->queue_sz)
        {
            err_code = NRF_ERROR_NO_MEM;
        }
        else
        {
            p_dst = (void *)((uintptr_t)p_dst + (p_mailbox->w_idx * p_mailbox->item_sz));
            p_mailbox->w_idx++;
            p_mailbox->len++;

            //Handle index wrapping.
            if (p_mailbox->w_idx == p_mailbox->queue_sz)
            {
                p_mailbox->w_idx = 0;
            }
            err_code = NRF_SUCCESS;
        }
        CRITICAL_REGION_EXIT();

        if (err_code == NRF_SUCCESS)
        {
            //Put data in mailbox.
            memcpy(p_dst, mail, p_mailbox->item_sz);
        }
    }

    return err_code;
}

uint32_t app_mailbox_get(app_mailbox_id_t mailbox_id, void * mail)
{
    app_mailbox_t * p_mailbox = (app_mailbox_t *)mailbox_id;
    void *          p_src     = p_mailbox->pool;
    uint32_t        err_code  = NRF_SUCCESS;

    if (mail == NULL)
    {
        err_code = NRF_ERROR_NULL;
    }
    else
    {
        CRITICAL_REGION_ENTER();

        if (p_mailbox->len == 0)
        {
            err_code = NRF_ERROR_NO_MEM;
        }
        else
        {
            p_src = (void *)((uintptr_t)p_src + (p_mailbox->r_idx * p_mailbox->item_sz));
            p_mailbox->r_idx++;
            p_mailbox->len--;

            if (p_mailbox->r_idx == p_mailbox->queue_sz)
            {
                p_mailbox->r_idx = 0;
            }
        }
        CRITICAL_REGION_EXIT();

        if (err_code == NRF_SUCCESS)
        {
            memcpy(mail, p_src, p_mailbox->item_sz);
        }
    }

    return err_code;
}

uint32_t app_mailbox_get_length (app_mailbox_id_t mailbox_id, uint32_t *p_len)
{
    app_mailbox_t * p_mailbox = (app_mailbox_t *)mailbox_id;
    uint32_t        err_code  = NRF_SUCCESS;

    if (p_len == NULL)
    {
        err_code = NRF_ERROR_NULL;
    }
    else
    {
        *p_len = (uint32_t) p_mailbox->len;
    }

    return err_code;
}
