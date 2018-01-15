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

/**@file
 *

 * @defgroup sdk_common_macros SDK Common Header
 * @ingroup app_common
 * @brief Macros for parameter checking and similar tasks
 * @{
 */

#ifndef SDK_MACROS_H__
#define SDK_MACROS_H__

#ifdef __cplusplus
extern "C" {
#endif


/**@brief Macro for verifying statement to be true. It will cause the exterior function to return
 *        err_code if the statement is not true.
 *
 * @param[in]   statement   Statement to test.
 * @param[in]   err_code    Error value to return if test was invalid.
 *
 * @retval      nothing, but will cause the exterior function to return @p err_code if @p statement
 *              is false.
 */
#define VERIFY_TRUE(statement, err_code)    \
do                                          \
{                                           \
    if (!(statement))                       \
    {                                       \
        return err_code;                    \
    }                                       \
} while (0)


/**@brief Macro for verifying statement to be true. It will cause the exterior function to return
 *        if the statement is not true.
 *
 * @param[in]   statement   Statement to test.
 */
#define VERIFY_TRUE_VOID(statement) VERIFY_TRUE((statement), )


/**@brief Macro for verifying statement to be false. It will cause the exterior function to return
 *        err_code if the statement is not false.
 *
 * @param[in]   statement   Statement to test.
 * @param[in]   err_code    Error value to return if test was invalid.
 *
 * @retval      nothing, but will cause the exterior function to return @p err_code if @p statement
 *              is true.
 */
#define VERIFY_FALSE(statement, err_code)   \
do                                          \
{                                           \
    if ((statement))                        \
    {                                       \
        return err_code;                    \
    }                                       \
} while (0)


/**@brief Macro for verifying statement to be false. It will cause the exterior function to return
 *        if the statement is not false.
 *
 * @param[in]   statement    Statement to test.
 */
#define VERIFY_FALSE_VOID(statement) VERIFY_FALSE((statement), )


/**@brief Macro for verifying that a function returned NRF_SUCCESS. It will cause the exterior
 *        function to return err_code if the err_code is not @ref NRF_SUCCESS.
 *
 * @param[in] err_code The error code to check.
 */
#ifdef DISABLE_PARAM_CHECK
#define VERIFY_SUCCESS()
#else
#define VERIFY_SUCCESS(err_code) VERIFY_TRUE((err_code) == NRF_SUCCESS, (err_code))
#endif /* DISABLE_PARAM_CHECK */


/**@brief Macro for verifying that a function returned NRF_SUCCESS. It will cause the exterior
 *        function to return if the err_code is not @ref NRF_SUCCESS.
 *
 * @param[in] err_code The error code to check.
 */
#ifdef DISABLE_PARAM_CHECK
#define VERIFY_SUCCESS_VOID()
#else
#define VERIFY_SUCCESS_VOID(err_code) VERIFY_TRUE_VOID((err_code) == NRF_SUCCESS)
#endif /* DISABLE_PARAM_CHECK */


/**@brief Macro for verifying that the module is initialized. It will cause the exterior function to
 *        return @ref NRF_ERROR_INVALID_STATE if not.
 *
 * @note MODULE_INITIALIZED must be defined in each module using this macro. MODULE_INITIALIZED
 *       should be true if the module is initialized, false if not.
 */
#ifdef DISABLE_PARAM_CHECK
#define VERIFY_MODULE_INITIALIZED()
#else
#define VERIFY_MODULE_INITIALIZED() VERIFY_TRUE((MODULE_INITIALIZED), NRF_ERROR_INVALID_STATE)
#endif /* DISABLE_PARAM_CHECK */


/**@brief Macro for verifying that the module is initialized. It will cause the exterior function to
 *        return if not.
 *
 * @note MODULE_INITIALIZED must be defined in each module using this macro. MODULE_INITIALIZED
 *       should be true if the module is initialized, false if not.
 */
#ifdef DISABLE_PARAM_CHECK
#define VERIFY_MODULE_INITIALIZED_VOID()
#else
#define VERIFY_MODULE_INITIALIZED_VOID() VERIFY_TRUE_VOID((MODULE_INITIALIZED))
#endif /* DISABLE_PARAM_CHECK */


/**@brief Macro for verifying that the module is initialized. It will cause the exterior function to
 *        return if not.
 *
 * @param[in] param  The variable to check if is NULL.
 */
#ifdef DISABLE_PARAM_CHECK
#define VERIFY_PARAM_NOT_NULL()
#else
#define VERIFY_PARAM_NOT_NULL(param) VERIFY_FALSE(((param) == NULL), NRF_ERROR_NULL)
#endif /* DISABLE_PARAM_CHECK */


/**@brief Macro for verifying that the module is initialized. It will cause the exterior function to
 *        return if not.
 *
 * @param[in] param  The variable to check if is NULL.
 */
#ifdef DISABLE_PARAM_CHECK
#define VERIFY_PARAM_NOT_NULL_VOID()
#else
#define VERIFY_PARAM_NOT_NULL_VOID(param) VERIFY_FALSE_VOID(((param) == NULL))
#endif /* DISABLE_PARAM_CHECK */

/** @} */

#ifdef __cplusplus
}
#endif

#endif // SDK_MACROS_H__

