#include "Header.h"

void HardwareInit(void){
	
	OLED_Init();
	MyI2C_Init();
				
	GY86Init();

	PPM_Init();
	Serial_Init();

	//TIM_Delay_Init();
	
	Serial_SendString("AT+RST\r\n");	//���������ܽ��յ������Ƿ�ɹ����ӵı�־
	Serial_WaitConnected();
}