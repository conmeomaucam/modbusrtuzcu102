/**
 * @file       msq_queue.c
 * @brief      
 * @author     Cong <congvmc1@gmail.com>
 * @date       2026-09-02
 * @version    2.0.0
 * 
 * @copyright  Copyright (c) 2026 Cong. All rights reserved.
 */

 #include "msq_queue.h"
 #include <errno.h>
 #include <pthread.h>


int msg_queue_init(msg_queue_t *q)
{
    if(q == NULL)
    {
        return -EINVAL;
    }

    q->head = 0 ; 
    q->tail = 0 ; 
    q->count = 0; 
    if(pthread_mutex_init(&q->lock , NULL) != 0)
    { 
       return -1;
    }

    if(pthread_cond_init(&q->cond , NULL) != 0)
    {
        pthread_mutex_destroy(&q->lock);
        return -1;
    }
    return 0;
}

int msg_queue_push(msg_queue_t *q, const app_msg_t *msg) {
  if (q == NULL || msg == NULL) {
    return -EINVAL;
  }

  pthread_mutex_lock(&q->lock);

  if (q->count >= QUEUE_MAX_SIZE) {
    pthread_mutex_unlock(&q->lock); 
    return -ENOSPC;                 /* Lỗi: Không còn chỗ trống */
  }
  q->buffer[q->tail] = *msg; 
  q->tail = (q->tail + 1) % QUEUE_MAX_SIZE;
  q->count++;
  pthread_cond_signal(&q->cond);
  pthread_mutex_unlock(&q->lock);
  return 0;
}

int msg_queue_pop(msg_queue_t *q, app_msg_t *msg) {

  if (q == NULL || msg == NULL) {
    return -EINVAL;
  }
  pthread_mutex_lock(&q->lock);

  while (q->count == 0) {
    pthread_cond_wait(&q->cond, &q->lock);
  }

  *msg = q->buffer[q->head];
  q->head = (q->head + 1) % QUEUE_MAX_SIZE;
  q->count--;
  pthread_mutex_unlock(&q->lock);
  return 0;
}

void msg_queue_destroy(msg_queue_t *q) 
{
    if(q == NULL)
    {
        return; 
    }
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->cond);
}
