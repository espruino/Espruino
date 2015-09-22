/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/


#include "n5sk_led.h"
#include "nrf_gpio.h"
#include "n5_starterkit.h"


////////////////////////////////////////////////////////////////////////////////
// Description: LED control functions for the N5 Starter Kit IO Board
// Note: N5 Starter Kit IO Board LEDs are active low.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////

#if 0
/**@brief Function for turning off all leds.
 *
 */
void n5_io_clear_leds(void)
{
    nrf_gpio_pin_set(LED_A);
    nrf_gpio_pin_set(LED_B);
    nrf_gpio_pin_set(LED_C);
    nrf_gpio_pin_set(LED_D);
}

/**@brief Function for turning on all leds.
 *
 */
void n5_io_set_leds(void)
{
    nrf_gpio_pin_clear(LED_A);
    nrf_gpio_pin_clear(LED_B);
    nrf_gpio_pin_clear(LED_C);
    nrf_gpio_pin_clear(LED_D);
}

/**@brief Function for initializing all leds as outputs.
 *
 * Leds are initialized as off
 */
void n5_io_init_leds(void)
{
    nrf_gpio_cfg_output(LED_A);
    nrf_gpio_cfg_output(LED_B);
    nrf_gpio_cfg_output(LED_C);
    nrf_gpio_cfg_output(LED_D);

    n5_io_clear_leds();
}

/**@brief Function for turning on a specified led.
 *
 * @param[in] led_num   The led to turn on.
 */
void n5_io_turn_on_led(uint8_t led_num)
{
    nrf_gpio_pin_clear(led_num);
}

/**@brief Function for turning off a specified led.
 *
 * @param[in] led_num   The led to turn off.
 */
void n5_io_turn_off_led(uint8_t led_num)
{
    nrf_gpio_pin_set(led_num);
}

/**@brief Function for toggling a specified led.
 *
 * @param[in] led_num   The led to toggle.
 */
void n5_io_toggle_led(uint8_t led_num)
{
    nrf_gpio_pin_toggle(led_num);
}

/**
 *@}
 **/
#endif // 0
