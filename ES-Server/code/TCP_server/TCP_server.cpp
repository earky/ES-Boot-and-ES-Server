#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <atomic>
#include "TCP_server.h"


void RESET_handler(ClientContext* ctx){
    char msg[8];
    msg[0] = msg[1] = 0xFE;
    msg[2] = msg[3] = 0xCD;
    msg[4] = RESET;
    msg[5] = msg[6] = msg[7] = 0;

    send(ctx->socket, msg, sizeof(msg), 0);
    Log(TCP_SERVER_TYPE ,"Send RESET msg to quadrotor\n");
    ctx->OTA_verify_state = 1;
}

void EXIT_handler(int* exit_flag){
    if(exit_flag)
        *exit_flag = 0;
}


void update_PID_handler(const char* str, uint32_t len, ClientContext* ctx){
    char msg[256];
    msg[0] = msg[1] = 0xFE;
    msg[2] = msg[3] = 0xCD;
    msg[4] = UPDATE_PID;

    msg[5] = (uint8_t)((len & 0x00FF0000) >> 16);
    msg[6] = (uint8_t)((len & 0x0000FF00) >> 8);
    msg[7] = (uint8_t) (len & 0x000000FF);

    memcpy(&msg[8], str, len);

    Log(TCP_SERVER_TYPE ,"Reader thread -> Send update PID msg to quadrotor");
    
    float f[9];
    memcpy(f, str, sizeof(float) * 9);
    // Log(TCP_SERVER_TYPE ,"[Reader thread] : %f %f %f %f %f %f %f %f %f\n",
    //                                                 f[0],f[1],f[2],
    //                                                 f[3],f[4],f[5],
    //                                                 f[6],f[7],f[8]);
    send(ctx->socket, msg, len + 8, 0);
}

void save_PID_handler(const char* str, uint32_t len, ClientContext* ctx){
    char msg[256];
    msg[0] = msg[1] = 0xFE;
    msg[2] = msg[3] = 0xCD;
    msg[4] = SAVE_PID;
 
    msg[5] = (uint8_t)((len & 0x00FF0000) >> 16);
    msg[6] = (uint8_t)((len & 0x0000FF00) >> 8);
    msg[7] = (uint8_t) (len & 0x000000FF);

    memcpy(&msg[8], str, len);

    send(ctx->socket, msg, len + 8, 0);
    Log(TCP_SERVER_TYPE ,"Reader thread -> Send save PID msg");
}

void init_PID_handler(ClientContext* ctx){
    char msg[8];
    msg[0] = msg[1] = 0xFE;
    msg[2] = msg[3] = 0xCD;
    msg[4] = INIT_PID;
 
    msg[5] = msg[6] = msg[7] = 0;

    send(ctx->socket, msg,  8, 0);
    Log(TCP_SERVER_TYPE ,"Reader thread -> Send INIT PID msg");
}

void* reader_handler(void *arg) {
    ClientContext* ctx = (ClientContext*)arg;
    int exit_flag = 1;
    
    while(exit_flag){
        char msg[1024] = {0};
        uint8_t type;
        uint32_t len;
        ctx->reader->read(msg, &len, &type);

        
        switch(type){
            case 0x00:
                // 休眠10ms
                usleep(10000);
                break;
            case RESET:
                RESET_handler(ctx);
                break;
            case EXIT:
                EXIT_handler(&exit_flag);
                break;
            case UPDATE_PID:
                update_PID_handler(msg, len, ctx);
                break;
            case SAVE_PID:
                save_PID_handler(msg, len, ctx);
                break;
            case INIT_PID:
                init_PID_handler(ctx);
                break;
            default:
                break;
        }
    }

    delete ctx->reader;
    Log(TCP_SERVER_TYPE ,"Reader thread exit");
}

void get_reader_thread(ClientContext* ctx){
    char sem_addr[50];
    char shared_memory_addr[50];
    sprintf(sem_addr, "%s%s2", SEMAPHORE_PRE, ctx->device->OTA_Device_ID_str);
    sprintf(shared_memory_addr, "%s%s2", SHARED_MEMORY_PRE, ctx->device->OTA_Device_ID_str);

    if(!ctx->reader){
        ctx->reader = new Reader(sem_addr, shared_memory_addr, INIT_MODE);
    }
    //创建线程读取websockets数据，同时发送数据给stm32
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, reader_handler, ctx) != 0) {
        perror("pthread_reader_create");
        delete ctx->reader;
    }
    
    pthread_detach(thread_id);
}


void ping_handler(void){

}

// 处理打印信息
void print_information_handler(char* buffer, unsigned int packet_len, ClientContext* ctx){
    Log(TCP_SERVER_TYPE ,"print -> %s", buffer);
    if(ctx->writer){
        ctx->writer->write(buffer, PRINT);
    }
}


void VL_header_handler(int sock, char* file_addr, ClientContext* ctx){

    Log(TCP_SERVER_TYPE ,"device id is %s\n",ctx->device->OTA_Device_ID_str);
    sprintf(file_addr, "%s/v%hu.%hu.bin", 
        ctx->device->OTA_Device_ID_str, ctx->device->Bversion, ctx->device->Sversion);

    // 读取并发送project.bin文件
    ctx->file = fopen(file_addr, "rb");
    if (ctx->file == NULL) {
        Log(TCP_SERVER_TYPE ,"Failed to open %s for %s:%d", 
                  file_addr, ctx->client_ip.c_str(), ctx->client_port);
        return;
    }

    // 获取文件长度
    fseek(ctx->file, 0, SEEK_END);
    ctx->file_length = ftell(ctx->file);
    fseek(ctx->file, 0, SEEK_SET);

    // 发送文件头（版本和长度）
    uint8_t header[6];
    header[0] = ctx->device->Bversion;
    header[1] = ctx->device->Sversion;
    header[2] = (ctx->file_length >> 24) & 0xFF;
    header[3] = (ctx->file_length >> 16) & 0xFF;
    header[4] = (ctx->file_length >> 8) & 0xFF;
    header[5] = ctx->file_length & 0xFF;

    send(sock, header, sizeof(header), 0);
    
    Log(TCP_SERVER_TYPE ,"lastest version is v%hu.%hu, file length is %d\n",
        ctx->device->Bversion, ctx->device->Sversion, ctx->file_length);
}


void bin_packet_handler(int sock, char* file_buffer, char* file_addr, ClientContext* ctx){
    size_t bytes_read;

    if (ctx->file == NULL) {
        Log(TCP_SERVER_TYPE ,"Failed to open %s for %s:%d", 
                  file_addr, ctx->client_ip.c_str(), ctx->client_port);
        return;
    }

    // 读取并发送下一个数据包
    size_t to_read = (ctx->file_length - ctx->total_sent) > PACKET_SIZE ? 
                        PACKET_SIZE : (ctx->file_length - ctx->total_sent);
    
    bytes_read = fread(file_buffer, 1, to_read, ctx->file);

    if (bytes_read <= 0) 
        return;

    ssize_t bytes_sent = send(sock, file_buffer, bytes_read, 0);
    if (bytes_sent < 0) {
        perror("send failed");
    }

    ctx->total_sent += bytes_sent;
    ctx->packet_count++;


    char msg[128];
    sprintf(msg, "Sent packet #%d to %s:%d: %zd/%ld bytes (%.2f%%)", 
                   ctx->packet_count, ctx->client_ip.c_str(), ctx->client_port,
                   ctx->total_sent, ctx->file_length, (double)ctx->total_sent/ctx->file_length * 100);
    Log(TCP_SERVER_TYPE ,"%s", msg);
    if(ctx->writer){
        ctx->writer->write(msg, PRINT);
    }


    if(ctx->total_sent == ctx->file_length) {
        sprintf(msg, "File sent successfully to %s:%d. Total bytes: %ld", 
                  ctx->client_ip.c_str(), ctx->client_port, ctx->file_length);
        Log(TCP_SERVER_TYPE ,"%s", msg);

        if(ctx->writer){
            ctx->writer->write(msg, PRINT);
        }
        ctx->total_sent = 0;
        ctx->packet_count = 0;
    }
}



void OTA_verify_handler(char* recv_buffer,unsigned int packet_len, ClientContext* ctx){
    // 使用OTA设备验证
    if (ctx->device->OTA_device_verify(recv_buffer)) {
        Log(TCP_SERVER_TYPE ,"OTA success for %s:%d (Device ID: %s, Version: V%hu.%hu)",
                  ctx->client_ip.c_str(), ctx->client_port,
                  ctx->device->OTA_Device_ID_str, ctx->device->Bversion, ctx->device->Sversion);
        
        ctx->OTA_verify_state = 2;

        // 创建与WebSocket通信所需要的内容
        char sem_addr[50];
        char shared_memory_addr[50];
        sprintf(sem_addr, "%s%s", SEMAPHORE_PRE, ctx->device->OTA_Device_ID_str);
        sprintf(shared_memory_addr, "%s%s", SHARED_MEMORY_PRE, ctx->device->OTA_Device_ID_str);

        if(!ctx->writer){
            ctx->writer = new Writer(sem_addr, shared_memory_addr, INIT_MODE);
        }
        get_reader_thread(ctx);
    } else {
        Log(TCP_SERVER_TYPE ,"OTA failed for %s:%d (Invalid Device ID: %s)",
                  ctx->client_ip.c_str(), ctx->client_port, recv_buffer);
        ctx->OTA_verify_state = 0;
        return;
    }
}

void init_PID_handler(char* recv_buffer, uint32_t len, ClientContext* ctx){
    if(ctx->writer){
        ctx->writer->write_len(recv_buffer, INIT_PID, len);
    }
}

void GY86_handler(char* recv_buffer, uint32_t len, ClientContext* ctx){
    if(ctx->writer){
        //Log(TCP_SERVER_TYPE ,"gy86_handler");
        ctx->writer->write_len(recv_buffer, GY86, len);
    }
}

void servo_handler(char* recv_buffer, uint32_t len, ClientContext* ctx){
    if(ctx->writer){
        //Log(TCP_SERVER_TYPE ,"servo_handler");
        ctx->writer->write_len(recv_buffer, SERVO, len);
    }
}

ssize_t read_blocking(int fd, void *buffer, size_t total_bytes, ClientContext* ctx) {
    if(total_bytes == 0)
        return 0;

    size_t bytes_read = 0;
    char *buf_ptr = (char *)buffer;
    
    fd_set set;
    struct timeval timeout;
    int rv;

    FD_ZERO(&set);
    FD_SET(fd, &set);

    timeout.tv_sec = TIMEOUT; // 设置超时时间为TIMEOUT秒
    timeout.tv_usec = 0;

    while (bytes_read < total_bytes) {
        rv = select(fd + 1, &set, NULL, NULL, &timeout);
        // select打开失败
        if (rv == -1) {
            perror("select");
            return -1;
        } else if (rv == 0) {   //超时
            Log(TCP_SERVER_TYPE ,"timeout");
            return -1;
        } else {                //成功读取
            ssize_t n = read(fd, buf_ptr + bytes_read, total_bytes - bytes_read);
        
            if (n < 0) {
                // 被信号中断则重试
                if (errno == EINTR) {
                    continue;
                }
                perror("read error");
                return -1;
            } 
            // 对端关闭连接
            else if (n == 0) {
                fprintf(stderr, "Connection closed by peer\n");
                return -1;
            }
            
            bytes_read += n;
        }
    }
    return bytes_read;
}

void *client_handler(void *arg) {
    ClientContext* ctx = (ClientContext*)arg;
    ctx->writer = NULL;
    ctx->reader = NULL;
    int sock = ctx->socket;
    
    char file_addr[40];
    char file_buffer[PACKET_SIZE];

    char header_buffer[4];
    char packet_type = 0;
    unsigned int packet_len = 0;
    ssize_t bytes_read; 
    char buffer[BUFFER_SIZE];
    /* 数据包前缀 */
    unsigned int packet_pre = 0;
    uint8_t data;

    
    while(1){
        /* OTA设备验证失败直接退出 */
        if(ctx->OTA_verify_state == 0){
            break;
        }

        /* 识别数据包前缀 未到前缀的无效数据包过滤 */
        while(bytes_read = read_blocking(sock, &data, 1, ctx)){
            if(bytes_read == -1) break;
            packet_pre = (packet_pre << 8) & 0xFFFFFF00;
            packet_pre = packet_pre | data;
            if(packet_pre == PACKET_PRE){
                break;
            }
        }
        if(bytes_read == -1) break;
        /* 识别数据包头 */
        bytes_read = read_blocking(sock, header_buffer, 4, ctx);
        if(bytes_read == -1) break;

        packet_type = header_buffer[0];
        packet_len = header_buffer[1] << 16 | header_buffer[2] << 8 | header_buffer[3];
        
        //Log("Packet_type is %#02X, Packet_len is %d\n", packet_type, packet_len);

        packet_len = packet_len >= BUFFER_SIZE ? BUFFER_SIZE - 1 : packet_len;
        bytes_read = read_blocking(sock, buffer, packet_len, ctx);
        buffer[packet_len] = '\0';
        if(bytes_read == -1) break;

        /* 根据解析得到的不同类型数据进行处理 */
        switch(packet_type){
            case PING:
                ping_handler();
                break;
            case BIN_TANS:
                if(ctx->OTA_verify_state == 2){
                    bin_packet_handler(sock, file_buffer, file_addr, ctx);
                }else{
                    Log(TCP_SERVER_TYPE ,"Device verification for %s:%d is Not Passed, not allowed to send bin file", 
                      ctx->client_ip.c_str(), ctx->client_port);
                }
                break;
            case OTA_VERIFY:      
                OTA_verify_handler(buffer, packet_len, ctx);
                break;
            case VL_HEADER:
                if(ctx->OTA_verify_state == 2){
                    VL_header_handler(sock, file_addr, ctx);
                }else{
                    Log(TCP_SERVER_TYPE ,"Device verification for %s:%d is Not Passed, not allowed to send header", 
                      ctx->client_ip.c_str(), ctx->client_port);
                }
                break;
            case PRINT:
                if(ctx->OTA_verify_state == 2){
                    print_information_handler(buffer, packet_len, ctx);
                }
                break;
            case INIT_PID:
                if(ctx->OTA_verify_state == 2){
                    init_PID_handler(buffer, packet_len, ctx);
                }
                break;
            case GY86:
                if(ctx->OTA_verify_state == 2){
                    GY86_handler(buffer, packet_len, ctx);
                }
                break;
            case SERVO:
                if(ctx->OTA_verify_state == 2){
                    servo_handler(buffer, packet_len, ctx);
                }
                break;
            default:
                Log(TCP_SERVER_TYPE ,"Device parse header type %d failed for %s:%d",
                    packet_type, ctx->client_ip.c_str(), ctx->client_port);
                break;
        }
        
    }

    if(ctx->file)
        fclose(ctx->file);
    
    close(ctx->socket);

    if(ctx->writer){
        //Log(TCP_SERVER_TYPE ,"writer exit sign");
        ctx->writer->write("DEVICE EXIT???", EXIT);
        sleep(1);   //休眠1s，让reader将数据读完
        ctx->writer->destory();
        delete ctx->writer;
    }

    Log(TCP_SERVER_TYPE ,"Thread id:%lu is exit",ctx->thread_id);
    return NULL;
}


int start_TCP_server(void){
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // 创建TCP套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    // 设置端口复用
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return EXIT_FAILURE;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(TCP_SERVER_PORT);

    // 绑定端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return EXIT_FAILURE;
    }

    // 开始监听
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    Log(TCP_SERVER_TYPE ,"Server listening on TCP_SERVER_PORT %d", TCP_SERVER_PORT);
   
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);
        
        Log(TCP_SERVER_TYPE ,"New connection from %s:%d", client_ip, client_port);

        // 创建客户端上下文
        ClientContext* ctx = new ClientContext(new_socket, client_ip, client_port);
        ctx->device = new OTA_Device();
        ctx->file = NULL;

        // 创建线程处理客户端
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_handler, ctx) != 0) {
            perror("pthread_create");
            delete ctx;
            close(new_socket);
            continue;
        }
        
        pthread_detach(thread_id);
    }

    close(server_fd);
}

