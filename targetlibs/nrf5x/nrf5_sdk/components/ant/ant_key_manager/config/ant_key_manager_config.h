/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
 
#ifndef ANT_KEY_MANAGER_CONFIG_H__
#define ANT_KEY_MANAGER_CONFIG_H__

/**
 * @addtogroup ant_key_manager
 * @{
 */

#ifndef ANT_PLUS_NETWORK_KEY
    #define ANT_PLUS_NETWORK_KEY    {0, 0, 0, 0, 0, 0, 0, 0}            /**< The ANT+ network key. */
#endif //ANT_PLUS_NETWORK_KEY

#ifndef ANT_FS_NETWORK_KEY
    #define ANT_FS_NETWORK_KEY      {0, 0, 0, 0, 0, 0, 0, 0}           /**< The ANT-FS network key. */
#endif // ANT_FS_NETWORK_KEY

/**
 * @}
 */

#endif // ANT_KEY_MANAGER_CONFIG_H__
