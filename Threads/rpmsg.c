/**
 * @file       rpmsg.c
 * @brief      
 * @author     Cong <congvmc1@gmail.com>
 * @date       2026-09-02
 * @version    2.0.0
 * 
 * @copyright  Copyright (c) 2026 Cong. All rights reserved.
 */

#include "rpmsg.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *rpmsg_worker_func(void *arg) {
  if (arg == NULL) {
    return NULL;
  }

  /* Giai đoạn 1: Ép kiểu và Khởi tạo Driver */
  rpmsg_thread_args_t *args = (rpmsg_thread_args_t *)arg;
  rpmsg_ctx_t dev_ctx;

  rpmsg_ctx_init(&dev_ctx);
  if (rpmsg_open(&dev_ctx, args->dev_path) != 0) {
    fprintf(stderr, "[RPMSG THREAD] Failed to open device: %s\n",
            args->dev_path);
    return NULL;
  }

  printf("[RPMSG THREAD] Worker started successfully on %s\n",
         dev_ctx.dev_path);

  /* Giai đoạn 2: Vòng lặp chính */
  while (*args->is_running) {
    app_msg_t rx_msg;

    /* A. Kiểm tra dữ liệu từ R5 gửi lên */
    int32_t ready = rpmsg_poll_ready(&dev_ctx, 50); /* Chờ tối đa 50ms */
    if (ready > 0) {
      if (rpmsg_recv_msg(&dev_ctx, &rx_msg, 0) == 0) {
        /* Đẩy gói tin nhận được vào hòm thư rx_queue */
        msg_queue_push(args->rx_queue, &rx_msg);
      }
    }

    /* B. Kiểm tra xem tx_queue có lệnh nào cần gửi xuống R5 không */
    /* (Chúng ta sẽ thêm logic rút lệnh từ tx_queue gửi xuống R5 ở đây) */
  }

  /* Giai đoạn 3: Dọn dẹp dẹp cửa hàng khi thoát */
  printf("[RPMSG THREAD] Stopping worker...\n");
  rpmsg_close(&dev_ctx);
  return NULL;
}
