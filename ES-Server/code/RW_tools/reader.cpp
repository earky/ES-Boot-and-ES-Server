#include "reader.h"

Reader::Reader(const char* sem_addr_, const char* shared_memory_addr_, RW_tools_type type){
    // 保存资源标识路径
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

    is_connected_ = false;
    int oflag;
    if(type == INIT_MODE){
        oflag = O_CREAT;
    }else{
        oflag = O_RDWR;
    }


     /* 创建信号量 */
    lock = sem_open(lock_addr, oflag, 0666, 1);
    if (lock == SEM_FAILED) {
        perror("sem_open");
        return;
    }
    /* 创建信号量 */
    produce_sem = sem_open(produce_addr, oflag, 0666, 1);
    if (produce_sem == SEM_FAILED) {
        perror("sem_open");
        return;
    }
    /* 创建信号量 */
    consume_sem = sem_open(consume_addr, oflag, 0666, 1);
    if (consume_sem == SEM_FAILED) {
        perror("sem_open");
        return;
    }
    /* 创建信号量 */
    num_sem = sem_open(num_addr, oflag, 0666, 1);
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

    is_connected_ = true;
}

bool Reader::read(char* out_data, uint32_t* len, uint8_t* out_type) {
    if (!is_connected_) return false;
    sem_wait(produce_sem);
    sem_wait(lock); // 等待信号量（阻塞直到获取锁）

    *out_type = ptr[0]; // 解析消息类型
    // 解析消息长度 (24位大端序)
    *len = (static_cast<uint32_t>(ptr[1]) << 16) |
                   (static_cast<uint32_t>(ptr[2]) << 8)  |
                   static_cast<uint32_t>(ptr[3]);
    // 拷贝数据内容
    memcpy(out_data, &ptr[4], *len);
    out_data[*len] = '\0'; // 添加字符串终止符

    sem_post(lock); // 释放信号量
    sem_post(consume_sem);
    
    return true;
}

// 字符串版本读取
bool Reader::read(std::string& out_str, uint32_t* len, uint8_t* out_type) {
    char raw_data[2048];
    if (!read(raw_data, len, out_type)) return false;
    out_str = std::string(raw_data);

    return true;
}


// 资源清理
void Reader::destory(void) {
    if(!is_connected_)
        return;
    
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

bool Reader::is_connected(void){
    return is_connected_;
}

int Reader::get_process_num(){
    int value;
    sem_getvalue(num_sem, &value);
    return value;
}
