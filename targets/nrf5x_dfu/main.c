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
#include "jspininfo.h"
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
#include "app_timer.h"
#include "nrf_bootloader_info.h"
#include "lcd.h"
#if NRF_SD_BLE_API_VERSION < 5
#include "dfu_status.h"
#endif


#ifdef LED3_PININDEX
#define UPDATE_IN_PROGRESS_LED          LED3_PININDEX                                            /**< Led used to indicate that DFU is active. */
#define UPDATE_IN_PROGRESS_LED_ONSTATE  LED3_ONSTATE                                            /**< Led used to indicate that DFU is active. */
#else
#define UPDATE_IN_PROGRESS_LED          LED1_PININDEX                                            /**< Led used to indicate that DFU is active. */
#define UPDATE_IN_PROGRESS_LED_ONSTATE  LED1_ONSTATE                                            /**< Led used to indicate that DFU is active. */
#endif
#if defined(LED2_PININDEX) && !defined(LED2_NO_BOOTLOADER)
#define BOOTLOADER_BUTTON_PRESS_LED          LED2_PININDEX                                            /**< Led used to indicate that DFU is active. */
#define BOOTLOADER_BUTTON_PRESS_LED_ONSTATE  LED2_ONSTATE                                            /**< Led used to indicate that DFU is active. */
#else
#define BOOTLOADER_BUTTON_PRESS_LED          LED1_PININDEX                                            /**< Led used to indicate that DFU is active. */
#define BOOTLOADER_BUTTON_PRESS_LED_ONSTATE  LED1_ONSTATE                                            /**< Led used to indicate that DFU is active. */
#endif
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

void ble_app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name) {
  lcd_print("NRF ERROR\r\n");
  nrf_delay_ms(10000);
  NVIC_SystemReset();
}

void pin_write(Pin pin, bool value) {
  nrf_gpio_cfg_output(pinInfo[pin].pin);
  nrf_gpio_pin_write(pinInfo[pin].pin, value ^ ((pinInfo[pin].port&JSH_PIN_NEGATED)!=0));
}

static void set_led_state(bool btn, bool progress)
{
#if defined(LED2_PININDEX) && defined(LED3_PININDEX)
  pin_write(LED3_PININDEX, progress);
  pin_write(LED2_PININDEX, btn);
#elif defined(LED1_PININDEX) && !defined(PIXLJS)
  pin_write(LED1_PININDEX, progress || btn);
#endif
}

static void hardware_init(void) {
  set_led_state(false, false);

  bool polarity = (BTN1_ONSTATE==1) ^ ((pinInfo[BTN1_PININDEX].port&JSH_PIN_NEGATED)!=0);
  nrf_gpio_cfg_sense_input(pinInfo[BTN1_PININDEX].pin,
          polarity ? NRF_GPIO_PIN_PULLDOWN : NRF_GPIO_PIN_PULLUP,
          polarity ? NRF_GPIO_PIN_SENSE_HIGH : NRF_GPIO_PIN_SENSE_LOW);

#ifdef VIBRATE_PIN
  pin_write(VIBRATE_PIN,0); // vibrate off
#endif
}

static bool get_btn_state()
{
  bool state = nrf_gpio_pin_read(pinInfo[BTN1_PININDEX].pin);
  if (pinInfo[BTN1_PININDEX].port&JSH_PIN_NEGATED) state=!state;
  return state == BTN1_ONSTATE;
}


// Override Weak version
#if NRF_SD_BLE_API_VERSION < 5
bool nrf_dfu_enter_check(void) {
#else
// dfu_enter_check must be modified to add the __WEAK keyword
bool dfu_enter_check(void) {
#endif
    bool dfu_start = get_btn_state();

    // If button is held down for 3 seconds, don't start bootloader.
    // This means that we go straight to Espruino, where the button is still
    // pressed and can be used to stop execution of the sent code.
    if (dfu_start) {
      lcd_print("BOOTLOADER\r\n==========\r\n\nRELEASE BTN1 FOR DFU\r\n<                     >\r");
      int count = 3000;
      while (get_btn_state() && count) {
        nrf_delay_us(999);
        set_led_state((count&3)==0, false);
        if ((count&127)==0) lcd_print("=");
        count--;
      }
      if (!count) {
        dfu_start = false;
      } else {
        lcd_print("\r\nDFU STARTED\r\n");
      }
      set_led_state(true, true);
    }

    if (!dfu_start) {
#ifdef LCD
      lcd_print("\r\nRESUMING BOOT...\r\n");
      nrf_delay_us(500000);
#endif
#ifdef BUTTONPRESS_TO_REBOOT_BOOTLOADER
      // turn on watchdog - bootloader should override this if it starts,
      // but if we go straight through to run code then if the code fails to boot
      // we'll restart.
      NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
      NRF_WDT->CRV = (int)(10*32768); // 10 seconds
      NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  // Enable reload register 0
      NRF_WDT->TASKS_START = 1;
      NRF_WDT->RR[0] = 0x6E524635;
#endif
    }

    return dfu_start;
}

#ifdef BUTTONPRESS_TO_REBOOT_BOOTLOADER
APP_TIMER_DEF(m_reboot_timer_id);
int rebootCounter = 0;

void reboot_check_handler() {
  if (get_btn_state()) rebootCounter++;
  else rebootCounter=0;
  if (rebootCounter>10) NVIC_SystemReset();
  // We enabled the watchdog, so we must kick it (as it stays set even after restart)
  NRF_WDT->RR[0] = 0x6E524635;
}
#endif

#if NRF_SD_BLE_API_VERSION < 5
extern void dfu_set_status(DFUStatus status) {
  switch (status) {
  case DFUS_ADVERTISING_START:
    lcd_print("READY TO UPDATE\r\n");
    set_led_state(true,false);
#ifdef BUTTONPRESS_TO_REBOOT_BOOTLOADER
    uint32_t err_code;
    err_code = app_timer_create(&m_reboot_timer_id,
                        APP_TIMER_MODE_REPEATED,
                        reboot_check_handler);
    err_code = app_timer_start(m_reboot_timer_id, APP_TIMER_TICKS(100, 0), NULL);
    lcd_print("HOLD BTN1 TO REBOOT\r\n");
#endif
    break;
  case DFUS_ADVERTISING_STOP:
    break;
  case DFUS_CONNECTED:
    lcd_print("CONNECTED\r\n");
    set_led_state(false,true); break;
  case DFUS_DISCONNECTED:
    lcd_print("DISCONNECTED\r\n");
    break;
  }
}
#else
static void dfu_observer(nrf_dfu_evt_type_t evt_type)
{
    switch (evt_type)
    {
        case NRF_DFU_EVT_DFU_FAILED:
        case NRF_DFU_EVT_DFU_ABORTED:
        case NRF_DFU_EVT_DFU_INITIALIZED:
          set_led_state(true,false);
          break;
        case NRF_DFU_EVT_TRANSPORT_ACTIVATED:
          set_led_state(false,true);
          break;
        case NRF_DFU_EVT_DFU_STARTED:
            break;
        default:
            break;
    }
}
#endif

/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t ret_val;

    (void) NRF_LOG_INIT(NULL);

    NRF_LOG_INFO("Inside main\r\n");

    hardware_init();

#ifdef LCD
    lcd_init();
    bool wait = false;
    if (NRF_POWER->RESETREAS & POWER_RESETREAS_LOCKUP_Msk) {
      lcd_print("LOCKUP DETECTED\r\n");
      wait = true;
    }
    if (NRF_POWER->RESETREAS & POWER_RESETREAS_SREQ_Msk) {
      lcd_print("SOFTWARE RESET\r\n");
      wait = true;
    }
    if (NRF_POWER->RESETREAS & POWER_RESETREAS_DOG_Msk) {
      lcd_print("WATCHDOG TIMEOUT\r\n");
      wait = true;
    }
    if (NRF_POWER->RESETREAS & POWER_RESETREAS_RESETPIN_Msk) {
      lcd_print("RESET BY PIN\r\n");
      wait = true;
    }
    // Clear reset reason flags
    NRF_POWER->RESETREAS = 0xFFFFFFFF;
    if (wait) {
      lcd_print("\nHOLD BTN1 FOR DFU\r\n\n\n");
      nrf_delay_us(1000000); // 1 sec delay
    }
#endif

#if NRF_SD_BLE_API_VERSION < 5
    ret_val = nrf_bootloader_init();
    APP_ERROR_CHECK(ret_val);
    // Either there was no DFU functionality enabled in this project or the DFU module detected
    // no ongoing DFU operation and found a valid main application.
    // Boot the main application.
    nrf_bootloader_app_start(MAIN_APPLICATION_START_ADDR);
#else
    ret_val = nrf_bootloader_init(dfu_observer);
    APP_ERROR_CHECK(ret_val);
    // Either there was no DFU functionality enabled in this project or the DFU module detected
    // no ongoing DFU operation and found a valid main application.
    // Boot the main application.
    nrf_bootloader_app_start();
#endif


    // Should never be reached.
    NRF_LOG_INFO("After main\r\n");
}

/**
 * @}
 */
