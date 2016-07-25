/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
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

#ifndef SYNAPTICS_TOUCHPAD_H
#define SYNAPTICS_TOUCHPAD_H

/*lint ++flb "Enter library region" */

#include <stdbool.h>
#include <stdint.h>

/** @file
* @brief Synaptics Touchpad driver
*
*
* @defgroup nrf_drivers_synaptics_touchpad Synaptics Touchpad driver.
* @{
* @ingroup nrf_drivers
* @brief Synaptics Touchpad driver.
*/

/**
  Touchpad register addresses. 
*/
#define TOUCHPAD_INT_STATUS    0x14    //!< Interrupt status register
#define TOUCHPAD_BUTTON_STATUS 0x41    //!< Button status register
#define TOUCHPAD_FINGER0_REL   0x30    //!< First register in finger delta block
#define TOUCHPAD_GESTURE_FLAGS 0x3A    //!< Gesture flags 0
#define TOUCHPAD_SCROLL        0x3F    //!< Scroll zone X / horizontal multifinger scroll
#define TOUCHPAD_CONTROL       0x42    //!< Device control register
#define TOUCHPAD_COMMAND       0x8F    //!< Device command register

#define TOUCHPAD_RESET 0x54 //!< Address of reset
#define TOUCHPAD_PAGESELECT 0xFF //!< Address of page select (can be found in every page at the same address)
#define TOUCHPAD_PRODUCT_ID 0xA2 //!< Address of product ID string

/**
  Operational states
*/
typedef enum                     
{  
  SleepmodeNormal        = 0x00,  //!< Normal operation
  SleepmodeSensorSleep   = 0x01  //!< Low power operation
} TouchpadSleepMode_t;

/**
  @brief Function for Touchpad initialization.
  @param device_address TWI address of the device in bits [6:0]
  @retval true Touchpad was successfully identified and initialized
  @retval false Unexpected product ID or communication failure
*/
bool touchpad_init(uint8_t device_address);

/**
  @brief Function for attempting to soft-reset the device.
  @retval true Reset succeeded
  @retval false Reset failed
*/
bool touchpad_reset(void);

/**
  @brief Function for reading the interrupt status register of the device. This clears all interrupts.
  @param interrupt_status Address to store interrupt status to.
  @retval true Register contents read successfully to interrupt_status
  @retval false Reading failed
*/
bool touchpad_interrupt_status_read(uint8_t *interrupt_status);

/**
  @brief Function for sleep mode configuration.
  @note In low power mode the touchpad do not generate interrupts from touch sensing.
  @param[in] mode Operational mode
  @retval true Sleep mode set successfully
  @retval false Sleep mode setting failed
*/
bool touchpad_set_sleep_mode(TouchpadSleepMode_t mode);

/**
  @brief Function for reading a touchpad register contents over TWI.  
  @param[in] register_address Register address
  @param[out] value Pointer to a data buffer where read data will be stored
  @retval true Register read succeeded
  @retval false Register read failed
*/
bool touchpad_read_register(uint8_t register_address, uint8_t *value);

/**
  @brief Function for writing a touchpad register contents over TWI.
  @param[in]  register_address Register address
  @param[in] value Value to write to register
  @retval true Register write succeeded
  @retval false Register write failed
*/
bool touchpad_write_register(uint8_t register_address, uint8_t value);

/**
  @brief Function for writing touchpad register contents over TWI.
  Writes one or more consecutive registers.
  @param[out] product_id Pointer to a address to store product ID. Memory must be allocated for product_id_bytes number of bytes.
  @param[in]  product_id_bytes Number of bytes to read
  @retval true Product ID read succeeded
  @retval false Product ID read failed
*/
bool touchpad_product_id_read(uint8_t *product_id, uint8_t product_id_bytes);

/**
  @brief Function for reading and verifying touchpad's product ID.
  @retval true Product ID is what was expected
  @retval false Product ID was not what was expected
*/
bool touchpad_product_id_verify(void);

/**
 *@}
 **/

/*lint --flb "Leave library region" */

#endif /* __TOUCHPAD_H__ */
