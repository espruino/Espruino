/**
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Platform Specific Bluetooth Functionality
 * ----------------------------------------------------------------------------
 */

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "jsdevices.h"

#ifdef NRF5X
#include "ble.h"
#include "ble_advdata.h"
#else
typedef struct {
  uint16_t uuid;
  uint8_t type;			//see BLE_UUID_TYPE_... definitions
  uint8_t uuid128[16];  //BLE knows 16/32/128 bit uuids. Espruino supports 16/128.
} PACKED_FLAGS ble_uuid_t;
typedef struct {
  //uint8_t addr_id_peer;
  uint8_t addr_type;
  uint8_t addr[6];
} ble_gap_addr_t;
#define BLE_GATT_HANDLE_INVALID (0)
#define BLE_GAP_ADDR_TYPE_PUBLIC (0)
#define BLE_GAP_ADDR_TYPE_RANDOM_STATIC (1)
#define BLE_GAP_ADV_MAX_SIZE (31)
#define BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE   0x02
#define BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE         0x03
#define BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE  0x06
#define BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE        0x07
#define BLE_GAP_AD_TYPE_SERVICE_DATA                        0x16
#define BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME                    0x08
#define BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME                 0x09
#define BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA          0xFF
#define BLE_UUID_TYPE_UNKNOWN (0)
#define BLE_UUID_TYPE_BLE (1)
#define BLE_UUID_TYPE_128 2
#define MSEC_TO_UNITS(MS,MEH) MS
#endif

#ifdef NRF52
// nRF52 gets the ability to connect to other
#define CENTRAL_LINK_COUNT              1                                           /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                           /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#else
#define CENTRAL_LINK_COUNT              0                                           /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                           /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/
#endif

#ifdef ESP32
#define CENTRAL_LINK_COUNT				1
#endif

#define APP_TIMER_OP_QUEUE_SIZE         2                                           /**< Size of timer operation queues. */
#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */

// BLE HID stuff
#define HID_KEYS_MAX_LEN                     16                                      /**< Maximum length of the Input Report characteristic. */
#define HID_MODIFIER_KEY_POS                 0                                       /**< Position of the modifier byte in the Input Report. */
#define HID_SCAN_CODE_POS                    2                                       /**< This macro indicates the start position of the key scan code in a HID Report. As per the document titled 'Device Class Definition for Human Interface Devices (HID) V1.11, each report shall have one modifier byte followed by a reserved constant byte and then the key scan code. */

#define DEFAULT_ADVERTISING_INTERVAL    MSEC_TO_UNITS(375, UNIT_0_625_MS)           /**< The advertising interval (in units of 0.625 ms). */


typedef enum  {
  BLE_NONE = 0,
  BLE_IS_SENDING = 1,         //< sending data with jswrap_nrf_transmit_string?
  BLE_IS_SCANNING = 2,        //< scanning for BLE devices?
  BLE_IS_ADVERTISING = 4,     //< currently advertising info? stops when connected
  BLE_NEEDS_SOFTDEVICE_RESTART = 8,  //< We need to reset the services we're reporting, but we can't because we're connected
  BLE_SERVICES_WERE_SET = 16, //< setServices was called already, so we need to restart softdevice before we can call it again

  BLE_NUS_INITED = 32,        //< Has the Nordic UART service been initialised?
  BLE_HID_INITED = 64,        //< Has the BLE HID service been initialised?
  BLE_IS_SENDING_HID = 128,   //< Are we waiting to send data for USB HID?
  BLE_IS_RSSI_SCANNING = 256, //< Are we scanning for RSSI values
  BLE_IS_SLEEPING = 512,      //< NRF.sleep has been called
  BLE_PM_INITIALISED = 1024,  //< Set when the Peer Manager has been initialised (only needs doing once, even after SD restart)
  BLE_IS_NOT_CONNECTABLE = 2048, //< Is the device connectable?
  BLE_WHITELIST_ON_BOND = 4096,  //< Should we write to the whitelist whenever we bond to a device?

  BLE_IS_ADVERTISING_MULTIPLE = 8192, // We have multiple different advertising packets
  BLE_ADVERTISING_MULTIPLE_ONE = 16384,
  BLE_ADVERTISING_MULTIPLE_SHIFT = GET_BIT_NUMBER(BLE_ADVERTISING_MULTIPLE_ONE),
  BLE_ADVERTISING_MULTIPLE_MASK = 255 << BLE_ADVERTISING_MULTIPLE_SHIFT,
} BLEStatus;

typedef enum {
  BLEP_NONE,
  BLEP_CONNECTED,                   //< Peripheral connected (address as buffer)
  BLEP_DISCONNECTED,                //< Peripheral disconnected
  BLEP_RSSI_PERIPH,                 //< RSSI data from peripheral connection (rssi as data)
  BLEP_ADV_REPORT,                  //< Advertising received (as buffer)
  BLEP_RSSI_CENTRAL,                //< RSSI data from central connection (rssi as data)
  BLEP_TASK_FAIL_CONN_TIMEOUT,      //< Central: Connection timeout
  BLEP_TASK_FAIL_DISCONNECTED,      //< Central: Task failed because disconnected
  BLEP_TASK_CENTRAL_CONNECTED,      //< Central: Connected
  BLEP_TASK_DISCOVER_SERVICE,       //< New service discovered (as buffer)
  BLEP_TASK_DISCOVER_SERVICE_COMPLETE,       //< Service discovery complete
  BLEP_TASK_DISCOVER_CHARACTERISTIC, //< New characteristic discovered (as buffer)
  BLEP_TASK_DISCOVER_CHARACTERISTIC_COMPLETE, //< Characteristic discovery complete
  BLEP_TASK_DISCOVER_CCCD,          //< Discovery of CCCD for characteristic finished (cccd in data)
  BLEP_TASK_CHARACTERISTIC_READ,    //< Central: Characteristic read finished (as buffer)
  BLEP_TASK_CHARACTERISTIC_WRITE,   //< Central: Characteristic write finished
  BLEP_TASK_CHARACTERISTIC_NOTIFY,  //< Central: Started requesting notifications
  BLEP_CENTRAL_DISCONNECTED,        //< Central: Disconnected (reason as data)
  BLEP_TASK_BONDING,                //< Bonding negotiation complete (success in data)
  BLEP_NFC_STATUS,                  //< NFC changed state
  BLEP_NFC_RX,                      //< NFC data received (as buffer)
  BLEP_NFC_TX,                      //< NFC data sent
  BLEP_HID_SENT,                    //< A HID report has been sent
  BLEP_WRITE,                       //< One of our characteristics written by someone else
  BLEP_NOTIFICATION,                //< A characteristic we were watching has changes
} BLEPending;


extern volatile BLEStatus bleStatus;
extern uint16_t bleAdvertisingInterval;           /**< The advertising interval (in units of 0.625 ms). */
extern volatile uint16_t                         m_conn_handle;    /**< Handle of the current connection. */
#if CENTRAL_LINK_COUNT>0
extern volatile uint16_t                         m_central_conn_handle; /**< Handle for central mode connection */
#endif

/** Initialise the BLE stack */
void jsble_init();
/** Completely deinitialise the BLE stack */
void jsble_kill();
/** Add a task to the queue to be executed (to be called mainly from IRQ-land) - with a buffer of data */
void jsble_queue_pending_buf(BLEPending blep, uint16_t data, char *ptr, size_t len);
/** Add a task to the queue to be executed (to be called mainly from IRQ-land) - with simple data */
void jsble_queue_pending(BLEPending blep, uint16_t data);
/** Execute a task that was added by jsble_queue_pending - this is done outside of IRQ land. Returns number of events handled */
int jsble_exec_pending(IOEvent *event);

/** Stop and restart the softdevice so that we can update the services in it -
 * both user-defined as well as UART/HID */
void jsble_restart_softdevice();

void jsble_advertising_start();
void jsble_advertising_stop();


/** Is BLE connected to any device at all? */
bool jsble_has_connection();

/** Is BLE connected to a central device at all? */
bool jsble_has_central_connection();

/** Is BLE connected to a server device at all (eg, the simple, 'slave' mode)? */
bool jsble_has_simple_connection();

/// Checks for error and reports an exception if there was one. Return true on error
bool jsble_check_error(uint32_t err_code);

/// Scanning for advertisign packets
uint32_t jsble_set_scanning(bool enabled);

/// returning RSSI values for current connection
uint32_t jsble_set_rssi_scan(bool enabled);

/** Actually set the services defined in the 'data' object. Note: we can
 * only do this *once* - so to change it we must reset the softdevice and
 * then call this again */
void jsble_set_services(JsVar *data);

/// Disconnect from the given connection
uint32_t jsble_disconnect(uint16_t conn_handle);

/// For BLE HID, send an input report to the receiver. Must be <= HID_KEYS_MAX_LEN
void jsble_send_hid_input_report(uint8_t *data, int length);

// ------------------------------------------------- lower-level utility fns

#ifdef NRF5X
/// Build advertising data struct to pass into @ref ble_advertising_init.
void jsble_setup_advdata(ble_advdata_t *advdata);
#endif

#ifdef USE_NFC

#define TAG_HEADER_LEN            0x0A

#define NDEF_HEADER "\x00\x00\x00\x00" /* |      UID/BCC      | TT = Tag Type            */ \
                    "\x00\x00\x00\x00" /* |      UID/BCC      | ML = NDEF Message Length */ \
                    "\x00\x00\xFF\xFF" /* | UID/BCC |   LOCK  | TF = TNF and Flags       */ \
                    "\xE1\x11\x7C\x0F" /* |  Cap. Container   | TL = Type Legnth         */ \
                    "\x03\x00\xC1\x01" /* | TT | ML | TF | TL | RT = Record Type         */ \
                    "\x00\x00\x00\x00" /* |  Payload Length   | IC = URI Identifier Code */ \
                    "\x55\x00"         /* | RT | IC | Payload |      0x00: No prepending */

#define NDEF_FULL_RAW_HEADER_LEN  0x12 /* full header until ML */
#define NDEF_FULL_URL_HEADER_LEN  0x1A /* full header until IC */

#define NDEF_RECORD_HEADER_LEN    0x08 /* record header (TF, TL, PL, RT, IC ) */
#define NDEF_IC_OFFSET            0x19
#define NDEF_IC_LEN               0x01

#define NDEF_MSG_LEN_OFFSET       0x11
#define NDEF_PL_LEN_LSB_OFFSET    0x17 /* we support pl < 256 */

#define NDEF_TERM_TLV             0xfe /* last TLV block / byte */
#define NDEF_TERM_TLV_LEN         0x01

void jsble_nfc_stop();
void jsble_nfc_start(const uint8_t *data, size_t len);
void jsble_nfc_get_internal(uint8_t *data, size_t *max_len);
void jsble_nfc_send(const uint8_t *data, size_t len);
void jsble_nfc_send_rsp(const uint8_t data, size_t len);
#endif

#if CENTRAL_LINK_COUNT>0
/// Connect to the given peer address. When done call bleCompleteTask
void jsble_central_connect(ble_gap_addr_t peer_addr);
/// Get primary services. Filter by UUID unless UUID is invalid, in which case return all. When done call bleCompleteTask
void jsble_central_getPrimaryServices(ble_uuid_t uuid);
/// Get characteristics. Filter by UUID unless UUID is invalid, in which case return all. When done call bleCompleteTask
void jsble_central_getCharacteristics(JsVar *service, ble_uuid_t uuid);
// Write data to the given characteristic. When done call bleCompleteTask
void jsble_central_characteristicWrite(JsVar *characteristic, char *dataPtr, size_t dataLen);
// Read data from the given characteristic. When done call bleCompleteTask
void jsble_central_characteristicRead(JsVar *characteristic);
// Discover descriptors of characteristic
void jsble_central_characteristicDescDiscover(JsVar *characteristic);
// Set whether to notify on the given characteristic. When done call bleCompleteTask
void jsble_central_characteristicNotify(JsVar *characteristic, bool enable);
/// Start bonding on the current central connection
void jsble_central_startBonding(bool forceRePair);
/// Get the security status of the current link
JsVar *jsble_central_getSecurityStatus();
/// RSSI monitoring in central mode
uint32_t jsble_set_central_rssi_scan(bool enabled);
// Set whether or not the whitelist is enabled
void jsble_central_setWhitelist(bool whitelist);
#endif

#endif // BLUETOOTH_H
