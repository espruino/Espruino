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

#include "bootloader.h"
#include <string.h>
#include "bootloader_types.h"
#include "bootloader_util.h"
#include "dfu.h"
#include "dfu_transport.h"
#include "nrf51.h"
#include "app_error.h"
#include "nrf_sdm.h"
#include "nrf_nvmc.h"
#include "nordic_common.h"
#include "crc16.h"
#include "pstorage.h"
#include "app_scheduler.h"
#include "nrf_delay.h"

#include "debug_pin.h"

#define IRQ_ENABLED             0x01                    /**< Field identifying if an interrupt is enabled. */
#define MAX_NUMBER_INTERRUPTS   32                      /**< Maximum number of interrupts available. */

/*
* bootloader_settings space is subdivided into 8 128byte sized blocks to properly use pstorage data abstraction scheme
* with the intent to use the last block located, specifically at 0x3FF80, for ant_boot_settings matters.
*/
#define BOOTLOADER_SETTINGS_FLASH_BLOCK_SIZE    128
#define BOOTLOADER_SETTINGS_FLASH_BLOCK_COUNT   8       /* Total of 1024 */

typedef enum
{
    BOOTLOADER_UPDATING,
    BOOTLOADER_SETTINGS_SAVING,
    BOOTLOADER_COMPLETE,
    BOOTLOADER_TIMEOUT,
    BOOTLOADER_RESET,
} bootloader_status_t;

static pstorage_handle_t        m_bootsettings_handle;  /**< Pstorage handle to use for registration and identifying the bootloader module on subsequent calls to the pstorage module for load and store of bootloader setting in flash. */
static bootloader_status_t      m_update_status;        /**< Current update status for the bootloader module to ensure correct behaviour when updating settings and when update completes. */
static uint8_t                  m_delay_applied = false;/**< Delay has been applied before the initial access to flash > */

/* NOTE: Temporary use of this page until the bootloader_settings slotting is implemented */
#define BOOT_SETTINGS_PEND_ADDRESS      (BOOTLOADER_SETTINGS_ADDRESS - CODE_PAGE_SIZE)
#define BOOT_SETTINGS_PEND_VALUE        0xFFFFFFFE
uint8_t  m_boot_settings_pend[CODE_PAGE_SIZE] __attribute__((at(BOOTLOADER_SETTINGS_ADDRESS - CODE_PAGE_SIZE))) __attribute__((used));

static void pstorage_callback_handler(pstorage_handle_t * handle, uint8_t op_code, uint32_t result, uint8_t * p_data, uint32_t data_len)
{
    // If we are in BOOTLOADER_SETTINGS_SAVING state and we receive an PSTORAGE_STORE_OP_CODE
    // response then settings has been saved and update has completed.
    if ((m_update_status == BOOTLOADER_SETTINGS_SAVING) && (op_code == PSTORAGE_STORE_OP_CODE) && (handle->block_id == BOOTLOADER_SETTINGS_ADDRESS)) //
    {
        m_update_status = BOOTLOADER_COMPLETE;

        /*Clears bootloader_settings critical flag*/
        if (*((uint32_t *)BOOT_SETTINGS_PEND_ADDRESS) == BOOT_SETTINGS_PEND_VALUE)
        {
            nrf_nvmc_page_erase(BOOT_SETTINGS_PEND_ADDRESS);
        }
    }
    APP_ERROR_CHECK(result);
}


/**@brief   Function for waiting for events.
 *
 * @details This function will place the chip in low power mode while waiting for events from
 *          the S110 SoftDevice or other peripherals. When interrupted by an event, it will call the
 *          @ref app_sched_execute function to process the received event. This function will return
 *          when the final state of the firmware update is reached OR when a tear down is in
 *          progress.
 */
static void wait_for_events(void)
{
    for (;;)
    {
        // Wait in low power state for any events.
        uint32_t err_code = sd_app_evt_wait();
        APP_ERROR_CHECK(err_code);

        // Event received. Process it from the scheduler.
        app_sched_execute();

        if ((m_update_status == BOOTLOADER_COMPLETE) ||
            (m_update_status == BOOTLOADER_TIMEOUT)  ||
            (m_update_status == BOOTLOADER_RESET))
        {
            // When update has completed or a timeout/reset occured we will return.
            return;
        }
    }
}


bool bootloader_app_is_valid(uint32_t app_addr)
{
    const bootloader_settings_t * p_bootloader_settings;

    // Critical flag was not cleared.
    if (*((uint32_t *)BOOT_SETTINGS_PEND_ADDRESS) == BOOT_SETTINGS_PEND_VALUE)
    {
        return false;
    }

    // There exists an application in CODE region 1.
    if (*((uint32_t *) app_addr) == EMPTY_FLASH_MASK)
    {
        return false;
    }

    bootloader_util_settings_get(&p_bootloader_settings);

    // The application in CODE region 1 was not flagged as invalid.
    if (p_bootloader_settings->valid_app == BOOTLOADER_SETTINGS_INVALID_APPLICATION)
    {
        return false;
    }

    return true;
}


static void bootloader_settings_save(bootloader_settings_t * p_settings)
{
    //TODO. This is temporary
    static uint8_t              ant_boot_settings[BOOTLOADER_SETTINGS_FLASH_BLOCK_SIZE];
    static pstorage_handle_t    ant_boot_settings_handle;

    /* Backing up ant_boot_settings. */
    ant_boot_settings_handle.module_id  = m_bootsettings_handle.module_id;
    ant_boot_settings_handle.block_id   = m_bootsettings_handle.block_id + (BOOTLOADER_SETTINGS_FLASH_BLOCK_SIZE * (BOOTLOADER_SETTINGS_FLASH_BLOCK_COUNT - 1));
    memcpy(ant_boot_settings, (uint8_t*)ant_boot_settings_handle.block_id, BOOTLOADER_SETTINGS_FLASH_BLOCK_SIZE);

    /* NOTE: Must erase the whole module to prevent pstorage block erasing which uses swap space*/
    uint32_t err_code = pstorage_clear(&m_bootsettings_handle,
                                        BOOTLOADER_SETTINGS_FLASH_BLOCK_SIZE * BOOTLOADER_SETTINGS_FLASH_BLOCK_COUNT);
    APP_ERROR_CHECK(err_code);

    /* Write back ant_boot_settings */
    err_code = pstorage_store(&ant_boot_settings_handle,
                              ant_boot_settings,
                              BOOTLOADER_SETTINGS_FLASH_BLOCK_SIZE,
                              0);
    APP_ERROR_CHECK(err_code);

    err_code = pstorage_store(&m_bootsettings_handle,
                              (uint8_t *)p_settings,
                              sizeof(bootloader_settings_t),
                              0);
    APP_ERROR_CHECK(err_code);
}


void bootloader_dfu_update_process(dfu_update_status_t update_status)
{
    static bootloader_settings_t settings;
    const bootloader_settings_t * p_bootloader_settings;

    bootloader_util_settings_get(&p_bootloader_settings);                                   /* Extract current values of the bootloader_settings*/
    memcpy(&settings, p_bootloader_settings, sizeof(bootloader_settings_t));                /* Copy over to local bootloader_settings*/

    if (update_status.status_code == DFU_UPDATE_NEW_IMAGES)
    {
        if (update_status.sd_image_size != NEW_IMAGE_SIZE_EMPTY)
        {
            settings.sd_image.st.size = update_status.sd_image_size;
            settings.sd_image.st.bank = update_status.bank_used;
        }
        else
        {
            settings.sd_image.st.size = NEW_IMAGE_SIZE_EMPTY;
        }

        if (update_status.bl_image_size != NEW_IMAGE_SIZE_EMPTY)
        {
            settings.bl_image.st.size = update_status.bl_image_size;
            settings.bl_image.st.bank = update_status.bank_used;
        }
        else
        {
            settings.bl_image.st.size = NEW_IMAGE_SIZE_EMPTY;
        }

        if (update_status.ap_image_size != NEW_IMAGE_SIZE_EMPTY)
        {
            settings.ap_image.st.size = update_status.ap_image_size;
            settings.ap_image.st.bank = update_status.bank_used;
        }
        else
        {
            settings.ap_image.st.size = NEW_IMAGE_SIZE_EMPTY;
        }

        settings.src_image_address = update_status.src_image_address;
        m_update_status             = BOOTLOADER_SETTINGS_SAVING;

        // TEMPORARY: This serves as a critical flag for the bootloader_settings updating.
        nrf_nvmc_write_word(BOOT_SETTINGS_PEND_ADDRESS, BOOT_SETTINGS_PEND_VALUE);

        bootloader_settings_save(&settings);
    }
    else if (update_status.status_code == DFU_UPDATE_AP_SWAPPED)
    {
        settings.ap_image.st.bank   = NEW_IMAGE_BANK_DONE;
        settings.ap_image.st.size   = p_bootloader_settings->ap_image.st.size;
        settings.valid_app          = BOOTLOADER_SETTINGS_VALID_APPLICATION;
        m_update_status             = BOOTLOADER_SETTINGS_SAVING;

        // TEMPORARY: This serves as a critical flag for the bootloader_settings updating.
        nrf_nvmc_write_word(BOOT_SETTINGS_PEND_ADDRESS, BOOT_SETTINGS_PEND_VALUE);

        bootloader_settings_save(&settings);
    }
    else if (update_status.status_code == DFU_UPDATE_AP_INVALIDATED)
    {
        if (p_bootloader_settings->valid_app != BOOTLOADER_SETTINGS_INVALID_APPLICATION)
        {
            settings.valid_app          = BOOTLOADER_SETTINGS_INVALID_APPLICATION;
            m_update_status             = BOOTLOADER_UPDATING;
            bootloader_settings_save(&settings);
        }
    }
    else if (update_status.status_code == DFU_TIMEOUT)
    {
        // Timeout has occurred. Close the connection with the DFU Controller.
//        dfu_transport_close();

        m_update_status      = BOOTLOADER_TIMEOUT;
    }
    else if (update_status.status_code == DFU_RESET)
    {
        // Timeout has occurred. Close the connection with the DFU Controller.
//        dfu_transport_close();

        m_update_status      = BOOTLOADER_RESET;
    }
    else
    {
        // No implementation needed.
    }
}


uint32_t bootloader_init(void)
{
    uint32_t err_code;
    pstorage_module_param_t storage_params;

    storage_params.cb          = pstorage_callback_handler;
    storage_params.block_size  = BOOTLOADER_SETTINGS_FLASH_BLOCK_SIZE;
    storage_params.block_count = BOOTLOADER_SETTINGS_FLASH_BLOCK_COUNT;

    err_code = pstorage_init();
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = pstorage_register(&storage_params, &m_bootsettings_handle);

    m_delay_applied = false;
    return err_code;
}


uint32_t bootloader_dfu_start(void)
{
    uint32_t                err_code = NRF_SUCCESS;

    err_code = dfu_init();
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = dfu_transport_update_start();

    wait_for_events();

    err_code = dfu_transport_close();

    return err_code;
}


/**@brief Function for disabling all interrupts before jumping from bootloader to application.
 */
static void interrupts_disable(void)
{
    uint32_t interrupt_setting_mask;
    uint8_t  irq;

    // We start the loop from first interrupt, i.e. interrupt 0.
    irq                    = 0;
    // Fetch the current interrupt settings.
    interrupt_setting_mask = NVIC->ISER[0];

    for (; irq < MAX_NUMBER_INTERRUPTS; irq++)
    {
        if (interrupt_setting_mask & (IRQ_ENABLED << irq))
        {
            // The interrupt was enabled, and hence disable it.
            NVIC_DisableIRQ((IRQn_Type) irq);
        }
    }
}

// Ensure that flash operations are not executed within the first 100 ms seconds to allow
//  a debugger to be attached
static void debugger_delay (void)
{
    if (m_delay_applied == false)
    {
        m_delay_applied = true;
        nrf_delay_ms(100);
    }
}

void bootloader_app_start(uint32_t app_addr)
{
    // If the applications CRC has been checked and passed, the magic number will be written and we
    // can start the application safely.
    uint32_t err_code = sd_softdevice_disable();
    APP_ERROR_CHECK(err_code);

    interrupts_disable();

#if defined (S210_V3_STACK)
    err_code = sd_softdevice_forward_to_application();
#else
    err_code = sd_softdevice_vector_table_base_set(CODE_REGION_1_START);
#endif
    APP_ERROR_CHECK(err_code);

    bootloader_util_app_start(CODE_REGION_1_START);
}


#if !defined (S210_V3_STACK)
uint32_t temp_value;
uint32_t bootloader_dfu_sd_update_continue()
{
    uint32_t err_code = NRF_SUCCESS;
    const bootloader_settings_t * p_bootloader_settings;

    bootloader_util_settings_get(&p_bootloader_settings);

    /* Ignore update attempts on invalid src_image_address */
    if( (p_bootloader_settings->src_image_address == SRC_IMAGE_ADDRESS_EMPTY)   ||
        (p_bootloader_settings->src_image_address == SRC_IMAGE_ADDRESS_INVALID))
    {
        return NRF_SUCCESS;
    }

    if( (p_bootloader_settings->sd_image.st.bank == NEW_IMAGE_BANK_0)           ||
        (p_bootloader_settings->sd_image.st.bank == NEW_IMAGE_BANK_1))
    {
        debugger_delay();

        err_code = dfu_sd_image_swap();
        if (dfu_sd_image_validate() == NRF_SUCCESS)
        {
            /* This is a manual write to flash, non-softdevice managed */
            uint32_t address    = (uint32_t)p_bootloader_settings + BOOTLOADER_SETTINGS_SD_IMAGE_SIZE_ADR_OFFSET;
            temp_value      = p_bootloader_settings->sd_image.all & 0x3FFFFFFF; // clears image bank bits.
            nrf_nvmc_write_word( address, temp_value);
            //TODO need to catch verification error
        }
    }
    return err_code;
}

uint32_t bootloader_dfu_bl_update_continue(void)
{
    uint32_t err_code = NRF_SUCCESS;
    const bootloader_settings_t * p_bootloader_settings;

    bootloader_util_settings_get(&p_bootloader_settings);

    /* Ignore update attempts on invalid src_image_address */
    if( (p_bootloader_settings->src_image_address == SRC_IMAGE_ADDRESS_EMPTY)   ||
        (p_bootloader_settings->src_image_address == SRC_IMAGE_ADDRESS_INVALID))
    {
        return NRF_SUCCESS;
    }

    if( (p_bootloader_settings->bl_image.st.bank == NEW_IMAGE_BANK_0)           ||
        (p_bootloader_settings->bl_image.st.bank == NEW_IMAGE_BANK_1))
    {
        debugger_delay();

        if (dfu_bl_image_validate() != NRF_SUCCESS)
        {
            err_code = dfu_bl_image_swap(); // reset is built in to the mbr bootloader swap
        }
        else
        {
            /* This is a manual write to flash, non-softdevice managed */
            uint32_t address    = (uint32_t)p_bootloader_settings + BOOTLOADER_SETTINGS_BL_IMAGE_SIZE_ADR_OFFSET;
            uint32_t value      = p_bootloader_settings->bl_image.all & 0x3FFFFFFF; // clears image bank bits.
            nrf_nvmc_write_word( address, value);
            //TODO need to catch verification error
        }
    }
    return err_code;
}
#endif // !S210_V3_STACK

uint32_t bootloader_dfu_ap_update_continue(void)
{
    uint32_t err_code = NRF_SUCCESS;
    const bootloader_settings_t * p_bootloader_settings;

    bootloader_util_settings_get(&p_bootloader_settings);

    /* Ignore update attempts on invalid src_image_address */
    if( (p_bootloader_settings->src_image_address == SRC_IMAGE_ADDRESS_EMPTY)   ||
        (p_bootloader_settings->src_image_address == SRC_IMAGE_ADDRESS_INVALID))
    {
        return NRF_SUCCESS;
    }

    if( (p_bootloader_settings->ap_image.st.bank == NEW_IMAGE_BANK_0)           ||
        (p_bootloader_settings->ap_image.st.bank == NEW_IMAGE_BANK_1))
    {
        /* If updating application only, we can start the copy right now*/
        if ((p_bootloader_settings->sd_image.st.size == NEW_IMAGE_SIZE_EMPTY)   &&
            (p_bootloader_settings->bl_image.st.size == NEW_IMAGE_SIZE_EMPTY))
        {
            err_code = dfu_ap_image_swap();

            dfu_update_status_t update_status = {DFU_UPDATE_AP_SWAPPED, };

            bootloader_dfu_update_process(update_status);

            wait_for_events();
        }
    }
   return err_code;
}
