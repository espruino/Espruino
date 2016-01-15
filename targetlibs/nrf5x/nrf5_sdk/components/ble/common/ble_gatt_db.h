
#ifndef BLE_GATT_DB_H__
#define BLE_GATT_DB_H__

#include <stdint.h>
#include "ble.h"
#include "ble_gattc.h"

#define BLE_GATT_DB_MAX_CHARS                                 4          /**< The maximum number of characteristics present in a service record. */

/**@brief Structure for holding the characteristic and the handle of its CCCD present on a server.
 */
typedef struct
{
    ble_gattc_char_t characteristic;  /**< Structure containing information about the characteristic. */
    uint16_t         cccd_handle;     /**< CCCD Handle value for this characteristic. This will be set to BLE_GATT_HANDLE_INVALID if a CCCD is not present at the server. */
} ble_gatt_db_char_t;

/**@brief Structure for holding information about the service and the characteristics present on a
 *        server.
 */
typedef struct
{
    ble_uuid_t               srv_uuid;                                  /**< UUID of the service. */    
    uint8_t                  char_count;                                /**< Number of characteristics present in the service. */
    ble_gattc_handle_range_t handle_range;                              /**< Service Handle Range. */
    ble_gatt_db_char_t       charateristics[BLE_GATT_DB_MAX_CHARS];     /**< Array of information related to the characteristics present in the service. This list can extend further than one. */
} ble_gatt_db_srv_t;

#endif /* BLE_GATT_DB_H__ */
