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

#define NRF51_GPIOTE_CHANNEL 4
#define NRF51_PINS           31

#ifdef NRF51
#define SWI_IRQHandler SWI1_IRQHandler
#define SWI_IRQn       SWI1_IRQn
#elif defined NRF52
#define SWI_IRQHandler SWI1_EGU1_IRQHAndler
#define SWI_IRQn       SWI1_EGU1_IRQn
#endif

/**@brief GPIOTE user type. */
typedef struct
{
    uint32_t                   pins_mask;             /**< Mask defining which pins user wants to monitor. */
    uint32_t                   pins_low_to_high_mask; /**< Mask defining which pins will generate events to this user when toggling low->high. */
    uint32_t                   pins_high_to_low_mask; /**< Mask defining which pins will generate events to this user when toggling high->low. */
    uint32_t                   sense_high_pins;       /**< Mask defining which pins are configured to generate GPIOTE interrupt on transition to high level. */
    app_gpiote_event_handler_t event_handler;         /**< Pointer to function to be executed when an event occurs. */
} gpiote_user_t;

STATIC_ASSERT(sizeof (gpiote_user_t) <= GPIOTE_USER_NODE_SIZE);
STATIC_ASSERT(sizeof (gpiote_user_t) % 4 == 0);

static uint32_t        m_enabled_users_mask; /**< Mask for tracking which users are enabled. */
static uint8_t         m_user_array_size;    /**< Size of user array. */
static uint8_t         m_user_count;         /**< Number of registered users. */
static gpiote_user_t * mp_users = NULL;      /**< Array of GPIOTE users. */


static app_gpiote_input_event_handler_t m_app_gpiote_input_event_handlers[4] = {0};

static app_gpiote_input_event_handler_t m_app_gpiote_end_irq_event_handler = NULL;

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

            //Invert sensing.
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


/**@brief Function for handling the GPIOTE interrupt.
 */
void GPIOTE_IRQHandler(void)
{
    bool gpiote_in_evt   = false;
    bool gpiote_port_evt = false;

    if ((NRF_GPIOTE->EVENTS_IN[0] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk))
    {
        NRF_GPIOTE->EVENTS_IN[0] = 0;
        gpiote_in_evt            = true;

        if (m_app_gpiote_input_event_handlers[0])
        {
            m_app_gpiote_input_event_handlers[0]();
        }
    }

    if ((NRF_GPIOTE->EVENTS_IN[1] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN1_Msk))
    {
        NRF_GPIOTE->EVENTS_IN[1] = 0;
        gpiote_in_evt            = true;

        if (m_app_gpiote_input_event_handlers[1])
        {
            m_app_gpiote_input_event_handlers[1]();
        }
    }

    if ((NRF_GPIOTE->EVENTS_IN[2] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN2_Msk))
    {
        NRF_GPIOTE->EVENTS_IN[2] = 0;
        gpiote_in_evt            = true;

        if (m_app_gpiote_input_event_handlers[2])
        {
            m_app_gpiote_input_event_handlers[2]();
        }
    }

    if ((NRF_GPIOTE->EVENTS_IN[3] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN3_Msk))
    {
        NRF_GPIOTE->EVENTS_IN[3] = 0;
        gpiote_in_evt            = true;

        if (m_app_gpiote_input_event_handlers[3])
        {
            m_app_gpiote_input_event_handlers[3]();
        }
    }

    if ((NRF_GPIOTE->EVENTS_PORT == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_PORT_Msk))
    {
        //Clear event.
        NRF_GPIOTE->EVENTS_PORT = 0;

        gpiote_port_evt = true;
    }

    if (m_app_gpiote_end_irq_event_handler && gpiote_in_evt)
    {
        m_app_gpiote_end_irq_event_handler();
    }

    if (gpiote_port_evt)
    {
        NVIC_SetPendingIRQ(SWI_IRQn);
    }
}


/**@brief Function for handling the SWI1 interrupt.
 */
void SWI_IRQHandler(void)
{
    uint8_t  i;
    uint32_t pins_changed;
    uint32_t pins_state = NRF_GPIO->IN;

    //Check all users.
    for (i = 0; i < m_user_count; i++)
    {
        gpiote_user_t * p_user = &mp_users[i];

        //Check if user is enabled.
        if (((1 << i) & m_enabled_users_mask) != 0)
        {
            uint32_t transition_pins;
            uint32_t event_low_to_high;
            uint32_t event_high_to_low;

            //Find set of pins on which there has been a transition.
            transition_pins = (pins_state ^ ~p_user->sense_high_pins) & p_user->pins_mask;

            //Toggle SENSE level for all pins that have changed state.
            sense_level_toggle(p_user, transition_pins);

            //Second read after setting sense.
            //Check if any pins have changed while serving this interrupt.
            pins_changed = NRF_GPIO->IN ^ pins_state;

            if (pins_changed)
            {
                //Transition pins detected in late stage.
                uint32_t late_transition_pins;

                pins_state |= pins_changed;

                //Find set of pins on which there has been a transition.
                late_transition_pins = (pins_state ^ ~p_user->sense_high_pins) & p_user->pins_mask;

                //Toggle SENSE level for all pins that have changed state in last phase.
                sense_level_toggle(p_user, late_transition_pins);

                //Update pins that has changed state since the interrupt occurred.
                transition_pins |= late_transition_pins;
            }

            //Call user event handler if an event has occurred.
            event_high_to_low = (~pins_state & p_user->pins_high_to_low_mask) & transition_pins;
            event_low_to_high = (pins_state & p_user->pins_low_to_high_mask) & transition_pins;

            if ((event_low_to_high | event_high_to_low) != 0)
            {
                p_user->event_handler(event_low_to_high, event_high_to_low);
            }
        }
    }
}


/**@brief Function for enabling interrupt SWI1.
 */
static void app_gpiote_swi1_enable_irq(void)
{
    NVIC_ClearPendingIRQ(SWI_IRQn);
    NVIC_SetPriority(SWI_IRQn, APP_IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(SWI_IRQn);
}

/**@brief Function for disabling interrupt SWI1.
 */
static void app_gpiote_swi1_disable_irq(void)
{
    NVIC_DisableIRQ(SWI_IRQn);
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

    //Check that buffer is correctly aligned.
    if (!is_word_aligned(p_buffer))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    //Initialize file globals.
    mp_users             = (gpiote_user_t *)p_buffer;
    m_user_array_size    = max_users;
    m_user_count         = 0;
    m_enabled_users_mask = 0;

    memset(mp_users, 0, m_user_array_size * sizeof (gpiote_user_t));

    (void)app_gpiote_enable_interrupts();

    return NRF_SUCCESS;
}

uint32_t app_gpiote_user_register(app_gpiote_user_id_t *     p_user_id,
                                  uint32_t                   pins_low_to_high_mask,
                                  uint32_t                   pins_high_to_low_mask,
                                  app_gpiote_event_handler_t event_handler)
{
    //Check state and parameters.
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

    //Allocate new user.
    mp_users[m_user_count].pins_mask             = pins_low_to_high_mask | pins_high_to_low_mask;
    mp_users[m_user_count].pins_low_to_high_mask = pins_low_to_high_mask;
    mp_users[m_user_count].pins_high_to_low_mask = pins_high_to_low_mask;
    mp_users[m_user_count].event_handler         = event_handler;

    *p_user_id = m_user_count++;

    //Make sure SENSE is disabled for all pins.
    pins_sense_disable(*p_user_id);

    return NRF_SUCCESS;
}

uint32_t app_gpiote_enable_interrupts(void)
{
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    NVIC_SetPriority(GPIOTE_IRQn, APP_IRQ_PRIORITY_HIGH);
    NVIC_EnableIRQ(GPIOTE_IRQn);

    //Enable interrupt SWI1.
    app_gpiote_swi1_enable_irq();

    return NRF_SUCCESS;
}

uint32_t app_gpiote_disable_interrupts(void)
{
    NVIC_DisableIRQ(GPIOTE_IRQn);

    //Disable interrupt SWI1.
    app_gpiote_swi1_disable_irq();

    return NRF_SUCCESS;
}

uint32_t app_gpiote_input_event_handler_register(const uint8_t                    channel,
                                                 const uint32_t                   pin,
                                                 const uint32_t                   polarity,
                                                 app_gpiote_input_event_handler_t event_handler)
{
    if (channel < NRF51_GPIOTE_CHANNEL && pin < NRF51_PINS)
    {
        NRF_GPIOTE->CONFIG[channel] = (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) |
                                      (pin << GPIOTE_CONFIG_PSEL_Pos) |
                                      (polarity << GPIOTE_CONFIG_POLARITY_Pos);

        switch (channel)
        {
            case 0:
                NRF_GPIOTE->INTENSET |= (GPIOTE_INTENSET_IN0_Enabled << GPIOTE_INTENSET_IN0_Pos);
                break;

            case 1:
                NRF_GPIOTE->INTENSET |= (GPIOTE_INTENSET_IN1_Enabled << GPIOTE_INTENSET_IN1_Pos);
                break;

            case 2:
                NRF_GPIOTE->INTENSET |= (GPIOTE_INTENSET_IN2_Enabled << GPIOTE_INTENSET_IN2_Pos);
                break;

            case 3:
                NRF_GPIOTE->INTENSET |= (GPIOTE_INTENSET_IN3_Enabled << GPIOTE_INTENSET_IN3_Pos);
                break;

            default:
                break;
        }

        m_app_gpiote_input_event_handlers[channel] = event_handler;

        return NRF_SUCCESS;
    }

    return NRF_ERROR_INVALID_PARAM;
}

uint32_t app_gpiote_input_event_handler_unregister(const uint8_t channel)
{
    if (channel < NRF51_GPIOTE_CHANNEL)
    {
        const uint32_t pin_not_connected = 31UL;

        NRF_GPIOTE->CONFIG[channel] = (GPIOTE_CONFIG_MODE_Disabled << GPIOTE_CONFIG_MODE_Pos) |
                                      (pin_not_connected << GPIOTE_CONFIG_PSEL_Pos) |
                                      (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos);

        switch (channel)
        {
            case 0:
                NRF_GPIOTE->INTENSET |= (GPIOTE_INTENSET_IN0_Disabled << GPIOTE_INTENSET_IN0_Pos);
                break;

            case 1:
                NRF_GPIOTE->INTENSET |= (GPIOTE_INTENSET_IN1_Disabled << GPIOTE_INTENSET_IN1_Pos);
                break;

            case 2:
                NRF_GPIOTE->INTENSET |= (GPIOTE_INTENSET_IN2_Disabled << GPIOTE_INTENSET_IN2_Pos);
                break;

            case 3:
                NRF_GPIOTE->INTENSET |= (GPIOTE_INTENSET_IN3_Disabled << GPIOTE_INTENSET_IN3_Pos);
                break;

            default:
                break;
        }

        m_app_gpiote_input_event_handlers[channel] = NULL;

        return NRF_SUCCESS;
    }

    return NRF_ERROR_INVALID_PARAM;
}

uint32_t app_gpiote_end_irq_event_handler_register(app_gpiote_input_event_handler_t event_handler)
{
    m_app_gpiote_end_irq_event_handler = event_handler;
    return NRF_SUCCESS;
}

uint32_t app_gpiote_end_irq_event_handler_unregister(void)
{
    m_app_gpiote_end_irq_event_handler = NULL;
    return NRF_SUCCESS;
}

uint32_t app_gpiote_user_enable(app_gpiote_user_id_t user_id)
{
    uint32_t pin_no;
    uint32_t pins_state;

    //Check state and parameters.
    if (mp_users == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (user_id >= m_user_count)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    //Clear any pending event.
    NRF_GPIOTE->EVENTS_PORT = 0;
    pins_state              = NRF_GPIO->IN;

    //Enable user.
    if (m_enabled_users_mask == 0)
    {
        NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
    }
    m_enabled_users_mask |= (1 << user_id);

    //Enable sensing for all pins for specified user.
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
                sense = GPIO_PIN_CNF_SENSE_High <<
                        GPIO_PIN_CNF_SENSE_Pos;
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
    //Check state and parameters.
    if (mp_users == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (user_id >= m_user_count)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    //Disable sensing for all pins for specified user.
    pins_sense_disable(user_id);

    //Disable user.
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

    //Check state and parameters.
    if (mp_users == NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (user_id >= m_user_count)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    //Get pins.
    p_user  = &mp_users[user_id];
    *p_pins = NRF_GPIO->IN & p_user->pins_mask;

    return NRF_SUCCESS;
}
