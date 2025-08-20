#ifndef __WRITER_H
#define __WRITER_H

#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <atomic>
#include "../OTA_device/OTA_device.h"
#include "../define.h"

class Writer{
public:
    Writer(const char* sem_addr, const char* shared_memory_addr, RW_tools_type type);
    ~Writer();

    void destory(void);
    void write(const char* str, uint8_t type);
    void write_len(const char* str, uint8_t type, int len);
    void write(std::string& str, uint8_t type);

    int get_process_num(void);
    
private:
    sem_t* lock;
    sem_t* produce_sem;
    sem_t* consume_sem;
    sem_t* num_sem;
    
    void* header;
    char* ptr;
    int shm_fd;
    int type_;

    char sem_addr[50];
    char shared_memory_addr[50];

};

#endif