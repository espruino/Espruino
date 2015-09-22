/*
This software is subject to the license described in the License.txt file
included with this software distribution. You may not use this file except in compliance
with this license.

Copyright (c) Dynastream Innovations Inc. 2014
All rights reserved.
*/
#ifndef ASC_DEVICE_REGISTRY_H__
#define ASC_DEVICE_REGISTRY_H__

#include <stdint.h>
#include <stdbool.h>

/**@todo add two byte address support */

////////////////////////////////////////////////////////////////////////////////
// Module Description
////////////////////////////////////////////////////////////////////////////////

/*
 * This module is responsible for management of the device registry used to store information about peripheral devices that have registered wih the hub
 */


////////////////////////////////////////////////////////////////////////////////
// Public Definitions
////////////////////////////////////////////////////////////////////////////////
#define MAX_DEVICES                             ((uint8_t) 0x0F)
#define INVALID_SHARED_ADDRESS                  ((uint8_t) 0x00)

#define DEVICEREGISTRY_EVENT_DEVICE_ADDED       ((uint32_t)0x00000001)
#define DEVICEREGISTRY_EVENT_DEVICE_REMOVED     ((uint32_t)0x00000002)

////////////////////////////////////////////////////////////////////////////////
// Public structs
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    uint32_t    shared_address;
    uint32_t    serial_number;
    uint16_t    model_number;
    uint8_t     hw_revision;
    uint8_t     sw_revision;
    uint8_t     missed_polls;
    uint8_t     poll_count;
} asc_device_t;

typedef struct
{
    uint8_t         count;                         //Total number of registered devices
    uint32_t        event_flags;
    uint8_t         highest_registered_address;    //The highest valued address that is registered
    asc_device_t    devices[MAX_DEVICES];
} asc_device_registry_t;


////////////////////////////////////////////////////////////////////////////////
// Public Function Prototypes
////////////////////////////////////////////////////////////////////////////////
void deviceregistry_clear_event(asc_device_registry_t * const p_registry, uint32_t event);

void deviceregistry_setup(asc_device_registry_t * p_registry);

uint8_t deviceregistry_get_next_free_shared_address(asc_device_registry_t * p_registry);

uint8_t deviceregistry_get_first_registered_shared_address(asc_device_registry_t * p_registry);

uint8_t deviceregistry_get_next_registered_shared_address(asc_device_registry_t * p_registry, uint8_t previous_shared_address);

bool deviceregistry_add_device(asc_device_registry_t * p_registry, asc_device_t* p_device);

bool deviceregistry_remove_device(asc_device_registry_t * p_registry, uint8_t shared_address);

bool deviceregistry_is_full(asc_device_registry_t * p_registry);

asc_device_t* deviceregistry_get_device(asc_device_registry_t * p_registry, uint8_t shared_address);

bool deviceregistry_is_device_registered(asc_device_registry_t * p_registry, uint8_t shared_address);

#endif /* ASC_DEVICE_REGISTRY_H__ */

