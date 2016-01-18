/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

/** @cond */
/**@file
 *
 * @ingroup experimental_api
 * @defgroup sdk_common SDK Common Header
 * @breif All common headers needed for SDK examples will be included here so that application
 *       developer does not have to include headers on him/herself.
 * @{
 */

#ifndef SDK_COMMON_H__
#define SDK_COMMON_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nordic_common.h"
#include "compiler_abstraction.h"
#include "sdk_os.h"
#include "sdk_errors.h"
#include "app_util.h"

/**@brief Macro for verifying that the module is initialized. It will cause the function to return
 *        if not.
 *
 * @param[in] param  The variable to check if is NULL.
 */
#ifndef DISABLE_PARAM_CHECK
#define VERIFY_PARAM_NOT_NULL(param)                \
do                                                  \
{                                                   \
    if (param == NULL)                              \
    {                                               \
        return NRF_ERROR_NULL;                      \
    }                                               \
} while(0)
#else
#define VERIFY_PARAM_NOT_NULL()
#endif /* DISABLE_PARAM_CHECK */


/**@brief Macro for verifying that the module is initialized. It will cause the function to return
 *        if not.
 *
 * @param[in] param  The variable to check if is NULL.
 */
#ifndef DISABLE_PARAM_CHECK
#define VERIFY_PARAM_NOT_NULL_VOID(param)           \
do                                                  \
{                                                   \
    if (param == NULL)                              \
    {                                               \
        return;                                     \
    }                                               \
} while(0)
#else
#define VERIFY_PARAM_NOT_NULL_VOID()
#endif /* DISABLE_PARAM_CHECK */


/**@brief Macro for verifying that a function returned NRF_SUCCESS. Will return the err code
 * if not.
 *
 * @param[in] err_code The error code to check.
 */
#ifndef DISABLE_PARAM_CHECK
#define VERIFY_SUCCESS(err_code)            \
do                                          \
{                                           \
    if (err_code != NRF_SUCCESS)            \
    {                                       \
        return err_code;                    \
    }                                       \
} while(0)
#else
#define VERIFY_SUCCESS()
#endif /* DISABLE_PARAM_CHECK */


/**@brief Macro for verifying that a function returned NRF_SUCCESS. Will return if not.
 *
 * @param[in] err_code The error code to check.
 */
#ifndef DISABLE_PARAM_CHECK
#define VERIFY_SUCCESS_VOID(err_code)       \
do                                          \
{                                           \
    if (err_code != NRF_SUCCESS)            \
    {                                       \
        return;                             \
    }                                       \
} while(0)
#else
#define VERIFY_SUCCESS_VOID()
#endif /* DISABLE_PARAM_CHECK */

/** @} */
/** @endcond */
#endif // SDK_COMMON_H__

