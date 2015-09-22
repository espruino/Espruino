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

#include "app_gpiote.h"
#include <stdlib.h>
#include <string.h>
#include "app_util.h"
#include "app_util_platform.h"
#include "nrf_error.h"
#include "nrf_gpio.h"


/**@brief GPIOTE user type. */
typedef struct
{
    uint32_t                   pins_mask;             /**< Mask defining which pins user wants to monitor. */
    uint32_t                   pins_low_to_high_mask; /**< Mask defining which pins will generate events to this user when toggling low->high. */
    uint32_t                   pins_high_to_low_mask; /**< Mask defining which pins will generate events to this user when toggling high->low. */
    uint32_t                   sense_high_pins;       /**< Mask defining which pins are configured to generate GPIOTE interrupt on transition to high level. */
    app_gpiote_event_handler_t event_handler;         /**< Pointer to function to be executed when an event occurs. */
} gpiote_user_t;

STATIC_ASSERT(sizeof(gpiote_user_t) <= GPIOTE_USER_NODE_SIZE);
STATIC_ASSERT(sizeof(gpiote_user_t) % 4 == 0);

static uint32_t        m_enabled_users_mask;          /**< Mask for tracking which users are enabled. */
static uint8_t         m_user_array_size;             /**< Size of user array. */
static uint8_t         m_user_count;                  /**< Number of registered users. */
static gpiote_user_t * mp_users = NULL;               /**< Array of GPIOTE users. */


/**@brief Function for toggling sense level for specified pins.
 *
 * @param[in]   p_user   Pointer to user structure.
 * @param[in]   pins     Bitmask specifying for which pins the sense level is to be toggled.
 */
static void sense_level_toggle(gpiote_user_t * p_user, uint32_t pins)
{
    uint32_t pin_no;

    for (pin_no = 0; pin_no < NO_OF_PINS; pin_no++)
    {
        uint32_t pin_mask = (1 << pin_no);

        if ((pins & pin_mask) != 0)
        {
            uint32_t sense;

            // Invert sensing.
            if ((p_user->sense_high_pins & pin_mask) == 0)
            {
                sense                    = GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos;
                p_user->sense_high_pins |= pin_mask;
            }
            else
            {
                sense                    = GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos;
                p_user->sense_high_pins &= ~pin_mask;
            }

            NRF_GPIO->PIN_CNF[pin_no] &= ~GPIO_PIN_CNF_SENSE_Msk;
            NRF_GPIO->PIN_CNF[pin_no] |= sense;
        }
    }
}


static void sense_level_disable(uint32_t pins)
{
    uint32_t pin_no;

    for (pin_no = 0; pin_no < 32; pin_no++)
    {
        uint32_t pin_mask = (1 << pin_no);

        if ((pins & pin_mask) != 0)
        {
            NRF_GPIO->PIN_CNF[pin_no] &= ~GPIO_PIN_CNF_SENSE_Msk;
            NRF_GPIO->PIN_CNF[pin_no] |= GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos;
        }
    }
}



/**@brief Function for handling the GPIOTE interrupt.
 */
void GPIOTE_IRQHandler(void)
{
    uint8_t  i;
    uint32_t pins_changed        = 1;
    uint32_t pins_sense_enabled  = 0;
    uint32_t pins_sense_disabled = 0;
    uint32_t pins_state          = NRF_GPIO->IN;

    // Clear event.
    NRF_GPIOTE->EVENTS_PORT = 0;

    while (pins_changed)
    {
        // Check all users.
        for (i = 0; i < m_user_count; i++)
        {
            gpiote_user_t * p_user = &mp_users[i];

            // Check if user is enabled.
            if (((1 << i) & m_enabled_users_mask) != 0)
            {
                uint32_t transition_pins;
                uint32_t event_low_to_high = 0;
                uint32_t event_high_to_low = 0;

                pins_sense_enabled |= (p_user->pins_mask & ~pins_sense_disabled);

                // Find set of pins on which there has been a transition.
                transition_pins = (pins_state ^ ~p_user->sense_high_pins) & (p_user->pins_mask & ~pins_sense_disabled);

                sense_level_disable(transition_pins);
                pins_sense_disabled |= transition_pins;
                pins_sense_enabled  &= ~pins_sense_disabled;

                // Call user event handler if an event has occurred.
                event_high_to_low |= (~pins_state & p_user->pins_high_to_low_mask) & transition_pins;
                event_low_to_high |= (pins_state & p_user->pins_low_to_high_mask) & transition_pins;

                if ((event_low_to_high | event_high_to_low) != 0)
                {
                    p_user->event_handler(event_low_to_high, event_high_to_low);
                }
            }
        }

        // Second read after setting sense.
        // Check if any pins with sense enabled have changed while serving this interrupt.
        pins_changed = (NRF_GPIO->IN ^ pins_state) & pins_sense_enabled;
        pins_state  ^= pins_changed;
    }

    // Now re-enabling sense on all pins that have sense disabled.
    // Note: a new interrupt might fire immediatly.
    for (i = 0; i < m_user_count; i++)
    {
        gpiote_user_t * p_user = &mp_users[i];

        // Check if user is enabled.
        if (((1 << i) & m_enabled_users_mask) != 0)
        {
            if (pins_sense_disabled & p_user->pins_mask)
            {
                sense_level_toggle(p_user, pins_sense_disabled & p_user->pins_mask);
            }
        }
    }
}


/**@brief Function for sense disabling for all pins for specified user.
 *
 * @param[in]  user_id   User id.
 */
static void pins_sense_disable(app_gpiote_user_id_t user_id)
{
    uint32_t pin_no;

    for (pin_no = 0; pin_no < 32; pin_no++)
    {
        if ((mp_users[user_id].pins_mask & (1 << pin_no)) != 0)
        {
            NRF_GPIO->PIN_CNF[pin_no] &= ~GPIO_PIN_CNF_SENSE_Msk;
            NRF_GPIO->PIN_CNF[pin_no] |= GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos;
        }
    }
}


uint32_t app_gpiote_init(uint8_t max_users, void * p_buffer)
{
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
    m_enabled_users_mask = 0;

    memset(mp_users, 0, m_user_array_size * sizeof(gpiote_user_t));

    // Initialize GPIOTE interrupt (will not be enabled until app_gpiote_user_enable() is called).
    NRF_GPIOTE->INTENCLR = 0xFFFFFFFF;

    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    NVIC_SetPriority(GPIOTE_IRQn, APP_IRQ_PRIORITY_HIGH);
    NVIC_EnableIRQ(GPIOTE_IRQn);

    return NRF_SUCCESS;
}


uint32_t app_gpiote_user_register(app_gpiote_user_id_t     * p_user_id,
                                  uint32_t                   pins_low_to_high_mask,
                                  uint32_t                   pins_high_to_low_mask,
                                  app_gpiote_event_handler_t event_handler)
{
    // Check state and parameters.
    if (mp_users == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (event_handler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    if (m_user_count >= m_user_array_size)
    {
        return NRF_ERROR_NO_MEM;
    }

    // Allocate new user.
    mp_users[m_user_count].pins_mask             = pins_low_to_high_mask | pins_high_to_low_mask;
    mp_users[m_user_count].pins_low_to_high_mask = pins_low_to_high_mask;
    mp_users[m_user_count].pins_high_to_low_mask = pins_high_to_low_mask;
    mp_users[m_user_count].event_handler         = event_handler;

    *p_user_id = m_user_count++;

    // Make sure SENSE is disabled for all pins.
    pins_sense_disable(*p_user_id);

    return NRF_SUCCESS;
}


uint32_t app_gpiote_user_enable(app_gpiote_user_id_t user_id)
{
    uint32_t pin_no;
    uint32_t pins_state;

    // Check state and parameters.
    if (mp_users == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (user_id >= m_user_count)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Clear any pending event.
    NRF_GPIOTE->EVENTS_PORT = 0;
    pins_state              = NRF_GPIO->IN;

    // Enable user.
    if (m_enabled_users_mask == 0)
    {
        NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
    }
    m_enabled_users_mask |= (1 << user_id);

    // Enable sensing for all pins for specified user.
    mp_users[user_id].sense_high_pins = 0;
    for (pin_no = 0; pin_no < 32; pin_no++)
    {
        uint32_t pin_mask = (1 << pin_no);

        if ((mp_users[user_id].pins_mask & pin_mask) != 0)
        {
            uint32_t sense;

            if ((pins_state & pin_mask) != 0)
            {
                sense = GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos;
            }
            else
            {
                sense = GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos;
                mp_users[user_id].sense_high_pins |= pin_mask;
            }

            NRF_GPIO->PIN_CNF[pin_no] &= ~GPIO_PIN_CNF_SENSE_Msk;
            NRF_GPIO->PIN_CNF[pin_no] |= sense;
        }
    }

    return NRF_SUCCESS;
}


uint32_t app_gpiote_user_disable(app_gpiote_user_id_t user_id)
{
    // Check state and parameters.
    if (mp_users == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (user_id >= m_user_count)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Disable sensing for all pins for specified user.
    pins_sense_disable(user_id);

    // Disable user.
    m_enabled_users_mask &= ~(1UL << user_id);
    if (m_enabled_users_mask == 0)
    {
        NRF_GPIOTE->INTENCLR = GPIOTE_INTENSET_PORT_Msk;
    }

    return NRF_SUCCESS;
}


uint32_t app_gpiote_pins_state_get(app_gpiote_user_id_t user_id, uint32_t * p_pins)
{
    gpiote_user_t * p_user;

    // Check state and parameters.
    if (mp_users == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    if (user_id >= m_user_count)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    // Get pins.
    p_user  = &mp_users[user_id];
    *p_pins = NRF_GPIO->IN & p_user->pins_mask;

    return NRF_SUCCESS;
}

#if defined(SVCALL_AS_NORMAL_FUNCTION) || defined(SER_CONNECTIVITY)
uint32_t app_gpiote_input_event_handler_register(const uint8_t                    channel,
                                                 const uint32_t                   pin,
                                                 const uint32_t                   polarity,
                                                 app_gpiote_input_event_handler_t event_handler)
{
    (void)sense_level_toggle(NULL, pin);
    return NRF_ERROR_NOT_SUPPORTED;
}

uint32_t app_gpiote_input_event_handler_unregister(const uint8_t channel)
{
    return NRF_ERROR_NOT_SUPPORTED;
}

uint32_t app_gpiote_end_irq_event_handler_register(app_gpiote_input_event_handler_t event_handler)
{
    return NRF_ERROR_NOT_SUPPORTED;
}

uint32_t app_gpiote_end_irq_event_handler_unregister(void)
{
    return NRF_ERROR_NOT_SUPPORTED;
}

uint32_t app_gpiote_enable_interrupts(void)
{
    return NRF_ERROR_NOT_SUPPORTED;
}

uint32_t app_gpiote_disable_interrupts(void)
{
    return NRF_ERROR_NOT_SUPPORTED;
}
#endif // SVCALL_AS_NORMAL_FUNCTION || SER_CONNECTIVITY
