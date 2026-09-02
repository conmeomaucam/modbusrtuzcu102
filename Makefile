# ==============================================================================
# @file       Makefile
# @brief      Build System cho ứng dụng C POSIX linux_app trên Cortex-A53
# @author     Cong <congvmc1@gmail.com>
# @date       2026-09-02
# ==============================================================================

# Hỗ trợ Cross-Compilation: gõ `make CROSS_COMPILE=aarch64-linux-gnu-` khi build cho ARM
CROSS_COMPILE ?=
CC            := $(CROSS_COMPILE)gcc

# Cờ biên dịch C chuẩn C99, bật tất cả cảnh báo, hỗ trợ POSIX Threads và tối ưu O2
CFLAGS        := -Wall -Wextra -std=c99 -pthread -O2 \
                 -Icore -Idrivers -IThreads

# Cờ liên kết thư viện
LDFLAGS       := -pthread

# Tên file thực thi đầu ra
TARGET        := linux_app

# Danh sách tất cả các file nguồn C
SRCS          := main.c \
                 core/msq_queue.c \
                 drivers/rpmsg_driver.c \
                 Threads/rpmsg.c \
                 Threads/GUI.c

# Chuyển đổi tên file .c thành file .o tương ứng trong thư mục build/
OBJS          := $(SRCS:.c=.o)

# Target mặc định khi gõ `make`
all: $(TARGET)

# Quy tắc liên kết các file .o thành file thực thi TARGET
$(TARGET): $(OBJS)
	@echo "[LINK] Linking binary: $@"
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	@echo "[SUCCESS] Build finished successfully: $@"

# Quy tắc biên dịch từng file .c thành file .o
%.o: %.c
	@echo "[CC] Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Quy tắc dọn dẹp file rác
clean:
	@echo "[CLEAN] Removing object files and binary..."
	rm -f $(OBJS) $(TARGET)

# Quy tắc chạy ứng dụng trực tiếp trên PC
run: all
	@echo "[RUN] Executing $(TARGET)..."
	./$(TARGET)

.PHONY: all clean run
