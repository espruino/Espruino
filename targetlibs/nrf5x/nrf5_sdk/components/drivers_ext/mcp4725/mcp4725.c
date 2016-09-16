/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

#include "mcp4725.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_util_platform.h"

/*lint ++flb "Enter library region" */
#define MCP4725_BASE_ADDRESS    0x60        //!< MCP4725 base address

#define MCP4725_DAC_ADDRESS     0x40        //!< MCP4725 write-to-dac register
#define MCP4725_EEPROM_ADDRESS  0x60        //!< MCP4725 write-to-eeprom register

#define RDY_BIT_POS             0x07        //!< Position of RDY bit

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID_USED);

/* Twi transfer indicators. */
volatile bool m_xfer_done = false;
volatile bool m_read_done = false;

/**
 * @brief TWI events handler.
 */
static void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_TX)
            {
                m_xfer_done = true;
            }
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
            {
                m_read_done = true;
            }
            break;
        default:
            break;
    }
}

/**
 * @brief TWI initialization.
 */
static ret_code_t twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_mcp4725_config = {
       .scl                = ARDUINO_SCL_PIN,
       .sda                = ARDUINO_SDA_PIN,
       .frequency          = NRF_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_mcp4725_config, twi_handler, NULL);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    nrf_drv_twi_enable(&m_twi);
    return NRF_SUCCESS;
}

ret_code_t mcp4725_setup(void)
{
    ret_code_t err_code = twi_init();
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

ret_code_t mcp4725_set_voltage(uint16_t val, bool write_eeprom)
{
    /* Shift parameter val to get 2 8-bits values. */
    uint8_t reg[3] = {write_eeprom ? MCP4725_EEPROM_ADDRESS : MCP4725_DAC_ADDRESS,
                      (val>>4), (val<<4)};

    m_xfer_done = false;

    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, MCP4725_BASE_ADDRESS, reg, sizeof(reg), false);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    while (m_xfer_done == false);

    return NRF_SUCCESS;
}

bool mcp4725_is_busy(void)
{
    uint8_t busy;
    m_read_done = false;

    ret_code_t err_code = nrf_drv_twi_rx(&m_twi, MCP4725_BASE_ADDRESS, &busy, sizeof(busy));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    while (m_read_done == false);

    return (bool)(!(busy >> RDY_BIT_POS));
}

/*lint --flb "Leave library region" */
