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

/**
 * @cond (NODOX)
 * @defgroup app_mailbox_internal Auxiliary internal types declarations
 * @{
 * @ingroup app_mailbox
 * @internal
 *
 * @brief Module for internal usage inside the library only
 *
 * Some definitions must be included in the header file because
 * of the way the library is set up. In this way, the are accessible to the user.
 * However, any functions and variables defined here may change at any time
 * without a warning, so you should not access them directly.
 */
     /**
     * @brief Mailbox handle used for managing a mailbox queue.
     */
    typedef struct
    {
        uint8_t                      r_idx;    /**< Read index for the mailbox queue. */
        uint8_t                      w_idx;    /**< Write index for the mailbox queue. */
        uint8_t                      len;      /**< Number of elements currently in the mailbox queue. */
        app_mailbox_overflow_mode_t  mode;     /**< Mode of overflow handling. */
    } app_mailbox_cb_t;


/** @} 
 * @endcond
 */
