/**
 * @file       data_types.h
 * @brief      Shared IPC Data Structures & Protocol Definitions (A53 <-> R5)
 * @details    Strictly compliant with MISRA C:2012 and HMP Binary Data Alignment.
 *             Ensures 100% identical memory layout between 64-bit A53 and 32-bit R5.
 * @author     Cong <congvmc1@gmail.com>
 * @date       2026-09-03
 * @version    2.1.0
 * 
 * @copyright  Copyright (c) 2026 Cong. All rights reserved.
 */

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

/* --- 1. System Standard Headers --- */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* --- 2. Protocol System Constants --- */
#define MAX_PAYLOAD_SIZE    256U
#define QUEUE_MAX_SIZE      100U

/* --- 3. Message Type Definitions (Fixed-width uint32_t for HMP compatibility) --- */
#define MSG_TYPE_UNKNOWN     0x00000000U
#define MSG_TYPE_SENSOR_DATA 0x00000001U  /* Telemetry: R5 -> A53 */
#define MSG_TYPE_CONTROL_CMD 0x00000002U  /* Command:   A53 -> R5 */
#define MSG_TYPE_HEARTBEAT   0x00000003U  /* Watchdog:  Bidirectional */

typedef uint32_t msg_type_t;

/* --- 4. Packed Protocol Payload Structures --- */
#pragma pack(push, 1)

/**
 * @brief Telemetry Sensor Data structure (R5 -> A53)
 * @note Ordered by 64-bit -> 32-bit alignment to eliminate padding holes.
 */
typedef struct sensor_data_s {
    uint64_t timestamp;     /* Hardware system timestamp (microseconds/ticks) */
    uint32_t stack_usage;   /* R5 FreeRTOS task stack high water mark (bytes) */
    uint32_t cpu_usage;     /* R5 CPU load percentage (0-100%) */
    uint32_t pkt_cnt;       /* Total Modbus packets processed */
    uint32_t err_cnt;       /* Modbus CRC/Timeout error counter */
} sensor_data_t;

/**
 * @brief Control Command structure (A53 -> R5)
 */
typedef struct control_cmd_s {
    uint32_t id_node;       /* Target Modbus Slave ID (1-247) */
    uint16_t reg_addr;      /* Modbus Register Starting Address */
    uint16_t value;         /* Value to write / Quantity to read */
} control_cmd_t;

/**
 * @brief Unified IPC Message Packet (OpenAMP RPMsg Shared Buffer Payload)
 * @note Enforces 1-byte packing and includes CRC32 integrity checksum.
 */
typedef struct app_msg_s {
    msg_type_t type;        /* 4 bytes: Message type identifier */
    uint32_t   crc32;       /* 4 bytes: Packet integrity verification checksum */
    union {
        sensor_data_t sensor;
        control_cmd_t cmd;
        uint8_t       raw_bytes[MAX_PAYLOAD_SIZE];
    } body;
} app_msg_t;

#pragma pack(pop)

#endif /* DATA_TYPES_H */