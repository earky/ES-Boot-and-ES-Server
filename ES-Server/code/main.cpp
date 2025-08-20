#include "TCP_server/TCP_server.h"
#include "OTA_device/OTA_device.h"
#include "WebSocket_server/WebSocket.h"
#include "HTTP_server/HTTP_server.h"
#include "Log/Log.h"

int main(int argc, char* argv[]) {
    bool arr[3] = {1, 1, 1};
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i], "-sWebSocket") == 0){
            arr[0] = 0;
        }else if(strcmp(argv[i], "-sTCP") == 0){
            arr[1] = 0;
        }else if(strcmp(argv[i], "-sHTTP") == 0){
            arr[2] = 0;
        }else if(strcmp(argv[i], "-silent") == 0){
            arr[0] = arr[1] = arr[2] = 0;
        }else{
            printf("Invalid operation %s\n", argv[i]);
            return 0;
        }
    }
    Log_init(arr);

    // 将信号量 共享内存 原进程删除
    system("rm -r /dev/shm/sem.* /dev/shm/shared*");
    system("pkill -f \"HTTP_Server\"");
    system("pkill -f \"WebSocket_Server\"");
    system("pkill -f \"TCP_Server\"");

    pid_t pid;
    for(int i=0;i<2;i++){
        pid = fork();
        if (pid < 0) {
            printf("error in fork!");
        } else if (pid == 0) {
            if(i == 0){
                setsid();
                strcpy(argv[0], "WebSocket_Server");
                start_WebSocket_server();
                break;
            }else if(i == 1){
                setsid();
                strcpy(argv[0], "HTTP_Server");
                start_HTTP_server();
                break;
            }
        } else {
            continue;
        }
    }

    strcpy(argv[0], "TCP_Server");
    start_TCP_server();
    
    return 0;

}