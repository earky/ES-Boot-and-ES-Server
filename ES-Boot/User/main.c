#include "stm32f4xx.h"                  // Device header
#include "Delay.h"
#include "OTA.h"
#include "Serial.h"


int main(){
	/* 将向量表重置为FLASH_BASE,同时打开中断 */
	SCB->VTOR = FLASH_BASE;
	__enable_irq();
	
	Serial_Init();
	Serial_SendString("+++");	//退出传输数据模式，进入AT指令模式
	Delay_ms(500);						//等待模式退出，同时防止模块第一次上电发送的信息被接收
	
	/* 判断是否链接上了TCP_server,连接到了则直接进入Bootloader进行OTA身份验证 */
	mode = CIPSTATE;					
	Serial_SendString("AT+CIPSTATE?\r\n");
	Delay_ms(500);
	
	if(cipstate_count > 10){
		mode = WIFI_TCP;	
		State = 1;
		Serial_SendString("AT+CIPSEND\r\n");	//进入发送数据模式
	}else{
		mode = NONE;
		Serial_SendString("AT+RST\r\n");	    //重启
	}
		
	Serial_WaitConnected();
	Start_BootLoader();
	
	while(1){
			Serial_SendString("Bootloader started failed\r\n");
			Delay_ms(100);
	}
}


