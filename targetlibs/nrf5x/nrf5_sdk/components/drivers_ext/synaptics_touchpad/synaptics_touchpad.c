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

#include <stdbool.h>
#include <stdint.h>

#include "twi_master.h"
#include "synaptics_touchpad.h"

/*lint ++flb "Enter library region" */

#define PRODUCT_ID_BYTES 10U //!< Number of bytes to expect to be in product ID

static uint8_t       m_device_address; // !< Device address in bits [7:1]
static const uint8_t expected_product_id[PRODUCT_ID_BYTES] = {'T', 'M', '1', '9', '4', '4', '-', '0', '0', '2'};  //!< Product ID expected to get from product ID query

bool touchpad_init(uint8_t device_address)
{
    bool transfer_succeeded = true;

    m_device_address = (uint8_t)(device_address << 1);

    // Do a soft reset
    uint8_t reset_command = 0x01;
    transfer_succeeded &= touchpad_write_register(TOUCHPAD_RESET, reset_command);

    // Page select 0
    uint8_t page_to_select = 0x00;
    transfer_succeeded &= touchpad_write_register(TOUCHPAD_PAGESELECT, page_to_select);

    // Read and verify product ID
    transfer_succeeded &= touchpad_product_id_verify();

    return transfer_succeeded;
}


bool touchpad_product_id_verify(void)
{
    bool    transfer_succeeded = true;
    uint8_t product_id[PRODUCT_ID_BYTES];
    transfer_succeeded &= touchpad_product_id_read(product_id, PRODUCT_ID_BYTES);

    for (uint8_t i = 0; i < 10; i++)
    {
        if (product_id[i] != expected_product_id[i])
        {
            transfer_succeeded = false;
        }
    }

    return transfer_succeeded;
}

bool touchpad_reset(void)
{
    uint8_t w2_data[2] = {TOUCHPAD_COMMAND, 0x01};

    return twi_master_transfer(m_device_address, w2_data, 2, TWI_ISSUE_STOP);
}

bool touchpad_interrupt_status_read(uint8_t *interrupt_status)
{
    return touchpad_read_register(TOUCHPAD_INT_STATUS, interrupt_status);
}

bool touchpad_set_sleep_mode(TouchpadSleepMode_t mode)
{
    return touchpad_write_register(TOUCHPAD_CONTROL, (uint8_t)mode);
}

bool touchpad_read_register(uint8_t register_address, uint8_t *value)
{
    bool transfer_succeeded = true;
    transfer_succeeded &= twi_master_transfer(m_device_address, &register_address, 1, TWI_DONT_ISSUE_STOP);
    if (transfer_succeeded) 
    {
        transfer_succeeded &= twi_master_transfer(m_device_address | TWI_READ_BIT, value, 1, TWI_ISSUE_STOP);
    }
    return transfer_succeeded;
}

bool touchpad_write_register(uint8_t register_address, const uint8_t value)
{
    uint8_t w2_data[2];

    w2_data[0] = register_address;
    w2_data[1] = value;
    return twi_master_transfer(m_device_address, w2_data, 2, TWI_ISSUE_STOP);
}

bool touchpad_product_id_read(uint8_t * product_id, uint8_t product_id_bytes)
{
    uint8_t w2_data[1];
    bool transfer_succeeded = true;

    w2_data[0] = TOUCHPAD_PRODUCT_ID;
    transfer_succeeded &= twi_master_transfer(m_device_address, w2_data, 1, TWI_DONT_ISSUE_STOP);
    if (transfer_succeeded) 
    {
        transfer_succeeded &= twi_master_transfer(m_device_address | TWI_READ_BIT, product_id, product_id_bytes, TWI_ISSUE_STOP);
    }
    return transfer_succeeded;
}

/*lint --flb "Leave library region" */
