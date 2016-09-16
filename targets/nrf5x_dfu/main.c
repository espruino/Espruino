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

/** @file
 *
 * @defgroup bootloader_secure main.c
 * @{
 * @ingroup dfu_bootloader_api
 * @brief Bootloader project main file for secure DFU.
 *
 */

#include <stdint.h>
#include "platform_config.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_mbr.h"
#include "nrf_bootloader.h"
#include "nrf_bootloader_app_start.h"
#include "nrf_dfu.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "app_error.h"
#include "app_error_weak.h"
#include "nrf_bootloader_info.h"

#undef BOOTLOADER_BUTTON
#define BOOTLOADER_BUTTON               BTN1_PININDEX                                            /**< Button used to enter SW update mode. */
#define BOOTLOADER_BUTTON_ONSTATE       BTN1_ONSTATE                                            /**< Button used to enter SW update mode. */
#define UPDATE_IN_PROGRESS_LED          LED3_PININDEX                                            /**< Led used to indicate that DFU is active. */
#define UPDATE_IN_PROGRESS_LED_ONSTATE  LED3_ONSTATE                                            /**< Led used to indicate that DFU is active. */
#define BOOTLOADER_BUTTON_PRESS_LED          LED2_PININDEX                                            /**< Led used to indicate that DFU is active. */
#define BOOTLOADER_BUTTON_PRESS_LED_ONSTATE  LED2_ONSTATE                                            /**< Led used to indicate that DFU is active. */
// Other LED is set in targetlibs/nrf5x/nrf5_sdk/components/libraries/bootloader_dfu/dfu_transport_ble.c (currently LED1)


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_ERROR("received a fault! id: 0x%08x, pc: 0x&08x\r\n", id, pc);
    NVIC_SystemReset();
}

void app_error_handler_bare(uint32_t error_code)
{
    (void)error_code;
    NRF_LOG_ERROR("received an error: 0x%08x!\r\n", error_code);
    NVIC_SystemReset();
}


/**@brief Function for initialization of LEDs.
 */
static void leds_init(void)
{
    nrf_gpio_cfg_output(UPDATE_IN_PROGRESS_LED);
    nrf_gpio_pin_write(UPDATE_IN_PROGRESS_LED, !UPDATE_IN_PROGRESS_LED_ONSTATE);
    nrf_gpio_cfg_output(BOOTLOADER_BUTTON_PRESS_LED);
    nrf_gpio_pin_write(BOOTLOADER_BUTTON_PRESS_LED, !BOOTLOADER_BUTTON_PRESS_LED_ONSTATE);
}


/**@brief Function for initializing the button module.
 */
static void buttons_init(void)
{
    nrf_gpio_cfg_sense_input(BOOTLOADER_BUTTON,
                             BOOTLOADER_BUTTON_ONSTATE ? NRF_GPIO_PIN_PULLDOWN : NRF_GPIO_PIN_PULLUP,
                             BOOTLOADER_BUTTON_ONSTATE ? NRF_GPIO_PIN_SENSE_HIGH : NRF_GPIO_PIN_SENSE_LOW);

}

// Override Weak version
bool nrf_dfu_enter_check(void) {
    bool dfu_start = (nrf_gpio_pin_read(BOOTLOADER_BUTTON) == BOOTLOADER_BUTTON_ONSTATE) ? true: false;

    // If button is held down for 3 seconds, don't start bootloader.
    // This means that we go straight to Espruino, where the button is still
    // pressed and can be used to stop execution of the sent code.
    if (dfu_start) {
      nrf_gpio_pin_write(BOOTLOADER_BUTTON_PRESS_LED, BOOTLOADER_BUTTON_PRESS_LED_ONSTATE);
      int count = 3000;
      while (nrf_gpio_pin_read(BOOTLOADER_BUTTON) == BOOTLOADER_BUTTON_ONSTATE && count) {
        nrf_delay_us(999);
        count--;
      }
      if (!count)
        dfu_start = false;
      nrf_gpio_pin_write(BOOTLOADER_BUTTON_PRESS_LED, !BOOTLOADER_BUTTON_PRESS_LED_ONSTATE);
    }

    return dfu_start;
}


/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t ret_val;

    (void) NRF_LOG_INIT(NULL);

    NRF_LOG_INFO("Inside main\r\n");

    leds_init();
    buttons_init();

    ret_val = nrf_bootloader_init();
    APP_ERROR_CHECK(ret_val);

    // Either there was no DFU functionality enabled in this project or the DFU module detected
    // no ongoing DFU operation and found a valid main application.
    // Boot the main application.
    nrf_bootloader_app_start(MAIN_APPLICATION_START_ADDR);

    // Should never be reached.
    NRF_LOG_INFO("After main\r\n");
}

/**
 * @}
 */
