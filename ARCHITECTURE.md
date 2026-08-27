# KIẾN TRÚC HỆ THỐNG: OpenAMP + Modbus RTU + Qt GUI + MQTT

## 1. SƠ ĐỒ TỔNG THỂ

```
┌─────────────────────────────────────────────────────────────────┐
│                    HOST PC (Ubuntu)                             │
│                                                                 │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │                 QEMU (Zynq MPSoC)                         │  │
│  │                                                           │  │
│  │  ┌─────────────────────────────────────────────────────┐  │  │
│  │  │ CORTEX-A53  ──  PetaLinux                           │  │  │
│  │  │                                                     │  │  │
│  │  │  [Qt GUI App]    [MQTT Client]    [/dev/rpmsg0]     │  │  │
│  │  │       │               │                │            │  │  │
│  │  │       └───────────────┴────────────────┘            │  │  │
│  │  │                       │ (read/write userspace)      │  │  │
│  │  │              Linux RPMsg Driver                     │  │  │
│  │  └───────────────────────┬─────────────────────────────┘  │  │
│  │                          │                                │  │
│  │           ════════ OpenAMP  (Shared Memory) ════════      │  │
│  │                          │                                │  │
│  │  ┌───────────────────────┴─────────────────────────────┐  │  │
│  │  │ CORTEX-R5  ──  FreeRTOS (Baremetal)                 │  │  │
│  │  │                                                     │  │  │
│  │  │  [RPMsg Endpoint] ──► [Modbus RTU Master] ──► UART  │  │  │
│  │  └───────────────────────┬─────────────────────────────┘  │  │
│  │                          │ QEMU UART → /dev/pts/X         │  │
│  └──────────────────────────┼────────────────────────────────┘  │
│                             │                                   │
│                    USB-to-RS485 (/dev/ttyUSB0)                  │
└─────────────────────────────┼───────────────────────────────────┘
                              │ RS485 (A+ / B- / GND)
┌─────────────────────────────┴───────────────────────────────────┐
│                    STM32  (Phần cứng thật)                      │
│                                                                 │
│  [UART+RS485] ──► [Modbus RTU Slave] ──► [Sensor/Relay/PWM]    │
└─────────────────────────────────────────────────────────────────┘
```

---

## 2. CÁC THÀNH PHẦN & VỊ TRÍ VIẾT CODE MỚI

### 2.1 Firmware R5 (Modbus RTU Master + OpenAMP RPMsg)

**Vai trò:** Chạy trên Cortex-R5 (FreeRTOS), nhận lệnh từ A53 qua RPMsg,
gửi frame Modbus RTU xuống STM32, trả kết quả ngược lại A53.

**Nơi viết code:**
```
project-spec/meta-user/recipes-apps/r5-modbus-master/
├── r5-modbus-master.bb          # Yocto recipe (nếu build qua PetaLinux)
└── files/
    ├── main.c                   # Entry point: khởi tạo OpenAMP + Modbus
    ├── rpmsg_task.c             # Task nhận/gửi RPMsg từ A53
    ├── modbus_rtu_master.c      # Modbus RTU Master: build frame, CRC, timeout
    ├── modbus_rtu_master.h
    ├── uart_driver.c            # UART Low-Level driver cho R5 (Zynq PS UART)
    ├── uart_driver.h
    └── Makefile / CMakeLists.txt
```

**Hoặc viết tách riêng ngoài PetaLinux (khuyến nghị khi dev nhanh):**
```
~/r5_firmware/
├── src/
│   ├── main.c
│   ├── rpmsg_task.c
│   ├── modbus_rtu_master.c
│   ├── uart_driver.c
│   └── platform_info.c         # OpenAMP platform config (shared mem addr, IPI)
├── include/
│   ├── modbus_rtu_master.h
│   └── uart_driver.h
├── lscript.ld                   # Linker script cho R5 memory map
└── CMakeLists.txt
```

---

### 2.2 Linux App: Qt GUI + MQTT Client (chạy trên A53)

**Vai trò:** Đọc /dev/rpmsg0 để lấy data sensor từ R5, hiển thị Dashboard,
publish lên MQTT Broker, nhận lệnh điều khiển từ Cloud.

**Nơi viết code (tạo recipe mới trong PetaLinux):**
```
project-spec/meta-user/recipes-apps/modbus-dashboard/
├── modbus-dashboard.bb          # Yocto recipe
└── files/
    ├── main.cpp
    ├── rpmsg_interface.cpp      # Đọc/ghi /dev/rpmsg0
    ├── rpmsg_interface.h
    ├── mqtt_client.cpp          # Paho MQTT: pub sensor data, sub commands
    ├── mqtt_client.h
    ├── mainwindow.cpp           # Qt GUI: Dashboard, Chart, Controls
    ├── mainwindow.h
    ├── mainwindow.ui            # Qt Designer UI file
    └── CMakeLists.txt
```

**Hoặc dev nhanh ngoài PetaLinux:**
```
~/qt_dashboard/
├── src/
│   ├── main.cpp
│   ├── rpmsg_interface.cpp
│   ├── mqtt_client.cpp
│   └── mainwindow.cpp
├── include/
├── ui/
│   └── mainwindow.ui
├── CMakeLists.txt
└── deploy.sh                   # Script copy binary vào rootfs QEMU
```

---

### 2.3 STM32 Firmware (Modbus RTU Slave)

**Vai trò:** Nhận Modbus frame từ R5 Master qua RS485, đọc sensor / điều khiển actuator,
trả response frame.

**Nơi viết code (project STM32CubeIDE riêng, KHÔNG nằm trong PetaLinux):**
```
~/stm32_modbus_slave/
├── Core/
│   ├── Src/
│   │   ├── main.c
│   │   ├── modbus_rtu_slave.c   # Modbus Slave: parse frame, dispatch function code
│   │   ├── modbus_rtu_slave.h
│   │   ├── holding_registers.c  # Bảng thanh ghi Modbus (sensor data, control flags)
│   │   ├── sensor_adc.c         # Đọc ADC / I2C sensor
│   │   └── relay_ctrl.c         # Điều khiển Relay/PWM
│   └── Inc/
├── Drivers/                     # STM32 HAL (auto-generated)
├── STM32F4xx.ld                 # Linker script
└── Makefile
```

---

## 3. CROSS-COMPILE NHANH

### 3.1 R5 Firmware (ARM Cortex-R5)

**Toolchain:** Xilinx Vitis SDK hoặc `arm-none-eabi-gcc`

```bash
# Cách 1: Dùng Vitis SDK (nếu đã cài)
source /tools/Xilinx/Vitis/2022.2/settings64.sh
mb-gcc  # hoặc armr5-none-eabi-gcc từ Vitis

# Cách 2: Dùng arm-none-eabi-gcc (generic)
sudo apt install gcc-arm-none-eabi
arm-none-eabi-gcc -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 \
    -T lscript.ld -o r5_firmware.elf src/*.c \
    -I include/ -L lib/ -lopen_amp -lmetal -lxil
```

**Output:** `r5_firmware.elf` → nạp vào QEMU qua `remoteproc`

---

### 3.2 Linux App - Qt + MQTT (ARM Cortex-A53 aarch64)

**Toolchain:** PetaLinux SDK (sysroot có sẵn Qt + paho-mqtt libs)

```bash
# Bước 1: Export PetaLinux SDK sysroot
cd ~/openamp_modbusrtu
petalinux-build --sdk
petalinux-package --sysroot

# Bước 2: Source SDK environment
source /opt/petalinux/2022.2/environment-setup-cortexa72-cortexa53-xilinx-linux

# Bước 3: Cross-compile
cd ~/qt_dashboard
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=/opt/petalinux/2022.2/sysroots/cortexa53-xilinx-linux/usr/share/cmake/toolchain.cmake
make -j$(nproc)
```

**Hoặc compile nhanh không cần CMake:**
```bash
# Sau khi source SDK, biến $CC, $CXX đã trỏ tới cross-compiler
$CXX -o modbus-dashboard src/*.cpp \
    $(pkg-config --cflags --libs Qt5Widgets) \
    -lpaho-mqtt3c
```

**Output:** `modbus-dashboard` → copy vào rootfs QEMU

---

### 3.3 STM32 Firmware (ARM Cortex-M4/M7)

```bash
# Dùng arm-none-eabi-gcc (hoặc STM32CubeIDE)
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
    -T STM32F4xx.ld -o stm32_slave.elf Core/Src/*.c Drivers/Src/*.c \
    -I Core/Inc -I Drivers/Inc \
    -DSTM32F407xx

# Flash vào STM32 thật
st-flash write stm32_slave.bin 0x08000000
# Hoặc dùng OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
    -c "program stm32_slave.elf verify reset exit"
```

---

## 4. CHẠY TRÊN QEMU

### 4.1 Boot PetaLinux + R5 trên QEMU

```bash
# Bước 1: Build PetaLinux image
cd ~/openamp_modbusrtu
petalinux-build

# Bước 2: Chạy QEMU với UART passthrough tới USB-RS485
petalinux-boot --qemu --kernel \
    --qemu-args "-serial pty"
# QEMU sẽ in ra: "char device redirected to /dev/pts/3"

# Bước 3: Nối /dev/pts/3 (QEMU UART) với /dev/ttyUSB0 (RS485 adapter)
socat /dev/pts/3,raw,echo=0 /dev/ttyUSB0,raw,echo=0,b115200
```

### 4.2 Load R5 firmware qua remoteproc (bên trong Linux QEMU)

```bash
# Trong Linux shell của QEMU:
cp r5_firmware.elf /lib/firmware/
echo r5_firmware.elf > /sys/class/remoteproc/remoteproc0/firmware
echo start > /sys/class/remoteproc/remoteproc0/state

# Kiểm tra RPMsg device xuất hiện:
ls /dev/rpmsg*
# Kết quả mong đợi: /dev/rpmsg0
```

### 4.3 Chạy Qt Dashboard + MQTT
```bash
# Trong Linux shell của QEMU:
export DISPLAY=:0           # Nếu có framebuffer/VNC
./modbus-dashboard &

# Hoặc chạy MQTT client riêng:
./mqtt-publisher --broker 192.168.1.100 --topic sensors/data
```

---

## 5. WORKFLOW PHÁT TRIỂN HÀNG NGÀY (DEV LOOP NHANH)

```
 ┌──────────────────────────────────────────────────────────────┐
 │                  DAILY DEV WORKFLOW                          │
 │                                                             │
 │  1. Sửa code trên Host PC (VS Code)                        │
 │           │                                                 │
 │  2. Cross-compile (< 5 giây nếu chỉ sửa app)              │
 │           │                                                 │
 │  3. Copy binary vào QEMU rootfs                             │
 │     ├─ R5:  scp r5_fw.elf root@qemu:/lib/firmware/          │
 │     └─ App: scp dashboard root@qemu:/usr/bin/               │
 │           │                                                 │
 │  4. Trong QEMU: restart remoteproc / restart app            │
 │           │                                                 │
 │  5. Xem log, test Modbus giao tiếp STM32 thật              │
 │           │                                                 │
 │  6. Commit code lên Git                                     │
 └──────────────────────────────────────────────────────────────┘
```

### Mẹo tăng tốc dev:

| Thao tác | Cách chậm | Cách nhanh |
|---|---|---|
| Rebuild PetaLinux | `petalinux-build` (30+ phút) | Chỉ build lại app: `petalinux-build -c modbus-dashboard` (< 1 phút) |
| Copy file vào QEMU | Tắt QEMU, mount rootfs, copy, boot lại | `scp` qua mạng QEMU (không cần reboot) |
| Test R5 firmware | Reboot QEMU | `echo stop > remoteproc0/state && echo start > remoteproc0/state` |
| Debug Modbus | Đoán mò | Dùng `minicom` hoặc `modbus-cli` gửi frame test |
| Flash STM32 | Mở STM32CubeIDE | `st-flash` hoặc `openocd` command line |

---

## 6. CẤU TRÚC THƯ MỤC TỔNG THỂ SAU KHI HOÀN THIỆN

```
~/
├── openamp_modbusrtu/                    # PetaLinux project (QEMU Zynq)
│   ├── project-spec/meta-user/
│   │   └── recipes-apps/
│   │       ├── gpio-demo/                # [Có sẵn] App mẫu
│   │       ├── r5-modbus-master/         # [MỚI] R5 firmware recipe
│   │       └── modbus-dashboard/         # [MỚI] Qt+MQTT app recipe
│   ├── components/
│   ├── images/linux/
│   └── ARCHITECTURE.md                   # ← File này
│
├── r5_firmware/                          # [MỚI] R5 dev nhanh (ngoài PetaLinux)
│   ├── src/
│   └── CMakeLists.txt
│
├── qt_dashboard/                         # [MỚI] Qt+MQTT dev nhanh
│   ├── src/
│   └── CMakeLists.txt
│
└── stm32_modbus_slave/                   # [MỚI] STM32 project riêng
    ├── Core/Src/
    └── Makefile
```

---

## 7. THỨ TỰ PHÁT TRIỂN KHUYẾN NGHỊ

1. **STM32 Slave trước** → Vì là hardware thật, test độc lập bằng USB-UART + PC tool
2. **R5 Modbus Master** → Test giao tiếp R5 ↔ STM32 qua QEMU UART
3. **OpenAMP RPMsg** → Kết nối A53 ↔ R5 bên trong QEMU
4. **Qt GUI** → Dashboard hiển thị data từ RPMsg
5. **MQTT** → Kết nối ra Cloud, hoàn thiện hệ thống end-to-end
