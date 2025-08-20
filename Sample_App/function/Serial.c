#include "stm32f4xx.h"
#include "Serial.h"
#include "Delay.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


uint8_t ifConnected = 0;	  //记录BLE是否已经连接
struct buffer RxPacket;
struct buffer socketPacket;
//uint8_t ifBLU = 0;					//是否打开透传模式
//uint8_t Serial_RxFlag = 0;	//定义接收数据包标志位
uint8_t num = 0;
uint32_t x =0;

/**
  * @brief 串口初始化
  */
void Serial_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//开启USART1的时钟
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_USART1);
	
	GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 		
	GPIO_InitStructure.GPIO_OType= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/*USART初始化*/
	USART_InitTypeDef USART_InitStructure;					//定义结构体变量
	USART_InitStructure.USART_BaudRate = 115200;				//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//硬件流控制，不需要
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//模式，发送模式和接收模式均选择
	USART_InitStructure.USART_Parity = USART_Parity_No;		//奇偶校验，不需要
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//停止位，选择1位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//字长，选择8位
	USART_Init(USART1, &USART_InitStructure);				//将结构体变量交给USART_Init，配置USART1

	
	/*中断输出配置*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);			//开启串口接收数据的中断
	
	
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//配置NVIC为分组2
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;					//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		//选择配置NVIC的USART1线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//指定NVIC线路的抢占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);							//将结构体变量交给NVIC_Init，配置NVIC外设
	
	/*USART使能*/
	USART_Cmd(USART1, ENABLE);								//使能USART1，串口开始运行
	
	for(int i=0;i<BUFSIZE;i++)
		RxPacket.data[i] = socketPacket.data[i] = 0;
	RxPacket.len = RxPacket.fstPos = RxPacket.lstPos = 0;
	socketPacket.len = socketPacket.fstPos = socketPacket.lstPos = 0;

}

/**
 * @brief 从Buffer中阻塞获取指定长度数据
 * @param Packet     指定的Buffer
 * @param size       需要获取的字节
 */
void Serial_SendString(char* str){
	for(int i=0;str[i]!='\0';i++){
		Serial_SendByte(str[i]);
	}
}

/**
  * @brief 串口发送一个字节
  * @param Byte 要发送的一个字节
  */
void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);		//将字节数据写入数据寄存器，写入后USART自动生成时序波形
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	//等待发送完成
	/*下次写入数据寄存器会自动清除发送完成标志位，故此循环后，无需清除标志位*/
}

/**
  * @brief 串口发送字节
  * @param base 传输数据起始地址
  * @param len  插入数据长度
  */
void Serial_SendBytes(void* base, uint32_t len){
	char* p = (char*)base;
	for(uint32_t i=0;i<len;i++){
		USART_SendData(USART1,(char)(*(p++)));		//将字节数据写入数据寄存器，写入后USART自动生成时序波形
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	//等待发送完成
	}
}

/**
  * @brief 获取串口接收数据包标志位
  */
uint8_t Serial_GetRxFlag(void)
{
	if (Serial_Flag == 1)			//如果标志位为1
	{
		Serial_Flag = 0;
		return 1;					//则返回1，并自动清零标志位
	}
	return 0;						//如果标志位为0，则返回0
}

/**
  * @brief 向指定Buffer插入数据
  * @param RxData 要插入的数据
  * @param Packet 插入的Buffer
  */
void insertData(uint8_t RxData,struct buffer* Packet){

	if(Packet->lstPos == BUFSIZE - 1){
		Packet->data[Packet->lstPos] = RxData;
		Packet->lstPos = 0;
	}else{
		Packet->data[Packet->lstPos++] = RxData;
	}
	
	if(Packet->len != BUFSIZE)
		Packet->len++;
	else
		Packet->fstPos = Packet->lstPos;
}

uint8_t count = 0;
uint8_t State = 0;
uint8_t mode  = PACKET_PRE;
uint8_t  packet_i		 = 0;
uint8_t  packet_type = 0;
uint32_t packet_pre  = 0;
uint32_t packet_len  = 0;
uint32_t len = 0;
int Serial_Flag = 0;

/**
 * @brief 将串口中断的状态清0，同时将Serial_Flag置1
 */
void clear(){
	count = 0;
	State = 0;
	mode  = PACKET_PRE;
	
	packet_i		= 0;
	packet_pre  = 0;
	packet_len  = 0;
	
	Serial_Flag = 1;
}

/**
 * @brief 串口中断，状态机解析数据包
 */
void USART1_IRQHandler(void) 
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)		//判断是否是USART1的接收事件触发的中断
	{
		uint8_t RxData = USART_ReceiveData(USART1);				    //读取数据寄存器，存放在接收的数据变量
		
		switch(mode){
			/* 解析数据包头 */
			case PACKET_PRE:
				packet_pre = (packet_pre << 8) | RxData;
				if(packet_pre == PACKET_PRE_STR){
						mode = PACKET_TYPE;
				}
				break;
			/* 解析数据包类型 */
			case PACKET_TYPE:
				packet_type = RxData;
				mode = PACKET_LEN; 
				count = 0;          // 准备接收长度
        packet_len = 0;     // 重置长度变量
				break;
			/* 解析数据包长度 */
			case PACKET_LEN:
				count++;
				packet_len = (packet_len << 8) | RxData;
				if(count == 3){
					mode = PACKET_RCV;
					if(packet_len == 0)
						clear();
				}
				break;
			/* 接收数据包数据 */
			case PACKET_RCV:
				// 目前只接受UPDATE_PID SAVE_PID类型数据
				if((packet_type == UPDATE_PID) || (packet_type == SAVE_PID)){
							pid_param_buffer[packet_i++] = RxData;
							pid_param_buffer[packet_i]   = '\0';
				}

				if(-- packet_len == 0){
					clear();
				}
				break;
		}

		USART_ClearITPendingBit(USART1, USART_IT_RXNE);		//清除标志位
	}
}

/**
 * @brief 等待连接建立
 */
void Serial_WaitConnected(void){	
	while(ifConnected == 0){	
			Delay_ms(100);
	}
}

/**
 * @brief 移动Buffer第一个指针位置，即标志数据已使用
 * @param Packet     指定的Buffer
 * @param step       移动的步长
 */
uint8_t moveFstPos(struct buffer* Packet,uint8_t step){
	if(Packet->len < step){
		return 0;
	}
	if(Packet->fstPos + step >= BUFSIZE)
		Packet->fstPos = step - (BUFSIZE - Packet->fstPos);
	else
		Packet->fstPos += step;
	Packet->len -= step;
	return 1;
}

/**
 * @brief 获取并移动第一个数据
 * @param Packet     指定的Buffer
 */
char getFstData(struct buffer* Packet){
	while(Packet->len == 0);
	char data = Packet->data[Packet->fstPos];
	moveFstPos(Packet,1);
	return data;
}
 

/**
 * @brief 从Buffer中阻塞获取指定长度数据
 * @param Packet     指定的Buffer
 * @param size       需要获取的字节
 */
uint32_t __getBufferData(struct buffer* Packet, char* str,uint32_t size){
	
	uint32_t i;
	while(size > Packet->len){
		Delay_ms(100);
	}
	
	for(i=0;i<size;i++){
		str[i] = Packet->data[Packet->fstPos++];
		if(Packet->fstPos == BUFSIZE)
			Packet->fstPos = 0;
	}
	Packet->len -= size;
	str[i] = '\0';
	return size;
}

/**
 * @brief 从SocketBuffer中阻塞获取指定长度数据
 * @param str        存放的字符串数组
 * @param size       需要获取的字节
 */
void getSocketData(char* str, uint32_t size)
{	
	__getBufferData(&socketPacket, str, size);
}

/**
 * @brief 从Flash读取数据
 * @param type       数据包类型
 * @param packet_len 数据包长度
 */
void SendPacketHeader(uint8_t type, uint32_t packet_len){
	static uint8_t header[8] = {0xFE, 0xFE, 0xCD, 0xCD} ;
	
	header[4] = type;
	header[5] = (packet_len & 0x00FF0000) >> 16;
	header[6] = (packet_len & 0x0000FF00) >> 8;
	header[7] = packet_len & 0x000000FF;
	
	Serial_SendBytes(header, 8);
}
	
