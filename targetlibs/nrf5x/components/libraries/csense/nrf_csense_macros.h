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

#ifndef NRF_CSENSE_MACROS_H__
#define NRF_CSENSE_MACROS_H__

/** @file
 *
 * @defgroup nrf_csense_macros Capacitive Sensor
 * @{
 * @ingroup nrf_csense
 *
 * @brief A set of macros to facilitate creation of a new capacitive sensor instance.
 */

#define NRF_CSENSE_INTERNAL_BUTTON_DEF(name, p1)         \
    static nrf_csense_pad_t CONCAT_2(name, _pad) =       \
    {                                                    \
        .p_next_pad          = NULL,                     \
        .threshold           = GET_ARG_2 p1,             \
        .pad_index           = 0,                        \
        .analog_input_number = GET_ARG_1 p1              \
    };                                                   \
    static nrf_csense_min_max_t CONCAT_2(name, _minmax); \
    static nrf_csense_instance_t name =                  \
    {                                                    \
        .p_nrf_csense_pad = &CONCAT_2(name, _pad),       \
        .min_max          = &CONCAT_2(name, _minmax),    \
        .steps            = 1,                           \
        .number_of_pads   = 1,                           \
        .is_active        = false,                       \
        .is_touched       = false                        \
    };

#define NRF_CSENSE_INTERNAL_SLIDER_2_DEF(name, steps_no, p1, p2) \
    static nrf_csense_pad_t CONCAT_2(name, _pad)[2] =        \
    {                                                        \
        {                                                    \
            .p_next_pad          = &CONCAT_2(name, _pad)[1], \
            .threshold           = GET_ARG_2 p1,             \
            .pad_index           = 0,                        \
            .analog_input_number = GET_ARG_1 p1              \
        },                                                   \
        {                                                    \
            .p_next_pad          = NULL,                     \
            .threshold           = GET_ARG_2 p2,             \
            .pad_index           = 1,                        \
            .analog_input_number = GET_ARG_1 p2              \
        }                                                    \
    };                                                       \
                                                             \
    static nrf_csense_min_max_t CONCAT_2(name, _minmax)[2];  \
    static nrf_csense_instance_t name =                      \
    {                                                        \
        .p_nrf_csense_pad = CONCAT_2(name, _pad),            \
        .min_max          = CONCAT_2(name, _minmax),         \
        .steps            = steps_no,                        \
        .number_of_pads   = 2,                               \
        .is_active        = false,                           \
        .is_touched       = false                            \
    };

#define NRF_CSENSE_INTERNAL_SLIDER_3_DEF(name, steps_no, p1, p2, p3)  \
    static nrf_csense_pad_t CONCAT_2(name, _pad)[3] =        \
    {                                                        \
        {                                                    \
            .p_next_pad          = &CONCAT_2(name, _pad)[1], \
            .threshold           = GET_ARG_2 p1,             \
            .pad_index           = 0,                        \
            .analog_input_number = GET_ARG_1 p1              \
        },                                                   \
        {                                                    \
            .p_next_pad          = &CONCAT_2(name, _pad)[2], \
            .threshold           = GET_ARG_2 p2,             \
            .pad_index           = 1,                        \
            .analog_input_number = GET_ARG_1 p2              \
        },                                                   \
        {                                                    \
            .p_next_pad          = NULL,                     \
            .threshold           = GET_ARG_2 p3,             \
            .pad_index           = 2,                        \
            .analog_input_number = GET_ARG_1 p3              \
        }                                                    \
    };                                                       \
                                                             \
    static nrf_csense_min_max_t CONCAT_2(name, _minmax)[3];  \
    static nrf_csense_instance_t name =                      \
    {                                                        \
        .p_nrf_csense_pad = CONCAT_2(name, _pad),            \
        .min_max          = CONCAT_2(name, _minmax),         \
        .steps            = steps_no,                        \
        .number_of_pads   = 3,                               \
        .is_active        = false,                           \
        .is_touched       = false                            \
    };

#define NRF_CSENSE_INTERNAL_SLIDER_4_DEF(name, steps_no, p1, p2, p3, p4) \
    static nrf_csense_pad_t CONCAT_2(name, _pad)[4] =           \
    {                                                           \
        {                                                       \
            .p_next_pad          = &CONCAT_2(name, _pad)[1],    \
            .threshold           = GET_ARG_2 p1,                \
            .pad_index           = 0,                           \
            .analog_input_number = GET_ARG_1 p1                 \
        },                                                      \
        {                                                       \
            .p_next_pad          = &CONCAT_2(name, _pad)[2],    \
            .threshold           = GET_ARG_2 p2,                \
            .pad_index           = 1,                           \
            .analog_input_number = GET_ARG_1 p2                 \
        },                                                      \
        {                                                       \
            .p_next_pad          = &CONCAT_2(name, _pad)[3],    \
            .threshold           = GET_ARG_2 p3,                \
            .pad_index           = 2,                           \
            .analog_input_number = GET_ARG_1 p3                 \
        },                                                      \
        {                                                       \
            .p_next_pad          = NULL,                        \
            .threshold           = GET_ARG_2 p4,                \
            .pad_index           = 3,                           \
            .analog_input_number = GET_ARG_1 p4                 \
        }                                                       \
    };                                                          \
    static nrf_csense_min_max_t CONCAT_2(name, _minmax)[4];     \
    static nrf_csense_instance_t name =                         \
    {                                                           \
        .p_nrf_csense_pad = CONCAT_2(name, _pad),               \
        .min_max          = CONCAT_2(name, _minmax),            \
        .steps            = steps_no,                           \
        .number_of_pads   = 4,                                  \
        .is_active        = false,                              \
        .is_touched       = false                               \
    };

#define NRF_CSENSE_INTERNAL_SLIDER_5_DEF(name, steps_no, p1, p2, p3, p4, p5) \
    static nrf_csense_pad_t CONCAT_2(name, _pad)[5] =               \
    {                                                               \
        {                                                           \
            .p_next_pad          = &CONCAT_2(name, _pad)[1],        \
            .threshold           = GET_ARG_2 p1,                    \
            .pad_index           = 0,                               \
            .analog_input_number = GET_ARG_1 p1                     \
        },                                                          \
        {                                                           \
            .p_next_pad          = &CONCAT_2(name, _pad)[2],        \
            .threshold           = GET_ARG_2 p2,                    \
            .pad_index           = 1,                               \
            .analog_input_number = GET_ARG_1 p2                     \
        },                                                          \
        {                                                           \
            .p_next_pad          = &CONCAT_2(name, _pad)[3],        \
            .threshold           = GET_ARG_2 p3,                    \
            .pad_index           = 2,                               \
            .analog_input_number = GET_ARG_1 p3                     \
        },                                                          \
        {                                                           \
            .p_next_pad          = &CONCAT_2(name, _pad)[4],        \
            .threshold           = GET_ARG_2 p4,                    \
            .pad_index           = 3,                               \
            .analog_input_number = GET_ARG_1 p4                     \
        },                                                          \
        {                                                           \
            .p_next_pad          = NULL,                            \
            .threshold           = GET_ARG_2 p5,                    \
            .pad_index           = 4,                               \
            .analog_input_number = GET_ARG_1 p5                     \
        }                                                           \
    };                                                              \
                                                                    \
    static nrf_csense_min_max_t CONCAT_2(name, _minmax)[5];         \
    static nrf_csense_instance_t name =                             \
    {                                                               \
        .p_nrf_csense_pad = CONCAT_2(name, _pad),                   \
        .min_max          = CONCAT_2(name, _minmax),                \
        .steps            = steps_no,                               \
        .number_of_pads   = 5,                                      \
        .is_active        = false,                                  \
        .is_touched       = false                                   \
    };

#define NRF_CSENSE_INTERNAL_WHEEL_3_DEF(name, steps_no, p1, p2, p3)   \
    static nrf_csense_pad_t CONCAT_2(name, _pad)[4] =        \
    {                                                        \
        {                                                    \
            .p_next_pad          = &CONCAT_2(name, _pad)[1], \
            .threshold           = GET_ARG_2 p1,             \
            .pad_index           = 0,                        \
            .analog_input_number = GET_ARG_1 p1              \
        },                                                   \
        {                                                    \
            .p_next_pad          = &CONCAT_2(name, _pad)[2], \
            .threshold           = GET_ARG_2 p2,             \
            .pad_index           = 1,                        \
            .analog_input_number = GET_ARG_1 p2              \
        },                                                   \
        {                                                    \
            .p_next_pad          = &CONCAT_2(name, _pad)[3], \
            .threshold           = GET_ARG_2 p3,             \
            .pad_index           = 2,                        \
            .analog_input_number = GET_ARG_1 p3              \
        },                                                   \
        {                                                    \
            .p_next_pad          = NULL,                     \
            .threshold           = GET_ARG_2 p1,             \
            .pad_index           = 3,                        \
            .analog_input_number = GET_ARG_1 p1              \
        }                                                    \
    };                                                       \
                                                             \
    static nrf_csense_min_max_t CONCAT_2(name, _minmax)[4];  \
    static nrf_csense_instance_t name =                      \
    {                                                        \
        .p_nrf_csense_pad = CONCAT_2(name, _pad),            \
        .min_max          = CONCAT_2(name, _minmax),         \
        .steps            = steps_no,                        \
        .number_of_pads   = 4,                               \
        .is_active        = false,                           \
        .is_touched       = false                            \
    };


#define NRF_CSENSE_INTERNAL_WHEEL_4_DEF(name, steps_no, p1, p2, p3, p4) \
    static nrf_csense_pad_t CONCAT_2(name, _pad)[5] =          \
    {                                                          \
        {                                                      \
            .p_next_pad          = &CONCAT_2(name, _pad)[1],   \
            .threshold           = GET_ARG_2 p1,               \
            .pad_index           = 0,                          \
            .analog_input_number = GET_ARG_1 p1                \
        },                                                     \
        {                                                      \
            .p_next_pad          = &CONCAT_2(name, _pad)[2],   \
            .threshold           = GET_ARG_2 p2,               \
            .pad_index           = 1,                          \
            .analog_input_number = GET_ARG_1 p2                \
        },                                                     \
        {                                                      \
            .p_next_pad          = &CONCAT_2(name, _pad)[3],   \
            .threshold           = GET_ARG_2 p3,               \
            .pad_index           = 2,                          \
            .analog_input_number = GET_ARG_1 p3                \
        },                                                     \
        {                                                      \
            .p_next_pad          = &CONCAT_2(name, _pad)[4],   \
            .threshold           = GET_ARG_2 p4,               \
            .pad_index           = 3,                          \
            .analog_input_number = GET_ARG_1 p4                \
        },                                                     \
        {                                                      \
            .p_next_pad          = NULL,                       \
            .threshold           = GET_ARG_2 p1,               \
            .pad_index           = 4,                          \
            .analog_input_number = GET_ARG_1 p1                \
        }                                                      \
    };                                                         \
                                                               \
    static nrf_csense_min_max_t CONCAT_2(name, _minmax)[5];    \
    static nrf_csense_instance_t name =                        \
    {                                                          \
        .p_nrf_csense_pad = CONCAT_2(name, _pad),              \
        .min_max          = CONCAT_2(name, _minmax),           \
        .steps            = steps_no,                          \
        .number_of_pads   = 5,                                 \
        .is_active        = false,                             \
        .is_touched       = false                              \
    };

#define NRF_CSENSE_INTERNAL_WHEEL_5_DEF(name, steps_no, p1, p2, p3, p4, p5)\
    static nrf_csense_pad_t CONCAT_2(name, _pad)[6] =              \
    {                                                              \
        {                                                          \
            .p_next_pad          = &CONCAT_2(name, _pad)[1],       \
            .threshold           = GET_ARG_2 p1,                   \
            .pad_index           = 0,                              \
            .analog_input_number = GET_ARG_1 p1                    \
        },                                                         \
        {                                                          \
            .p_next_pad          = &CONCAT_2(name, _pad)[2],       \
            .threshold           = GET_ARG_2 p2,                   \
            .pad_index           = 1,                              \
            .analog_input_number = GET_ARG_1 p2                    \
        },                                                         \
        {                                                          \
            .p_next_pad          = &CONCAT_2(name, _pad)[3],       \
            .threshold           = GET_ARG_2 p3,                   \
            .pad_index           = 2,                              \
            .analog_input_number = GET_ARG_1 p3                    \
        },                                                         \
        {                                                          \
            .p_next_pad          = &CONCAT_2(name, _pad)[4],       \
            .threshold           = GET_ARG_2 p4,                   \
            .pad_index           = 3,                              \
            .analog_input_number = GET_ARG_1 p4                    \
        },                                                         \
        {                                                          \
            .p_next_pad          = &CONCAT_2(name, _pad)[5],       \
            .threshold           = GET_ARG_2 p5,                   \
            .pad_index           = 4,                              \
            .analog_input_number = GET_ARG_1 p5                    \
        },                                                         \
        {                                                          \
            .p_next_pad          = NULL,                           \
            .threshold           = GET_ARG_2 p1,                   \
            .pad_index           = 5,                              \
            .analog_input_number = GET_ARG_1 p1                    \
        }                                                          \
    };                                                             \
                                                                   \
    static nrf_csense_min_max_t CONCAT_2(name, _minmax)[6];        \
    static nrf_csense_instance_t name =                            \
    {                                                              \
        .p_nrf_csense_pad = CONCAT_2(name, _pad),                  \
        .min_max          = CONCAT_2(name, _minmax),               \
        .steps            = steps_no,                              \
        .number_of_pads   = 6,                                     \
        .is_active        = false,                                 \
        .is_touched       = false                                  \
    };

/** @} */

#endif // NRF_CSENSE_MACROS_H__
