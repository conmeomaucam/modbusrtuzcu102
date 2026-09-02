# KIẾN TRÚC PHẦN MỀM LINUX (THUẦN C / POSIX)

Tài liệu này mô tả chi tiết kiến trúc phần mềm, phân luồng (POSIX Threads), cơ chế giao tiếp nội bộ (IPC Queue) và cấu trúc cây thư mục dành riêng cho ứng dụng Linux viết bằng **THUẦN NGÔN NGỮ C (C99/C11)** chạy trên Cortex-A53 (PetaLinux).

---

## 1. SƠ ĐỒ KIẾN TRÚC THUẦN C (A53)

```
╔════════════════════════════════════════════════════════════════════════════════════════════════════════╗
║                                   LINUX PTHREAD APP ARCHITECTURE (C99)                                 ║
║                                                                                                        ║
║   ┌─────────────────────────────────────────────────────────────────────────────────────────────────┐  ║
║   │                                    CORE / SHARED DATA LAYER                                     │  ║
║   │                                                                                                 │  ║
║   │   ┌───────────────────────────┐                       ┌───────────────────────────┐             │  ║
║   │   │         tx_queue          │                       │         rx_queue          │             │  ║
║   │   │  (Hàng đợi Lệnh gửi R5)   │                       │ (Hàng đợi Dữ liệu từ R5)  │             │  ║
║   │   │  [pthread_mutex/cond_t]   │                       │  [pthread_mutex/cond_t]   │             │  ║
║   │   └─────────────▲─────────────┘                       └─────────────┬─────────────┘             │  ║
║   └─────────┬───────┴───────────────────────────────────────────────────┼───────────────────────────┘  ║
║             │ Pop Cmd                                                   │ Push Sensor Data             ║
║             │                                                           │                              ║
║   ┌─────────▼───────────────────────────────────────────────────────────┴─────────────────────────┐  ║
║   │                                   POSIX WORKER THREADS (C)                                      │  ║
║   │                                                                                                 │  ║
║   │  ┌─────────────────────────┐   ┌─────────────────────────┐   ┌───────────────────────────────┐  ║
║   │  │ THREAD 1: rpmsg_worker  │   │ THREAD 2: mqtt_worker   │   │ THREAD 3: app_main / gui_task │  ║
║   │  │ (pthread_create)        │   │ (pthread_create)        │   │ (pthread_create)              │  ║
║   │  │                         │   │                         │   │                               │  ║
║   │  │  - open("/dev/rpmsg0")  │   │  - Connect MQTT Broker  │   │  - Display / Process Dashboard│  ║
║   │  │  - read/write POSIX I/O │   │  - Publish "sensors/t"  │   │  - Log Sensor Data            │  ║
║   │  │  - non-blocking poll()  │   │  - Subscribe "ctrl/cmd" │   │  - User Input --> Push tx_q   │  ║
║   │  └──────────┬──────────────┘   └────────────┬────────────┘   └──────────────┬────────────────┘  ║
║   └─────────────┼───────────────────────────────┼───────────────────────────────┼──────────────────┘  ║
║                 │ read() / write()              │ Publish / Subscribe           │ Socket / WebGL Stream│
║                 ▼                               ▼                               ▼                     ║
║          ┌──────────────┐             ┌───────────────────┐           ┌───────────────────┐           ║
║          │ /dev/rpmsg0  │             │ MQTT Broker (LAN) │           │ Ethernet Stream   │           ║
║          │  (OpenAMP)   │             │  (Port 1883/8883) │           │  (Port 8080 Web)  │           ║
║          └──────────────┘             └───────────────────┘           └───────────────────┘           ║
╚════════════════════════════════════════════════════════════════════════════════════════════════════════╝
```

---

## 2. CHỨC NĂNG CỦA 3 POSIX THREAD (LẬP TRÌNH C)

### 2.1 Thread 1: `rpmsg_worker` (POSIX Thread 1)
* **File:** `threads/rpmsg_worker.c`
* **Nhiệm vụ:** Chuyên trách giao tiếp phần cứng qua Linux System Call `open()`, `read()`, `write()`, `poll()`.
* **Luồng nhận (Rx):** Đọc dữ liệu từ R5 (dữ liệu sensor từ STM32) -> Đóng gói thành `sensor_data_t` -> Ném vào `rx_queue`.
* **Luồng gửi (Tx):** Lắng nghe `tx_queue`. Khi có lệnh từ App/MQTT -> Đọc lệnh -> `write()` xuống `/dev/rpmsg0` gửi cho R5.

### 2.2 Thread 2: `mqtt_worker` (POSIX Thread 2)
* **File:** `threads/mqtt_worker.c`
* **Nhiệm vụ:** Dùng thư viện C Paho MQTT (`paho-mqtt3c`) để kết nối Broker.
* **Publish:** Lấy dữ liệu từ `rx_queue` -> Đóng gói chuỗi C-string JSON -> Publish lên topic `sensors/data`.
* **Subscribe:** Lắng nghe topic `control/cmd` từ Cloud -> Parse chuỗi C-string -> Ném vào `tx_queue`.

### 2.3 Thread 3: `app_main` / `gui_worker` (POSIX Thread 3)
* **File:** `threads/gui_worker.c` (hoặc `main.c`)
* **Nhiệm vụ:** Nhận dữ liệu từ `rx_queue` để xử lý logic chính, in log hiển thị hoặc xuất giao diện Web/Socket qua Ethernet.
* **Tác động người dùng:** Khi có sự kiện điều khiển -> Ném struct `control_cmd_t` vào `tx_queue`.

---

## 3. CẤU TRÚC CÂY THƯ MỤC THUẦN C (`linux_app/`)

Cấu trúc thư mục 100% bằng **File C (`.c` và `.h`)**:

```
linux_app/
├── Makefile                        # Makefile biên dịch bằng gcc/clang (-pthread -std=c99)
│
└── src/
    ├── main.c                      # Entry point: Khởi tạo POSIX Mutex, pthread_create()
    │
    ├── core/                       # 🧠 CỐT LÕI (C Structs & POSIX Queues)
    │   ├── data_types.h            # C Str `3uct định nghĩa gói tin (sensor_data_t, control_cmd_t)
    │   ├── msg_queue.h             # Interface hàng đợi dùng pthread_mutex_t & pthread_cond_t
    │   └── msg_queue.c             # Implementation hàng đợi bằng mảng C (Circular Buffer)
    │
    ├── drivers/                    # 🔌 HẠ TẦNG (Linux File System Calls)
    │   ├── rpmsg_driver.h          # Header wrapper các hàm C: open(), read(), write() /dev/rpmsg0
    │   └── rpmsg_driver.c          # Code C gọi Linux System Calls
    │
    └── threads/                    # ⚙️ LUỒNG XỬ LÝ (POSIX Threads)
        ├── rpmsg_worker.h          # Header cho Thread 1 (rpmsg_worker_func)
        ├── rpmsg_worker.c          # Code C cho POSIX Thread 1
        ├── mqtt_worker.h           # Header cho Thread 2 (mqtt_worker_func)
        ├── mqtt_worker.c           # Code C dùng thư viện paho-mqtt3c
        ├── gui_worker.h            # Header cho Thread 3 (gui_worker_func)
        └── gui_worker.c            # Code C xử lý giao diện / Socket / WebGL
```

---

## 4. BẢNG TÓM TẮT TRÁCH NHIỆM FILE (THUẦN C)

| Thư mục | File | Trách nhiệm chính trong C |
|---|---|---|
| `src/core/` | `data_types.h` | Typedef các `struct` dữ liệu trong C (`sensor_data_t`, `control_cmd_t`). |
| `src/core/` | `msg_queue.c` | Hàng đợi vòng (Ring Buffer) trong C, đồng bộ bằng `pthread_mutex_lock()` & `pthread_cond_wait()`. |
| `src/drivers/` | `rpmsg_driver.c` | Gọi trực tiếp C System Calls: `open("/dev/rpmsg0", O_RDWR)`, `read()`, `write()`. |
| `src/threads/` | `rpmsg_worker.c` | Hàm `void* rpmsg_worker_func(void* arg)` chạy vòng lặp POSIX thread. |
| `src/threads/` | `mqtt_worker.c` | Hàm `void* mqtt_worker_func(void* arg)` dùng C SDK `MQTTClient.h`. |
| `src/threads/` | `gui_worker.c` | Hàm `void* gui_worker_func(void* arg)` xử lý logic hiển thị/Socket C. |

---

## 5. BƯỚC THỰC HÀNH TIẾP THEO

1. **Tạo thư mục `linux_app/`** chuẩn ngôn ngữ C.
2. **Tạo file `src/core/data_types.h`** bằng các `typedef struct` chuẩn C99.
3. **Tạo file `src/core/msg_queue.h` và `msg_queue.c`** sử dụng `pthread_mutex_t` và `pthread_cond_t`.
