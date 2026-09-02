/**
 * @file       GUI.h
 * @brief      
 * @author     Cong <congvmc1@gmail.com>
 * @date       2026-09-02
 * @version    2.0.0
 * 
 * @copyright  Copyright (c) 2026 Cong. All rights reserved.
 */

#ifndef GUI_H
#define GUI_H

#include "../core/data_types.h"
#include "../core/msq_queue.h"
#include <stdbool.h>

typedef struct gui_thread_args_s {
  msg_queue_t *rx_queue; /* Hòm thư chứa dữ liệu sensor lấy ra vẽ Dashboard */
  msg_queue_t *tx_queue; /* Hòm thư để ném lệnh bàn phím xuống R5 */
  volatile bool *is_running; /* Con trỏ cờ báo hiệu chạy/dừng luồng */
} gui_thread_args_t;

/**
 * @brief  Hàm chạy chính của POSIX Thread quản lý Giao diện Dashboard & Bàn
 * phím
 * @param  arg Con trỏ trỏ tới gui_thread_args_t
 * @return NULL khi luồng kết thúc
 */
void *gui_worker_func(void *arg);

#endif /* GUI_H */