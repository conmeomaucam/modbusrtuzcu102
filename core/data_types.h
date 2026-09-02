/**
 * @file       data_types.h
 * @brief      data types definition for linux application
 * @author     Cong <congvmc1@gmail.com>
 * @date       2026-08-27
 * @version    2.0.0
 * 
 * @copyright  Copyright (c) 2026 Cong. All rights reserved.
 */

#ifndef data_types_H
#define data_types_H


/* --- 1. System standard headers --- */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* --- 2. System Macros & Constants --- */
#define MAX_PAYLOAD_SIZE    256   
#define QUEUE_MAX_SIZE      100


/* --- 3. Message Type Enumeration --- */

typedef enum {
        MSG_TYPE_UNKNOWN = 0,
        MSG_TYPE_SENSOR_DATA,     //r5-a53
        MSG_TYPE_CONTROL_CMD,     //a53->r5
        MSG_TYPE_HEARTBEAT // watchdog         
} msg_type_t;

/* --- 4. Payload Structs --- */
//dữ liệu slave gửi lên
typedef struct sensor_data_s {
    // giá trị sử dụng stack , cpu_usage , timestamp, số gói tin nhận được ,....
    uint32_t stack_usage;
    uint32_t cpu_usage;
    uint64_t timestamp;
    uint32_t pkt_cnt;
    uint32_t err_cnt;    
} sensor_data_t;


// gửi lệnh điều khiển xuống r5_0
typedef struct control_cmd_s {
        uint32_t ID_node; 
        uint16_t reg_addr; 
        uint16_t value;
    
} control_cmd_t;

typedef struct  app_msg_s  {
    msg_type_t type;        
    union {                 
        sensor_data_t sensor;
        control_cmd_t cmd;
        uint8_t       raw_bytes[MAX_PAYLOAD_SIZE];
    } body;
} app_msg_t;


#endif /* data_types_H */