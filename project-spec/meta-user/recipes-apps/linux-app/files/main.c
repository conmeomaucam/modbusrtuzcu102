/**
 * @file       main.c
 * @brief      Entry point cho ứng dụng POSIX C đa luồng trên Cortex-A53
 * @author     Cong <congvmc1@gmail.com>
 * @date       2026-09-02
 * @version    2.0.0
 * 
 * @copyright  Copyright (c) 2026 Cong. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#include "core/data_types.h"
#include "core/msq_queue.h"
#include "drivers/rpmsg_driver.h"
#include "Threads/rpmsg.h"
#include "Threads/GUI.h"

/* Cờ báo hiệu trạng thái hoạt động toàn hệ thống */
static volatile bool g_is_running = true;

/**
 * @brief Hàm bắt và xử lý tín hiệu ngắt từ Linux OS (Ctrl+C, systemctl stop)
 * @param sig Mã tín hiệu (SIGINT, SIGTERM)
 */
static void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\n[MAIN] Signal %d received. Initiating graceful shutdown sequence...\n", sig);
        g_is_running = false;
    }
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    /* ========================================================================= */
    /* STEP 1: ĐĂNG KÝ BẮT TÍN HIỆU HỆ THỐNG                                    */
    /* ========================================================================= */
    if (signal(SIGINT, signal_handler) == SIG_ERR || signal(SIGTERM, signal_handler) == SIG_ERR) {
        fprintf(stderr, "[MAIN_ERR] Failed to register signal handlers!\n");
        return EXIT_FAILURE;
    }

    printf("==============================================================\n");
    printf("     LINUX A53 EMBEDDED SYSTEM APPLICATION - C99 POSIX       \n");
    printf("==============================================================\n");

    /* ========================================================================= */
    /* STEP 2: KHỞI TẠO CÁC HÀNG ĐỢI NỘI BỘ (THREAD-SAFE QUEUES)                */
    /* ========================================================================= */
    msg_queue_t rx_queue;
    msg_queue_t tx_queue;

    if (msg_queue_init(&rx_queue) != 0) {
        fprintf(stderr, "[MAIN_ERR] Failed to initialize rx_queue!\n");
        return EXIT_FAILURE;
    }

    if (msg_queue_init(&tx_queue) != 0) {
        fprintf(stderr, "[MAIN_ERR] Failed to initialize tx_queue!\n");
        msg_queue_destroy(&rx_queue);
        return EXIT_FAILURE;
    }

    printf("[MAIN] Thread-safe message queues initialized successfully.\n");

    /* ========================================================================= */
    /* STEP 3: CHUẨN BỊ THAM SỐ CHO CÁC WORKER THREADS                           */
    /* ========================================================================= */
    rpmsg_thread_args_t rpmsg_args = {
        .rx_queue = &rx_queue,
        .tx_queue = &tx_queue,
        .is_running = &g_is_running,
        .dev_path = "/dev/rpmsg0"
    };

    gui_thread_args_t gui_args = {
        .rx_queue = &rx_queue,
        .tx_queue = &tx_queue,
        .is_running = &g_is_running
    };

    /* ========================================================================= */
    /* STEP 4: KHỞI TẠO VÀ BẬT CÁC POSIX WORKER THREADS                         */
    /* ========================================================================= */
    pthread_t t_rpmsg;
    pthread_t t_gui;

    printf("[MAIN] Spawning RPMSG Worker Thread...\n");
    if (pthread_create(&t_rpmsg, NULL, rpmsg_worker_func, &rpmsg_args) != 0) {
        fprintf(stderr, "[MAIN_ERR] Failed to create RPMSG worker thread!\n");
        g_is_running = false;
        msg_queue_destroy(&rx_queue);
        msg_queue_destroy(&tx_queue);
        return EXIT_FAILURE;
    }

    printf("[MAIN] Spawning GUI Dashboard Worker Thread...\n");
    if (pthread_create(&t_gui, NULL, gui_worker_func, &gui_args) != 0) {
        fprintf(stderr, "[MAIN_ERR] Failed to create GUI worker thread!\n");
        g_is_running = false;
        pthread_join(t_rpmsg, NULL);
        msg_queue_destroy(&rx_queue);
        msg_queue_destroy(&tx_queue);
        return EXIT_FAILURE;
    }

    printf("[MAIN] System up and running. Press Ctrl+C to terminate gracefully.\n");

    /* ========================================================================= */
    /* STEP 5: VÒNG LẶP CHỜ CÁC LUỒNG THOÁT KHỎI HỆ THỐNG                       */
    /* ========================================================================= */
    pthread_join(t_rpmsg, NULL);
    pthread_join(t_gui, NULL);

    /* ========================================================================= */
    /* STEP 6: DỌN DẸP TÀI NGUYÊN BỘ NHỚ TRƯỚC KHÍ THOÁT                        */
    /* ========================================================================= */
    printf("[MAIN] Cleaning up queues and mutexes...\n");
    msg_queue_destroy(&rx_queue);
    msg_queue_destroy(&tx_queue);

    printf("==============================================================\n");
    printf("     APPLICATION TERMINATED CLEANLY. GOODBYE!                 \n");
    printf("==============================================================\n");

    return EXIT_SUCCESS;
}
