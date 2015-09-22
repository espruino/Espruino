/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#include "dfu.h"
#include "dfu_transport.h"
#include "bootloader.h"
#include "bootloader_util.h"
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_nvmc.h"
#include "nrf_delay.h"
#include "nrf51_bitfields.h"
#include "ant_interface.h"
#include "ant_parameters.h"
#include "ant_error.h"
#include "nrf51.h"
#include "app_scheduler.h"
#include "app_timer.h"
#include "nrf_error.h"
#include "boards.h"
#include "softdevice_handler.h"
#include "pstorage_platform.h"
#include "ant_boot_settings_api.h"
#include "antfs_ota.h"
#if !defined (S210_V3_STACK)
#include "nrf_mbr.h"
#endif // !S210_V3_STACK
#include "ant_stack_config.h"
#include "debug_pin.h"

#define ENABLE_BUTTON // include button detection
//#define ENABLE_IO_LED // include LED status on N5DK1 i/o board

#if defined (ENABLE_BUTTON)
   #if defined (BOARD_N5DK1)
      #define BOOTLOADER_BUTTON_PIN        BUTTON_D                                                /**< Button used to enter SW update mode. */
   #else
      #define BOOTLOADER_BUTTON_PIN        BUTTON_1                                                /**< Button used to enter SW update mode. */
   #endif
#endif

#if defined (ENABLE_IO_LED)
   #define BOOTLOADER_ERROR_LED         LED_C                                                   /**< N5DK Leds, set=led off, clr=led on  */
   #define BOOTLOADER_ACTIVE_LED        LED_D
#endif // ENABLE_IO_LED

#define APP_TIMER_PRESCALER             0                                                       /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS            3                                                       /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                                       /**< Size of timer operation queues. */

#define SCHED_MAX_EVENT_DATA_SIZE       MAX(ANT_STACK_EVT_MSG_BUF_SIZE, 0)                      /**< Maximum size of scheduler events. */

#define SCHED_QUEUE_SIZE                20                                                      /**< Maximum number of events in the scheduler queue. */


/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
#if defined (DEBUG_DFU_BOOTLOADER)
uint32_t error_code_;
uint32_t line_num_;
const uint8_t * p_file_name_;
#endif // DEBUG_DFU_BOOTLOADER

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
#if defined (DEBUG_DFU_BOOTLOADER)
    error_code_      = error_code;
    line_num_        = line_num;
    p_file_name_     = p_file_name;
#endif // DEBUG_DFU_BOOTLOADER

#if defined (ENABLE_IO_LED)
//    nrf_gpio_pin_set(BOOTLOADER_ERROR_LED);
#endif // ENABLE_IO_LED

    // This call can be used for debug purposes during application development.
    // On assert, the system can only recover on reset.
     if(error_code)
          NVIC_SystemReset();
}

void HardFault_Handler(uint32_t ulProgramCounter, uint32_t ulLinkRegister)
{
   (void)ulProgramCounter;
   (void)ulLinkRegister;

   NVIC_SystemReset();
}

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] file_name   File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}


#if defined (ENABLE_IO_LED)
/**@brief Function for initialization of LEDs.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    nrf_gpio_cfg_output(LED_A);
    nrf_gpio_cfg_output(LED_B);
    nrf_gpio_cfg_output(LED_C);
    nrf_gpio_cfg_output(LED_D);

     // turn on all leds
     nrf_gpio_pin_clear(LED_A);
     nrf_gpio_pin_clear(LED_B);
     nrf_gpio_pin_clear(LED_C);
     nrf_gpio_pin_clear(LED_D);
}
#endif // ENABLE_IO_LED


#if defined (ENABLE_IO_LED)
/**@brief Function for clearing the LEDs.
 *
 * @details Clears all LEDs used by the application.
 */
static void leds_off(void)
{
    nrf_gpio_pin_set(LED_A); // unused
    nrf_gpio_pin_set(LED_B); // unused
    nrf_gpio_pin_set(LED_C);
    nrf_gpio_pin_set(LED_D);
}
#endif // ENABLE_IO_LED


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    // Initialize timer module, making it use the scheduler.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, true);
}


#if defined (ENABLE_BUTTON)
/**@brief Function for initializing the button module.
 */
static void buttons_init(void)
{
    nrf_gpio_cfg_sense_input(BOOTLOADER_BUTTON_PIN,
                             BUTTON_PULL,
                             NRF_GPIO_PIN_SENSE_LOW);
}
#endif // ENABLE_BUTTON


/**@brief Function for dispatching a stack event to all modules with a stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a stack
 *          event has been received.
 *
 * @param[in]   event   stack event.
 */
static void sys_evt_dispatch(uint32_t event)
{
    pstorage_sys_event_handler(event);
}


/**@brief Function for initializing the ANT stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ant_stack_init(void)
{
    uint32_t         err_code;

#if !defined (S210_V3_STACK)
    sd_mbr_command_t com = {SD_MBR_COMMAND_INIT_SD, };

    err_code = sd_mbr_command(&com);
    APP_ERROR_CHECK(err_code);

    err_code = sd_softdevice_vector_table_base_set(BOOTLOADER_REGION_START);
    APP_ERROR_CHECK(err_code);
#endif // !S210_V3_STACK

    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, true);

    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
    
    // Configure ant stack regards used channels.
    err_code = ant_stack_static_config();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for event scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

static uint32_t enter_boot_get (void )
{
    uint32_t val = PARAM_FLAGS_ENTER_BOOT_BypassInit;

    if (((*ANT_BOOT_PARAM_FLAGS & PARAM_FLAGS_PARAM_VALID_Msk) >> PARAM_FLAGS_PARAM_VALID_Pos) == PARAM_FLAGS_PARAM_VALID_True )
    {
        val = (*ANT_BOOT_PARAM_FLAGS & PARAM_FLAGS_ENTER_BOOT_Msk) >> PARAM_FLAGS_ENTER_BOOT_Pos;
    }

    return val;
}

static void enter_boot_set (uint32_t value)
{
    uint32_t ant_boot_param_flags = *ANT_BOOT_PARAM_FLAGS;

    uint32_t enter_boot = (ant_boot_param_flags >> PARAM_FLAGS_ENTER_BOOT_Pos) & PARAM_FLAGS_ENTER_BOOT_Msk;
    if (enter_boot == value)
    {
        return; // no need to rewrite the same value.
    }

    ant_boot_param_flags &= ~PARAM_FLAGS_ENTER_BOOT_Msk;
    ant_boot_param_flags |= value << PARAM_FLAGS_ENTER_BOOT_Pos;

    nrf_nvmc_write_word(ANT_BOOT_PARAM_FLAGS_BASE, ant_boot_param_flags);
}

static void enter_boot_update (void)
{
    const bootloader_settings_t * p_bootloader_settings;

    bootloader_util_settings_get(&p_bootloader_settings);

   if(p_bootloader_settings->ap_image.st.bank == NEW_IMAGE_BANK_0 || p_bootloader_settings->ap_image.st.bank == NEW_IMAGE_BANK_1)
   {
       enter_boot_set(PARAM_FLAGS_ENTER_BOOT_BypassDone);
   }
   else
   {
       if (p_bootloader_settings->valid_app != BOOTLOADER_SETTINGS_INVALID_APPLICATION)
       {
           enter_boot_set(PARAM_FLAGS_ENTER_BOOT_BypassDone);
           return;
       }
       enter_boot_set(PARAM_FLAGS_ENTER_BOOT_EnterBoot);
   }

   // If the current application has been invalidated, then application's self protection is of no use now.
   // Lets clear it.
   if (p_bootloader_settings->valid_app == BOOTLOADER_SETTINGS_INVALID_APPLICATION)
   {
       if(*ANT_BOOT_APP_SIZE != APP_SIZE_Empty)
       {
           nrf_nvmc_write_word(ANT_BOOT_APP_SIZE_BASE, APP_SIZE_Clear);
       }
   }
}

/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    bool     bootloader_is_pushed = false;

#if defined (ENABLE_IO_LED)
    leds_init();
#endif // ENABLE_IO_LED
#if defined (ENABLE_BUTTON)
    buttons_init();
#endif

#if defined (DEBUG_DFU_BOOTLOADER)
    NRF_GPIO->DIRSET = 0x40000908;  //stack debugging
    DBG_PIN_DIR_INIT;
#endif //DEBUG_DFU_BOOTLOADER
#if defined (DBG_DFU_BOOTLOADER_PATH)
    DEBUG_PIN_FALL(DBG_DFU_BOOTLOADER_PATH);
#endif //DBG_DFU_BOOTLOADER_PATH

    // This check ensures that the defined fields in the bootloader corresponds with actual
    // setting in the nRF51 chip.
    APP_ERROR_CHECK_BOOL(*((uint32_t *)NRF_UICR_BOOT_START_ADDRESS) == BOOTLOADER_REGION_START);
    APP_ERROR_CHECK_BOOL(NRF_FICR->CODEPAGESIZE == CODE_PAGE_SIZE);

#if !defined (S210_V3_STACK)
    err_code = bootloader_dfu_sd_update_continue();
    APP_ERROR_CHECK(err_code);

    err_code = bootloader_dfu_bl_update_continue();
    APP_ERROR_CHECK(err_code);
#endif // !S210_V3_STACK

    // Initialize.
    timers_init();
    (void)bootloader_init();
    ant_stack_init();
    scheduler_init();

#if defined (ENABLE_BUTTON)
    // Push button switch
    bootloader_is_pushed = ((nrf_gpio_pin_read(BOOTLOADER_BUTTON_PIN) == 0) ? true: false);
    if (bootloader_is_pushed)
    {
        enter_boot_set(PARAM_FLAGS_ENTER_BOOT_EnterBoot);
    }
#endif // ENABLE_BUTTON

    if ((enter_boot_get() == PARAM_FLAGS_ENTER_BOOT_EnterBoot)  ||
        (bootloader_is_pushed)                                  ||
        (!bootloader_app_is_valid(DFU_BANK_0_REGION_START)))
    {
        #if defined (DBG_DFU_BOOTLOADER_PATH)
        DEBUG_PIN_FALL(DBG_DFU_BOOTLOADER_PATH);DEBUG_PIN_FALL(DBG_DFU_BOOTLOADER_PATH);
        #endif //DBG_DFU_BOOTLOADER_PATH

#if defined (ENABLE_IO_LED)
        leds_off();
        nrf_gpio_pin_clear(BOOTLOADER_ACTIVE_LED);
#endif // ENABLE_IO_LED

        // Initiate an update of the firmware.
        err_code = bootloader_dfu_start();
        APP_ERROR_CHECK(err_code);

        enter_boot_update();
    }

#if defined (ENABLE_IO_LED)
        leds_off();
#endif // ENABLE_IO_LED

    err_code = bootloader_dfu_ap_update_continue();
    APP_ERROR_CHECK(err_code);

    if (bootloader_app_is_valid(DFU_BANK_0_REGION_START))
    {
        #if defined (DBG_DFU_BOOTLOADER_PATH)
        DEBUG_PIN_FALL(DBG_DFU_BOOTLOADER_PATH);DEBUG_PIN_FALL(DBG_DFU_BOOTLOADER_PATH);DEBUG_PIN_FALL(DBG_DFU_BOOTLOADER_PATH);
        #endif //DBG_DFU_BOOTLOADER_PATH
        // Select a bank region to use as application region.
        // @note: Only applications running from DFU_BANK_0_REGION_START is supported.
        bootloader_app_start(DFU_BANK_0_REGION_START);
    }

#if defined (DBG_DFU_BOOTLOADER_PATH)
DEBUG_PIN_FALL(DBG_DFU_BOOTLOADER_PATH);DEBUG_PIN_FALL(DBG_DFU_BOOTLOADER_PATH);DEBUG_PIN_FALL(DBG_DFU_BOOTLOADER_PATH);DEBUG_PIN_FALL(DBG_DFU_BOOTLOADER_PATH);
#endif //DBG_DFU_BOOTLOADER_PATH
    NVIC_SystemReset();
}
