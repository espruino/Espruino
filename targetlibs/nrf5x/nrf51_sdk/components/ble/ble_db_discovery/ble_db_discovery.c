/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

#include "ble_db_discovery.h"
#include <stdlib.h>
#include <string.h>
#include "nrf_error.h"
#include "ble.h"
#include "app_trace.h"
#include "nordic_common.h"

#define SRV_DISC_START_HANDLE  0x0001                    /**< The start handle value used during service discovery. */
#define DB_DISCOVERY_MAX_USERS BLE_DB_DISCOVERY_MAX_SRV  /**< The maximum number of users/registrations allowed by this module. */
#define DB_LOG                 app_trace_log             /**< A debug logger macro that can be used in this file to do logging information over UART. */

/**@brief Array of structures containing information about the registered application modules. */
static struct
{
    ble_uuid_t                     srv_uuid;     /**< The UUID of the service for which the application module had registered itself.*/
    ble_db_discovery_evt_handler_t evt_handler;  /**< The event handler of the application module to be called in case there are any events.*/
} m_registered_handlers[DB_DISCOVERY_MAX_USERS];

/**@brief   Array of structures containing pending events to be sent to the application modules.
 *
 * @details Whenever a discovery related event is to be raised to a user module, it will be stored
 *          in this array first. When all services needed to be discovered have been
 *          discovered, all pending events will be sent to the corresponding user modules.
 **/
static struct
{
    ble_db_discovery_evt_t         evt;          /**< The pending event. */
    ble_db_discovery_evt_handler_t evt_handler;  /**< The event handler which should be called to raise this event. */
} m_pending_user_evts[DB_DISCOVERY_MAX_USERS];

static uint32_t m_pending_usr_evt_index;    /**< The index to the pending user event array, pointing to the last added pending user event. */
static uint32_t m_num_of_handlers_reg;      /**< The number of handlers registered with the DB Discovery module. */
static uint32_t m_num_of_discoveries_made;  /**< The total number of service discoveries (successful or unsuccessful) made since initialization. */
static bool     m_initialized = false;      /**< This variable Indicates if the module is initialized or not. */

/**@brief     Function for fetching the event handler provided by a registered application module.
 *
 * @param[in] srv_uuid UUID of the service.
 *
 * @retval    evt_handler Event handler of the module, registered for the given service UUID.
 * @retval    NULL If no event handler is found.
 */
static ble_db_discovery_evt_handler_t registered_handler_get(ble_uuid_t * p_srv_uuid)
{
    uint32_t i;

    for (i = 0; i < m_num_of_handlers_reg; i++)
    {
        if (BLE_UUID_EQ(&(m_registered_handlers[i].srv_uuid), p_srv_uuid))
        {
            return (m_registered_handlers[i].evt_handler);
        }
    }

    return NULL;
}


/**@brief     Function for storing the event handler provided by a registered application module.
 *
 * @param[in] p_srv_uuid    The UUID of the service.
 * @param[in] p_evt_handler The event handler provided by the application.
 *
 * @retval    NRF_SUCCESS If the handler was stored.
 * @retval    NRF_ERROR_NO_MEM If there is no space left to store the handler.
 */
static uint32_t registered_handler_set(const ble_uuid_t * const       p_srv_uuid,
                                       ble_db_discovery_evt_handler_t p_evt_handler)
{
    if (m_num_of_handlers_reg < DB_DISCOVERY_MAX_USERS)
    {
        m_registered_handlers[m_num_of_handlers_reg].srv_uuid    = *p_srv_uuid;
        m_registered_handlers[m_num_of_handlers_reg].evt_handler = p_evt_handler;

        m_num_of_handlers_reg++;

        return NRF_SUCCESS;
    }
    else
    {
        return NRF_ERROR_NO_MEM;
    }
}


/**@brief Function for sending all pending discovery events to the corresponding user modules.
 */
static void pending_user_evts_send(void)
{
    uint32_t i;

    for (i = 0; i < m_num_of_handlers_reg; i++)
    {
        // Pass the event to the corresponding event handler.
        m_pending_user_evts[i].evt_handler(&(m_pending_user_evts[i].evt));
    }
    
    m_pending_usr_evt_index = 0;
}


/**@brief     Function for indicating error to the application.
 *
 * @details   This function will fetch the event handler based on the UUID of the service being
 *            discovered. (The event handler is registered by the application beforehand).
 *            The error code is added to the pending events together with the event handler.
 *            If no event handler was found, then this function will do nothing.
 *
 * @param[in] p_db_discovery Pointer to the DB discovery structure.
 * @param[in] err_code       Error code that should be provided to the application.
 *
 */
static void discovery_error_evt_trigger(ble_db_discovery_t * const p_db_discovery,
                                        uint32_t                   err_code)
{
    ble_db_discovery_evt_handler_t p_evt_handler;
    ble_db_discovery_srv_t       * p_srv_being_discovered;

    p_srv_being_discovered = &(p_db_discovery->services[p_db_discovery->curr_srv_ind]);

    p_evt_handler = registered_handler_get(&(p_srv_being_discovered->srv_uuid));

    if (p_evt_handler != NULL)
    {
        if (m_pending_usr_evt_index < DB_DISCOVERY_MAX_USERS)
        {
            // Insert an event into the pending event list.
            m_pending_user_evts[m_pending_usr_evt_index].evt.conn_handle     =
                p_db_discovery->conn_handle;
            m_pending_user_evts[m_pending_usr_evt_index].evt.evt_type        =
                BLE_DB_DISCOVERY_ERROR;
            m_pending_user_evts[m_pending_usr_evt_index].evt.params.err_code = err_code;
            m_pending_user_evts[m_pending_usr_evt_index].evt_handler         = p_evt_handler;

            m_pending_usr_evt_index++;

            if (m_pending_usr_evt_index == m_num_of_handlers_reg)
            {
                // All registered modules have pending events.
                // Send all pending events to the user modules.
                pending_user_evts_send();
            }
        }
        else
        {
            // Too many events pending. Do nothing. (Ideally this should not happen.)
        }
    }
}


/**@brief     Function for triggering a Discovery Complete or Service Not Found event to the
 *            application.
 *
 * @details   This function will fetch the event handler based on the UUID of the service being
 *            discovered. (The event handler is registered by the application beforehand).
 *            It then triggers an event indicating the completion of the service discovery.
 *            If no event handler was found, then this function will do nothing.
 *
 * @param[in] p_db_discovery Pointer to the DB discovery structure.
 * @param[in] is_srv_found   Variable to indicate if the service was found at the peer.
 */
static void discovery_complete_evt_trigger(ble_db_discovery_t * const p_db_discovery,
                                           bool                       is_srv_found)
{
    ble_db_discovery_evt_handler_t p_evt_handler;
    ble_db_discovery_srv_t       * p_srv_being_discovered;

    p_srv_being_discovered = &(p_db_discovery->services[p_db_discovery->curr_srv_ind]);

    p_evt_handler = registered_handler_get(&(p_srv_being_discovered->srv_uuid));

    if (p_evt_handler != NULL)
    {
        if (m_pending_usr_evt_index < DB_DISCOVERY_MAX_USERS)
        {
            // Insert an event into the pending event list.
            m_pending_user_evts[m_pending_usr_evt_index].evt.conn_handle =
                p_db_discovery->conn_handle;

            if (is_srv_found)
            {
                m_pending_user_evts[m_pending_usr_evt_index].evt.evt_type =
                    BLE_DB_DISCOVERY_COMPLETE;

                m_pending_user_evts[m_pending_usr_evt_index].evt.params.discovered_db =
                    *p_srv_being_discovered;
            }
            else
            {
                m_pending_user_evts[m_pending_usr_evt_index].evt.evt_type =
                    BLE_DB_DISCOVERY_SRV_NOT_FOUND;
            }
            m_pending_user_evts[m_pending_usr_evt_index].evt_handler = p_evt_handler;

            m_pending_usr_evt_index++;

            if (m_pending_usr_evt_index == m_num_of_handlers_reg)
            {
                // All registered modules have pending events. Send all pending events to the user
                // modules.
                pending_user_evts_send();
            }
            else
            {
                // Too many events pending. Do nothing. (Ideally this should not happen.)
            }
        }
    }
}


/**@brief     Function for handling service discovery completion.
 *
 * @details   This function will be used to determine if there are more services to be discovered,
 *            and if so, initiate the discovery of the next service.
 *
 * @param[in] p_db_discovery Pointer to the DB Discovery Structure.
 */
static void on_srv_disc_completion(ble_db_discovery_t * p_db_discovery)
{
    m_num_of_discoveries_made++;

    // Check if more services need to be discovered.
    if (m_num_of_discoveries_made < m_num_of_handlers_reg)
    {
        // Reset the current characteristic index since a new service discovery is about to start.
        p_db_discovery->curr_char_ind = 0;

        // Initiate discovery of the next service.
        p_db_discovery->curr_srv_ind++;

        ble_db_discovery_srv_t * p_srv_being_discovered;

        p_srv_being_discovered = &(p_db_discovery->services[p_db_discovery->curr_srv_ind]);

        p_srv_being_discovered->srv_uuid =
            m_registered_handlers[p_db_discovery->curr_srv_ind].srv_uuid;

        // Reset the characteristic count in the current service to zero since a new service
        // discovery is about to start.
        p_srv_being_discovered->char_count = 0;

        DB_LOG("[DB]: Starting discovery of service with UUID 0x%x for Connection handle %d\r\n",
               p_srv_being_discovered->srv_uuid.uuid, p_db_discovery->conn_handle);

        uint32_t err_code;

        err_code = sd_ble_gattc_primary_services_discover
                   (
                   p_db_discovery->conn_handle,
                   SRV_DISC_START_HANDLE,
                   &(p_srv_being_discovered->srv_uuid)
                   );
        if (err_code != NRF_SUCCESS)
        {
            p_db_discovery->discovery_in_progress = false;

            // Error with discovering the service.
            // Indicate the error to the registered user application.
            discovery_error_evt_trigger(p_db_discovery, err_code);

            return;
        }
    }
    else
    {
        // No more service discovery is needed.
        p_db_discovery->discovery_in_progress = false;
    }
}


/**@brief     Function for finding out if a characteristic discovery should be performed after the
 *            last discovered characteristic.
 *
 * @details   This function is used during the time of database discovery to find out if there is
 *            a need to do more characteristic discoveries. The value handles of the
 *            last discovered characteristic is compared with the end handle of the service.
 *            If the service handle is greater than one of the former characteristic handles,
 *            it means that a characteristic discovery is required.
 *
 * @param[in] p_db_discovery The pointer to the DB Discovery structure.
 * @param[in] p_after_char   The pointer to the last discovered characteristic.
 *
 * @retval    True if a characteristic discovery is required. 
 * @retval    False if a characteristic discovery is NOT required.  
 */
static bool is_char_discovery_reqd(ble_db_discovery_t * const p_db_discovery,
                                   ble_gattc_char_t         * p_after_char)
{
    if (
        p_after_char->handle_value <
        p_db_discovery->services[p_db_discovery->curr_srv_ind].handle_range.end_handle
       )
    {
        // Handle value of the characteristic being discovered is less than the end handle of
        // the service being discovered. There is a possibility of more characteristics being
        // present. Hence a characteristic discovery is required.
        return true;
    }
    
    return false;
}


/**@brief      Function to find out if a descriptor discovery is required.
 *
 * @details    This function finds out if there is a possibility of existence of descriptors between
 *             current characteristic and the next characteristic. If so, this function will compute
 *             the handle range on which the descriptors may be present and will return it.
 *             If the current characteristic is the last known characteristic, then this function 
 *             will use the service end handle to find out if the current characteristic can have
 *             descriptors.
 *
 * @param[in]  p_db_discovery Pointer to the DB Discovery structure.
 * @param[in]  p_curr_char    Pointer to the current characteristic.
 * @param[in]  p_next_char    Pointer to the next characteristic. This should be NULL if the
 *                            caller knows that there is no characteristic after the current
 *                            characteristic at the peer.
 * @param[out] p_handle_range Pointer to the handle range in which descriptors may exist at the
 *                            the peer.
 *
 * @retval     True If a descriptor discovery is required.
 * @retval     False If a descriptor discovery is NOT required.
 */
static bool is_desc_discovery_reqd(ble_db_discovery_t       * p_db_discovery,
                                   ble_db_discovery_char_t  * p_curr_char,
                                   ble_db_discovery_char_t  * p_next_char,
                                   ble_gattc_handle_range_t * p_handle_range)
{
    if (p_next_char == NULL)
    {
        // Current characteristic is the last characteristic in the service. Check if the value
        // handle of the current characteristic is equal to the service end handle.
        if (
            p_curr_char->characteristic.handle_value ==
            p_db_discovery->services[p_db_discovery->curr_srv_ind].handle_range.end_handle
           )
        {
            // No descriptors can be present for the current characteristic. p_curr_char is the last
            // characteristic with no descriptors.
            return false;
        }

        p_handle_range->start_handle = p_curr_char->characteristic.handle_value + 1;
        
        // Since the current characteristic is the last characteristic in the service, the end
        // handle should be the end handle of the service.
        p_handle_range->end_handle =
            p_db_discovery->services[p_db_discovery->curr_srv_ind].handle_range.end_handle;

        return true;
    }

    // p_next_char != NULL. Check for existence of descriptors between the current and the next
    // characteristic.
    if ((p_curr_char->characteristic.handle_value + 1) == p_next_char->characteristic.handle_decl)
    {
        // No descriptors can exist between the two characteristic.
        return false;
    }

    p_handle_range->start_handle = p_curr_char->characteristic.handle_value + 1;
    p_handle_range->end_handle   = p_next_char->characteristic.handle_decl - 1;

    return true;
}


/**@brief     Function for performing characteristic discovery.
 *
 * @param[in] p_db_discovery Pointer to the DB Discovery structure.
 *
 * @return    NRF_SUCCESS if the SoftDevice was successfully requested to perform the characteristic
 *            discovery. Otherwise an error code. This function returns the error code returned
 *            by the SoftDevice API @ref sd_ble_gattc_characteristics_discover.
 */
static uint32_t characteristics_discover(ble_db_discovery_t * const p_db_discovery)
{
    ble_db_discovery_srv_t * p_srv_being_discovered;
    ble_gattc_handle_range_t handle_range;

    p_srv_being_discovered = &(p_db_discovery->services[p_db_discovery->curr_srv_ind]);

    if (p_db_discovery->curr_char_ind != 0)
    {
        // This is not the first characteristic being discovered. Hence the 'start handle' to be
        // used must be computed using the handle_value of the previous characteristic.
        ble_gattc_char_t * p_prev_char;
        uint8_t            prev_char_ind = p_db_discovery->curr_char_ind - 1;

        p_srv_being_discovered = &(p_db_discovery->services[p_db_discovery->curr_srv_ind]);

        p_prev_char = &(p_srv_being_discovered->charateristics[prev_char_ind].characteristic);

        handle_range.start_handle = p_prev_char->handle_value + 1;
    }
    else
    {
        // This is the first characteristic of this service being discovered.
        handle_range.start_handle = p_srv_being_discovered->handle_range.start_handle;
    }

    handle_range.end_handle = p_srv_being_discovered->handle_range.end_handle;

    return sd_ble_gattc_characteristics_discover(p_db_discovery->conn_handle,
                                                 &handle_range);
}


/**@brief      Function for performing descriptor discovery, if required.
 *
 * @details    This function will check if descriptor discovery is required and then perform it if
 *             needed. If no more descriptor discovery is required for the service, then the output
 *             parameter p_raise_discov_complete is set to true, indicating to the caller that a
 *             discovery complete event can be triggered to the application.
 *
 * @param[in]  p_db_discovery           Pointer to the DB Discovery structure.
 * @param[out] p_raise_discov_complete  The value pointed to by this pointer will be set to true if
 *                                      the Discovery Complete event can be triggered to the
 *                                      application.
 *
 * @return     NRF_SUCCESS if the SoftDevice was successfully requested to perform the descriptor
 *             discovery, or if no more descriptor discovery is required. Otherwise an error code.
 *             This function returns the error code returned by the SoftDevice API @ref
 *             sd_ble_gattc_descriptors_discover.
 */
static uint32_t descriptors_discover(ble_db_discovery_t * const p_db_discovery,
                                     bool *                     p_raise_discov_complete)
{
    ble_gattc_handle_range_t   handle_range;
    ble_db_discovery_char_t  * p_curr_char_being_discovered;
    ble_db_discovery_srv_t   * p_srv_being_discovered;
    bool                       is_discovery_reqd = false;    

    p_srv_being_discovered = &(p_db_discovery->services[p_db_discovery->curr_srv_ind]);

    p_curr_char_being_discovered =
        &(p_srv_being_discovered->charateristics[p_db_discovery->curr_char_ind]);

    if ((p_db_discovery->curr_char_ind + 1) == p_srv_being_discovered->char_count)
    {
        // This is the last characteristic of this service.
        is_discovery_reqd = is_desc_discovery_reqd(p_db_discovery,
                                                   p_curr_char_being_discovered,
                                                   NULL,
                                                   &handle_range);
    }
    else
    {
        uint8_t                   i;
        ble_db_discovery_char_t * p_next_char;

        for (i = p_db_discovery->curr_char_ind;
             i < p_srv_being_discovered->char_count;
             i++)
        {

            if (i == (p_srv_being_discovered->char_count - 1))
            {
                // The current characteristic is the last characteristic in the service.
                p_next_char = NULL;
            }
            else
            {
                p_next_char = &(p_srv_being_discovered->charateristics[i + 1]);
            }

            // Check if it is possible for the current characteristic to have a descriptor.
            if (is_desc_discovery_reqd(p_db_discovery,
                                       p_curr_char_being_discovered,
                                       p_next_char,
                                       &handle_range))
            {
                is_discovery_reqd = true;
                break;
            }
            else
            {
                // No descriptors can exist.
                p_curr_char_being_discovered = p_next_char;
                p_db_discovery->curr_char_ind++;
            }
        }
    }

    if (!is_discovery_reqd)
    {
        // No more descriptor discovery required. Discovery is complete.
        // This informs the caller that a discovery complete event can be triggered.
        *p_raise_discov_complete = true;

        return NRF_SUCCESS;
    }

    *p_raise_discov_complete = false;

    return sd_ble_gattc_descriptors_discover(p_db_discovery->conn_handle,
                                             &handle_range);
}


/**@brief     Function for handling primary service discovery response.
 *
 * @details   This function will handle the primary service discovery response and start the
 *            discovery of characteristics within that service.
 *
 * @param[in] p_db_discovery    Pointer to the DB Discovery structure.
 * @param[in] p_ble_gattc_evt   Pointer to the GATT Client event.
 */
static void on_primary_srv_discovery_rsp(ble_db_discovery_t * const    p_db_discovery,
                                         const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    if (p_ble_gattc_evt->gatt_status == BLE_GATT_STATUS_SUCCESS)
    {
        uint32_t err_code;
        const ble_gattc_evt_prim_srvc_disc_rsp_t * p_prim_srvc_disc_rsp_evt;
        ble_db_discovery_srv_t                   * p_srv_being_discovered;

        p_srv_being_discovered = &(p_db_discovery->services[p_db_discovery->curr_srv_ind]);

        p_prim_srvc_disc_rsp_evt = &(p_ble_gattc_evt->params.prim_srvc_disc_rsp);

        p_srv_being_discovered->srv_uuid     = p_prim_srvc_disc_rsp_evt->services[0].uuid;
        p_srv_being_discovered->handle_range = p_prim_srvc_disc_rsp_evt->services[0].handle_range;

        err_code = characteristics_discover(p_db_discovery);

        if (err_code != NRF_SUCCESS)
        {
            p_db_discovery->discovery_in_progress = false;

            // Error with discovering the service.
            // Indicate the error to the registered user application.
            discovery_error_evt_trigger(p_db_discovery, err_code);
        }
    }
    else
    {
        // Trigger Service Not Found event to the application.
        discovery_complete_evt_trigger(p_db_discovery, false);

        on_srv_disc_completion(p_db_discovery);
    }
}


/**@brief     Function for handling characteristic discovery response.
 *
 * @param[in] p_db_discovery    Pointer to the DB Discovery structure.
 * @param[in] p_ble_gattc_evt   Pointer to the GATT Client event.
 */
static void on_characteristic_discovery_rsp(ble_db_discovery_t * const    p_db_discovery,
                             const ble_gattc_evt_t * const                p_ble_gattc_evt)
{
    uint32_t                 err_code;
    ble_db_discovery_srv_t * p_srv_being_discovered;
    bool                     perform_desc_discov = false;    

    p_srv_being_discovered = &(p_db_discovery->services[p_db_discovery->curr_srv_ind]);

    if (p_ble_gattc_evt->gatt_status == BLE_GATT_STATUS_SUCCESS)
    {
        const ble_gattc_evt_char_disc_rsp_t * p_char_disc_rsp_evt;

        p_char_disc_rsp_evt = &(p_ble_gattc_evt->params.char_disc_rsp);

        // Find out the number of characteristics that were previously discovered (in earlier
        // characteristic discovery responses, if any).
        uint8_t num_chars_prev_disc = p_srv_being_discovered->char_count;
        
        // Find out the number of characteristics that are currently discovered (in the
        // characteristic discovery response being handled).
        uint8_t num_chars_curr_disc = p_char_disc_rsp_evt->count;

        // Check if the total number of discovered characteristics are supported by this module.
        if ((num_chars_prev_disc + num_chars_curr_disc) <= BLE_DB_DISCOVERY_MAX_CHAR_PER_SRV)
        {
            // Update the characteristics count.
            p_srv_being_discovered->char_count += num_chars_curr_disc;
        }
        else
        {
            // The number of characteristics discovered at the peer is more than the supported
            // maximum. This module will store only the characteristics found up to this point.
            p_srv_being_discovered->char_count = BLE_DB_DISCOVERY_MAX_CHAR_PER_SRV;
        }

        uint32_t i;
        uint32_t j;

        for (i = num_chars_prev_disc, j = 0; i < p_srv_being_discovered->char_count; i++, j++)
        {
            p_srv_being_discovered->charateristics[i].characteristic =
                p_char_disc_rsp_evt->chars[j];

            p_srv_being_discovered->charateristics[i].cccd_handle = BLE_GATT_HANDLE_INVALID;
        }
        
        ble_gattc_char_t * p_last_known_char;
        
        p_last_known_char = &(p_srv_being_discovered->charateristics[i - 1].characteristic);

        // If no more characteristic discovery is required, or if the maximum number of supported
        // characteristic per service has been reached, descriptor discovery will be performed.
        if (
            !is_char_discovery_reqd(p_db_discovery, p_last_known_char) ||
            (p_srv_being_discovered->char_count == BLE_DB_DISCOVERY_MAX_CHAR_PER_SRV)
           )
        {
            perform_desc_discov = true;
        }
        else
        {
            // Update the current characteristic index.
            p_db_discovery->curr_char_ind = p_srv_being_discovered->char_count;

            // Perform another round of characteristic discovery.
            err_code = characteristics_discover(p_db_discovery);

            if (err_code != NRF_SUCCESS)
            {
                p_db_discovery->discovery_in_progress = false;

                discovery_error_evt_trigger(p_db_discovery, err_code);

                return;
            }
        }
    }
    else
    {
        // The previous characteristic discovery resulted in no characteristics.
        // descriptor discovery should be performed.
        perform_desc_discov = true;
    }

    if (perform_desc_discov)
    {
        bool raise_discov_complete;

        p_db_discovery->curr_char_ind = 0;

        err_code = descriptors_discover(p_db_discovery, &raise_discov_complete);

        if (err_code != NRF_SUCCESS)
        {
            p_db_discovery->discovery_in_progress = false;

            discovery_error_evt_trigger(p_db_discovery, err_code);

            return;
        }
        if (raise_discov_complete)
        {
            // No more characteristics and descriptors need to be discovered. Discovery is complete.
            // Send a discovery complete event to the user application.
            DB_LOG("[DB]: Discovery of service with UUID 0x%x completed with success for Connection"
                   " handle %d\r\n", p_srv_being_discovered->srv_uuid.uuid,
                   p_db_discovery->conn_handle);

            discovery_complete_evt_trigger(p_db_discovery, true);

            on_srv_disc_completion(p_db_discovery);
        }
    }
}


/**@brief     Function for handling descriptor discovery response.
 *
 * @param[in] p_db_discovery    Pointer to the DB Discovery structure.
 * @param[in] p_ble_gattc_evt   Pointer to the GATT Client event.
 */
static void on_descriptor_discovery_rsp(ble_db_discovery_t * const    p_db_discovery,
                                        const ble_gattc_evt_t * const p_ble_gattc_evt)
{
    const ble_gattc_evt_desc_disc_rsp_t * p_desc_disc_rsp_evt;
    ble_db_discovery_srv_t              * p_srv_being_discovered;

    p_srv_being_discovered = &(p_db_discovery->services[p_db_discovery->curr_srv_ind]);

    p_desc_disc_rsp_evt = &(p_ble_gattc_evt->params.desc_disc_rsp);

    ble_db_discovery_char_t * p_char_being_discovered =
        &(p_srv_being_discovered->charateristics[p_db_discovery->curr_char_ind]);

    if (p_ble_gattc_evt->gatt_status == BLE_GATT_STATUS_SUCCESS)
    {
        // The descriptor was found at the peer.
        // If the descriptor was a CCCD, then the cccd_handle needs to be populated.

        uint32_t i;

        // Loop through all the descriptors to find the CCCD.
        for (i = 0; i < p_desc_disc_rsp_evt->count; i++)
        {
            if (
                p_desc_disc_rsp_evt->descs[i].uuid.uuid ==
                BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG
               )
            {
                p_char_being_discovered->cccd_handle = p_desc_disc_rsp_evt->descs[i].handle;

                break;
            }
        }
    }

    bool raise_discov_complete = false;

    if ((p_db_discovery->curr_char_ind + 1) == p_srv_being_discovered->char_count)
    {
        // No more characteristics and descriptors need to be discovered. Discovery is complete.
        // Send a discovery complete event to the user application.

        raise_discov_complete = true;
    }
    else
    {
        // Begin discovery of descriptors for the next characteristic.
        uint32_t err_code;

        p_db_discovery->curr_char_ind++;

        err_code = descriptors_discover(p_db_discovery, &raise_discov_complete);

        if (err_code != NRF_SUCCESS)
        {
            p_db_discovery->discovery_in_progress = false;

            // Error with discovering the service.
            // Indicate the error to the registered user application.
            discovery_error_evt_trigger(p_db_discovery, err_code);

            return;
        }
    }

    if (raise_discov_complete)
    {
        DB_LOG("[DB]: Discovery of service with UUID 0x%x completed with success for Connection"
               "handle %d\r\n", p_srv_being_discovered->srv_uuid.uuid,
               p_db_discovery->conn_handle);

        discovery_complete_evt_trigger(p_db_discovery, true);

        on_srv_disc_completion(p_db_discovery);
    }
}


uint32_t ble_db_discovery_init(void)
{
    m_num_of_handlers_reg      = 0;
    m_initialized              = true;
    m_num_of_discoveries_made  = 0;
    m_pending_usr_evt_index    = 0;

    return NRF_SUCCESS;
}


uint32_t ble_db_discovery_close()
{
    m_num_of_handlers_reg      = 0;
    m_initialized              = false;
    m_num_of_discoveries_made  = 0;
    m_pending_usr_evt_index    = 0;

    return NRF_SUCCESS;
}


uint32_t ble_db_discovery_evt_register(const ble_uuid_t * const             p_uuid,
                                       const ble_db_discovery_evt_handler_t evt_handler)
{
    if ((p_uuid == NULL) || (evt_handler == NULL))
    {
        return NRF_ERROR_NULL;
    }

    if (!m_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (m_num_of_handlers_reg == DB_DISCOVERY_MAX_USERS)
    {
        // The limit on the maximum number of supported users has been reached. Return error.
        return NRF_ERROR_NOT_SUPPORTED;
    }

    return registered_handler_set(p_uuid, evt_handler);
}


uint32_t ble_db_discovery_start(ble_db_discovery_t * const p_db_discovery,
                                uint16_t                   conn_handle)
{
    if (p_db_discovery == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if (!m_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (m_num_of_handlers_reg == 0)
    {
        // No user modules were registered. There are no services to discover.
        return NRF_ERROR_INVALID_STATE;
    }

    if (p_db_discovery->discovery_in_progress)
    {
        return NRF_ERROR_BUSY;
    }

    ble_db_discovery_srv_t * p_srv_being_discovered;

    m_num_of_discoveries_made = 0;
    m_pending_usr_evt_index   = 0;

    p_db_discovery->curr_srv_ind = 0;
    p_db_discovery->conn_handle  = conn_handle;

    p_srv_being_discovered = &(p_db_discovery->services[p_db_discovery->curr_srv_ind]);

    p_srv_being_discovered->srv_uuid = m_registered_handlers[p_db_discovery->curr_srv_ind].srv_uuid;
    
    DB_LOG("[DB]: Starting discovery of service with UUID 0x%x for Connection handle %d\r\n",
           p_srv_being_discovered->srv_uuid.uuid, p_db_discovery->conn_handle);
    
    uint32_t err_code;

    err_code = sd_ble_gattc_primary_services_discover(p_db_discovery->conn_handle,
                                                      SRV_DISC_START_HANDLE,
                                                      &(p_srv_being_discovered->srv_uuid));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    p_db_discovery->discovery_in_progress = true;

    return NRF_SUCCESS;
}


void ble_db_discovery_on_ble_evt(ble_db_discovery_t * const p_db_discovery,
                                 const ble_evt_t * const    p_ble_evt)
{
    if ((p_db_discovery == NULL) || (p_ble_evt == NULL))
    {
        // Null pointer provided. Return.
        return;
    }

    if (!m_initialized)
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            p_db_discovery->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        
        case BLE_GAP_EVT_DISCONNECTED:
            memset(p_db_discovery, 0, sizeof(ble_db_discovery_t));
            p_db_discovery->conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
            on_primary_srv_discovery_rsp(p_db_discovery, &(p_ble_evt->evt.gattc_evt));
            break;

        case BLE_GATTC_EVT_CHAR_DISC_RSP:
            on_characteristic_discovery_rsp(p_db_discovery, &(p_ble_evt->evt.gattc_evt));
            break;

        case BLE_GATTC_EVT_DESC_DISC_RSP:
            on_descriptor_discovery_rsp(p_db_discovery, &(p_ble_evt->evt.gattc_evt));
            break;

        default:
            break;
    }
}
