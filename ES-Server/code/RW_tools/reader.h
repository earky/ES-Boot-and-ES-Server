#ifndef __READER_H
#define __READER_H

#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <atomic>
#include <stdlib.h>
#include "../OTA_device/OTA_device.h"
#include "../define.h"

class Reader{
public:
    Reader(const char* sem_addr_, const char* shared_memory_addr_, RW_tools_type type);
    bool read(char* out_data, uint32_t* len, uint8_t* out_type);
    bool read(std::string& out_str, uint32_t* len, uint8_t* out_type);
    void destory(void);

    bool is_connected(void);
    bool is_connected_;      // 连接状态标志

    int get_process_num(void);

private:
    sem_t* lock;             // 信号量指针
    sem_t* produce_sem;
    sem_t* consume_sem;
    sem_t* num_sem;
    int type_;

    char* ptr;              // 共享内存映射地址
    int shm_fd;             // 共享内存文件描述符
    char sem_addr[50];     // 信号量标识路径
    char shared_memory_addr[50];     // 共享内存标识路径

};


#endif
