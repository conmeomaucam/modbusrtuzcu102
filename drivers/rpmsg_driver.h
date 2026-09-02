/**
 * @file       rpmsg_driver.h
 * @brief      
 * @author     Cong <congvmc1@gmail.com>
 * @date       2026-08-28
 * @version    2.0.0
 * 
 * @copyright  Copyright (c) 2026 Cong. All rights reserved.
 */

#ifndef rpmsg_driver_H
#define rpmsg_driver_H
/*
┌──────────────────┐          ┌────────────────── ┐          ┌──────────────┐
│  rpmsg_worker    │          │  rpmsg_driver     │          │ /dev/rpmsg0  │
│  (Thread 1)      │          │  (HAL layer)      │          │ (kernel)     │
│                  │          │                   │          │              │
│  Loop:           │          │                   │          │              │
│  ┌─poll_ready()──┼─────────►│ poll(fd, POLLIN)  ├─────────►│              │
│  │               │          │                   │          │              │
│  │ if data ready │          │                   │          │              │
│  │ recv_msg()────┼─────────►│ read(fd, buf)     │◄─────────| R5 → A53     │
│  │  │            │          │ memcpy → app_msg  │          │              │
│  │  ▼            │          │                   │          │              │
│  │ push(rx_queue)│          │                   │          │              │
│  │               │          │                   │          │              │
│  │ if tx_queue   │          │                   │          │              │
│  │ pop(tx_queue) │          │                   │          │              │
│  │  │            │          │                   │          │              │
│  │  ▼            │          │                   │          │              │
│  │ send_msg()────┼─────────►│ write(fd, msg)    ├─────────►│ A53 → R5     │
│  └───────────────┘          └──────────────────┘           └──────────────┘


*/
#include "../core/data_types.h" 
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief RPMsg Channel Context Structure
 */
typedef struct rpmsg_ctx_s {
    int32_t  fd;           /**< File descriptor (-1 nếu chưa mở) */
    char     dev_path[64]; /**< Đường dẫn thiết bị (vd: "/dev/rpmsg0") */
    uint32_t tx_count;     /**< Tổng số tin nhắn đã gửi thành công */
    uint32_t rx_count;     /**< Tổng số tin nhắn đã nhận thành công */
    uint32_t err_count;    /**< Tổng số lỗi I/O ghi nhận */
} rpmsg_ctx_t;

/* ========================================================================= */
/* 1. LIFECYCLE MANAGEMENT (Quản lý vòng đời)                               */
/* ========================================================================= */

/**
 * @brief Khởi tạo biến context về trạng thái mặc định (fd = -1, stats = 0)
 * @param ctx Con trỏ tới rpmsg_ctx_t
 */
void rpmsg_ctx_init(rpmsg_ctx_t *ctx);

/**
 * @brief Mở channel giao tiếp RPMsg (/dev/rpmsgX)
 * @param ctx Con trỏ tới rpmsg_ctx_t
 * @param dev_path Đường dẫn thiết bị (vd: "/dev/rpmsg0"). Nếu NULL dùng "/dev/rpmsg0"
 * @return 0 nếu thành công, âm (-errno) nếu thất bại
 */
int32_t rpmsg_open(rpmsg_ctx_t *ctx, const char *dev_path);

/**
 * @brief Đóng channel giao tiếp và giải phóng file descriptor
 * @param ctx Con trỏ tới rpmsg_ctx_t
 */
void rpmsg_close(rpmsg_ctx_t *ctx);

/**
 * @brief Kiểm tra xem channel RPMsg có đang sẵn sàng/mở hay không
 * @param ctx Con trỏ tới rpmsg_ctx_t
 * @return true nếu đang mở, false nếu chưa mở hoặc lỗi
 */
bool rpmsg_is_open(const rpmsg_ctx_t *ctx);

/* ========================================================================= */
/* 2. I/O OPERATIONS (Đọc / Ghi dữ liệu)                                    */
/* ========================================================================= */

/**
 * @brief Gửi 1 tin nhắn định dạng app_msg_t xuống R5 qua RPMsg
 * @param ctx Con trỏ tới rpmsg_ctx_t
 * @param msg Con trỏ tới tin nhắn app_msg_t cần gửi
 * @return 0 nếu thành công, âm (-errno) nếu thất bại
 */
int32_t rpmsg_send_msg(rpmsg_ctx_t *ctx, const app_msg_t *msg);

/**
 * @brief Nhận 1 tin nhắn định dạng app_msg_t từ R5 (chờ có timeout)
 * @param ctx Con trỏ tới rpmsg_ctx_t
 * @param msg Con trỏ chứa tin nhắn nhận được
 * @param timeout_ms Thời gian chờ tối đa (ms). -1: Chờ vô hạn, 0: Non-blocking
 * @return 0 nếu thành công, -ETIMEDOUT nếu hết thời gian chờ, âm (-errno) nếu lỗi khác
 */
int32_t rpmsg_recv_msg(rpmsg_ctx_t *ctx, app_msg_t *msg, int32_t timeout_ms);

/**
 * @brief Ghi dữ liệu thô (raw bytes) xuống kênh RPMsg
 * @param ctx Con trỏ tới rpmsg_ctx_t
 * @param buf Bộ đệm dữ liệu cần gửi
 * @param len Độ dài dữ liệu (bytes)
 * @return Số byte đã ghi (>0) hoặc âm (-errno) nếu thất bại
 */
int32_t rpmsg_write_raw(rpmsg_ctx_t *ctx, const void *buf, size_t len);

/**
 * @brief Đọc dữ liệu thô (raw bytes) từ kênh RPMsg với timeout
 * @param ctx Con trỏ tới rpmsg_ctx_t
 * @param buf Bộ đệm chứa dữ liệu đọc ra
 * @param len Kích thước tối đa của bộ đệm (bytes)
 * @param timeout_ms Thời gian chờ (ms)
 * @return Số byte đọc được (>0), 0 nếu timeout, hoặc âm (-errno) nếu lỗi
 */
int32_t rpmsg_read_raw(rpmsg_ctx_t *ctx, void *buf, size_t len, int32_t timeout_ms);

/* ========================================================================= */
/* 3. DIAGNOSTICS & HEALTH (Chẩn đoán & Giám sát)                           */
/* ========================================================================= */

/**
 * @brief Lấy thông số thống kê I/O (tx_count, rx_count, err_count)
 * @param ctx Con trỏ tới rpmsg_ctx_t
 * @param tx Con trỏ chứa số gói tin đã gửi (có thể NULL)
 * @param rx Con trỏ chứa số gói tin đã nhận (có thể NULL)
 * @param err Con trỏ chứa số lỗi ghi nhận (có thể NULL)
 */
void rpmsg_get_stats(const rpmsg_ctx_t *ctx, uint32_t *tx, uint32_t *rx, uint32_t *err);

/**
 * @brief Reset các bộ đếm thống kê về 0
 * @param ctx Con trỏ tới rpmsg_ctx_t
 */
void rpmsg_reset_stats(rpmsg_ctx_t *ctx);

/* ========================================================================= */
/* 4. UTILITY HELPERS                                                        */
/* ========================================================================= */

/**
 * @brief Kiểm tra xem trên kênh RPMsg có dữ liệu sẵn sàng để đọc không (không thực hiện read)
 * @param ctx Con trỏ tới rpmsg_ctx_t
 * @param timeout_ms Thời gian chờ (ms)
 * @return 1 nếu có dữ liệu chờ đọc, 0 nếu hết giờ, âm (-errno) nếu lỗi
 */
int32_t rpmsg_poll_ready(const rpmsg_ctx_t *ctx, int32_t timeout_ms);

#endif /* rpmsg_driver_H */