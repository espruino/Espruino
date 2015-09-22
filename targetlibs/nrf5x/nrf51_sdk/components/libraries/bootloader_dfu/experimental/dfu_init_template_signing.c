/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup nrf_dfu_init_template Template file with an DFU init packet handling example.
 * @{
 *
 * @ingroup nrf_dfu
 *
 * @brief This file contains a template on how to implement DFU init packet handling.
 *
 * @details The template shows how device type and revision can be used for a safety check of the 
 *          received image. It shows how validation can be performed in two stages:
 *          - Stage 1: Pre-check of firmware image before transfer to ensure the firmware matches:
 *                     - Device Type.
 *                     - Device Revision.
 *                     Installed SoftDevice.
 *                     This template can be extended with additional checks according to needs.
 *                     For example, such a check could be the origin of the image (trusted source) 
 *                     based on a signature scheme.
 *          - Stage 2: Post-check of the image after image transfer but before installing firmware.
 *                     For example, such a check could be an integrity check in form of hashing or 
 *                     verification of a signature.
 *                     In this template, a simple CRC check is carried out.
 *                     The CRC check can be replaced with other mechanisms, like signing.
 *
 * @note This module does not support security features such as image signing, but the 
 *       implementation allows for such extension.
 *       If the init packet is signed by a trusted source, it must be decrypted before it can be
 *       processed.
 */

#include "dfu_init.h"
#include <stdint.h>
#include <string.h>
#include <dfu_types.h>
#include "nrf_sec.h"
#include "nrf_error.h"
#include "crc16.h"

// The following is the layout of the extended init packet if using image length and sha256 to validate image
// and NIST P-256 + SHA256 to sign the init_package including the extended part
#define DFU_INIT_PACKET_POS_EXT_IDENTIFIER                  0       //< Position of the identifier for the ext package
#define DFU_INIT_PACKET_POS_EXT_IMAGE_LENGTH                4       //< Position of the image length    
#define DFU_INIT_PACKET_POS_EXT_IMAGE_HASH256               8       //< Position of the 256
#define DFU_INIT_PACKET_POS_EXT_INIT_SIGNATURE_R            40      //< Position of the Signature R
#define DFU_INIT_PACKET_POS_EXT_INIT_SIGNATURE_S            72      //< Position of the Signature S
#define DFU_INIT_PACKET_EXT_LENGTH_SIGNED                   40      //< Length of the extended init packet that is part of the signed data
#define DFU_INIT_PACKET_EXT_BEGIN

#define DFU_INIT_PACKET_EXT_LENGTH_MIN          6                   //< Minimum length of the extended init packet. Init packet and CRC16
#define DFU_INIT_PACKET_EXT_LENGTH_MAX          104                 //< Identifier (4 bytes) + Image length (4 bytes) + SHA-256 digest (32 bytes) + NIST P-256 using SHA-256 (64 bytes)

#define DFU_SHA256_DIGEST_LENGTH                32                  //< Length of SHA-256 digest
#define DFU_SIGNATURE_R_LENGTH                  32                  //< Length of the signature part r
#define DFU_SIGNATURE_S_LENGTH                  32                  //< Length of the signature part s

static uint8_t m_extended_packet[DFU_INIT_PACKET_EXT_LENGTH_MAX];   //< Data array for storage of the extended data received. The extended data follows the normal init data of type \ref dfu_init_packet_t. Extended data can be used for a CRC, hash, signature, or other data. */
static uint8_t m_extended_packet_length;                            //< Length of the extended data received with init packet. */
 
 #define DFU_INIT_PACKET_USES_CRC16 (0)
 #define DFU_INIT_PACKET_USES_HASH  (1)
 #define DFU_INIT_PACKET_USES_ECDS  (2)
 

/** @snippet [DFU BLE Signing public key curve points] */
static uint8_t Qx[] = { 0x39, 0xb0, 0x58, 0x3d, 0x27, 0x07, 0x91, 0x38, 0x6a, 0xa3, 0x36, 0x0f, 0xa2, 0xb5, 0x86, 0x7e, 0xae, 0xba, 0xf7, 0xa3, 0xf4, 0x81, 0x5f, 0x78, 0x02, 0xf2, 0xa1, 0x21, 0xd5, 0x21, 0x84, 0x12 };
static uint8_t Qy[] = { 0x4a, 0x0d, 0xfe, 0xa4, 0x77, 0x50, 0xb1, 0xb5, 0x26, 0xc0, 0x9d, 0xdd, 0xf0, 0x24, 0x90, 0x57, 0x6c, 0x64, 0x3b, 0xd3, 0xdf, 0x92, 0x3b, 0xb3, 0x47, 0x97, 0x83, 0xd4, 0xfc, 0x76, 0xf5, 0x9d };
/** @snippet [DFU BLE Signing public key curve points] */

static nrf_sec_ecc_point_t Q = {.p_x   = Qx, 
                                .x_len = sizeof(Qx),
                                .p_y   = Qy, 
                                .y_len = sizeof(Qy)};

uint32_t dfu_init_prevalidate(uint8_t * p_init_data, uint32_t init_data_len)
{
    uint32_t                i = 0;
    static uint32_t         err_code;
    nrf_sec_data_t          init_data;
    nrf_sec_ecc_signature_t signature;
        
    // In order to support encryption then any init packet decryption function / library
    // should be called from here or implemented at this location.

    // Length check to ensure valid data are parsed.
    if (init_data_len < sizeof(dfu_init_packet_t))
    {
        return NRF_ERROR_INVALID_LENGTH;
    }

    // Current template uses clear text data so they can be casted for pre-check.
    dfu_init_packet_t * p_init_packet = (dfu_init_packet_t *)p_init_data;
    
    
    m_extended_packet_length = ((uint32_t)p_init_data + init_data_len) -
                                (uint32_t)&p_init_packet->softdevice[p_init_packet->softdevice_len];
    
    if (m_extended_packet_length < DFU_INIT_PACKET_EXT_LENGTH_MIN)
    {
        return NRF_ERROR_INVALID_LENGTH;
    }

    if (((uint32_t)p_init_data + init_data_len) < 
         (uint32_t)&p_init_packet->softdevice[p_init_packet->softdevice_len])
    {
        return NRF_ERROR_INVALID_LENGTH;
    }
    
    memcpy(&m_extended_packet,
           &p_init_packet->softdevice[p_init_packet->softdevice_len],
           m_extended_packet_length);

/** [DFU init application version] */
    // To support application versioning, this check should be updated.
    // This template allows for any application to be installed. However, 
    // customers can place a revision number at the bottom of the application 
    // to be verified by the bootloader. This can be done at a location 
    // relative to the application, for example the application start 
    // address + 0x0100.
/** [DFU init application version] */
    
    // First check to verify the image to be transfered matches the device type.
    // If no Device type is present in DFU_DEVICE_INFO then any image will be accepted.
    if ((DFU_DEVICE_INFO->device_type != DFU_DEVICE_TYPE_EMPTY) &&
        (p_init_packet->device_type != DFU_DEVICE_INFO->device_type))
    {
        return NRF_ERROR_INVALID_DATA;
    }
    
    // Second check to verify the image to be transfered matches the device revision.
    // If no Device revision is present in DFU_DEVICE_INFO then any image will be accepted.
    if ((DFU_DEVICE_INFO->device_rev != DFU_DEVICE_REVISION_EMPTY) &&
        (p_init_packet->device_rev != DFU_DEVICE_INFO->device_rev))
    {
        return NRF_ERROR_INVALID_DATA;
    }

    // Third check: Check the array of supported SoftDevices by this application.
    //              If the installed SoftDevice does not match any SoftDevice in the list then an
    //              error is returned.
    while (i < p_init_packet->softdevice_len)
    {
        if (p_init_packet->softdevice[i]   == DFU_SOFTDEVICE_ANY ||
            p_init_packet->softdevice[i++] == SD_FWID_GET(MBR_SIZE))
        {
            // Found a match. Break the loop.
            break;
        }
        // No matching SoftDevice found - Return NRF_ERROR_INVALID_DATA.
        return NRF_ERROR_INVALID_DATA;
    }
    
    // Check that we have the correct identifier indicating that ECDS is used
    if(*(uint32_t*)&m_extended_packet[DFU_INIT_PACKET_POS_EXT_IDENTIFIER] != DFU_INIT_PACKET_USES_ECDS)
    {
        return NRF_ERROR_INVALID_DATA;
    }
    
    // init_data consists of the regular init-packet and all the extended packet data excluding the signing key
    init_data.length = (init_data_len - m_extended_packet_length) + DFU_INIT_PACKET_EXT_LENGTH_SIGNED;
    init_data.p_data = p_init_data;

    signature.p_r   = &m_extended_packet[DFU_INIT_PACKET_POS_EXT_INIT_SIGNATURE_R];
    signature.r_len = DFU_SIGNATURE_R_LENGTH;
    signature.p_s   = &m_extended_packet[DFU_INIT_PACKET_POS_EXT_INIT_SIGNATURE_S];
    signature.s_len = DFU_SIGNATURE_S_LENGTH;

    err_code = nrf_sec_svc_verify(&init_data, &Q ,&signature, NRF_SEC_NIST256_SHA256);
    return err_code;
}

uint32_t dfu_init_postvalidate(uint8_t * p_image, uint32_t image_len)
{
    uint8_t   image_digest[DFU_SHA256_DIGEST_LENGTH];
    uint8_t * received_digest;
    
    nrf_sec_data_t data = {.p_data = p_image,
                           .length = image_len};
    
    
    // Compare image size received with signed init_packet data
    if(image_len != *(uint32_t*)&m_extended_packet[DFU_INIT_PACKET_POS_EXT_IMAGE_LENGTH])
    {
        return NRF_ERROR_INVALID_DATA;
    }
                          
    // Calculate digest from active block.
    nrf_sec_svc_hash(&data, image_digest, NRF_SEC_SHA256);

    received_digest = &m_extended_packet[DFU_INIT_PACKET_POS_EXT_IMAGE_HASH256];

    // Compare the received and calculated digests.
    if (memcmp(&image_digest[0], received_digest, DFU_SHA256_DIGEST_LENGTH) != 0)
    {
        return NRF_ERROR_INVALID_DATA;
    }

    return NRF_SUCCESS;
}
