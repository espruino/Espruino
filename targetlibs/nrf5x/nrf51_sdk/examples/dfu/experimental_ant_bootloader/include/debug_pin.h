/*
This software is subject to the license described in the license.txt file included with this software distribution.You may not use this file except in compliance with this license.
Copyright © Dynastream Innovations Inc. 2014
All rights reserved.
*/

#ifndef DEBUG_PIN_H_
#define DEBUG_PIN_H_

#include "nrf51.h"
#include "nrf51_bitfields.h"

/**********************************************************************************/
/* Comment this out to DISABLE all Debugging pins especially on official releases.*/
//#define DEBUGGING_PINS_ENABLE

#if defined (DEBUGGING_PINS_ENABLE)
   #define DEBUG_DFU_BOOTLOADER
//   #define DEBUG_UART_STACKCHECK

#define DEBUG_USE_UART_OUT
#endif // STACK_DEBUGGING_PINS_ENABLE

/**********************************************************************************/

#define DEBUG_PIN_ON(pin)        { NRF_GPIO->OUTSET = (1UL << (pin)); }
#define DEBUG_PIN_OFF(pin)       { NRF_GPIO->OUTCLR = (1UL << (pin)); }
#define DEBUG_PIN_RISE(pin)      { NRF_GPIO->OUTCLR = (1UL << (pin)); NRF_GPIO->OUTSET = (1UL << (pin));}
#define DEBUG_PIN_FALL(pin)      { NRF_GPIO->OUTSET = (1UL << (pin)); NRF_GPIO->OUTCLR = (1UL << (pin));}

/*DEBUG OUT, !!!!WARNING THIS USES UART0!!!! */

#if defined (DEBUG_USE_UART_OUT)

#define DEBUG_UART_INIT(pin)        NRF_UART0->PSELRXD = 0xFFFFFFFF;\
                                    NRF_UART0->PSELTXD = pin;\
                                    NRF_UART0->CONFIG = 0x00;\
                                    NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud1M;\
                                    NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos;\
                                    NRF_UART0->TASKS_STARTTX = 1;
#define DEBUG_UART_OUT(val)         NRF_UART0->TXD = val

#else

#define DEBUG_UART_INIT(pin)
#define DEBUG_UART_OUT(val)

#endif

//////////////////////////////////////////////////////////////////
// CONFIGURE DEBUG PINS HERE
//////////////////////////////////////////////////////////////////

/*****************************************
 * APP_DFU
 */
#if defined (DEBUG_DFU_BOOTLOADER)

/*starts at 24 ends at 32*/
#define DBG_DFU_BOOTLOADER_PATH             24
#define DBG_DFU_FLASH_IMAGE_STATUS          25
#define DBG_DFU_CKPT_PINC                   25
#define DBG_DFU_CKPT_PINA                   28
#define DBG_DFU_CKPT_PINB                   29
//#define DBG_DFU_FLASH_PIN                   29
//    #define DBG_DFU_FLASH_ERASE                 8
//    #define DBG_DFU_FLASH_WRITE                 9
//    #define DBG_DFU_FLASH_RESP                  10
//    #define DBG_DFU_FLASH_PSTORAGE_CB           11
//    #define DBG_DFU_FLASH_CB_HANDLER            12

#define DBG_DFU_UART_OUT_PIN                30          //antfs_event_process
#define DBG_UART_DFU_ANTFS_EVENT_PIN        30
#define DBG_UART_ANTFS_DFU_STATE_PIN        30
#define DBG_UART_DFU_DATA_OFFSET_PIN      30


   #define DBG_PIN_DIR_INIT               { NRF_GPIO->DIRSET = 0xFFFF0000;\
                                             DEBUG_UART_INIT(DBG_DFU_UART_OUT_PIN);}
#endif //DEBUG_DFU_BOOTLOADER
/*
 * APP_DFU END
 *****************************************/

#ifndef DBG_PIN_DIR_INIT
   #define DBG_PIN_DIR_INIT
#endif

void stack_debug_Manchester_Start(uint8_t ucPin, uint8_t ucCode);
void stack_debug_Manchester_Stop(uint8_t ucPin);

#endif /* DEBUG_PIN_H_ */
