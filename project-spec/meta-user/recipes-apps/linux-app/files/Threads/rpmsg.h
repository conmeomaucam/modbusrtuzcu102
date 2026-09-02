/**
 * @file       rpmsg.h
 * @brief      
 * @author     Cong <congvmc1@gmail.com>
 * @date       2026-09-02
 * @version    2.0.0
 * 
 * @copyright  Copyright (c) 2026 Cong. All rights reserved.
 */

#ifndef rpmsg_H
#define rpmsg_H

#include "../core/data_types.h"
#include "../core/msq_queue.h"
#include "../drivers/rpmsg_driver.h"

#include "stdbool.h"

typedef struct rpmsg_thread_args_s {
  msg_queue_t *rx_queue;
  msg_queue_t *tx_queue;
  volatile bool *is_running;
  char dev_path[64];
} rpmsg_thread_args_t;

/**
 * @brief  Hàm chạy chính của POSIX Thread giao tiếp RPMSG (A53 <-> R5)
 * @param  arg Con trỏ trỏ tới cấu trúc tham số rpmsg_thread_args_t
 * @return NULL khi luồng kết thúc
 */
void *rpmsg_worker_func(void *arg);

#endif /* rpmsg_H */