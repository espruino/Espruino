/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup ble_dfu Buttonless DFU Service
 * @{
 * @ingroup ble_sdk_srv
 * @brief Buttonless DFU Service module.
 *
 * @details
 *
 * @note Attention!
 *  To maintain compliance with Nordic Semiconductor ASA Bluetooth profile
 *  qualification listings, this section of source code must not be modified.
 */

#ifndef BLE_DFU_H__
#define BLE_DFU_H__

#include <stdint.h>
#include "ble_srv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_UUID_DFU_SERVICE 0x0001
#define BLE_DFU_BASE_UUID   {{0x50, 0xEA, 0xDA, 0x30, 0x88, 0x83, 0xB8, 0x9F, 0x60, 0x4F, 0x15, 0xF3, 0x00, 0x00, 0x40, 0x8E}} /**< Used vendor specific UUID. */

#define BLE_DFU_ENTER_BOOTLOADER 0x01

typedef enum {
    BLE_DFU_EVT_ENTERING_BOOTLOADER,   /**< Event indicating that the bootloader will be entered after return of this event.*/
    BLE_DFU_EVT_INDICATION_ENABLED,    /**< Indication that the control point is enabled.*/
    BLE_DFU_EVT_INDICATION_DISABLED    /**< Indication that the control point is disabled.*/
} ble_dfu_evt_type_t;

typedef struct {
    ble_dfu_evt_type_t type;
} ble_dfu_evt_t;
/* Forward declaration of the ble_nus_t type. */
typedef struct ble_dfu_s ble_dfu_t;

/**@brief Nordic UART Service event handler type. */
typedef void (*ble_dfu_evt_handler_t) (ble_dfu_t * p_dfu, ble_dfu_evt_t * p_evt);



// Control Point response values
typedef enum
{
    DFU_RSP_RESERVED              = 0x00,                                        /**< Reserved for future use. */
    DFU_RSP_SUCCESS               = 0x01,                                        /**< Success. */
    DFU_RSP_OP_CODE_NOT_SUPPORTED = 0x02,                                        /**< Op Code not supported. */
    DFU_RSP_OPERATION_FAILED      = 0x04,                                        /**< Operation Failed. */
    DFU_RSP_CCCD_CONFIG_IMPROPER  = BLE_GATT_STATUS_ATTERR_CPS_CCCD_CONFIG_ERROR /**< CCCD is improperly configured. */
} ble_dfu_rsp_code_t;

// Control Point Op Code values
typedef enum
{
    DFU_OP_RESERVED                         = 0x00, /**< Reserved for future use. */
    DFU_OP_ENTER_BOOTLOADER                 = 0x01, /**< Enter bootloader. */
    DFU_OP_RESPONSE_CODE                    = 0x20  /**< Response code. */
} ble_dfu_buttonless_op_code_t;



struct ble_dfu_s {
    uint8_t                     uuid_type;                      /**< UUID type for DFU UUID. */
    uint16_t                    service_handle;                 /**< Handle of DFU (as provided by the SoftDevice). */
    uint16_t                    conn_handle;
    ble_gatts_char_handles_t    control_point_char;             /**< Handles related to the DFU Control Point characteristic. */
    bool                        is_ctrlpt_notification_enabled;

    ble_dfu_evt_handler_t       evt_handler;                    /**< Event handler which is called right before. */

    bool                        is_waiting_for_disconnection;
};

typedef struct {
    ble_dfu_evt_handler_t       evt_handler;                    /**< Event handler which is called right before. */
    security_req_t              ctrl_point_security_req_write_perm;      /**< Read security level of the LN Control Point characteristic. */
    security_req_t              ctrl_point_security_req_cccd_write_perm; /**< CCCD write security level of the LN Control Point characteristic. */
} ble_dfu_init_t;


/**@brief Function for initializing the Device Firmware Update module
 *
 *
 * @param[in]   p_dfu        DFU Service structure.
 * @param[in]   p_dfu_init   The structure containing the values of characteristics needed by the
 *                           service.
 *
 * @return      NRF_SUCCESS on successful initialization of service.
 */
uint32_t ble_dfu_init(ble_dfu_t * p_dfu, const ble_dfu_init_t * p_dfu_init);


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @param[in]   p_dfu      DFU Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_dfu_on_ble_evt(ble_dfu_t * p_dfu, ble_evt_t * p_ble_evt);


#ifdef __cplusplus
}
#endif

#endif // BLE_DIS_H__

/** @} */
