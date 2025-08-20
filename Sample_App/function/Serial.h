#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f4xx.h"
#include "PID.h"
#include <stdio.h>

#define BUFSIZE 1024

#define PACKET_PRE  0
#define PACKET_TYPE 1
#define PACKET_LEN  2
#define PACKET_RCV  3
#define PACKET_PRE_STR 0xFEFECDCD

#define RESET_TYPE 0x05

#define BOOTLOADER_ENTRY 		0x08000189
#define BOOTLOADER_SP_ENTRY 0x08000000

struct buffer{
	char data[BUFSIZE];
	int len;
	int fstPos;//有效数据起始地址
	int lstPos;//有效数据末尾地址
};


// buffer
extern struct buffer RxPacket;
extern struct buffer socketPacket;
extern uint8_t  packet_type;		//数据包类型
extern uint32_t packet_len;			//数据包长度

extern uint32_t len;

extern int Serial_Flag;					//串口标志

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_WaitConnected(void);
uint8_t Serial_GetRxFlag(void);
void Serial_SendString(char* str);

char* getData(struct buffer* Packet,uint8_t size);
void sendData(char* str);
void Serial_SendBytes(void* base, uint32_t len);
uint8_t getDataLen(void);

void SendPacketHeader(uint8_t type, uint32_t packet_len);

#endif 
