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

#include "dfu_transport.h"
#include <stddef.h>
#include "dfu.h"
#include <dfu_types.h>
#include "app_error.h"
#include "app_util.h"
#include "hci_transport.h"
#include "app_timer.h"
#include "app_scheduler.h"

#define MAX_BUFFERS          4u                                                      /**< Maximum number of buffers that can be received queued without being consumed. */

/**
 * defgroup Data Packet Queue Access Operation Macros
 * @{
 */

/** Provides status showing if the queue is full or not. */
#define DATA_QUEUE_FULL()                                                                         \
        (((MAX_BUFFERS -1) == m_data_queue.count) ? true : false)

/** Provides status showing if the queue is empty or not */
#define DATA_QUEUE_EMPTY()                                                                        \
        ((0 == m_data_queue.count) ? true : false)

/** Initializes an element of the data queue. */
#define DATA_QUEUE_ELEMENT_INIT(i)                                                                \
        m_data_queue.data_packet[(i)].packet_type = INVALID_PACKET

/** Sets the packet type of an element of the data queue. */
#define DATA_QUEUE_ELEMENT_SET_PTYPE(i, t)                                                         \
        m_data_queue.data_packet[(i)].packet_type = (t)

/** Copies a data packet pointer of an element of the data queue. */
#define DATA_QUEUE_ELEMENT_COPY_PDATA(i, dp)                                                       \
        m_data_queue.data_packet[(i)].params.data_packet.p_data_packet = (uint32_t *)(dp)

/** Sets the packet length of an element in the data queue. */
#define DATA_QUEUE_ELEMENT_SET_PLEN(i, l)                                                          \
        m_data_queue.data_packet[(i)].params.data_packet.packet_length = (l)

/** Gets a data packet pointer of an element in the data queue. */
#define DATA_QUEUE_ELEMENT_GET_PDATA(i)                                                           \
        (m_data_queue.data_packet[(i)].params.data_packet.p_data_packet)

/** Gets the packet type of an element in the data queue. */
#define DATA_QUEUE_ELEMENT_GET_PTYPE(i)                                                           \
        m_data_queue.data_packet[(i)].packet_type

/* @} */

/** Abstracts data packet queue */
typedef struct
{
    dfu_update_packet_t   data_packet[MAX_BUFFERS];                                  /**< Bootloader data packets used when processing data from the UART. */
    volatile uint8_t      count;                                                     /**< Counter to maintain number of elements in the queue. */
} dfu_data_queue_t;

static dfu_data_queue_t      m_data_queue;                                           /**< Received-data packet queue. */

/**@brief Initializes an element of the data buffer queue.
 *
 * @param[in] element_index index of the element.
 */
static void data_queue_element_init (uint8_t element_index)
{
    DATA_QUEUE_ELEMENT_INIT(element_index);
    DATA_QUEUE_ELEMENT_SET_PTYPE(element_index, INVALID_PACKET);
    DATA_QUEUE_ELEMENT_COPY_PDATA(element_index, NULL);
    DATA_QUEUE_ELEMENT_SET_PLEN(element_index, 0);
}

/** Initializes data buffer queue */
static void data_queue_init(void)
{
    uint32_t index;

    m_data_queue.count = 0;

    for (index = 0; index < MAX_BUFFERS; index++)
    {
        data_queue_element_init(index);
    }
}

/**@brief Function for freeing an element.
 *
 * @param[in] element_index index of the element.
 */
static uint32_t data_queue_element_free(uint8_t element_index)
{
    uint8_t * p_data;
    uint32_t  retval;

    retval = NRF_ERROR_INVALID_PARAM;
    
    if (MAX_BUFFERS > element_index)
    {
        p_data = (uint8_t *)DATA_QUEUE_ELEMENT_GET_PDATA(element_index);
        if (INVALID_PACKET != DATA_QUEUE_ELEMENT_GET_PTYPE(element_index))
        {
            m_data_queue.count--;
            data_queue_element_init (element_index);
            retval = hci_transport_rx_pkt_consume((p_data - 4));
            APP_ERROR_CHECK(retval);
        }
    }
    else
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    return NRF_SUCCESS;
}

/**@brief Function for Allocating element.
 *
 * @param[in]   packet_type      packet type.
 * @param[out]  p_element_index  index of the element.
 */
static uint32_t data_queue_element_alloc(uint8_t * p_element_index, uint8_t packet_type)
{
    uint32_t retval;
    uint32_t index;

    retval = NRF_ERROR_NO_MEM;
    
    if (INVALID_PACKET == packet_type)
    {
        retval = NRF_ERROR_INVALID_PARAM;
    }
    else if (true == DATA_QUEUE_FULL())
    {
        retval = NRF_ERROR_NO_MEM;
    }
    else
    {
        for (index = 0; index < MAX_BUFFERS; index++)
        {
            if (INVALID_PACKET == DATA_QUEUE_ELEMENT_GET_PTYPE(index))
            {
                // Found a free element: allocate, and end search.
                *p_element_index = index;
                DATA_QUEUE_ELEMENT_SET_PTYPE(index, packet_type);
                retval = NRF_SUCCESS;
                m_data_queue.count++;
                break;
            }
        }
    }

    return retval;
}

// Flush everything on disconnect or stop.
static void data_queue_flush(void)
{
    uint32_t index;

    for (index = 0; index < MAX_BUFFERS; index++)
    {
         // In this case it does not matter if free succeeded or not as data packets are being flushed because DFU Trnsport was closed
        (void)data_queue_element_free(index);
    }
}


/**@brief       Function for handling the callback events from the dfu module.
 *              Callbacks are expected when \ref dfu_data_pkt_handle has been executed.
 *
 * @param[in]   packet  Packet type for which this callback is related. START_PACKET, DATA_PACKET.
 * @param[in]   result  Operation result code. NRF_SUCCESS when a queued operation was successful.
 * @param[in]   p_data  Pointer to the data to which the operation is related.
 */
static void dfu_cb_handler(uint32_t packet, uint32_t result, uint8_t * p_data)
{
    APP_ERROR_CHECK(result);
}


static void process_dfu_packet(void * p_event_data, uint16_t event_size)
{
    uint32_t              retval;
    uint32_t              index;
    dfu_update_packet_t * packet;

        while (false == DATA_QUEUE_EMPTY())
        {
            // Fetch the element to be processed.
            for (index = 0; index < MAX_BUFFERS ; index++)
            {
                packet = &m_data_queue.data_packet[index];
                if (INVALID_PACKET != packet->packet_type)
                {

                    switch (DATA_QUEUE_ELEMENT_GET_PTYPE(index))
                    {
                        case DATA_PACKET:
                            (void)dfu_data_pkt_handle(packet);
                            break;

                        case START_PACKET:
                            packet->params.start_packet = 
                                (dfu_start_packet_t*)packet->params.data_packet.p_data_packet;
                            retval = dfu_start_pkt_handle(packet);
                            APP_ERROR_CHECK(retval);
                            break;

                        case INIT_PACKET:
                            (void)dfu_init_pkt_handle(packet);
                            retval = dfu_init_pkt_complete();
                            APP_ERROR_CHECK(retval);
                            break;

                        case STOP_DATA_PACKET:
                            (void)dfu_image_validate();
                            (void)dfu_image_activate();

                            // Break the loop by returning.
                            return;

                        default:
                            // No implementation needed.
                            break;
                    }

                    // Free the processed element.
                    retval = data_queue_element_free(index);
                    APP_ERROR_CHECK(retval);
                }
            }
        }
}


void rpc_transport_event_handler(hci_transport_evt_t event)
{
    uint32_t  retval;
    uint16_t  rpc_cmd_length_read;
    uint8_t * p_rpc_cmd_buffer;
    uint8_t   element_index;

    retval = hci_transport_rx_pkt_extract(&p_rpc_cmd_buffer, &rpc_cmd_length_read);
    if (NRF_SUCCESS == retval)
    {
        // Verify if the data queue can buffer the packet.
        retval = data_queue_element_alloc(&element_index, p_rpc_cmd_buffer[0]);
        if (NRF_SUCCESS == retval)
        {
            //subtract 1 since we are interested in payload length and not the type field.
            DATA_QUEUE_ELEMENT_SET_PLEN(element_index,(rpc_cmd_length_read / sizeof(uint32_t)) - 1);
            DATA_QUEUE_ELEMENT_COPY_PDATA(element_index, &p_rpc_cmd_buffer[4]);
            retval = app_sched_event_put(NULL, 0, process_dfu_packet);
        }
    }
    
    if (NRF_SUCCESS != retval)
    {
        // Free the packet that could not be processed.
        retval = hci_transport_rx_pkt_consume(p_rpc_cmd_buffer);
        APP_ERROR_CHECK(retval);
    }
}


uint32_t dfu_transport_update_start(void)
{
    uint32_t err_code;

    // Initialize data buffer queue.
    data_queue_init();

    dfu_register_callback(dfu_cb_handler);

    // Open transport layer.
    err_code = hci_transport_open();
    APP_ERROR_CHECK(err_code);

    // Register callback to be run when commands have been received by the transport layer.
    err_code = hci_transport_evt_handler_reg(rpc_transport_event_handler);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}


uint32_t dfu_transport_close(void)
{
    // Remove all buffered packets.
    data_queue_flush();
    
    return hci_transport_close();
}

