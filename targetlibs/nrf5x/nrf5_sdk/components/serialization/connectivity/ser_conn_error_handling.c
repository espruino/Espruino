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

#include "nrf_assert.h"
#include "app_error.h"
#include "ser_config.h"

#include "boards.h"

/** @file
 *
 * @defgroup ser_conn_error_handling Errors handling for the Connectivity Chip.
 * @{
 * @ingroup sdk_lib_serialization
 *
 * @brief   A module to handle the Connectivity Chip errors and warnings.
 *
 * @details This file contains definitions of functions used for handling the Connectivity Chip
 *          errors and warnings.
 */

/**@brief Function for handling the Connectivity Chip errors and warnings.
 *
 * @details This function will be called if the ASSERT macro in the connectivity application fails.
 *          Function declaration can be found in the app_error.h file.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
 
#include "app_util_platform.h"
#include "nrf_soc.h"

// uint32_t m_error_code;
// uint32_t m_error_line_num;
// const uint8_t *m_p_error_file_name;

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
	// disable INTs
    CRITICAL_REGION_ENTER();

		#if LEDS_NUMBER > 0
	
			/* Light a LED on error or warning. */
    // nrf_gpio_cfg_output(SER_CONN_ASSERT_LED_PIN);
    // nrf_gpio_pin_set(SER_CONN_ASSERT_LED_PIN);
	
		#endif

   	// m_p_error_file_name = p_file_name;
    // m_error_code = error_code;
    // m_error_line_num = line_num;

    /* Do not reset when warning. */
    if(SER_WARNING_CODE != id)
    {
        /* This call can be used for debug purposes during application development.
        * @note CAUTION: Activating code below will write the stack to flash on an error.
        * This function should NOT be used in a final product.
        * It is intended STRICTLY for development/debugging purposes.
        * The flash write will happen EVEN if the radio is active, thus interrupting any communication.
        * Use with care. Un-comment the line below to use. */

        /* ble_debug_assert_handler(error_code, line_num, p_file_name); */

#if 0
        /* Reset the chip. Should be used in the release version. */
        NVIC_SystemReset();
#else   /* Debug version. */
        /* To be able to see function parameters in a debugger. */
        uint32_t temp = 1;
        while(temp);
#endif

    }

    CRITICAL_REGION_EXIT();
}


/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called if the ASSERT macro in the SoftDevice fails. Function
 *          declaration can be found in the nrf_assert.h file.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(SER_SD_ERROR_CODE, line_num, p_file_name);
}
/** @} */
