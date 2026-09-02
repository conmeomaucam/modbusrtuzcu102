/**
 * @file       rpmsg_driver.c
 * @brief      
 * @author     Cong <congvmc1@gmail.com>
 * @date       2026-09-02
 * @version    2.0.0
 * 
 * @copyright  Copyright (c) 2026 Cong. All rights reserved.
 */

#include "rpmsg_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEFAULT_DEV_PATH "/dev/rpmsg0"

void rpmsg_ctx_init(rpmsg_ctx_t *ctx) {
    if (ctx == NULL) {
        return;
    }
    ctx->fd = -1;
    strncpy(ctx->dev_path, DEFAULT_DEV_PATH, sizeof(ctx->dev_path) - 1);
    ctx->dev_path[sizeof(ctx->dev_path) - 1] = '\0';
    ctx->tx_count = 0;
    ctx->rx_count = 0;
    ctx->err_count = 0;
}

int32_t rpmsg_open(rpmsg_ctx_t *ctx, const char *dev_path) {
    if (ctx == NULL) {
        return -EINVAL;
    }

    /* Nếu đã mở kênh trước đó, tiến hành đóng lại sạch sẽ */
    if (ctx->fd >= 0) {
        rpmsg_close(ctx);
    }

    if (dev_path != NULL && strlen(dev_path) > 0) {
        strncpy(ctx->dev_path, dev_path, sizeof(ctx->dev_path) - 1);
        ctx->dev_path[sizeof(ctx->dev_path) - 1] = '\0';
    } else if (strlen(ctx->dev_path) == 0) {
        strncpy(ctx->dev_path, DEFAULT_DEV_PATH, sizeof(ctx->dev_path) - 1);
        ctx->dev_path[sizeof(ctx->dev_path) - 1] = '\0';
    }

    /* Mở file thiết bị Linux với cờ Non-blocking */
    int fd = open(ctx->dev_path, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        ctx->err_count++;
        return -errno;
    }

    ctx->fd = fd;
    return 0;
}

void rpmsg_close(rpmsg_ctx_t *ctx) {
    if (ctx == NULL) {
        return;
    }
    if (ctx->fd >= 0) {
        close(ctx->fd);
        ctx->fd = -1;
    }
}

bool rpmsg_is_open(const rpmsg_ctx_t *ctx) {
    if (ctx == NULL) {
        return false;
    }
    return (ctx->fd >= 0);
}

int32_t rpmsg_poll_ready(const rpmsg_ctx_t *ctx, int32_t timeout_ms) {
    if (!rpmsg_is_open(ctx)) {
        return -EINVAL;
    }

    struct pollfd pfd;
    pfd.fd = ctx->fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int ret = poll(&pfd, 1, timeout_ms);
    if (ret < 0) {
        return -errno;
    } else if (ret == 0) {
        return 0; /* Timeout */
    }

    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
        return -EIO;
    }

    if (pfd.revents & POLLIN) {
        return 1; /* Dữ liệu sẵn sàng đọc */
    }

    return 0;
}

int32_t rpmsg_write_raw(rpmsg_ctx_t *ctx, const void *buf, size_t len) {
    if (!rpmsg_is_open(ctx) || buf == NULL || len == 0) {
        return -EINVAL;
    }

    ssize_t bytes_written = write(ctx->fd, buf, len);
    if (bytes_written < 0) {
        ctx->err_count++;
        return -errno;
    }

    ctx->tx_count++;
    return (int32_t)bytes_written;
}

int32_t rpmsg_read_raw(rpmsg_ctx_t *ctx, void *buf, size_t len, int32_t timeout_ms) {
    if (!rpmsg_is_open(ctx) || buf == NULL || len == 0) {
        return -EINVAL;
    }

    if (timeout_ms != 0) {
        int32_t ready = rpmsg_poll_ready(ctx, timeout_ms);
        if (ready == 0) {
            return 0; /* Timeout: không có dữ liệu */
        } else if (ready < 0) {
            return ready; /* Lỗi poll */
        }
    }

    ssize_t bytes_read = read(ctx->fd, buf, len);
    if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; /* Non-blocking: chưa có dữ liệu sẵn sàng */
        }
        ctx->err_count++;
        return -errno;
    }

    if (bytes_read > 0) {
        ctx->rx_count++;
    }
    return (int32_t)bytes_read;
}

int32_t rpmsg_send_msg(rpmsg_ctx_t *ctx, const app_msg_t *msg) {
    if (msg == NULL) {
        return -EINVAL;
    }

    int32_t ret = rpmsg_write_raw(ctx, msg, sizeof(app_msg_t));
    if (ret == (int32_t)sizeof(app_msg_t)) {
        return 0; /* Thành công */
    } else if (ret < 0) {
        return ret;
    }

    return -EIO; /* Lỗi ghi không đủ gói */
}

int32_t rpmsg_recv_msg(rpmsg_ctx_t *ctx, app_msg_t *msg, int32_t timeout_ms) {
    if (msg == NULL) {
        return -EINVAL;
    }

    int32_t ret = rpmsg_read_raw(ctx, msg, sizeof(app_msg_t), timeout_ms);
    if (ret == (int32_t)sizeof(app_msg_t)) {
        return 0; /* Thành công */
    } else if (ret == 0) {
        return -ETIMEDOUT;
    } else if (ret < 0) {
        return ret;
    }

    return -EIO; /* Gói tin bị cắt vụn hoặc đọc không đủ kích thước */
}

void rpmsg_get_stats(const rpmsg_ctx_t *ctx, uint32_t *tx, uint32_t *rx, uint32_t *err) {
    if (ctx == NULL) {
        return;
    }
    if (tx != NULL) *tx = ctx->tx_count;
    if (rx != NULL) *rx = ctx->rx_count;
    if (err != NULL) *err = ctx->err_count;
}

void rpmsg_reset_stats(rpmsg_ctx_t *ctx) {
    if (ctx == NULL) {
        return;
    }
    ctx->tx_count = 0;
    ctx->rx_count = 0;
    ctx->err_count = 0;
}



