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


#include "peer_id.h"

#include <stdint.h>
#include <string.h>
#include "sdk_errors.h"
#include "peer_manager_types.h"
#include "pm_mutex.h"


typedef struct
{
    uint8_t peer_ids[MUTEX_STORAGE_SIZE(PM_PEER_ID_N_AVAILABLE_IDS)]; /*< bitmap. */
} pi_t;


static pi_t m_pi = {.peer_ids = {0}};


static void internal_state_reset(pi_t * p_pi)
{
    memset(p_pi, 0, sizeof(pi_t));
}


void peer_id_init(void)
{
    internal_state_reset(&m_pi);
    pm_mutex_init(m_pi.peer_ids, PM_PEER_ID_N_AVAILABLE_IDS);
}


pm_peer_id_t peer_id_allocate(pm_peer_id_t peer_id)
{
    pm_peer_id_t allocated_peer_id = PM_PEER_ID_INVALID;
    if (peer_id == PM_PEER_ID_INVALID)
    {
        allocated_peer_id = pm_mutex_lock_first_available(m_pi.peer_ids, PM_PEER_ID_N_AVAILABLE_IDS);
        if (allocated_peer_id == PM_PEER_ID_N_AVAILABLE_IDS)
        {
            allocated_peer_id = PM_PEER_ID_INVALID;
        }
    }
    else if (peer_id < PM_PEER_ID_N_AVAILABLE_IDS)
    {
        bool lock_success = pm_mutex_lock(m_pi.peer_ids, peer_id);
        allocated_peer_id = lock_success ? peer_id : PM_PEER_ID_INVALID;
    }
    return allocated_peer_id;
}


void peer_id_free(pm_peer_id_t peer_id)
{
    if (peer_id < PM_PEER_ID_N_AVAILABLE_IDS)
    {
        pm_mutex_unlock(m_pi.peer_ids, peer_id);
    }
}


bool peer_id_is_allocated(pm_peer_id_t peer_id)
{
    if (peer_id < PM_PEER_ID_N_AVAILABLE_IDS)
    {
        return pm_mutex_lock_status_get(m_pi.peer_ids, peer_id);
    }
    return false;
}


pm_peer_id_t peer_id_next_id_get(pm_peer_id_t prev_peer_id)
{
    pm_peer_id_t i = (prev_peer_id == PM_PEER_ID_INVALID) ? 0 : (prev_peer_id + 1);
    for (; i < PM_PEER_ID_N_AVAILABLE_IDS; i++)
    {
        if (pm_mutex_lock_status_get(m_pi.peer_ids, i))
        {
            return i;
        }
    }

    return PM_PEER_ID_INVALID;
}


uint32_t peer_id_n_ids(void)
{
    uint32_t n_ids = 0;

    for (pm_peer_id_t i = 0; i < PM_PEER_ID_N_AVAILABLE_IDS; i++)
    {
        n_ids += pm_mutex_lock_status_get(m_pi.peer_ids, i);
    }

    return n_ids;
}


