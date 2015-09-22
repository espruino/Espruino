/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup ram_retention_example_main main.c
 * @{
 * @ingroup ram_retention_example
 * @brief RAM Retention Example Application main file.
 *
 * This file contains the source code for a sample application using RAM retention.
 * 
 */

#include <stdbool.h>
#include <stdint.h>
#include "bsp.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#define RAM_MEMORY_TEST_ADDRESS (0x20002000UL)  /**< Address in RAM where test word (RAM_MEMORY_TEST_WORD) is written before System OFF and checked after System RESET.*/
#define RAM_MEMORY_TEST_WORD    (0xFEEDBEEFUL)  /**< Test word that is written to RAM address RAM_MEMORY_TEST_ADDRESS. */
#define RESET_MEMORY_TEST_BYTE  (0x0DUL)        /**< Known sequence written to a special register to check if this wake up is from System OFF. */
#define MAX_TEST_ITERATIONS     (1)             /**< Maximum number of iterations this example will run. */
#define SUCCESS_OUTPUT_VALUE    (0xAB)          /**< If RAM retention is tested for MAX_TEST_ITERATIONS, this value will be given as output.*/

#ifdef BSP_BUTTON_0
    #define PIN_GPIO_WAKEUP BSP_BUTTON_0        /**< Wake up pin number. */
#endif
#ifndef PIN_GPIO_WAKEUP
    #error "Please indicate wake up pin"
#endif

#ifdef BSP_LED_0
    #define READY_PIN_NUMBER BSP_LED_0  /**< Pin number for output. */
#endif
#ifndef READY_PIN_NUMBER
    #error "Please indicate output pin"
#endif

#ifdef BSP_LED_1
    #define SUCCESS_PIN_NUMBER BSP_LED_1  /**< Pin number for output. */
#endif
#ifndef SUCCESS_PIN_NUMBER
    #error "Please indicate output pin"
#endif

/** @brief Function for handling HardFaults. In case something went wrong
 * or System OFF did not work and reached the end of the program.
 */
void HardFault_Handler()
{
    LEDS_ON(LEDS_MASK);

    // Loop forever.
    while (true)
    {
        // Do nothing.
    }
}


/** @brief Function for indicating failure.
 */
static void display_failure(void)
{
    LEDS_ON(LEDS_MASK);

    // Loop forever.
    while (true)
    {
        // Do nothing.
    }
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    uint32_t * volatile p_ram_test = (uint32_t *)RAM_MEMORY_TEST_ADDRESS;
    uint32_t            loop_count = 0;

    LEDS_CONFIGURE(LEDS_MASK);
    LEDS_OFF(LEDS_MASK);

    // This pin is used for waking up from System OFF and is active low, enabling sense capabilities.
    nrf_gpio_cfg_sense_input(PIN_GPIO_WAKEUP, BUTTON_PULL, NRF_GPIO_PIN_SENSE_LOW);
	
    // Workaround for PAN_028 rev1.1 anomaly 22 - System: Issues with disable System OFF mechanism
    nrf_delay_ms(1);

    // Check if the system woke up from System OFF mode by reading the NRF_POWER->GPREGRET register which has
    // retained the value written before going to System OFF. Below is the layout for usage for
    // NRF_POWER->GPREGRET register.
    //  BITS |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0
    //  --------------------------------------------------------------------
    //       |        SPECIAL_SEQUENCE       |          LOOP_COUNT
    //  --------------------------------------------------------------------
    ///

    if ((NRF_POWER->GPREGRET >> 4) == RESET_MEMORY_TEST_BYTE)
    {
        // Take the loop_count value.
        loop_count          = (uint8_t)(NRF_POWER->GPREGRET & 0xFUL);
        NRF_POWER->GPREGRET = 0;

        if (loop_count >= (uint8_t)MAX_TEST_ITERATIONS)
        {
            // clear GPREGRET register before exit.
            NRF_POWER->GPREGRET = 0;
            LEDS_ON(1 << SUCCESS_PIN_NUMBER);

            while (true)
            {
                // Do nothing.
            }
        }

        if (*p_ram_test != RAM_MEMORY_TEST_WORD)
        {
            display_failure();
        }

        *p_ram_test = 0;
    }

    // Write the known sequence + loop_count to the GPREGRET register.
    loop_count++;
    NRF_POWER->GPREGRET = ( (RESET_MEMORY_TEST_BYTE << 4) | loop_count);

    // Write the known value to the known address in RAM, enable RAM retention, set System OFF, and wait
    // for GPIO wakeup from external source.

    LEDS_ON(1 << READY_PIN_NUMBER);
    nrf_delay_ms(1000);

    // Switch on both RAM banks when in System OFF mode.
    NRF_POWER->RAMON |= (POWER_RAMON_OFFRAM0_RAM0On << POWER_RAMON_OFFRAM0_Pos) |
                        (POWER_RAMON_OFFRAM1_RAM1On << POWER_RAMON_OFFRAM1_Pos);

    // Write test word to RAM memory.
    *p_ram_test = RAM_MEMORY_TEST_WORD;
    LEDS_OFF(LEDS_MASK);
    // Enter System OFF and wait for wake up from GPIO detect signal.
    NRF_POWER->SYSTEMOFF = 0x1;

    // This code will only be reached if System OFF did not work and will trigger a hard-fault which will 
    // be handled in HardFault_Handler(). If wake the up condition is already active while System OFF is triggered,
    // then the system will go to System OFF and wake up immediately with a System RESET.
    display_failure();
}


/** @} */
