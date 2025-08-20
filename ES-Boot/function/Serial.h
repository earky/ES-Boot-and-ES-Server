#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f4xx.h"
#include <stdio.h>

#define BUFSIZE 2048
#define SOCKET_DATA_DELAY 10

#define NONE 		 		0	//WIFI未连接
#define WIFI 				1	//连接了WIFI但是没有连接 TCP_server
#define WIFI_TCP 		2	//连接了WIFI & TCP_server
#define RCV_DATA 		3	//获取数据
#define CIPSTATE    4 //查询TCP是否连接
#define CWSTATE     5 //查询wifi是否连接


struct buffer{
	char data[BUFSIZE];
	int len;
	int fstPos;//有效数据起始地址
	int lstPos;//有效数据末尾地址
};

extern struct buffer socketPacket;

extern uint8_t count;
extern uint8_t State;
extern uint8_t mode;

extern uint8_t cipstate_count;

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_WaitConnected(void);
uint8_t Serial_GetRxFlag(void);
void Serial_SendString(char* str);


uint32_t getData(struct buffer* Packet, char* src, uint32_t src_size,uint32_t size);
void Serial_SendBytes(void* base, uint32_t len);

void getSocketData(char* str, uint32_t size);
void SendPacketHeader(uint8_t type, uint32_t packet_len);

#endif 
