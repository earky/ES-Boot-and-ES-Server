#include "Log.h"

#include <cstdio>
#include <cstdarg>
#include "../define.h"

char Log_Buffer[LOG_BUFFER_SIZE];
bool WebSocket, TCP, HTTP;

void Log(int type, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(Log_Buffer, LOG_BUFFER_SIZE, format, args);
	va_end(args);
	
    switch(type){
        case STM32_TYPE:
            printf("[STM32] : %s\n", Log_Buffer);
            break;
        case TCP_SERVER_TYPE:
            if(TCP)
                printf("[TCP] : %s\n", Log_Buffer);
            break;
        case WEBSOCKET_TYPE:
            if(WebSocket)
                printf("[WebSocket] : %s\n", Log_Buffer);
            break;
        case HTTP_SERVER_TYPE:
            if(HTTP)
                printf("[HTTP] : %s\n", Log_Buffer);
            break;
    }
}

void Log_init(bool arr[]){
    WebSocket = arr[0];
    TCP = arr[1];
    HTTP = arr[2];
}