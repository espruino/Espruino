/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/


#include "n5sk_button.h"
#include "nrf_gpio.h"
#include "n5_starterkit.h"


////////////////////////////////////////////////////////////////////////////////
// Description: Button and switch control functions for the N5 Starter Kit
// IO Board (4 Buttons) and Battery Board (5 Buttons)
// Note: N5Starter Kit IO Board buttons and Battery Board switches are pull up.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////

/**@brief Function for initializing the IO board buttons as inputs.

 */
#if 0 // @todo REMOVE ME!
void n5_io_button_init(void)
{
    nrf_gpio_cfg_input(BUTTON_A, BUTTON_PULL);
    nrf_gpio_cfg_input(BUTTON_B, BUTTON_PULL);
    nrf_gpio_cfg_input(BUTTON_C, BUTTON_PULL);
    nrf_gpio_cfg_input(BUTTON_D, BUTTON_PULL);
}

/**@brief Function for initializing the battery board switches as inputs.

 */

void n5_io_switch_init(void)
{
    nrf_gpio_cfg_input(SWITCH_1, SWITCH_PULL);
    nrf_gpio_cfg_input(SWITCH_2, SWITCH_PULL);
    nrf_gpio_cfg_input(SWITCH_3, SWITCH_PULL);
    nrf_gpio_cfg_input(SWITCH_4, SWITCH_PULL);
    nrf_gpio_cfg_input(SWITCH_5, SWITCH_PULL);
}
#endif // 0
