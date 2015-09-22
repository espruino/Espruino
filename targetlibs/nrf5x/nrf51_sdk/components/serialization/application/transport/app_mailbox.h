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
#ifndef _APP_MAILBOX_H
#define _APP_MAILBOX_H

#include <stdint.h>

/**
 * @brief Identifier used by the application to identify unique mailbox queue.
 */
typedef struct app_mailbox *app_mailbox_id_t;

/**
 * @brief Mailbox definition structure.
 */
typedef struct app_mailbox_def  {
  uint32_t                queue_sz;    /**< number of elements in the queue.*/
  uint32_t                 item_sz;    /**< size of an item. */
  void                       *pool;    /**< memory array for mail. */
} app_mailbox_def_t;

/**
 * @brief A macro used to statically allocate memory for given mailbox queue.
 */
#define APP_MAILBOX_DEF(name, queue_sz, type)                  \
uint32_t os_mailQ_q_##name[3+((sizeof(type)+3)/4)*(queue_sz)]; \
const app_mailbox_def_t os_mailQ_def_##name =                  \
{ (queue_sz), sizeof(type), (os_mailQ_q_##name) }


/**
 * @brief A macro used to identify mailbox queue stucture.
 */
#define APP_MAILBOX(name) \
&os_mailQ_def_##name

/**
 * @brief Mailbox queue creation.
 *
 * @details Function initialize a mailbox queue based on provided definition. If queue is
 *          successfully created identifier is provided which is later on used access given queue.
 *
 * @param[in]  queue_def  Pointer to queue definition created using macro \ref APP_MAILBOX_DEF
 * @param[out] mailbox_id Pointer to identifier which if set after successful queue creation.
 *
 * @retval ::NRF_SUCCESS              Queue successfully created.
 * @retval ::NRF_ERROR_INVALID_PARAM  Queue creation failed. Wrong param provided.
 */
uint32_t app_mailbox_create(const app_mailbox_def_t *queue_def, app_mailbox_id_t * mailbox_id);


/**
 * @brief Putting item to mailbox queue.
 *
 * @details Function puts item to mailbox queue.
 *
 * @param[in] mailbox_id  Queue identifier.
 * @param[in] mail        Pointer to item to be queued.
 *
 * @retval ::NRF_SUCCESS              Item enqueued.
 * @retval ::NRF_ERROR_NO_MEM         Queue is full.
 * @retval ::NRF_ERROR_NULL           Null pointer provided.
 */
uint32_t app_mailbox_put (app_mailbox_id_t mailbox_id, void *mail);


/**
 * @brief Getting item from mailbox queue.
 *
 * @details Function gets item from mailbox queue.
 *
 * @param[in]  mailbox_id  Queue identifier.
 * @param[out] mail        Pointer to output location for dequeued item.
 *
 * @retval ::NRF_SUCCESS              Item enqueued.
 * @retval ::NRF_ERROR_NO_MEM         Queue is empty.
 * @retval ::NRF_ERROR_NULL           Null pointer provided.
 */
uint32_t app_mailbox_get (app_mailbox_id_t mailbox_id, void * mail);

/**
 * @brief Getting current length of the mailbox queue.
 *
 * @param[in]  mailbox_id  Queue identifier.
 * @param[out] p_len       Pointer to length.
 *
 * @retval ::NRF_SUCCESS              Item enqueued.
 * @retval ::NRF_ERROR_NULL           Null pointer provided.
 *
 */
uint32_t app_mailbox_get_length (app_mailbox_id_t mailbox_id, uint32_t *p_len);

#endif //_APP_MAILBOX_H
