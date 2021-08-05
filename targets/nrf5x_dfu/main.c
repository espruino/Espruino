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
#include "jsutils.h"
#include "hardware.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_mbr.h"
#include "nrf_bootloader.h"
#include "nrf_bootloader_app_start.h"
#include "nrf_dfu.h"
#include "nrf_dfu_types.h"
#include "nrf_dfu_settings.h"
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

/// Set up when DFU is connected to
static bool dfuIsConnected = false;
/// Did we just power on, or did we reset because of a software reset?
static bool dfuIsColdBoot = false;

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
/*  NRF_LOG_ERROR("received a fault! id: 0x%08x, pc: 0x&08x\r\n", id, pc);
  NVIC_SystemReset();*/
}

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name) {
  /*(void)error_code;
  NRF_LOG_ERROR("received an error: 0x%08x at %s:%d!\r\n", error_code, p_file_name?p_file_name:"?", line_num);
  NVIC_SystemReset();*/
}

void app_error_handler_bare(uint32_t error_code) {
/*  (void)error_code;
  NRF_LOG_ERROR("received an error: 0x%08x!\r\n", error_code);
  NVIC_SystemReset();*/
}

void ble_app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name) {
  /*lcd_println("NRF ERROR");
  nrf_delay_ms(10000);
  NVIC_SystemReset();*/
}

// Override Weak version
#if NRF_SD_BLE_API_VERSION < 5
bool nrf_dfu_enter_check(void) {
#else
// dfu_enter_check must be modified to add the __WEAK keyword
bool dfu_enter_check(void) {
#endif
  bool dfu_start;
#ifdef BTN1_PININDEX
  dfu_start = get_btn1_state();
#else
  dfu_start = dfuIsColdBoot; // if no button, enter bootloader if it's a cold boot, then exit after a few seconds
#endif
#ifdef BUTTONPRESS_TO_REBOOT_BOOTLOADER
    // if DFU looks invalid, go straight to bootloader
    if (s_dfu_settings.bank_0.bank_code == NRF_DFU_BANK_INVALID) {
      lcd_println("BANK0 INVALID");
      if (!dfu_start) return true;
    }
#endif

    // If button is held down for 3 seconds, don't start bootloader.
    // This means that we go straight to Espruino, where the button is still
    // pressed and can be used to stop execution of the sent code.
    if (dfu_start) {
#if defined(BUTTONPRESS_TO_REBOOT_BOOTLOADER) && defined(BTN2_PININDEX)
      lcd_print("RELEASE BTN1 FOR DFU\r\nBTN1 TO BOOT\r\nBTN1 + BTN2 TURN OFF\r\n\r\n<                   >\r");
#else
      lcd_print("RELEASE BTN1 FOR DFU\r\nBTN1 TO BOOT\r\n\r\n<                   >\r");
#endif
#ifdef BTN1_PININDEX
      int count = 20;
#ifdef BANGLEJS_F18
      while (get_btn1_state() && --count) {
        // the screen update takes long enough that
        // we don't need a delay
        lcd_print("=");
      }
#else
      count *= 128;
      while (get_btn1_state() && count) {
        nrf_delay_us(999);
        set_led_state((count&3)==0, false);
        if ((count&127)==0) lcd_print("=");
        count--;
      }
#endif
#else
      // no button, ensure we enter bootloader
      int count=1; 
#endif
      if (!count) {
        dfu_start = false;
#if defined(BUTTONPRESS_TO_REBOOT_BOOTLOADER) && defined(BTN2_PININDEX)
        if (jshPinGetValue(BTN2_PININDEX)) {
          lcd_kill();
          jshPinOutput(VIBRATE_PIN,1); // vibrate on
          while (get_btn1_state() || get_btn2_state()) {};
          jshPinSetValue(VIBRATE_PIN,0); // vibrate off
          set_led_state(0,0);
          nrf_gpio_cfg_sense_set(BTN2_PININDEX, NRF_GPIO_PIN_NOSENSE);
          nrf_gpio_cfg_sense_set(BTN3_PININDEX, NRF_GPIO_PIN_NOSENSE);
          nrf_gpio_cfg_sense_input(pinInfo[BTN1_PININDEX].pin, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
          nrf_gpio_cfg_sense_set(pinInfo[BTN1_PININDEX].pin, NRF_GPIO_PIN_SENSE_LOW);
          NRF_POWER->TASKS_LOWPWR = 1;
          NRF_POWER->SYSTEMOFF = 1;
          while (true) {};
          //NVIC_SystemReset(); // just in case!
        }
#endif
      } else {
        lcd_clear();
        lcd_println("DFU START");
      }
      set_led_state(true, true);
    }

    if (!dfu_start) {
#ifdef LCD
      lcd_println("\r\nBOOTING...");
#ifdef BANGLEJS
      nrf_delay_us(100000);
#endif
#endif
#ifdef BUTTONPRESS_TO_REBOOT_BOOTLOADER
      // turn on watchdog - bootloader should override this if it starts,
      // but if we go straight through to run code then if the code fails to boot
      // we'll restart.
      NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
      NRF_WDT->CRV = (int)(5*32768); // 5 seconds
      NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  // Enable reload register 0
      NRF_WDT->TASKS_START = 1;
      NRF_WDT->RR[0] = 0x6E524635; // Kick...
#endif
    }

    return dfu_start;
}

#if defined(BUTTONPRESS_TO_REBOOT_BOOTLOADER) || !defined(BTN1_PININDEX)
#define REBOOT_TIMER
APP_TIMER_DEF(m_reboot_timer_id);
int rebootCounter = 0;

void reboot_check_handler() {
#ifdef BUTTONPRESS_TO_REBOOT_BOOTLOADER
  if (get_btn1_state()) rebootCounter++;
  else rebootCounter=0;
  if (rebootCounter>10) {
    NVIC_SystemReset();
  }
  // We enabled the watchdog, so we must kick it (as it stays set even after restart)
  NRF_WDT->RR[0] = 0x6E524635;
#endif
#ifndef BTN1_PININDEX
  if (dfuIsConnected)
    rebootCounter = 0;
  else {
    rebootCounter++;
    if (rebootCounter>50) {
      // After 5 seconds of not being connected force reboot.
      // We'll then see that it wasn't a cold boot
      // and will start up normally
      NVIC_SystemReset();
    }
  }
#endif
}
#endif

#if NRF_SD_BLE_API_VERSION < 5
extern void dfu_set_status(DFUStatus status) {
  switch (status) {
  case DFUS_ADVERTISING_START:
    set_led_state(true,false);
#ifdef REBOOT_TIMER
    uint32_t err_code;
    err_code = app_timer_create(&m_reboot_timer_id,
                        APP_TIMER_MODE_REPEATED,
                        reboot_check_handler);
    err_code = app_timer_start(m_reboot_timer_id, APP_TIMER_TICKS(100, 0), NULL);
#endif
#ifdef BUTTONPRESS_TO_REBOOT_BOOTLOADER
    lcd_println("BTN1 = REBOOT");
#endif
    break;
/*  case DFUS_ADVERTISING_STOP:
    break;*/
  case DFUS_CONNECTED:
    lcd_println("CONNECT");
    set_led_state(false,true); 
    dfuIsConnected = true;
    break;
  case DFUS_DISCONNECTED:
    lcd_println("DISCONNECT");
    dfuIsConnected = false;
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

    // Did we just power on? If not (we watchdog/softreset) RESETREAS will be nonzero
    dfuIsColdBoot = NRF_POWER->RESETREAS==0;
#ifdef LCD
    lcd_init();
    int r = NRF_POWER->RESETREAS;
    const char *reasons = "PIN\0WATCHDOG\0SW RESET\0LOCKUP\0OFF\0";
    while (*reasons) {
      if (r&1)
        lcd_println(reasons);
      r>>=1;

      while (*reasons) reasons++;
      reasons++;
    }
    // Clear reset reason flags
    NRF_POWER->RESETREAS = 0xFFFFFFFF;
    lcd_println("DFU " JS_VERSION "\n");
#ifdef BANGLEJS
    nrf_delay_us(500000); // 500ms delay
#endif
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
