#include "writer.h"

Writer::Writer(const char* sem_addr_, const char* shared_memory_addr_, RW_tools_type type){
    strcpy(sem_addr, sem_addr_);
    strcpy(shared_memory_addr, shared_memory_addr_);
    type_ = type;

    char lock_addr[128];
    char produce_addr[128];
    char consume_addr[128];
    char num_addr[128];
    sprintf(lock_addr,    "%s-lock",    sem_addr);
    sprintf(produce_addr, "%s-produce", sem_addr);
    sprintf(consume_addr, "%s-consume", sem_addr);
    sprintf(num_addr,     "%s-num",     sem_addr);


    if(type == INIT_MODE){
        shm_unlink(shared_memory_addr);  // 忽略返回值（无论是否存在都尝试删除）
        sem_unlink(lock_addr);  
        sem_unlink(produce_addr);
        sem_unlink(consume_addr);
    }
    
    /* 创建信号量 */
    lock = sem_open(lock_addr, O_CREAT, 0666, 1);
    if (lock == SEM_FAILED) {
        perror("sem_open");
        return;
    }
    /* 创建信号量 */
    consume_sem = sem_open(consume_addr, O_CREAT, 0666, 1);
    if (consume_sem == SEM_FAILED) {
        perror("sem_open");
        return;
    }
    /* 创建信号量 */
    produce_sem = sem_open(produce_addr, O_CREAT, 0666, 1);
    if (produce_sem == SEM_FAILED) {
        perror("sem_open");
        return;
    }
    /* 创建信号量 */
    num_sem = sem_open(num_addr, O_CREAT, 0666, 1);
    if (num_sem == SEM_FAILED) {
        perror("sem_open");
        return;
    }

    /* 创建共享内存对象 */
    shm_fd = shm_open(shared_memory_addr, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHARED_MEMORY_SIZE); // 设置共享内存大小

    // 映射共享内存到进程地址空间
    ptr = (char*)mmap(0, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return;
    }


    if(type == INIT_MODE){
        sem_wait(produce_sem);
    }else{
        sem_post(num_sem);
    }
}

Writer::~Writer(){

}

void Writer::destory(void){
    /* 清理信号量 */
    char lock_addr[128];
    char produce_addr[128];
    char consume_addr[128];
    char num_addr[128];
    sprintf(lock_addr,    "%s-lock",    sem_addr);
    sprintf(produce_addr, "%s-consume", sem_addr);
    sprintf(consume_addr, "%s-produce", sem_addr);
    sprintf(num_addr,     "%s-num",     sem_addr);

    sem_wait(num_sem);  //process - 1

    if (lock != SEM_FAILED) {
        sem_close(lock);          // 先关闭信号量
        if(type_ == INIT_MODE)
            sem_unlink(lock_addr);  

    }
    if (consume_sem != SEM_FAILED) {
        sem_close(consume_sem);          // 先关闭信号量
        if(type_ == INIT_MODE)
            sem_unlink(consume_addr);   
    }
    if (produce_sem != SEM_FAILED) {
        sem_close(produce_sem);          // 先关闭信号量
        if(type_ == INIT_MODE)
            sem_unlink(produce_addr);  
    }
    if (num_sem != SEM_FAILED){
        sem_close(num_sem);
        if(type_ == INIT_MODE)
            sem_unlink(num_addr);  
    }

    /* 清理共享内存 */
    if (ptr != MAP_FAILED) {
        munmap(ptr, SHARED_MEMORY_SIZE);  // 解除内存映射
    }
    if (shm_fd != -1) {
        close(shm_fd);                   // 关闭文件描述符
    }
}

void Writer::write(const char* str, uint8_t type){
    write_len(str, type, strlen(str));
}

void Writer::write_len(const char* str, uint8_t type, int len){

    if(get_process_num() < 2)
        return;

    sem_wait(consume_sem);
    char msg[4];

    msg[0] = type;
    msg[1] = (uint8_t)((len & 0x00FF0000) >> 16);
    msg[2] = (uint8_t)((len & 0x0000FF00) >> 8);
    msg[3] = (uint8_t) (len & 0x000000FF);

    sem_wait(lock);
    memcpy(ptr, msg, 4);
    memcpy(&ptr[4], str, len);
    sem_post(lock);


    sem_post(produce_sem);
}

void Writer::write(std::string& str, uint8_t type){
    write(str.c_str(), type);
}

int Writer::get_process_num(){
    int value;
    sem_getvalue(num_sem, &value);
    return value;
}