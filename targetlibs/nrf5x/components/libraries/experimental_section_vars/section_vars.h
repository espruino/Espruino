/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

#ifndef SECTION_VARS_H__
#define SECTION_VARS_H__

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup section_vars Section variables
 * @ingroup app_common
 * @{
 *
 * @brief Section variables.
 */


#if defined(__ICCARM__)
// Enable IAR language extensions
#pragma language=extended
#endif


// Macro to delay macro expansion.
#define NRF_PRAGMA(x)                                       _Pragma(#x)


//lint -save -e27 Illegal character (0x24)


/**@brief   Macro to obtain the symbol marking the beginning of a given section.
 *
 * @details The symbol that this macro resolves to is used to obtain a section start address.
 *
 * @param[in]   section_name    Name of the section.
 */
#if defined(__CC_ARM)

#define NRF_SECTION_VARS_START_SYMBOL(section_name)         section_name ## $$Base

#elif defined(__GNUC__)

#define NRF_SECTION_VARS_START_SYMBOL(section_name)         __start_ ## section_name

#elif defined(__ICCARM__)

#define NRF_SECTION_VARS_START_SYMBOL(section_name)         __section_begin(#section_name)

#endif


/**@brief   Macro to obtain the symbol marking the end of a given section.
 *
 * @details The symbol that this macro resolves to is used to obtain a section stop address.
 *
 * @param[in]   section_name    Name of the section.
 */
#if defined(__CC_ARM)

#define NRF_SECTION_VARS_END_SYMBOL(section_name)           section_name ## $$Limit

#elif defined(__GNUC__)

#define NRF_SECTION_VARS_END_SYMBOL(section_name)           __stop_ ## section_name

#elif defined(__ICCARM__)

#define NRF_SECTION_VARS_END_SYMBOL(section_name)           __section_end(#section_name)

#endif


//lint -restore


/**@brief   Macro for retrieving the length of a given section, in bytes.
 *
 * @param[in]   section_name    Name of the section.
 */
#if defined(__CC_ARM)

#define NRF_SECTION_VARS_LENGTH(section_name) \
    ((uint32_t)&NRF_SECTION_VARS_END_SYMBOL(section_name) - (uint32_t)&NRF_SECTION_VARS_START_SYMBOL(section_name))

#elif defined(__GNUC__)

 #define NRF_SECTION_VARS_LENGTH(section_name) \
    ((uint32_t)&NRF_SECTION_VARS_END_SYMBOL(section_name) - (uint32_t)&NRF_SECTION_VARS_START_SYMBOL(section_name))

#elif defined(__ICCARM__)

 #define NRF_SECTION_VARS_LENGTH(section_name) \
    ((uint32_t)NRF_SECTION_VARS_END_SYMBOL(section_name) - (uint32_t)NRF_SECTION_VARS_START_SYMBOL(section_name))

#endif


/**@brief   Macro to obtain the address of the beginning of a section.
 *
 * param[in]    section_name    Name of the section.
 */
#if defined(__CC_ARM)

#define NRF_SECTION_VARS_START_ADDR(section_name)       (uint32_t)&NRF_SECTION_VARS_START_SYMBOL(section_name)

#elif defined(__GNUC__)

#define NRF_SECTION_VARS_START_ADDR(section_name)       (uint32_t)&NRF_SECTION_VARS_START_SYMBOL(section_name)

#elif defined(__ICCARM__)

#define NRF_SECTION_VARS_START_ADDR(section_name)       (uint32_t)iar_ ## section_name ## _start

#endif


/**@brief    Macro to obtain the address of end of a section.
 *
 * @param[in]   section_name    Name of the section.
 */
#if defined(__CC_ARM)

#define NRF_SECTION_VARS_END_ADDR(section_name)         (uint32_t)&NRF_SECTION_VARS_END_SYMBOL(section_name)

#elif defined(__GNUC__)

#define NRF_SECTION_VARS_END_ADDR(section_name)         (uint32_t)&NRF_SECTION_VARS_END_SYMBOL(section_name)

#elif defined(__ICCARM__)

#define NRF_SECTION_VARS_END_ADDR(section_name)         (uint32_t)iar_ ## section_name ## _end

#endif


//lint -save -e19 -esym(526, fs_dataBase) -esym(526, fs_dataLimit) -esym(526, dfu_transBase) -esym(526, dfu_transLimit)

/**@brief   Macro to create a section to register variables in.
 *
 * @param[in]   data_type       The data type of the variables to be registered in the section.
 * @param[in]   section_name    Name of the section.
 *
 * @warning The data type must be word aligned to prevent padding.
 */
#if defined(__CC_ARM)

#define NRF_SECTION_VARS_CREATE_SECTION(section_name, data_type)    \
    extern data_type * NRF_SECTION_VARS_START_SYMBOL(section_name); \
    extern void      * NRF_SECTION_VARS_END_SYMBOL(section_name)

#elif defined(__GNUC__)

#define NRF_SECTION_VARS_CREATE_SECTION(section_name, data_type)    \
    extern data_type * NRF_SECTION_VARS_START_SYMBOL(section_name); \
    extern void      * NRF_SECTION_VARS_END_SYMBOL(section_name)

#elif defined(__ICCARM__)

// No symbol registration required for IAR.
#define NRF_SECTION_VARS_CREATE_SECTION(section_name, data_type)                    \
    NRF_PRAGMA(section = #section_name);                                            \
    extern void * iar_ ## section_name ## _start = __section_begin(#section_name);  \
    extern void * iar_ ## section_name ## _end   = __section_end(#section_name)

#endif

//lint -restore


/**@brief   Macro to declare a variable and register it in a section.
 *
 * @details Declares a variable and registers it in a named section. This macro ensures that the
 *          variable is not stripped away when using optimizations.
 *
 * @note The order with which variables are placed in a section is dependant on the order with
 *       which the linker encouters the variables during linking.
 *
 * @param[in]   section_name    Name of the section.
 * @param[in]   section_var     The variable to register in the given section.
 */
#if defined(__CC_ARM)

#define NRF_SECTION_VARS_REGISTER_VAR(section_name, section_var) \
    static section_var __attribute__ ((section(#section_name))) __attribute__((used))

#elif defined(__GNUC__)

#define NRF_SECTION_VARS_REGISTER_VAR(section_name, section_var) \
    static section_var __attribute__ ((section("."#section_name))) __attribute__((used))

#elif defined(__ICCARM__)

#define NRF_SECTION_VARS_REGISTER_VAR(section_name, section_var) \
    __root section_var @ #section_name

#endif


/**@brief   Macro to retrieve a variable from a section.
 *
 * @warning     The stored symbol can only be resolved using this macro if the
 *              type of the data is word aligned. The operation of acquiring
 *              the stored symbol relies on sizeof of the stored type, no
 *              padding can exist in the named section in between individual
 *              stored items or this macro will fail.
 *
 * @param[in]   i               Index of the variable in section.
 * @param[in]   data_type       Data type of the variable.
 * @param[in]   section_name    Name of the section.
 */
#if defined(__CC_ARM)

#define NRF_SECTION_VARS_GET(i, data_type, section_name) \
    (data_type*)(NRF_SECTION_VARS_START_ADDR(section_name) + i * sizeof(data_type))

#elif defined(__GNUC__)

#define NRF_SECTION_VARS_GET(i, data_type, section_name) \
    (data_type*)(NRF_SECTION_VARS_START_ADDR(section_name) + i * sizeof(data_type))

#elif defined(__ICCARM__)

#define NRF_SECTION_VARS_GET(i, data_type, section_name) \
    (data_type*)(NRF_SECTION_VARS_START_ADDR(section_name) + i * sizeof(data_type))

#endif


/**@brief   Macro to get number of variables registered in a section.
 *
 * @param[in]   data_type       Data type of the variables in the section.
 * @param[in]   section_name    Name of the section.
 */
#define NRF_SECTION_VARS_COUNT(data_type, section_name) \
    NRF_SECTION_VARS_LENGTH(section_name) / sizeof(data_type)

/** @} */


//lint -restore Illegal character (0x24)


#ifdef __cplusplus
}
#endif

#endif // SECTION_VARS_H__
