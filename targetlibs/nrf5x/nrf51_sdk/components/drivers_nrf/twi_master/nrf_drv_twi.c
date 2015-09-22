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

#include "nrf_drv_twi.h"
#include "nrf_assert.h"
#include "app_util_platform.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

/**@brief Determine how long driver will be wait for event (in blocking mode). */
#define BUSY_LOOP_TIMEOUT   0xFFF

#define DISABLE_MASK        UINT32_MAX

#define SCL_PIN_CONF        ((GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)    \
                            | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos)   \
                            | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)    \
                            | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos)   \
                            | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos))
#define SDA_PIN_CONF        SCL_PIN_CONF

#define SCL_PIN_CONF_CLR    ((GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)    \
                            | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos)   \
                            | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)    \
                            | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos)   \
                            | (GPIO_PIN_CNF_DIR_Output     << GPIO_PIN_CNF_DIR_Pos))
#define SDA_PIN_CONF_CLR    SCL_PIN_CONF_CLR

/**@brief TWI powered on substates. */
typedef enum
{
    NRF_DRV_TWI_SUBSTATE_IDLE,          /**< Idle state. Transmission is stoped. */
    NRF_DRV_TWI_SUBSTATE_SUSPEND,       /**< Tx transmission is suspended. */
    NRF_DRV_TWI_SUBSTATE_RX_SUSPEND,    /**< Rx transmission is suspended. */
    NRF_DRV_TWI_SUBSTATE_STOP,          /**< Transmission will be stoped. */
    NRF_DRV_TWI_SUBSTATE_ERROR          /**< Error condition. */         
} substate_t;

/**@brief State machine events. */
typedef enum
{
    TX_ADDR_REQ,
    RX_ADDR_REQ,
    TX_DONE,
    RX_DONE,
    TO_STOP,
    ON_ERROR
} sm_evt_t;

/**@brief TWI transfer structure. */
typedef struct
{
    bool             is_tx;                 /**< Transmission from master to slave (if is_tx is set) or from slave to master (if not). */
    bool             xfer_pending;          /**< Transmission will be suspended (if xfer_pending is set) or stopped (if not). */
    bool             transfer_in_progress;  /**< Data transfer is in progress. */
    bool             error_condition;       /**< Error has occured. */
    uint8_t          address;               /**< Address of slave device. */
    uint8_t        * p_data;                /**< Pointer to data buffer. */
    uint16_t         length;                /**< Length of data buffer. */
    uint16_t         count;                 /**< Count of already transferred bytes. */
    nrf_twi_tasks_t  task;                  /**< Next task. */
    nrf_twi_events_t end_event;             /**< Event for which the state machine waits. */
    nrf_twi_int_mask_t end_int;             /**< Interrupt for which the state machine waits. */
} transfer_t;

/**@brief TWI driver instance control block structure. */
typedef struct
{
    nrf_drv_state_t state;    /**< Instance state. */
    substate_t      substate; /**< Instance substate when powered on. */
    transfer_t      transfer; /**< Transfer structure. */
} control_block_t;

/**@brief Macro for getting the number of elements of the array. */
#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a[0]))

/** @brief User callbacks storage. */
static volatile nrf_drv_twi_evt_handler_t m_handlers[TWI_COUNT];

/** @brief Control blocks storage. */
static volatile control_block_t           m_cb[TWI_COUNT];

/** @brief TWI instances pointers storage. */
static volatile nrf_drv_twi_t *           m_instances[TWI_COUNT];

static const nrf_drv_twi_config_t m_default_config[] = {
#if (TWI0_ENABLED == 1)
    NRF_DRV_TWI_DEFAULT_CONFIG(0),
#endif
#if (TWI1_ENABLED == 1)
    NRF_DRV_TWI_DEFAULT_CONFIG(1)
#endif
};

/**@brief TWI state machine action. */
typedef void (* sm_action_t)(volatile nrf_drv_twi_t const * const p_instance);

/**@brief TWI transition structure. */
typedef struct
{
    sm_evt_t    evt;
    substate_t  next_state;
    sm_action_t func;
} transition_t;

/**
 * @brief Function for initializing data transfer (from master to slave).
 *
 * @param[in] p_instance      TWI.
 */
static void address_req(volatile nrf_drv_twi_t const * const p_instance);

/**
 * @brief Function for initializing data transfer (from slave to master).
 *
 * @param[in] p_instance      TWI.
 */
static void rx_address_req(volatile nrf_drv_twi_t const * const p_instance);

/**
 * @brief Function for preparing byte transfer (from master to slave).
 *
 * @param[in] p_instance      TWI.
 */
static void tx_prepare(volatile nrf_drv_twi_t const * const p_instance);

/**
 * @brief Function for confirming byte transfer (from master to slave).
 *
 * @param[in] p_instance      TWI.
 */
static void tx_done(volatile nrf_drv_twi_t const * const p_instance);

/**
 * @brief Function for preparing byte transfer (from slave to master).
 *
 * @param[in] p_instance      TWI.
 */
static void rx_prepare(volatile nrf_drv_twi_t const * const p_instance);

/**
 * @brief Function for confirming byte transfer (from slave to master).
 *
 * @param[in] p_instance      TWI.
 */
static void rx_done(volatile nrf_drv_twi_t const * const p_instance);

/**
 * @brief Function for ending transfer after an error.
 *
 * @param[in] p_instance      TWI.
 */
static void on_error(volatile nrf_drv_twi_t const * const p_instance);

/**
 * @brief Function for ending transfer after an error.
 *
 * @param[in] p_instance      TWI.
 */
static void ack_error(volatile nrf_drv_twi_t const * const p_instance);

/**@brief List of idle state transitions. */
static const transition_t m_idle[] = {
    {TX_ADDR_REQ,  NRF_DRV_TWI_SUBSTATE_SUSPEND,    address_req},
    {RX_ADDR_REQ,  NRF_DRV_TWI_SUBSTATE_RX_SUSPEND, rx_address_req},
};

/**@brief List of TX suspended state transitions. */
static const transition_t m_tx_suspended[] = {
    {TX_DONE,         NRF_DRV_TWI_SUBSTATE_SUSPEND,    tx_done},
    {RX_ADDR_REQ,     NRF_DRV_TWI_SUBSTATE_RX_SUSPEND, rx_address_req},
    {TX_ADDR_REQ,     NRF_DRV_TWI_SUBSTATE_SUSPEND,    tx_prepare},
    {TO_STOP,         NRF_DRV_TWI_SUBSTATE_STOP,       NULL},
    {ON_ERROR,        NRF_DRV_TWI_SUBSTATE_ERROR,      on_error}
};

/**@brief List of RX suspended state transitions. */
static const transition_t m_rx_suspended[] = {
    {RX_ADDR_REQ,     NRF_DRV_TWI_SUBSTATE_RX_SUSPEND, rx_prepare},
    {RX_DONE,         NRF_DRV_TWI_SUBSTATE_RX_SUSPEND, rx_done},
    {TO_STOP,         NRF_DRV_TWI_SUBSTATE_STOP,       NULL},
    {ON_ERROR,        NRF_DRV_TWI_SUBSTATE_ERROR,      on_error}
};

/**@brief List of stopped state transitions. */
static const transition_t m_stopped[] = {
    {RX_DONE,   NRF_DRV_TWI_SUBSTATE_IDLE,  rx_done},
    {TX_DONE,   NRF_DRV_TWI_SUBSTATE_IDLE,  tx_done},
    {ON_ERROR,  NRF_DRV_TWI_SUBSTATE_IDLE,  ack_error}
};

/**@brief List of error state transitions. */
static const transition_t m_error[] = {
    {RX_DONE,   NRF_DRV_TWI_SUBSTATE_IDLE,  ack_error},
    {TX_DONE,   NRF_DRV_TWI_SUBSTATE_IDLE,  ack_error},
    {ON_ERROR,  NRF_DRV_TWI_SUBSTATE_IDLE,  ack_error}
};

/**@brief List of state machine states. */
static const transition_t * mp_states[] = {
    m_idle,
    m_tx_suspended,
    m_rx_suspended,
    m_stopped,
    m_error
};

/**@brief List of amounts for each state transitions. */
static const uint32_t m_states_len[] =
{
    ARRAY_LENGTH(m_idle),
    ARRAY_LENGTH(m_tx_suspended),
    ARRAY_LENGTH(m_rx_suspended),
    ARRAY_LENGTH(m_stopped),
    ARRAY_LENGTH(m_error)
};

/**
 * @brief Function for running state machine.
 *
 * @param[in] p_instance      TWI.
 * @param[in] evt             State machine event.
 */
static void state_machine(volatile nrf_drv_twi_t const * const p_instance, sm_evt_t evt)
{
    bool                 evt_found = false;
    substate_t           substate  = m_cb[p_instance->instance_id].substate;
    transition_t const * p_state   = mp_states[substate];

    for (uint32_t i = 0; i < m_states_len[substate]; i++)
    {
        if (evt == p_state[i].evt)
        {
            m_cb[p_instance->instance_id].substate = p_state[i].next_state;

            if (p_state[i].func)
            {
                p_state[i].func(p_instance);
            }
            evt_found = true;
            break;
        }
    }
    UNUSED_VARIABLE(evt_found);
    ASSERT(evt_found);
}


/**
 * @brief Function for prepering shortcut register.
 *
 * @param[in] p_instance      TWI.
 */
static void txrx_shorts_set_task_start(volatile nrf_drv_twi_t const * const p_instance)
{
    uint32_t short_mask;
    volatile transfer_t * p_transfer = &(m_cb[p_instance->instance_id].transfer);

    nrf_twi_shorts_clear(p_instance->p_reg,
                         NRF_TWI_SHORTS_BB_SUSPEND_MASK | NRF_TWI_SHORTS_BB_STOP_MASK);

    // if the last one and no pending transfer prepare to wait for stopped event
    if (((p_transfer->count + 1) == p_transfer->length) && p_transfer->xfer_pending == false)
    {
        short_mask = NRF_TWI_SHORTS_BB_STOP_MASK;

        p_transfer->end_event = NRF_TWI_EVENTS_STOPPED;
        nrf_twi_event_clear(p_instance->p_reg, p_transfer->end_event);

        if (m_handlers[p_instance->instance_id])
        {
            nrf_twi_int_disable(p_instance->p_reg, p_transfer->end_int);
            p_transfer->end_int   = NRF_TWI_INT_STOPPED_MASK;
            nrf_twi_int_enable(p_instance->p_reg, p_transfer->end_int);
        }

        state_machine(p_instance, TO_STOP);
    }
    else
    {
        short_mask = NRF_TWI_SHORTS_BB_SUSPEND_MASK;
    }

    nrf_twi_shorts_set(p_instance->p_reg, short_mask);
    nrf_twi_tasks_t prev_task = p_transfer->task;
    p_transfer->task = NRF_TWI_TASKS_RESUME;
    nrf_twi_task_set(p_instance->p_reg, prev_task);
}


static void address_req(volatile nrf_drv_twi_t const * const p_instance)
{
    volatile transfer_t * p_transfer = &(m_cb[p_instance->instance_id].transfer);
    nrf_twi_address_set(p_instance->p_reg, p_transfer->address);

    nrf_twi_task_set(p_instance->p_reg, NRF_TWI_TASKS_RESUME);
    p_transfer->task = NRF_TWI_TASKS_STARTTX;
    p_transfer->end_event = NRF_TWI_EVENTS_TXDSENT;
    nrf_twi_event_clear(p_instance->p_reg, p_transfer->end_event);

    if (m_handlers[p_instance->instance_id])
    {
        nrf_twi_int_disable(p_instance->p_reg, p_transfer->end_int);
        p_transfer->end_int   = NRF_TWI_INT_TXDSENT_MASK;
        nrf_twi_int_enable(p_instance->p_reg, p_transfer->end_int);
    }

    tx_prepare(p_instance);
}


static void rx_address_req(volatile nrf_drv_twi_t const * const p_instance)
{
    volatile transfer_t * p_transfer = &(m_cb[p_instance->instance_id].transfer);

    nrf_twi_address_set(p_instance->p_reg, p_transfer->address);
    nrf_twi_task_set(p_instance->p_reg, NRF_TWI_TASKS_RESUME);
    p_transfer->task = NRF_TWI_TASKS_STARTRX;
    p_transfer->end_event = NRF_TWI_EVENTS_RXDREADY;
    nrf_twi_event_clear(p_instance->p_reg, p_transfer->end_event);
    if (m_handlers[p_instance->instance_id])
    {
        nrf_twi_int_disable(p_instance->p_reg, p_transfer->end_int);
        p_transfer->end_int   = NRF_TWI_INT_RXDREADY_MASK;
        nrf_twi_int_enable(p_instance->p_reg, p_transfer->end_int);
    }

    rx_prepare(p_instance);
}


static void rx_prepare(volatile nrf_drv_twi_t const * const p_instance)
{
    txrx_shorts_set_task_start(p_instance);
}


static void rx_done(volatile nrf_drv_twi_t const * const p_instance)
{
    volatile transfer_t * p_transfer = &(m_cb[p_instance->instance_id].transfer);

    p_transfer->p_data[p_transfer->count] = nrf_twi_rxd_get(p_instance->p_reg);
    p_transfer->count++;

    if (p_transfer->count < p_transfer->length)
    {
        rx_prepare(p_instance);
    }
}


static void tx_prepare(volatile nrf_drv_twi_t const * const p_instance)
{
    volatile transfer_t * p_transfer = &(m_cb[p_instance->instance_id].transfer);

    nrf_twi_txd_set(p_instance->p_reg, p_transfer->p_data[p_transfer->count]);
    txrx_shorts_set_task_start(p_instance);
}


static void tx_done(volatile nrf_drv_twi_t const * const p_instance)
{
    volatile transfer_t * p_transfer = &(m_cb[p_instance->instance_id].transfer);

    p_transfer->count++;

    if (p_transfer->count < p_transfer->length)
    {
        tx_prepare(p_instance);
    }
}


static void on_error(volatile nrf_drv_twi_t const * const p_instance)
{
    volatile transfer_t * p_transfer = &(m_cb[p_instance->instance_id].transfer);

    p_transfer->end_event            = NRF_TWI_EVENTS_STOPPED;
    nrf_twi_event_clear(p_instance->p_reg, p_transfer->end_event);

    if (m_handlers[p_instance->instance_id])
    {
        nrf_twi_int_disable(p_instance->p_reg, p_transfer->end_int);
        p_transfer->end_int   = NRF_TWI_INT_STOPPED_MASK;
        nrf_twi_int_enable(p_instance->p_reg, p_transfer->end_int);
    }

    nrf_twi_task_set(p_instance->p_reg, NRF_TWI_TASKS_RESUME);
    nrf_twi_task_set(p_instance->p_reg, NRF_TWI_TASKS_STOP);
}

static void ack_error(volatile nrf_drv_twi_t const * const p_instance)
{
    volatile transfer_t * p_transfer = &(m_cb[p_instance->instance_id].transfer);

    p_transfer->error_condition = true;
}

/**
 * @brief Function for blocking the module until desired event occurs.
 *
 * @param[in] p_instance      TWI.
 *
 * @return    False if any error has occurred.
 */
static bool twi_action_wait(nrf_drv_twi_t const * const p_instance)
{
    bool     error;
    bool     done;
    uint32_t timeout = 0;
    volatile transfer_t * p_transfer = &(m_cb[p_instance->instance_id].transfer);

    do
    {
        done  = nrf_twi_event_check(p_instance->p_reg, p_transfer->end_event);
        error = nrf_twi_event_check(p_instance->p_reg, NRF_TWI_EVENTS_ERROR);
        error |= (++timeout < BUSY_LOOP_TIMEOUT) ? false : true;
    } while (!(error | done));
    return !error;
}

static void twi_clear_bus(nrf_drv_twi_t const * const p_instance, nrf_drv_twi_config_t const * p_config)
{
    NRF_GPIO->PIN_CNF[p_config->scl] = SCL_PIN_CONF;
    NRF_GPIO->PIN_CNF[p_config->sda] = SDA_PIN_CONF;

    nrf_gpio_pin_set(p_config->scl);
    nrf_gpio_pin_set(p_config->sda);

    NRF_GPIO->PIN_CNF[p_config->scl] = SCL_PIN_CONF_CLR;
    NRF_GPIO->PIN_CNF[p_config->sda] = SDA_PIN_CONF_CLR;

    nrf_delay_us(4);

    for(int i = 0; i < 9; i++)
    {
        if (nrf_gpio_pin_read(p_config->sda))
        {
            if(i == 0)
            {
                return;
            }
            else
            {
                break;
            }
        }
        nrf_gpio_pin_clear(p_config->scl);
        nrf_delay_us(4);
        nrf_gpio_pin_set(p_config->scl);
        nrf_delay_us(4);
    }
    nrf_gpio_pin_clear(p_config->sda);
    nrf_delay_us(4);
    nrf_gpio_pin_set(p_config->sda);
}

/**
 * @brief Function for transferring data.
 *
 * @note Transmission will be stopped when error or timeout occurs.
 *
 * @param[in] p_instance      TWI.
 * @param[in] address         Address of specific slave device (only 7 LSB).
 * @param[in] p_data          Pointer to a receive buffer.
 * @param[in] length          Number of bytes to be received.
 * @param[in] xfer_pending    After a specified number of bytes transmission will be
 *                            suspended (if xfer_pending is set) or stopped (if not)
 * @param[in] is_tx           Indicate transfer direction (true for master to slave transmission).
 *
 * @retval  NRF_SUCCESS        If the procedure was successful.
 * @retval  NRF_ERROR_BUSY     Driver is not ready for new transfer.
 * @retval  NRF_ERROR_INTERNAL NRF_TWI_EVENTS_ERROR or timeout has occured (only in blocking mode).
 */
static ret_code_t twi_transfer(nrf_drv_twi_t const * const p_instance,
                               uint8_t                     address,
                               uint8_t const             * p_data,
                               uint32_t                    length,
                               bool                        xfer_pending,
                               bool                        is_tx)
{
    ASSERT(m_cb[p_instance->instance_id].state == NRF_DRV_STATE_POWERED_ON);
    ASSERT(length > 0);

    volatile transfer_t * p_transfer = &(m_cb[p_instance->instance_id].transfer);


    bool is_busy = false;
    CRITICAL_REGION_ENTER();
    if (p_transfer->transfer_in_progress)
    {
        is_busy = true;
    }
    else
    {
        p_transfer->transfer_in_progress = true;
    }
    CRITICAL_REGION_EXIT();
    if (is_busy)
    {
        return NRF_ERROR_BUSY;
    }

    p_transfer->address         = address;
    p_transfer->length          = (uint16_t)length;
    p_transfer->p_data          = (uint8_t *)p_data;
    p_transfer->count           = 0;
    p_transfer->xfer_pending    = xfer_pending;
    p_transfer->is_tx           = is_tx;
    p_transfer->error_condition = false;

    state_machine(p_instance, p_transfer->is_tx ? TX_ADDR_REQ : RX_ADDR_REQ);

    if (!m_handlers[p_instance->instance_id])
    {
        // blocking mode
        sm_evt_t evt = p_transfer->is_tx ? TX_DONE : RX_DONE;
        do
        {
            if (twi_action_wait(p_instance) == false)
            {
                nrf_twi_event_clear(p_instance->p_reg, NRF_TWI_EVENTS_ERROR);
                evt = ON_ERROR;
            }
            nrf_twi_event_clear(p_instance->p_reg, p_transfer->end_event);
            state_machine(p_instance, evt);

            if (p_transfer->error_condition)
            {
                p_transfer->transfer_in_progress = false;
                return NRF_ERROR_INTERNAL;
            }
        }
        while (p_transfer->count < p_transfer->length);
        p_transfer->transfer_in_progress = false;
    }
    return NRF_SUCCESS;
}


ret_code_t nrf_drv_twi_init(nrf_drv_twi_t const * const  p_instance,
                            nrf_drv_twi_config_t const * p_config,
                            nrf_drv_twi_evt_handler_t    event_handler)
{
    if (m_cb[p_instance->instance_id].state != NRF_DRV_STATE_UNINITIALIZED)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (p_config == NULL)
    {
        p_config = &m_default_config[p_instance->instance_id];
    }


    m_handlers[p_instance->instance_id]  = event_handler;
    m_instances[p_instance->instance_id] = (nrf_drv_twi_t *)p_instance;

    twi_clear_bus(p_instance, p_config);

    /* To secure correct signal levels on the pins used by the TWI
       master when the system is in OFF mode, and when the TWI master is
       disabled, these pins must be configured in the GPIO peripheral.
    */
    NRF_GPIO->PIN_CNF[p_config->scl] = SCL_PIN_CONF;
    NRF_GPIO->PIN_CNF[p_config->sda] = SDA_PIN_CONF;

    nrf_twi_frequency_set(p_instance->p_reg, p_config->frequency);
    nrf_twi_pins_set(p_instance->p_reg, p_config->scl, p_config->sda);

    if (m_handlers[p_instance->instance_id])
    {
        nrf_drv_common_irq_enable(p_instance->irq, p_config->interrupt_priority);
    }

    m_cb[p_instance->instance_id].state = NRF_DRV_STATE_INITIALIZED;

    return NRF_SUCCESS;
}


void nrf_drv_twi_uninit(nrf_drv_twi_t const * const p_instance)
{
    ASSERT(m_cb[p_instance->instance_id].state != NRF_DRV_STATE_UNINITIALIZED);

    if (m_handlers[p_instance->instance_id])
    {
        nrf_drv_common_irq_disable(p_instance->irq);
    }
    nrf_drv_twi_disable(p_instance);
    nrf_twi_shorts_clear(p_instance->p_reg, DISABLE_MASK);

    m_cb[p_instance->instance_id].state = NRF_DRV_STATE_UNINITIALIZED;
}


void nrf_drv_twi_enable(nrf_drv_twi_t const * const p_instance)
{
    ASSERT(m_cb[p_instance->instance_id].state == NRF_DRV_STATE_INITIALIZED);

    nrf_twi_event_clear(p_instance->p_reg, NRF_TWI_EVENTS_STOPPED);
    nrf_twi_event_clear(p_instance->p_reg, NRF_TWI_EVENTS_RXDREADY);
    nrf_twi_event_clear(p_instance->p_reg, NRF_TWI_EVENTS_TXDSENT);
    nrf_twi_event_clear(p_instance->p_reg, NRF_TWI_EVENTS_ERROR);

    if (m_handlers[p_instance->instance_id])
    {
        nrf_twi_int_enable(p_instance->p_reg, NRF_TWI_INT_ERROR_MASK);
    }

    nrf_twi_enable(p_instance->p_reg);
    m_cb[p_instance->instance_id].state    = NRF_DRV_STATE_POWERED_ON;
    m_cb[p_instance->instance_id].substate = NRF_DRV_TWI_SUBSTATE_IDLE;
}


void nrf_drv_twi_disable(nrf_drv_twi_t const * const p_instance)
{
    ASSERT(m_cb[p_instance->instance_id].state != NRF_DRV_STATE_UNINITIALIZED);

    nrf_twi_task_set(p_instance->p_reg, NRF_TWI_TASKS_STOP);
    nrf_twi_disable(p_instance->p_reg);
    nrf_twi_int_disable(p_instance->p_reg, DISABLE_MASK);
    m_cb[p_instance->instance_id].state = NRF_DRV_STATE_INITIALIZED;
}


ret_code_t nrf_drv_twi_tx(nrf_drv_twi_t const * const p_instance,
                          uint8_t                     address,
                          uint8_t const *             p_data,
                          uint32_t                    length,
                          bool                        xfer_pending)
{
    return twi_transfer(p_instance, address,
                        p_data, length,
                        xfer_pending, true);
}


ret_code_t nrf_drv_twi_rx(nrf_drv_twi_t const * const p_instance,
                          uint8_t                     address,
                          uint8_t *                   p_data,
                          uint32_t                    length,
                          bool                        xfer_pending)
{
    return twi_transfer(p_instance, address,
                        p_data, length,
                        xfer_pending, false);
}


uint32_t nrf_data_count_get(nrf_drv_twi_t const * const p_instance)
{
    volatile transfer_t * p_transfer = &(m_cb[p_instance->instance_id].transfer);

    return p_transfer->count;
}


/**@brief Generic function for handling TWI interrupt
 *
 * @param[in]  p_reg         Pointer to instance register structure.
 * @param[in]  instance_id   Index of instance.
 */
__STATIC_INLINE void nrf_drv_twi_int_handler(NRF_TWI_Type * p_reg, uint32_t instance_id)
{
    volatile transfer_t * p_transfer = &(m_cb[instance_id].transfer);
    sm_evt_t sm_event;

    bool error_occured   = nrf_twi_event_check(p_reg, NRF_TWI_EVENTS_ERROR);
    bool end_evt_occured = nrf_twi_event_check(p_reg, p_transfer->end_event);

    nrf_twi_event_clear(p_reg, NRF_TWI_EVENTS_ERROR);
    nrf_twi_event_clear(p_reg, NRF_TWI_EVENTS_TXDSENT);
    nrf_twi_event_clear(p_reg, NRF_TWI_EVENTS_RXDREADY);
    nrf_twi_event_clear(p_reg, NRF_TWI_EVENTS_STOPPED);

    if (error_occured || end_evt_occured)
    {
        if (error_occured)
        {
            sm_event = ON_ERROR;
        }
        else
        {
            sm_event = p_transfer->is_tx ? TX_DONE : RX_DONE;
        }
        state_machine(m_instances[instance_id], sm_event);

        if (p_transfer->error_condition)
        {
            p_transfer->transfer_in_progress = false;
            nrf_drv_twi_evt_t evt =
            {
                .type               = NRF_DRV_TWI_ERROR,
                .p_data    = p_transfer->p_data,
                .length    = p_transfer->count,
                // Driver uses shortcuts, so NRF_TWI_ERROR_OVERRUN_NACK will not take place.
                .error_src = (nrf_twi_error_source_get(p_reg) &
                             NRF_TWI_ERROR_ADDRESS_NACK) ? NRF_TWI_ERROR_ADDRESS_NACK
                             : NRF_TWI_ERROR_DATA_NACK,
            };
            m_handlers[instance_id](&evt);
        }
        else if (p_transfer->count >= p_transfer->length)
        {
            p_transfer->transfer_in_progress = false;
            nrf_drv_twi_evt_t evt =
            {
                .type   = p_transfer->is_tx ? NRF_DRV_TWI_TX_DONE : NRF_DRV_TWI_RX_DONE,
                .p_data = p_transfer->p_data,
                .length = p_transfer->count,
            };
            m_handlers[instance_id](&evt);
        }
    }
}


#if (TWI0_ENABLED == 1)
void SPI0_TWI0_IRQHandler(void)
{
    nrf_drv_twi_int_handler(NRF_TWI0, TWI0_INSTANCE_INDEX);
}
#endif // (TWI0_ENABLED == 1)

#if (TWI1_ENABLED == 1)
void SPI1_TWI1_IRQHandler(void)
{
    nrf_drv_twi_int_handler(NRF_TWI1, TWI1_INSTANCE_INDEX);
}
#endif // (TWI1_ENABLED == 1)
