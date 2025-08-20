#include "stm32f4xx.h"                  // Device header
#include "Delay.h"
#include "OTA.h"
#include "Serial.h"


int main(){
	/* ������������ΪFLASH_BASE,ͬʱ���ж� */
	SCB->VTOR = FLASH_BASE;
	__enable_irq();
	
	Serial_Init();
	Serial_SendString("+++");	//�˳���������ģʽ������ATָ��ģʽ
	Delay_ms(500);						//�ȴ�ģʽ�˳���ͬʱ��ֹģ���һ���ϵ緢�͵���Ϣ������
	
	/* �ж��Ƿ���������TCP_server,���ӵ�����ֱ�ӽ���Bootloader����OTA�����֤ */
	mode = CIPSTATE;					
	Serial_SendString("AT+CIPSTATE?\r\n");
	Delay_ms(500);
	
	if(cipstate_count > 10){
		mode = WIFI_TCP;	
		State = 1;
		Serial_SendString("AT+CIPSEND\r\n");	//���뷢������ģʽ
	}else{
		mode = NONE;
		Serial_SendString("AT+RST\r\n");	    //����
	}
		
	Serial_WaitConnected();
	Start_BootLoader();
	
	while(1){
			Serial_SendString("Bootloader started failed\r\n");
			Delay_ms(100);
	}
}


