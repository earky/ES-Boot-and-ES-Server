#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <fcntl.h>
#include <vector>
#include <cstring>
#include <string>
#include <memory>
#include <stdarg.h>
#include <semaphore.h>
#include "../RW_tools/writer.h"
#include "../RW_tools/reader.h"
#include "../OTA_device/OTA_device.h"
#include "../define.h"
#include "../Log/Log.h"

struct ClientContext {
    int socket;
    pthread_t thread_id;
    pthread_mutex_t file_mutex;
    pthread_cond_t packet_cond;
    int packet_ready;
    int file_sent;
    OTA_Device* device;
    std::string client_ip;
    int client_port;
    Writer* writer;
    Reader* reader;

    // 文件指针
    FILE* file;
    // 版本 version 和 文件长度 file_length 头处理
    char file_addr[40];
    unsigned long file_length;
    // OTA验证状态 为0则不通过 为1则待验证 为2则通过
    int OTA_verify_state = 1;

    // 读取并发送文件内容
    char file_buffer[PACKET_SIZE];
    size_t total_sent = 0;
    int packet_count = 0;

    // 与WebSocket进程通信部分
    sem_t* sem;
    int shm_fd;
    void* ptr;

    ClientContext(int sock, const std::string& ip, int port) 
        : socket(sock), client_ip(ip), client_port(port) {
        pthread_mutex_init(&file_mutex, NULL);
        pthread_cond_init(&packet_cond, NULL);
        packet_ready = 0;
        file_sent = 0;
        file = NULL;
    }

    ~ClientContext() {
        pthread_mutex_destroy(&file_mutex);
        pthread_cond_destroy(&packet_cond);
    }
};

int start_TCP_server(void);


#endif
