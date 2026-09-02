/**
 * @file       GUI.c
 * @brief      
 * @author     Cong <congvmc1@gmail.com>
 * @date       2026-09-02
 * @version    2.0.0
 * 
 * @copyright  Copyright (c) 2026 Cong. All rights reserved.
 */
#include "GUI.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void *gui_worker_func(void *arg) {
  if (arg == NULL) {
    return NULL;
  }

  gui_thread_args_t *args = (gui_thread_args_t *)arg;
  printf("[GUI THREAD] Dashboard & Operator Interface Worker started.\n");

  while (*args->is_running) {
    app_msg_t rx_msg;

    /* Lấy dữ liệu từ Hòm thư rx_queue (msg_queue_pop sẽ tự động ngủ chờ khi rỗng) */
    if (msg_queue_pop(args->rx_queue, &rx_msg) == 0) {
      if (rx_msg.type == MSG_TYPE_SENSOR_DATA) {
        printf("\n======================================================\n");
        printf("              REAL-TIME DASHBOARD LOG                 \n");
        printf("======================================================\n");
        printf("  CPU Usage   : %u%%\n", rx_msg.body.sensor.cpu_usage);
        printf("  Stack Usage : %u bytes\n", rx_msg.body.sensor.stack_usage);
        printf("  Packet Count: %u\n", rx_msg.body.sensor.pkt_cnt);
        printf("  Error Count : %u\n", rx_msg.body.sensor.err_cnt);
        printf("  Timestamp   : %lu ms\n", (unsigned long)rx_msg.body.sensor.timestamp);
        printf("======================================================\n");
      } else if (rx_msg.type == MSG_TYPE_HEARTBEAT) {
        printf("[GUI THREAD] Heartbeat received from R5.\n");
      }
    }
  }

  printf("[GUI THREAD] Stopping GUI worker...\n");
  return NULL;
}
