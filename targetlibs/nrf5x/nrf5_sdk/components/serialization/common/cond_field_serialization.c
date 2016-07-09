#include "nrf_error.h"
#include "cond_field_serialization.h"
#include "ble_serialization.h"
#include <stddef.h>

uint32_t cond_field_enc(void const * const      p_field,
                        uint8_t * const         p_buf,
                        uint32_t                buf_len,
                        uint32_t * const        p_index,
                        field_encoder_handler_t fp_field_encoder)
{
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    p_buf[*p_index] = (p_field == NULL) ? SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;
    *p_index       += 1;

    if (p_field && (fp_field_encoder != NULL))
    {
        err_code = fp_field_encoder(p_field, p_buf, buf_len, p_index);
    }

    return err_code;
}


uint32_t cond_field_dec(uint8_t const * const   p_buf,
                        uint32_t                buf_len,
                        uint32_t * const        p_index,
                        void * * const          pp_field,
                        field_decoder_handler_t fp_field_parser)
{
    uint32_t err_code = NRF_SUCCESS;
    uint8_t  is_present;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    uint8_dec(p_buf, buf_len, p_index, &is_present);

    if (is_present == SER_FIELD_PRESENT)
    {
        SER_ASSERT_NOT_NULL(pp_field);
        SER_ASSERT_NOT_NULL(*pp_field);

        if (fp_field_parser != NULL)
        {
            err_code = fp_field_parser(p_buf, buf_len, p_index, *pp_field);
        }
    }
    else if (is_present == SER_FIELD_NOT_PRESENT)
    {
        if (pp_field != NULL)
        {
            *pp_field = NULL;
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_DATA;
    }

    return err_code;
}
