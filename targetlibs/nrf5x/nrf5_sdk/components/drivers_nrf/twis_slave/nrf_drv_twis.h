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
#ifndef NRF_DRV_TWIS_H__
#define NRF_DRV_TWIS_H__

#include "nrf_drv_config.h"
#include "nrf_drv_common.h"
#include "sdk_errors.h"
#include "nrf_twis.h"
#include <stdint.h>
#include "app_util.h"
/**
 * @ingroup  nrf_twi
 * @defgroup nrf_twis TWI slave HAL and driver
 * @brief @tagAPI52 TWI slave APIs.
 * @details The TWIS HAL provides basic APIs for accessing the registers of the TWIS.
 * The TWIS driver provides APIs on a higher level.
 */

/**
 * @ingroup  nrf_twis
 * @defgroup nrf_twis_drv TWI slave with EasyDMA driver
 * 
 * @brief @tagAPI52 TWI slave with EasyDMA driver.
 * @{
 */

/**
 * @defgroup nrf_twis_drv_config TWIS driver global configuration
 *
 * @brief This configuration is placed in the global configuration file @c nrf_drv_config.h.
 * @{
 */

    /**
     * @def TWIS_ASSUME_INIT_AFTER_RESET_ONLY
     * @brief Assume that any instance would be initialized only once
     *
     * Optimization flag.
     * Registers used by TWIS are shared by other peripherals.
     * Normally, during initialization driver tries to clear all registers to known state before
     * doing the initialization itself.
     * This gives initialization safe procedure, no matter when it would be called.
     * If you activate TWIS only once and do never uninitialize it - set this flag to 1 what gives
     * more optimal code.
     */

    /**
     * @def TWIS_NO_SYNC_MODE
     * @brief Remove support for synchronous mode
     *
     * Synchronous mode would be used in specific situations.
     * And it uses some additional code and data memory to safely process state machine
     * by polling it in status functions.
     * If this functionality is not required it may be disabled to free some resources.
     */
/** @} */

/**
 * @brief Event callback function event definitions.
 */
typedef enum
{
    TWIS_EVT_READ_REQ,     ///< Read request detected
                           /**< If there is no buffer prepared, buf_req flag in the even will be set.
                                Call then @ref nrf_drv_twis_tx_prepare to give parameters for buffer.
                                */
    TWIS_EVT_READ_DONE,    ///< Read request has finished - free any data
    TWIS_EVT_READ_ERROR,   ///< Read request finished with error
    TWIS_EVT_WRITE_REQ,    ///< Write request detected
                           /**< If there is no buffer prepared, buf_req flag in the even will be set.
                                Call then @ref nrf_drv_twis_rx_prepare to give parameters for buffer.
                                */
    TWIS_EVT_WRITE_DONE,   ///< Write request has finished - process data
    TWIS_EVT_WRITE_ERROR,  ///< Write request finished with error
    TWIS_EVT_GENERAL_ERROR ///< Error that happens not inside WRITE or READ transaction
} nrf_drv_twis_evt_type_t;

/**
 * @brief TWIS driver instance structure
 *
 * @note We only need instance number here so we could really use just a number
 * that would be send to every driver function.
 * But for compatibility reason this number is inserted into the structure.
 */
typedef struct
{
    uint8_t instNr; /**< Instance number */
}nrf_drv_twis_t;

/**
 * @brief TWIS driver event structure
 */
typedef struct
{
    nrf_drv_twis_evt_type_t type; ///< Event type
    union
    {
        bool buf_req;       ///< Flag for @ref TWIS_EVT_READ_REQ and @ref TWIS_EVT_WRITE_REQ
                            /**< Information if transmission buffer requires to be prepared */
        uint32_t tx_amount; ///< Data for @ref TWIS_EVT_READ_DONE
        uint32_t rx_amount; ///< Data for @ref TWIS_EVT_WRITE_DONE
        uint32_t error;     ///< Data for @ref TWIS_EVT_GENERAL_ERROR
    }data;
}nrf_drv_twis_evt_t;

/**
 * @brief TWI slave event callback function type.
 *
 * @param[in] p_event Event information structure.
 */
typedef void (*nrf_drv_twis_event_handler_t)(nrf_drv_twis_evt_t const * const p_event);

/**
 * @brief Structure for TWIS configuration
 */
typedef struct
{
    uint32_t addr[2];            //!< Set addresses that this slave should respond. Set 0 to disable.
    uint32_t scl;                //!< SCL pin number
    uint32_t sda;                //!< SDA pin number
    uint8_t  interrupt_priority; //!< The priority of interrupt for the module to set
}nrf_drv_twis_config_t;

/**
 * @brief Possible error sources
 *
 * This is flag enum - values from this enum can be connected using logical or operator.
 * @note
 * We could use directly @ref nrf_twis_error_t. Error type enum is redefined here becouse
 * of possible future extension (eg. supporting timeouts and synchronous mode).
 */
typedef enum
{
    NRF_DRV_TWIS_ERROR_OVERFLOW         = NRF_TWIS_ERROR_OVERFLOW,  /**< RX buffer overflow detected, and prevented */
    NRF_DRV_TWIS_ERROR_DATA_NACK        = NRF_TWIS_ERROR_DATA_NACK, /**< NACK sent after receiving a data byte */
    NRF_DRV_TWIS_ERROR_OVERREAD         = NRF_TWIS_ERROR_OVERREAD,  /**< TX buffer over-read detected, and prevented */
    NRF_DRV_TWIS_ERROR_UNEXPECTED_EVENT = 1 << 8                    /**< Unexpected event detected by state machine */
}nrf_drv_twis_error_t;

/**
 * @internal
 * @brief Internal macro for creating TWIS driver instance
 *
 * Second level of indirection in creating the instance.
 * Do not use this macro directly.
 * Use @ref NRF_DRV_TWIS_INSTANCE instead.
 */
#define NRF_DRV_TWIS_INSTANCE_x(id) \
    { \
        TWIS##id##_INSTANCE_INDEX \
    }

/**
 * @brief Macro for creating TWIS driver instance
 *
 * @param[in] id Instance index. Use 0 for TWIS0 and 1 for TWIS1
 */
#define NRF_DRV_TWIS_INSTANCE(id) NRF_DRV_TWIS_INSTANCE_x(id)

/**
 * @internal
 * @brief Internal macro for creating TWIS driver default configuration
 *
 * Second level of indirection in creating the instance.
 * Do not use this macro directly.
 * Use @ref NRF_DRV_TWIS_SLAVE_DEFAULT_CONFIG instead.
 */
#define NRF_DRV_TWIS_DEFAULT_CONFIG_x(id) \
{ \
    .addr               = { TWIS##id##_CONFIG_ADDR0, TWIS##id##_CONFIG_ADDR1 }, \
    .scl                = TWIS##id##_CONFIG_SCL, \
    .sda                = TWIS##id##_CONFIG_SDA, \
    .interrupt_priority = TWIS##id##_CONFIG_IRQ_PRIORITY \
}

/**
 * @brief Generate default configuration for TWIS driver instance
 *
 * @param[in] id Instance index. Use 0 for TWIS0 and 1 for TWIS1
 */
#define NRF_DRV_TWIS_DEFAULT_CONFIG(id) NRF_DRV_TWIS_DEFAULT_CONFIG_x(id)

/**
 * @brief Function for initializing the TWIS driver instance.
 *
 * Function initializes and enables TWIS driver.
 * @attention After driver initialization enable it by @ref nrf_drv_twis_enable
 *
 * @param[in] p_inst          TWIS driver instance.
 * @attention                 @em p_inst has to be global object.
 *                            It would be used by interrupts so make it sure that object
 *                            would not be destroyed when function is leaving.
 * @param[in] p_config        Initial configuration. If NULL, the default configuration is used.
 * @param[in] event_handler   Event handler provided by the user.
 *
 * @retval NRF_SUCCESS             If initialization was successful.
 * @retval NRF_ERROR_INVALID_STATE If the driver is already initialized.
 * @retval NRF_ERROR_BUSY          If some other peripheral with the same
 *                                 instance ID is already in use. This is 
 *                                 possible only if PERIPHERAL_RESOURCE_SHARING_ENABLED 
 *                                 is set to a value other than zero.
 */
ret_code_t nrf_drv_twis_init(
        nrf_drv_twis_t          const * const p_inst,
        nrf_drv_twis_config_t   const * p_config,
        nrf_drv_twis_event_handler_t    const event_handler);

/**
 * @brief Function for uninitializing the TWIS driver instance.
 *
 * Function initializes the peripheral and resets all registers to default values.
 * 
 * @param[in] p_inst TWIS driver instance to uninitialize.
 * @note
 * It is safe to call nrf_drv_twis_uninit even before initialization.
 * Actually @ref nrf_drv_twis_init function calls this function to
 * make sure that TWIS state is known.
 * @note
 * If TWIS driver was in uninitialized state before calling this function,
 * selected pins would not be reset to default configuration.
 */
void nrf_drv_twis_uninit(nrf_drv_twis_t const * const p_inst);

/**
 * @brief Enable TWIS instance
 *
 * This function enables TWIS instance.
 * Function defined if there is needs for dynamically enabling and disabling the peripheral.
 * Use @ref nrf_drv_twis_enable and @ref nrf_drv_twis_disable functions.
 * They do not change any configuration registers.
 * 
 * @param p_inst TWIS driver instance.
 */
void nrf_drv_twis_enable(nrf_drv_twis_t const * const p_inst);

/**
 * @brief Disable TWIS instance
 *
 * Disabling TWIS instance gives possibility to turn off the TWIS while 
 * holding configuration done by @ref nrf_drv_twis_init
 * 
 * @param p_inst TWIS driver instance.
 */
void nrf_drv_twis_disable(nrf_drv_twis_t const * const p_inst);

/**
 * @brief Get and clear last error flags
 *
 * Function gets information about errors.
 * This is also the only possibility to exit from error substate of the internal state machine.
 *
 * @param[in] p_inst TWIS driver instance.
 * @return Error flags defined in @ref nrf_drv_twis_error_t
 * @attention
 * This function clears error state and flags.
 */
uint32_t nrf_drv_twis_error_get_and_clear(nrf_drv_twis_t const * const p_inst);


/**
 * @brief Prepare data for sending
 *
 * This function should be used in response for @ref TWIS_EVT_READ_REQ event.
 * 
 * @param[in] p_inst TWIS driver instance.
 * @param[in] p_buf Transmission buffer
 * @attention       Transmission buffer has to be placed in RAM.
 * @param     size  Maximum number of bytes that master may read from buffer given.
 *
 * @retval NRF_SUCCESS              Preparation finished properly
 * @retval NRF_ERROR_INVALID_ADDR   Given @em p_buf is not placed inside the RAM
 * @retval NRF_ERROR_INVALID_LENGTH Wrong value in @em size parameter
 * @retval NRF_ERROR_INVALID_STATE  Module not initialized or not enabled
 */
ret_code_t nrf_drv_twis_tx_prepare(
        nrf_drv_twis_t const * const p_inst,
        void const * const p_buf,
        size_t size);

/**
 * @brief Get number of transmitted bytes
 *
 * Function returns number of bytes sent.
 * This function may be called after @ref TWIS_EVT_READ_DONE or @ref TWIS_EVT_READ_ERROR events.
 *
 * @param[in] p_inst TWIS driver instance.
 *
 * @return Number of bytes sent.
 */
size_t nrf_drv_twis_tx_amount(nrf_drv_twis_t const * const p_inst);

/**
 * @brief Prepare data for receiving
 *
 * This function should be used in response for @ref TWIS_EVT_WRITE_REQ event.
 * 
 * @param[in] p_inst TWIS driver instance.
 * @param[in] p_buf Buffer that would be filled with received data
 * @attention       Receiving buffer has to be placed in RAM.
 * @param     size  Size of the buffer (maximum amount of data to receive)
 *
 * @retval NRF_SUCCESS              Preparation finished properly
 * @retval NRF_ERROR_INVALID_ADDR   Given @em p_buf is not placed inside the RAM
 * @retval NRF_ERROR_INVALID_LENGTH Wrong value in @em size parameter
 * @retval NRF_ERROR_INVALID_STATE  Module not initialized or not enabled
 */
ret_code_t nrf_drv_twis_rx_prepare(
        nrf_drv_twis_t const * const p_inst,
        void * const p_buf,
        size_t size);

/**
 * @brief Get number of received bytes
 *
 * Function returns number of bytes received.
 * This function may be called after @ref TWIS_EVT_WRITE_DONE or @ref TWIS_EVT_WRITE_ERROR events.
 *
 * @param[in] p_inst TWIS driver instance.
 *
 * @return Number of bytes received.
 */
size_t nrf_drv_twis_rx_amount(nrf_drv_twis_t const * const p_inst);

/**
 * @brief Function checks if driver is busy right now
 *
 * Actual driver substate is tested.
 * If driver is in any other state than IDLE or ERROR this function returns true.
 *
 * @param[in] p_inst TWIS driver instance.
 *
 * @retval true  Driver is in state other than ERROR or IDLE
 * @retval false There is no transmission pending.
 */
bool nrf_drv_twis_is_busy(nrf_drv_twis_t const * const p_inst);

/**
 * @brief Function checks if driver is waiting for tx buffer
 *
 * If this function returns true, it means that driver is stalled expecting
 * of the @ref nrf_drv_twis_tx_prepare function call.
 *
 * @param[in] p_inst TWIS driver instance.
 *
 * @retval true Driver waits for @ref nrf_drv_twis_tx_prepare
 * @retval false Driver is not in the state where it waits for preparing tx buffer.
 */
bool nrf_drv_twis_is_waiting_tx_buff(nrf_drv_twis_t const * const p_inst);

/**
 * @brief Function checks if driver is waiting for rx buffer
 *
 * If this function returns true, it means that driver is staled expecting
 * of the @ref nrf_drv_twis_rx_prepare function call.
 *
 * @param[in] p_inst TWIS driver instance.
 *
 * @retval true Driver waits for @ref nrf_drv_twis_rx_prepare
 * @retval false Driver is not in the state where it waits for preparing rx buffer.
 */
bool nrf_drv_twis_is_waiting_rx_buff(nrf_drv_twis_t const * const p_inst);

/**
 * @brief Check if driver is sending data
 *
 * If this function returns true, it means that there is ongoing output transmission.
 *
 * @param[in] p_inst TWIS driver instance.
 *
 * @retval true There is ongoing output transmission.
 * @retval false Driver is in other state.
 */
bool nrf_drv_twis_is_pending_tx(nrf_drv_twis_t const * const p_inst);

/**
 * @brief Check if driver is receiving data
 *
 * If this function returns true, it means that there is ongoing input transmission.
 *
 * @param[in] p_inst TWIS driver instance.
 *
 * @retval true There is ongoing input transmission.
 * @retval false Driver is in other state.
 */
bool nrf_drv_twis_is_pending_rx(nrf_drv_twis_t const * const p_inst);

/** @} */ /* End of lib_twis_drv group */
#endif /* NRF_DRV_TWIS_H__ */
