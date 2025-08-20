#ifndef __DEFINE_H
#define __DEFINE_H

/* 协议的数据包类型定义 */
#define PING         0x00
#define BIN_TANS     0x01
#define OTA_VERIFY   0x02
#define VL_HEADER    0x03
#define PRINT        0x04
#define RESET        0x05
#define EXIT         0x06
#define UPDATE_PID   0x07
#define SAVE_PID     0x08
#define INIT_PID     0x09
#define GY86         0x10
#define SERVO        0x11

/* OTA_device定义 */
#define OTA_DEVICE_ID_LEN 8
#define FstPacketLen 8

/* Log类型定义 */
#define TCP_SERVER_TYPE  0
#define WEBSOCKET_TYPE   1
#define STM32_TYPE       2
#define HTTP_SERVER_TYPE 3

/* 服务器PORT定义 */
#define HTTP_SERVER_PORT 81
#define WEBSOCKET_PORT   9001
#define TCP_SERVER_PORT  9000

/* 数组长度定义 */
#define BUFFER_SIZE 1024
#define PACKET_SIZE 512
#define LOG_BUFFER_SIZE 1024

#define PACKET_PRE 0xFEFECDCD   //数据包前缀
#define TIMEOUT 10              //STM32与TCP超时时间

/* RW_tools初始化类型枚举 */
typedef enum rw_tools_type{
    INIT_MODE     = 0,
    NORMAL_MODE   = 1
}RW_tools_type;

#endif