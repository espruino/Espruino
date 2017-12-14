/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */


/** @file
 *
 * @defgroup ble_sdk_srv_cgms_db Continuous Glucose Monitoring Service database
 * @{
 * @ingroup ble_cgms
 *
 * @brief Continuous Glucose Monitoring Service database module.
 *
 * @details This module implements a database of stored glucose measurement values.
 *          This database is meant as an example of a database that the @ref ble_cgms can use.
 *          Replace this module if this implementation does not suit
 *          your application. Any replacement implementation should follow the API below to ensure
 *          that the qualification of the @ref ble_cgms is not compromised.
 */

#ifndef BLE_CGMS_DB_H__
#define BLE_CGMS_DB_H__

#include "sdk_errors.h"
#include "nrf_ble_cgms.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CGMS_DB_MAX_RECORDS 100 // !< Number of records that can be stored in the database.


/**@brief Function for initializing the glucose record database.
 *
 * @retval NRF_SUCCESS If the database was successfully initialized.
 */
ret_code_t cgms_db_init(void);


/**@brief Function for getting the number of records in the database.
 *
 * @return The number of records in the database.
 */
uint16_t cgms_db_num_records_get(void);


/**@brief Function for getting a specific record from the database.
 *
 * @param[in]  record_num Index of the record to retrieve.
 * @param[out] p_rec      Pointer to the record structure to which the retrieved record is copied.
 *
 * @retval NRF_SUCCESS If the record was successfully retrieved.
 */
ret_code_t cgms_db_record_get(uint8_t record_num, ble_cgms_rec_t * p_rec);


/**@brief Function for adding a record at the end of the database.
 *
 * @param[in] p_rec  Pointer to the record to add to the database.
 *
 * @retval NRF_SUCCESS If the record was successfully added to the database.
 */
ret_code_t cgms_db_record_add(ble_cgms_rec_t * p_rec);


/**@brief Function for deleting a database entry.
 *
 * @details This call deletes an record from the database.
 *
 * @param[in] record_num  Index of the record to delete.
 *
 * @retval NRF_SUCCESS If the record was successfully deleted from the database.
 */
ret_code_t cgms_db_record_delete(uint8_t record_num);


#ifdef __cplusplus
}
#endif

#endif // BLE_CGMS_DB_H__

/** @} */
