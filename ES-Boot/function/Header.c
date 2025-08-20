#include "Header.h"

void HardwareInit(void){
	
	OLED_Init();
	MyI2C_Init();
				
	GY86Init();

	PPM_Init();
	Serial_Init();

	//TIM_Delay_Init();
	
	Serial_SendString("AT+RST\r\n");	//先重启才能接收到蓝牙是否成功连接的标志
	Serial_WaitConnected();
}