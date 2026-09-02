/**
 * @file       msq_queue.h
 * @brief      Khai báo Hàng đợi tin nhắn an toàn đa luồng (Thread-safe Message Queue)
 * @author     Cong <congvmc1@gmail.com>
 * @date       2026-08-27
 * @version    2.0.0
 * 
 * @copyright  Copyright (c) 2026 Cong. All rights reserved.
 */

#ifndef MSQ_QUEUE_H
#define MSQ_QUEUE_H

#include <pthread.h>
#include <stdbool.h>
#include "data_types.h"

/**
 * @brief Cấu trúc quản lý Hàng đợi tin nhắn (Vòng đệm Ring Buffer)
 */
typedef struct msg_queue_s {
    app_msg_t       buffer[QUEUE_MAX_SIZE]; /* Mảng chứa tối đa 100 tin nhắn */
    int             head;                    /* Vị trí lấy tin nhắn ra (Đầu hàng) */
    int             tail;                    /* Vị trí cất tin nhắn vào (Cuối hàng) */
    int             count;                   /* Số lượng tin nhắn hiện có trong hàng */
    pthread_mutex_t lock;                    /* Ô khóa an toàn - Tránh 2 Thread tranh giành nhau */
    pthread_cond_t  cond;                    /* Chuông báo hiệu - Báo cho Thread biết có tin nhắn mới */
} msg_queue_t;

/**
 * @brief Khởi tạo Hàng đợi và các khóa an toàn
 * @param q Con trỏ trỏ tới Hàng đợi cần khởi tạo
 * @return 0 nếu thành công, âm nếu thất bại
 */
int msg_queue_init(msg_queue_t *q);

/**
 * @brief Đẩy 1 tin nhắn mới vào Hàng đợi (Dành cho Thread gửi dữ liệu)
 * @param q Con trỏ trỏ tới Hàng đợi
 * @param msg Tin nhắn cần cất vào
 * @return 0 nếu thành công, -1 nếu Hàng đợi đã đầy
 */

/*
 Thread 1 (Producer)                Hàng Đợi (Ring Buffer)               Thread
2 (Consumer) ────────────────────               ───────────────────────
──────────────────── Có tin nhắn mới! │
  1. Lock Mutex  ─────────► [ Bị Khóa Độc Quyền ]
         │
  2. Bỏ tin vào tail ─────► [ Msg 0 | Empty | Empty | ... ]
         │                   tail tăng 0 -> 1, count = 1
         │
  3. Bấm Chuông  ─────────► [ Đánh Chuông: KENG! ] ─────────► [ Đang Ngủ ] -> [
TỈNH DẬY! ] │
  4. Unlock Mutex ────────► [ Mở Khóa ]

*/
int msg_queue_push(msg_queue_t *q, const app_msg_t *msg);

/**
 * @brief Lấy 1 tin nhắn ra khỏi Hàng đợi (Dành cho Thread nhận dữ liệu)
 * @note Nếu Hàng đợi đang rỗng, hàm này sẽ TỰ ĐỘNG CHỜ cho tới khi có tin nhắn
 * mới
 * @param q Con trỏ trỏ tới Hàng đợi
 * @param msg Nơi chứa tin nhắn lấy ra
 * @return 0 nếu thành công, âm nếu thất bại
 */

/*THƯỜNG HỢP A : Hàng Đợi Đang Rỗng(count == 0)
 ───────────────────────────────────────────── Thread
               2(Consumer)Hàng Đợi Thread 1(Producer)
 ────────────────────                 ──────────                          ──────────────────── Muốn
               lấy tin nhắn !
         │ 1. Lock Mutex ──────────► [Bị Khóa]
         │ 2. Thấy count
               == 0 ─────► [Hàng Đợi Rỗng]
         │ 3. Gọi cond_wait ───────► [Nhả Mutex & ĐI NGỦ]
                                  │
                                  │ (Tỉnh dậy khi nhận KENG từ Producer)
                                  ▼ 4. Lấy tin từ head ─────► [Msg 0](
                      Giảm count = 0, head tăng 0->1)
         │ 5. Unlock Mutex ────────► [Mở Khóa]
*/
int msg_queue_pop(msg_queue_t *q, app_msg_t *msg);

/**
 * @brief Hủy Hàng đợi và giải phóng các ổ khóa
 * @param q Con trỏ trỏ tới Hàng đợi
 */
void msg_queue_destroy(msg_queue_t *q);


#endif /* MSQ_QUEUE_H */