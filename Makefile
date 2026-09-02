# ==============================================================================
# @file       Makefile
# @brief      Build System cho ứng dụng C POSIX linux_app trên Cortex-A53
# @author     Cong <congvmc1@gmail.com>
# @date       2026-09-02
# ==============================================================================

# Hỗ trợ Cross-Compilation: gõ `make CROSS_COMPILE=aarch64-linux-gnu-` khi build cho ARM
ifneq ($(CROSS_COMPILE),)
    CC := $(CROSS_COMPILE)gcc
else
    CC ?= gcc
endif


# Thư mục chứa tất cả các file rác .o và binary build
BUILD_DIR     := build

# Cờ biên dịch C chuẩn C99, bật tất cả cảnh báo, hỗ trợ POSIX Threads và tối ưu O2
CFLAGS        += -Wall -Wextra -std=c99 -pthread -O2 \
                 -Icore -Idrivers -IThreads

# Cờ liên kết thư viện
LDFLAGS       += -pthread


# Tên file thực thi đầu ra (nằm trong thư mục build/)
TARGET        := $(BUILD_DIR)/linux_app

# Danh sách tất cả các file nguồn C
SRCS          := main.c \
                 core/msq_queue.c \
                 drivers/rpmsg_driver.c \
                 Threads/rpmsg.c \
                 Threads/GUI.c

# Chuyển đổi đường dẫn file .c thành file .o tương ứng nằm trong thư mục build/
OBJS          := $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))

# Target mặc định khi gõ `make`
all: $(TARGET)

# Quy tắc liên kết các file .o trong build/ thành file thực thi TARGET
$(TARGET): $(OBJS)
	@echo "[LINK] Linking binary: $@"
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	@echo "[SUCCESS] Build finished successfully: $@"

# Quy tắc biên dịch từng file .c thành file .o và tự động tạo thư mục con trong build/
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "[CC] Compiling $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

# Quy tắc dọn dẹp toàn bộ thư mục build/
clean:
	@echo "[CLEAN] Removing build directory..."
	rm -rf $(BUILD_DIR)

# Quy tắc chạy ứng dụng trực tiếp trên PC
run: all
	@echo "[RUN] Executing $(TARGET)..."
	./$(TARGET)

.PHONY: all clean run
