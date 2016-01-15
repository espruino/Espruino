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

#include "app_gpiote.h"
#include "nrf_drv_gpiote.h"
#include "sdk_common.h"


/**@brief GPIOTE user type. */
typedef struct
{
    uint32_t                   pins_mask;             /**< Mask defining which pins user wants to monitor. */
    uint32_t                   pins_low_to_high_mask; /**< Mask defining which pins will generate events to this user when toggling low->high. */
    uint32_t                   pins_high_to_low_mask; /**< Mask defining which pins will generate events to this user when toggling high->low. */
    uint32_t                   sense_high_pins;       /**< Mask defining which pins are configured to generate GPIOTE interrupt on transition to high level. */
    app_gpiote_event_handler_t event_handler;         /**< Pointer to function to be executed when an event occurs. */
    bool                       enabled;               /**< Flag indicating whether user is enabled. */
} gpiote_user_t;

STATIC_ASSERT(sizeof(gpiote_user_t) <= GPIOTE_USER_NODE_SIZE);
STATIC_ASSERT(sizeof(gpiote_user_t) % 4 == 0);

static uint8_t         m_user_array_size;             /**< Size of user array. */
static uint8_t         m_user_count;                  /**< Number of registered users. */
static gpiote_user_t * mp_users = NULL;               /**< Array of GPIOTE users. */
static uint32_t        m_pins;                        /**< Mask of initialized pins. */
static uint32_t        m_last_pins_state;             /**< Most recent state of pins. */

#define MODULE_INITIALIZED (mp_users != NULL)
#include "sdk_macros.h"

void gpiote_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    int i;
    uint32_t pin_mask = 1 << pin;
    bool hitolo = (m_last_pins_state & pin_mask) ? true : false;
    m_last_pins_state = nrf_gpio_pins_read();

    for (i = 0; i < m_user_count; i++)
    {
        if (mp_users[i].enabled && (pin_mask & mp_users[i].pins_mask))
        {
            if ((pin_mask & mp_users[i].pins_high_to_low_mask) && hitolo)
            {
                mp_users[i].event_handler(0,pin_mask);
            }
            else if ((pin_mask & mp_users[i].pins_low_to_high_mask) && !hitolo)
            {
                mp_users[i].event_handler(pin_mask,0);
            }
        }
    }
}

uint32_t app_gpiote_init(uint8_t max_users, void * p_buffer)
{
    uint32_t ret_code = NRF_SUCCESS;

    if (p_buffer == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Check that buffer is correctly aligned.
    if (!is_word_aligned(p_buffer))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Initialize file globals.
    mp_users             = (gpiote_user_t *)p_buffer;
    m_user_array_size    = max_users;
    m_user_count         = 0;
    m_pins              = 0;

    memset(mp_users, 0, m_user_array_size * sizeof(gpiote_user_t));

    if (nrf_drv_gpiote_is_init()==false)
    {
        ret_code = nrf_drv_gpiote_init();
    }

    return ret_code;
}

uint32_t app_gpiote_user_register(app_gpiote_user_id_t     * p_user_id,
                                  uint32_t                   pins_low_to_high_mask,
                                  uint32_t                   pins_high_to_low_mask,
                                  app_gpiote_event_handler_t event_handler)
{
    uint32_t user_pin_mask;
    uint32_t ret_val = NRF_SUCCESS;

    // Check state and parameters.
    VERIFY_MODULE_INITIALIZED();

    if (event_handler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (m_user_count >= m_user_array_size)
    {
        return NRF_ERROR_NO_MEM;
    }

    user_pin_mask = pins_low_to_high_mask | pins_high_to_low_mask;
    // Allocate new user.
    mp_users[m_user_count].pins_mask             = user_pin_mask;
    mp_users[m_user_count].pins_low_to_high_mask = pins_low_to_high_mask;
    mp_users[m_user_count].pins_high_to_low_mask = pins_high_to_low_mask;
    mp_users[m_user_count].event_handler         = event_handler;
    mp_users[m_user_count].enabled               = false;

    *p_user_id = m_user_count++;

    uint32_t mask = 1;
    uint32_t i;
    const nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    for (i = 0; i < 32; i++)
    {
        if ((mask & user_pin_mask) & ~m_pins)
        {
            ret_val = nrf_drv_gpiote_in_init(i, &config, gpiote_handler);
            VERIFY_SUCCESS(ret_val);
            m_pins |= mask;
        }
        mask <<= 1;
    }
    return ret_val;
}

__STATIC_INLINE uint32_t error_check(app_gpiote_user_id_t user_id)
{
    // Check state and parameters.
    VERIFY_MODULE_INITIALIZED();

    if (user_id >= m_user_count)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    return NRF_SUCCESS;
}
/**
 * @brief Function for enabling event on pin (if not yet enabled) or disabling the event if no other
 *        user requires it.
 *
 * @param pin    Pin to enable
 * @param enable If true function will attempt to enable the pin else it will attempt to disable it.
 */
static void pin_event_enable(uint32_t pin, bool enable)
{
    uint32_t i;
    uint32_t pin_mask = 1UL << pin;
    bool enabled = false;
    //search if any user already enabled given pin
    for (i = 0; i < m_user_count; i++)
    {
        if (mp_users[i].enabled && (mp_users[i].pins_mask & pin_mask))
        {
            enabled = true;
            break;
        }
    }
    if (!enabled)
    {
        if (enable)
        {
            m_last_pins_state = nrf_gpio_pins_read();
            nrf_drv_gpiote_in_event_enable(pin, true);
        }
        else
        {
            nrf_drv_gpiote_in_event_disable(pin);
        }
    }
}

/**
 * @brief Function for enabling or disabling events for pins used by the user.
 *
 * Function will enable pin events only if they are not yet enabled. Function will disable pin
 * events only if there is no other enabled user that is using them.
 *
 * @param user_id  User id.
 * @param enable   If true function will attempt to enable the pin else it will attempt to disable it.
 */
static uint32_t user_enable(app_gpiote_user_id_t user_id, bool enable)
{
    uint32_t ret_code = error_check(user_id);

    if (ret_code == NRF_SUCCESS)
    {
        uint32_t i;
        uint32_t mask = 1UL;
        for (i = 0; i < 32; i++)
        {
            if (mp_users[user_id].pins_mask & mask)
            {
                pin_event_enable(i, enable);
            }
            mask <<= 1;
        }
    }
    return ret_code;
}

uint32_t app_gpiote_user_enable(app_gpiote_user_id_t user_id)
{
    uint32_t ret_code = NRF_SUCCESS;

    if (mp_users[user_id].enabled == false)
    {
        ret_code = user_enable(user_id, true);
        VERIFY_SUCCESS(ret_code);

        mp_users[user_id].enabled = true;
        return ret_code;
    }
    else
    {
        return ret_code;
    }
}

uint32_t app_gpiote_user_disable(app_gpiote_user_id_t user_id)
{
    uint32_t ret_code = NRF_SUCCESS;

    if (mp_users[user_id].enabled)
    {
        mp_users[user_id].enabled = false;
        ret_code = user_enable(user_id, false);
    }

    return ret_code;
}

uint32_t app_gpiote_pins_state_get(app_gpiote_user_id_t user_id, uint32_t * p_pins)
{
    gpiote_user_t * p_user;
    uint32_t ret_code = error_check(user_id);

    if (ret_code == NRF_SUCCESS)
    {
        p_user  = &mp_users[user_id];
        // Get pins.
        *p_pins = nrf_gpio_pins_read() & p_user->pins_mask;

    }
    return ret_code;
}
