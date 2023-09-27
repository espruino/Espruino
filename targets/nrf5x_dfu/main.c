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
#ifdef ESPR_BOOTLOADER_SPIFLASH
#include "flash.h"
#endif
#if NRF_SD_BLE_API_VERSION < 5
#include "dfu_status.h"
#endif

// LEDs to use are defined in set_led_state in hardware.h

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

void turn_off() {
  lcd_kill();
#if defined(SPIFLASH_SLEEP_CMD) && defined(ESPR_BOOTLOADER_SPIFLASH)
  flashPowerDown();  // Put the SPI Flash into deep power-down
#endif  
#if defined(VIBRATE_PIN) && !defined(DICKENS)
  jshPinOutput(VIBRATE_PIN,1); // vibrate whilst waiting for button release
#endif
#if defined(BTN2_PININDEX)
  while (get_btn1_state() || get_btn2_state()) {}; // wait for BTN1 and BTN2 to be released
#else
  while (get_btn1_state()) {}; // wait for BTN1 to be released
#endif
#ifdef VIBRATE_PIN
  jshPinSetValue(VIBRATE_PIN,0); // vibrate off
#endif
#ifdef DICKENS
  NRF_P0->OUT=0x03300f04; // 00000011 00110000 00001111 00000100 - high pins: D2, D8, SDA, SCL, LCD_CS, FLASH_CS, FLASH_WP, FLASH_RST, FLASH_SCK
//NRF_P0->OUT=0x03300e00; // 00000011 00110000 00001110 00000000 - high pins:         SDA, SCL, LCD_CS, FLASH_CS, FLASH_WP, FLASH_RST, FLASH_SCK
  if (pinInfo[LCD_BL].port&JSH_PIN_NEGATED) // if backlight negated
    NRF_P1->OUT=0x00000101; // High pins: LCD_BL, P1.16 (doesn't exist, but seems to draw around 3 µA extra if this is not set)
  else
    NRF_P1->OUT=0x00000100;
  // for (uint8_t pin=0; pin<48; pin++) {
  //   NRF_GPIO_PIN_CNF(pin,0x00000006); // Set all pins as input disconnect with pulldown
  // }
  NRF_GPIO_PIN_CNF(BAT_PIN_VOLTAGE,0x00000002);   //  D4 = battery voltage measurement (no pull, input buffer disconnected)
  NRF_GPIO_PIN_CNF(ACCEL_PIN_SDA,0x0000060d);     //  D9 = SDA open-drain output
  NRF_GPIO_PIN_CNF(ACCEL_PIN_SCL,0x0000060d);     // D10 = SCL open-drain output
#ifdef ACCEL_PIN_INT1
  NRF_GPIO_PIN_CNF(ACCEL_PIN_INT1,0x00000002);    // D21 = INT1 (no pull, input buffer disconnected)
  NRF_GPIO_PIN_CNF(ACCEL_PIN_INT2,0x00000002);    // D23 = INT2 (no pull, input buffer disconnected)
#endif
  NRF_GPIO_PIN_CNF(LCD_SPI_MISO,0x00000002);      // D27 = LCD_MISO (no pull, input buffer disconnected)
//if (pinInfo[LCD_BL].port&JSH_PIN_NEGATED) // if backlight negated
//  NRF_GPIO_PIN_CNF(LCD_BL,0x00000003);          // D32 = LCD backlight pin
//NRF_GPIO_PIN_CNF(BTN2_PININDEX,0x0003000c);     // D28 = BTN2 input (with pullup and low-level sense)
//NRF_GPIO_PIN_CNF(BTN3_PININDEX,0x0003000c);     // D29 = BTN3 input (with pullup and low-level sense)
//NRF_GPIO_PIN_CNF(31,0x00000003);                // D31 = Debug output pin (brought out to external header on)
//NRF_GPIO_PIN_CNF(BTN4_PININDEX,0x0003000c);     // D42 = BTN4 input (with pullup and low-level sense)
  NRF_GPIO_PIN_CNF(BTN1_PININDEX,0x0003000c);     // D46 = BTN1 input (with pullup and low-level sense)
  NRF_GPIO_PIN_CNF(BAT_PIN_CHARGING,0x0000000c);     // Charge input (with pullup)
#else  // !DICKENS
  set_led_state(0,0);
#if defined(BTN2_PININDEX)
  nrf_gpio_cfg_sense_set(BTN2_PININDEX, NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_cfg_sense_set(BTN3_PININDEX, NRF_GPIO_PIN_NOSENSE);
#endif
  nrf_gpio_cfg_sense_input(pinInfo[BTN1_PININDEX].pin, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
  nrf_gpio_cfg_sense_set(pinInfo[BTN1_PININDEX].pin, NRF_GPIO_PIN_SENSE_LOW);
#endif
  NRF_POWER->TASKS_LOWPWR = 1;
  NRF_POWER->SYSTEMOFF = 1;
  while (true) {};
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
          turn_off();
          //NVIC_SystemReset(); // just in case!
        }
#endif
      } else {
        lcd_clear();
        print_fw_version();
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
#if NRF_SD_BLE_API_VERSION>=5
  // not quite sure why this doesn't repeat in SDK15 when we
  // asked it to - maybe DFU kills all timers?
  app_timer_start(m_reboot_timer_id,
        APP_TIMER_TICKS(100),
        NULL);
#endif
}
#endif

void dfu_evt_init() {
  set_led_state(true,false);
#ifdef REBOOT_TIMER
  uint32_t err_code;
  err_code = app_timer_create(&m_reboot_timer_id,
                      APP_TIMER_MODE_REPEATED,
                      reboot_check_handler);
  err_code = app_timer_start(m_reboot_timer_id,
#if NRF_SD_BLE_API_VERSION<5
      APP_TIMER_TICKS(100, 0),
#else
      APP_TIMER_TICKS(100),
#endif
      NULL);
#endif
#ifdef BUTTONPRESS_TO_REBOOT_BOOTLOADER
  lcd_println("BTN1 = REBOOT");
#endif
}

void dfu_evt_connected() {
  lcd_println("CONNECT");
  set_led_state(false,true);
  dfuIsConnected = true;
}

void dfu_evt_disconnected() {
  lcd_println("DISCONNECT");
  dfuIsConnected = false;
}

#if NRF_SD_BLE_API_VERSION < 5
extern void dfu_set_status(DFUStatus status) {
  switch (status) {
  case DFUS_ADVERTISING_START:
    dfu_evt_init();
    break;
/*  case DFUS_ADVERTISING_STOP:
    break;*/
  case DFUS_CONNECTED:
    dfu_evt_connected();
    break;
  case DFUS_DISCONNECTED:
    dfu_evt_disconnected();
    break;
  }
}
#else
static void dfu_observer(nrf_dfu_evt_type_t evt_type)
{
    switch (evt_type)
    {
        case NRF_DFU_EVT_DFU_INITIALIZED:
          dfu_evt_init();
          break;
        case NRF_DFU_EVT_DFU_FAILED:
        case NRF_DFU_EVT_DFU_ABORTED:
          lcd_println("ERROR");
          set_led_state(true,false);
          break;
        case NRF_DFU_EVT_TRANSPORT_ACTIVATED:
          dfu_evt_connected();
          break;
        case NRF_DFU_EVT_TRANSPORT_DEACTIVATED:
          dfu_evt_disconnected();
          break;
        case NRF_DFU_EVT_DFU_STARTED:
          lcd_println("STARTED");
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
    
    /* NRF_POWER->RESETREAS reset reason flags:
     * 0x000001 : RESETPIN  Reset pin
     * 0x000002 : DOG       Watchdog
     * 0x000004 : SREQ      Software reset
     * 0x000008 : LOCKUP    CPU lock-up
     * 0x010000 : OFF       Woke up from system OFF via GPIO DETECT
     * 0x020000 : LPCOMP    Woke up from system OFF via LPCOMP ANADETECT
     * 0x040000 : DIF       Woke up from system OFF into debug inteface mode
     * 0x080000 : NFC       Woke up from system OFF by NFC field detector
     * 0x100000 : VBUS      Woke up from system OFF by VBUS rising into valid range
     */
    int r = NRF_POWER->RESETREAS;
    dfuIsColdBoot = (r&0xF)==0;

#if defined(DICKENS) || defined(BANGLEJS)  
    // On smartwatches, turn on only if BTN1 held for >1 second (or charging on Dickens)
    // This may help in cases where battery is TOTALLY flat
    if ((r&0b1011)==0) {
      // if not watchdog, lockup, or reset pin...
      if ((r&0xF)==0) { // Bangle.softOff causes 'SW RESET' after 1 sec, so r==4
        nrf_delay_ms(1000);
      }
#ifndef DICKENS
      if (!get_btn1_state() && (r&0xF)==0) { // Don't turn off after a SW reset, to avoid user input needed during reflashing
        turn_off();
      } else {
#else // DICKENS
      if (!get_btn1_state() && get_charging_state()) {
        nrf_delay_ms(3000); // wait 4 secs in total before booting if on charge
      }
      if (!get_btn1_state() && !get_charging_state() && (r&0xF)==0) { // Don't turn off after a SW reset, to avoid user input needed during reflashing
        turn_off();
      } else {
        // DICKENS: Enter bootloader only if BTN2 held as well
        if (!get_btn2_state()) {
          // Clear reset reason flags
          NRF_POWER->RESETREAS = 0xFFFFFFFF;
#ifdef ESPR_BOOTLOADER_SPIFLASH
          lcd_init();
#ifndef DICKENS
          print_fw_version();
#endif
          // Check if we should reflash new firmware
          flashCheckAndRun();
#endif
          // Run the main application.
          nrf_bootloader_app_start();
        } else {
        }
#endif // DICKENS
      }
    }
#endif

#ifdef LCD
    lcd_init();

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
    print_fw_version();
#ifdef ESPR_BOOTLOADER_SPIFLASH
    if (!get_btn1_state()) flashCheckAndRun();
#endif
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
