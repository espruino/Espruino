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

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "nrf.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_common.h"
#include "nrf_ppi.h"
#include "app_util_platform.h"

#if (NRF_PPI_RESTRICTED > 0)
#include "nrf_soc.h"
#endif


static nrf_drv_state_t     m_drv_state;            /**< Driver state */
static uint32_t            m_channels_allocated;   /**< Bitmap representing channels availability. 1 when a channel is allocated, 0 otherwise. */
static uint8_t             m_groups_allocated;     /**< Bitmap representing groups availability. 1 when a group is allocated, 0 otherwise.*/


/**@brief  Compute a group mask (needed for driver internals, not used for NRF_PPI registers).
 * @param[in]  group  Group number to transform to a mask.
 * @retval     Group mask.
 */
__STATIC_INLINE uint32_t group_to_mask(nrf_ppi_channel_group_t group)
{
    return (1uL << (uint32_t) group);
}


/**@brief  Check whether a channel is a programmable channel and can be used by an application.
 * @param[in]  channel  Channel to check.
 * @retval     true     The channel is a programmable application channel.
 *             false    The channel is used by a SoftDevice or is preprogrammed.
 */
__STATIC_INLINE bool is_programmable_app_channel(nrf_ppi_channel_t channel)
{
    return ((NRF_PPI_PROG_APP_CHANNELS_MASK & nrf_drv_ppi_channel_to_mask(channel)) != 0);
}


/**@brief  Check whether a channels can be used by an application.
 * @param[in]  channel  Channel mask to check.
 * @retval     true     All specified channels can be used by an application.
 *             false    At least one specified channel is used by a SoftDevice.
 */
__STATIC_INLINE bool are_app_channels(uint32_t channel_mask)
{
    return ((~(NRF_PPI_ALL_APP_CHANNELS_MASK) & channel_mask) == 0);
}


/**@brief  Check whether a channel can be used by an application.
 * @param[in]  channel  Channel to check.
 * @retval     true     The channel can be used by an application.
 *             false    The channel is used by a SoftDevice.
 */
__STATIC_INLINE bool is_app_channel(nrf_ppi_channel_t channel)
{
    return are_app_channels(nrf_drv_ppi_channel_to_mask(channel));
}


/**@brief  Check whether a channel group can be used by an application.
 * @param[in]  group    Group to check.
 * @retval     true     The group is an application group.
 *             false    The group is not an application group (this group either does not exist or
 *                      it is used by a SoftDevice).
 */
__STATIC_INLINE bool is_app_group(nrf_ppi_channel_group_t group)
{
    return ((NRF_PPI_ALL_APP_GROUPS_MASK & group_to_mask(group)) != 0);
}


/**@brief  Check whether a channel is allocated.
 * @param[in]  channel_num  Channel number to check.
 * @retval     true         The channel is allocated.
 *             false        The channel is not allocated.
 */
__STATIC_INLINE bool is_allocated_channel(nrf_ppi_channel_t channel)
{
    return ((m_channels_allocated & nrf_drv_ppi_channel_to_mask(channel)) != 0);
}


/**@brief  Set channel allocated indication.
 * @param[in]  channel_num  Specifies the channel to set the "allocated" indication.
 */
__STATIC_INLINE void channel_allocated_set(nrf_ppi_channel_t channel)
{
    m_channels_allocated |= nrf_drv_ppi_channel_to_mask(channel);
}


/**@brief  Clear channel allocated indication.
 * @param[in]  channel_num  Specifies the channel to clear the "allocated" indication.
 */
__STATIC_INLINE void channel_allocated_clr(nrf_ppi_channel_t channel)
{
    m_channels_allocated &= ~nrf_drv_ppi_channel_to_mask(channel);
}


/**@brief  Clear all allocated channels.
 */
__STATIC_INLINE void channel_allocated_clr_all(void)
{
    m_channels_allocated &= ~NRF_PPI_ALL_APP_CHANNELS_MASK;
}


/**@brief  Check whether a group is allocated.
 * @param[in]  group_num    Group number to check.
 * @retval     true         The group is allocated.
 *             false        The group is not allocated.
 */
__STATIC_INLINE bool is_allocated_group(nrf_ppi_channel_group_t group)
{
    return ((m_groups_allocated & group_to_mask(group)) != 0);
}


/**@brief  Set group allocated indication.
 * @param[in]  group_num  Specifies the group to set the "allocated" indication.
 */
__STATIC_INLINE void group_allocated_set(nrf_ppi_channel_group_t group)
{
    m_groups_allocated |= group_to_mask(group);
}


/**@brief  Clear group allocated indication.
 * @param[in]  group_num  Specifies the group to clear the "allocated" indication.
 */
__STATIC_INLINE void group_allocated_clr(nrf_ppi_channel_group_t group)
{
    m_groups_allocated &= ~group_to_mask(group);
}


/**@brief  Clear all allocated groups.
 */
__STATIC_INLINE void group_allocated_clr_all()
{
    m_groups_allocated &= ~NRF_PPI_ALL_APP_GROUPS_MASK;
}


uint32_t nrf_drv_ppi_init(void)
{
    uint32_t err_code;

    if (m_drv_state == NRF_DRV_STATE_UNINITIALIZED)
    {
        m_drv_state = NRF_DRV_STATE_INITIALIZED;
        err_code    = NRF_SUCCESS;
    }
    else
    {
        err_code = MODULE_ALREADY_INITIALIZED;
    }

    return err_code;
}


uint32_t nrf_drv_ppi_uninit(void)
{
    uint32_t err_code = NRF_SUCCESS;
    uint32_t mask = NRF_PPI_ALL_APP_GROUPS_MASK;
    nrf_ppi_channel_group_t group;

    if (m_drv_state != NRF_DRV_STATE_UNINITIALIZED)
    {
        m_drv_state = NRF_DRV_STATE_UNINITIALIZED;

        // Disable all channels and groups
#if (NRF_PPI_RESTRICTED > 0)
        err_code = sd_ppi_channel_enable_clr(NRF_PPI_ALL_APP_CHANNELS_MASK);
        if (err_code != NRF_SUCCESS)
        {
            return NRF_ERROR_INTERNAL;
        }

        for (group = NRF_PPI_CHANNEL_GROUP0; mask != 0; mask &= ~group_to_mask(group), group++)
        {
            if(mask & group_to_mask(group))
            {
                err_code = sd_ppi_group_task_disable((uint8_t)group);
                if (err_code != NRF_SUCCESS)
                {
                    return NRF_ERROR_INTERNAL;
                }
            }
        }
#else
        nrf_ppi_channel_disable_all();

        for (group = NRF_PPI_CHANNEL_GROUP0; mask != 0; mask &= ~group_to_mask(group), group++)
        {
            if(mask & group_to_mask(group))
            {
                nrf_ppi_channel_group_clear(group);
            }
        }
#endif

        channel_allocated_clr_all();
        group_allocated_clr_all();
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }
    return err_code;
}


uint32_t nrf_drv_ppi_channel_alloc(nrf_ppi_channel_t * p_channel)
{
    uint32_t err_code;
    nrf_ppi_channel_t channel;
    uint32_t mask = 0;

    err_code = NRF_ERROR_NO_MEM;

    mask = NRF_PPI_PROG_APP_CHANNELS_MASK;
    for (channel = NRF_PPI_CHANNEL0; mask != 0; mask &= ~nrf_drv_ppi_channel_to_mask(channel), channel++)
    {
        CRITICAL_REGION_ENTER();
        if ((mask & nrf_drv_ppi_channel_to_mask(channel)) && (!is_allocated_channel(channel)))
        {
            channel_allocated_set(channel);
            *p_channel = channel;
            err_code   = NRF_SUCCESS;
        }
        CRITICAL_REGION_EXIT();
        if (err_code == NRF_SUCCESS)
        {
            break;
        }
    }

    return err_code;
}


uint32_t nrf_drv_ppi_channel_free(nrf_ppi_channel_t channel)
{
    uint32_t err_code;

    if (!is_programmable_app_channel(channel))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else
    {
        // First disable this channel
#if (NRF_PPI_RESTRICTED > 0)
        err_code = sd_ppi_channel_enable_clr(nrf_drv_ppi_channel_to_mask(channel));
#else
        nrf_ppi_channel_disable(channel);
        err_code = NRF_SUCCESS;
#endif
        CRITICAL_REGION_ENTER();
        channel_allocated_clr(channel);
        CRITICAL_REGION_EXIT();
    }

    return err_code;
}


uint32_t nrf_drv_ppi_channel_assign(nrf_ppi_channel_t channel, uint32_t eep, uint32_t tep)
{
    uint32_t err_code;

    if (((uint32_t *)eep == NULL) || ((uint32_t *)tep == NULL))
    {
        err_code = NRF_ERROR_NULL;
    }
    else if (!is_programmable_app_channel(channel))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else if (!is_allocated_channel(channel))
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }
    else
    {
#if (NRF_PPI_RESTRICTED > 0)
        err_code = sd_ppi_channel_assign((uint8_t) channel, (const volatile void *) eep, (const volatile void *) tep);
        if (err_code != NRF_SUCCESS)
        {
            err_code = NRF_ERROR_INVALID_PARAM;
        }
#else
        nrf_ppi_channel_endpoint_setup(channel, eep, tep);
        err_code = NRF_SUCCESS;
#endif
    }

    return err_code;
}


uint32_t nrf_drv_ppi_channel_enable(nrf_ppi_channel_t channel)
{
    uint32_t err_code;

    if (!is_app_channel(channel))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else if (is_programmable_app_channel(channel) && !is_allocated_channel(channel))
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }
    else
    {
#if (NRF_PPI_RESTRICTED > 0)
        err_code = sd_ppi_channel_enable_set(nrf_drv_ppi_channel_to_mask(channel));
#else
        nrf_ppi_channel_enable(channel);
        err_code = NRF_SUCCESS;
#endif
    }

    return err_code;
}


uint32_t nrf_drv_ppi_channel_disable(nrf_ppi_channel_t channel)
{
    uint32_t err_code;

    if (!is_app_channel(channel))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else if (is_programmable_app_channel(channel) && !is_allocated_channel(channel))
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }
    else
    {
#if (NRF_PPI_RESTRICTED > 0)
        err_code = sd_ppi_channel_enable_clr(nrf_drv_ppi_channel_to_mask(channel));
#else
        nrf_ppi_channel_disable(channel);
        err_code = NRF_SUCCESS;
#endif
    }

    return err_code;
}


uint32_t nrf_drv_ppi_group_alloc(nrf_ppi_channel_group_t * p_group)
{
    uint32_t err_code;
    uint32_t mask = 0;
    nrf_ppi_channel_group_t group;

    err_code = NRF_ERROR_NO_MEM;

    mask = NRF_PPI_ALL_APP_GROUPS_MASK;
    for (group = NRF_PPI_CHANNEL_GROUP0; mask != 0; mask &= ~group_to_mask(group), group++)
    {
        CRITICAL_REGION_ENTER();
        if ((mask & group_to_mask(group)) && (!is_allocated_group(group)))
        {
            group_allocated_set(group);
            *p_group = group;
            err_code = NRF_SUCCESS;
        }
        CRITICAL_REGION_EXIT();
        if (err_code == NRF_SUCCESS)
        {
            break;
        }
    }

    return err_code;
}


uint32_t nrf_drv_ppi_group_free(nrf_ppi_channel_group_t group)
{
    uint32_t err_code;

    if (!is_app_group(group))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else if (!is_allocated_group(group))
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }
    else
    {
#if (NRF_PPI_RESTRICTED > 0)
        err_code = sd_ppi_group_task_disable((uint8_t) group);
#else
        nrf_ppi_group_disable(group);
        err_code = NRF_SUCCESS;
#endif
        CRITICAL_REGION_ENTER();
        group_allocated_clr(group);
        CRITICAL_REGION_EXIT();
    }

    return err_code;
}


uint32_t nrf_drv_ppi_group_enable(nrf_ppi_channel_group_t group)
{
    uint32_t err_code;

    if (!is_app_group(group))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else if (!is_allocated_group(group))
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }
    else
    {
#if (NRF_PPI_RESTRICTED > 0)
        err_code = sd_ppi_group_task_enable((uint8_t) group);
#else
        nrf_ppi_group_enable(group);
        err_code = NRF_SUCCESS;
#endif
    }

    return err_code;
}


uint32_t nrf_drv_ppi_group_disable(nrf_ppi_channel_group_t group)
{
    uint32_t err_code;

    if (!is_app_group(group))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else
    {
#if (NRF_PPI_RESTRICTED > 0)
        err_code = sd_ppi_group_task_disable((uint8_t) group);
#else
        nrf_ppi_group_disable(group);
        err_code = NRF_SUCCESS;
#endif
    }

    return err_code;
}

uint32_t nrf_drv_ppi_channels_remove_from_group(uint32_t channel_mask,
                                                nrf_ppi_channel_group_t group)
{
    uint32_t err_code;

    if (!is_app_group(group))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else if (!is_allocated_group(group))
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }
    else if (!are_app_channels(channel_mask))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else
    {
#if (NRF_PPI_RESTRICTED > 0)
        uint32_t channels_in_group = 0;

        CRITICAL_REGION_ENTER();
        err_code = sd_ppi_group_get((uint8_t) group, &channels_in_group);

        if (err_code != NRF_SUCCESS)
        {
            err_code = NRF_ERROR_INVALID_PARAM;
        }
        else
        {
            channels_in_group &= ~channel_mask;
            err_code = sd_ppi_group_assign((uint8_t) group, channels_in_group);

            if (err_code != NRF_SUCCESS)
            {
                err_code = NRF_ERROR_INVALID_PARAM;
            }
        }
        CRITICAL_REGION_EXIT();

#else
        CRITICAL_REGION_ENTER();
        nrf_ppi_channels_remove_from_group(channel_mask, group);
        CRITICAL_REGION_EXIT();
        err_code = NRF_SUCCESS;
#endif

    }
    return err_code;
}

uint32_t nrf_drv_ppi_channels_include_in_group(uint32_t channel_mask,
                                               nrf_ppi_channel_group_t group)
{
    uint32_t err_code;

    if (!is_app_group(group))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else if (!is_allocated_group(group))
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }
    else if (!are_app_channels(channel_mask))
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }
    else
    {
#if (NRF_PPI_RESTRICTED > 0)
        uint32_t channels_in_group = 0;

        CRITICAL_REGION_ENTER();
        err_code = sd_ppi_group_get((uint8_t) group, &channels_in_group);
        if (err_code != NRF_SUCCESS)
        {
            err_code = NRF_ERROR_INVALID_PARAM;
        }
        else
        {
            channels_in_group |= channel_mask;
            err_code = sd_ppi_group_assign((uint8_t) group, channels_in_group);
            if (err_code != NRF_SUCCESS)
            {
                err_code = NRF_ERROR_INVALID_PARAM;
            }
        }
        CRITICAL_REGION_EXIT();
#else

        CRITICAL_REGION_ENTER();
        nrf_ppi_channels_include_in_group(channel_mask, group);
        CRITICAL_REGION_EXIT();
        err_code = NRF_SUCCESS;

#endif
    }
    
    return err_code;
}
