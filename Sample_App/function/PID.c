#include "PID.h"
#include <stdlib.h>
#include <string.h>

PID_param pid_param;
char pid_param_buffer[128];

/**
 * @brief ��ȡFLASH�������
 * @param addr       ת����ַ
 */
static uint32_t GetSector(uint32_t Address)
{
    if     (Address < 0x08004000) return FLASH_Sector_0;
    else if(Address < 0x08008000) return FLASH_Sector_1;
    else if(Address < 0x0800C000) return FLASH_Sector_2;
    else if(Address < 0x08010000) return FLASH_Sector_3;
    else if(Address < 0x08020000) return FLASH_Sector_4;
    else if(Address < 0x08040000) return FLASH_Sector_5;
    else if(Address < 0x08060000) return FLASH_Sector_6;
    else if(Address < 0x08080000) return FLASH_Sector_7;
    return FLASH_Sector_7; // Ĭ�Ϸ����������
}

/**
 * @brief ����ָ����Χ��Flash
 * @param start_addr ��ʼ��ַ
 * @param byte_size  Ҫ�������ֽ���
 * @return 1:�ɹ� 0:ʧ��
 */
static int Erase_page(uint32_t start_addr, uint32_t byte_size)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                   FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    
    uint32_t start_sector = GetSector(start_addr);
    uint32_t end_addr = start_addr + byte_size - 1;
    uint32_t end_sector = GetSector(end_addr);
    
    for(uint32_t i = start_sector; i <= end_sector; i++) {
			

        if(FLASH_EraseSector(i, VoltageRange_3) != FLASH_COMPLETE) {
            FLASH_Lock();
						Log("erase failed!");
            return 0; // ����ʧ��
        }
    }
    
    FLASH_Lock();
    return 1; // �ɹ�
}

/**
 * @brief д���ݵ�Flash
 * @param addr       д���ַ
 * @param buff       ���ݻ�����
 * @param word_size  Ҫд�������(32λ)
 */
static void WriteFlash(uint32_t addr, uint32_t *buff, int word_size)
{   
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                   FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    
    for(int i = 0; i < word_size; i++) {
        if(FLASH_ProgramWord(addr + 4*i, buff[i]) != FLASH_COMPLETE) {
						Log("> Flass Write Failed\r\n");
            break; // д��ʧ��
        }
    }
    
    FLASH_Lock();
}

/**
 * @brief ��Flash��ȡ����
 * @param addr       ��ȡ��ַ
 * @param buff       ���ݻ�����
 * @param word_size  Ҫ��ȡ������(32λ)
 */
static void ReadFlash(uint32_t addr, uint32_t *buff, uint16_t word_size)
{
    for(int i = 0; i < word_size; i++) {
        buff[i] = *(__IO uint32_t*)(addr + 4*i);
    }
}

float *params[] = {
		&pid_param.Pitch_P, &pid_param.Pitch_I, &pid_param.Pitch_D,
		&pid_param.Yaw_P,   &pid_param.Yaw_I,   &pid_param.Yaw_D,
		&pid_param.Roll_P,  &pid_param.Roll_I,  &pid_param.Roll_D
};

/**
 * @brief ���ڲ��Խ���APP��PID����
 */
void log_init_PID(){
		float values[9];
		ReadFlash(PID_ADDR, (uint32_t*)values, 9);		
		
		for(int i=0;i<9;i++){
				*params[i] = values[i];
		}
		
		char msg[256];
		for(int i=0;i<9;i++){
			sprintf(msg, "%f", *params[i]);
			Log("> __init_pid to %s\n", msg);
		}
}

/**
 * @brief ����˷��ͳ�ʼ��PID������
 */
void init_PID(){
		char msg[128];
		sprintf(msg, "%f %f %f %f %f %f %f %f %f",
						pid_param.Pitch_P, pid_param.Pitch_I, pid_param.Pitch_D,
						pid_param.Yaw_P,   pid_param.Yaw_I,   pid_param.Yaw_D,
						pid_param.Roll_P,  pid_param.Roll_I,  pid_param.Roll_D);
		int len = strlen(msg);
		SendPacketHeader(INIT_PID, len);
		Serial_SendString(msg);
		Log("[STM32] : init_pid to %s, len is %d\n", msg, len);
}

/**
 * @brief ����PID����
 */
void update_PID(){
	
		for(int i=0;i<9;i++){
			memcpy(params[i], &pid_param_buffer[i*4], sizeof(float));
		}

		/* ���ﲻ����Log,vsnprintfƴ�Ӹ�������Ϊ�����ʾ�ˣ� */
		/* ��ӡ��ʾ��Ϣ */
		char str[100];
		sprintf(str, "update PID to %f %f %f %f %f %f %f %f %f\n",
						pid_param.Pitch_P, pid_param.Pitch_I, pid_param.Pitch_D,
						pid_param.Yaw_P,   pid_param.Yaw_I, 	pid_param.Yaw_D,
						pid_param.Roll_P,  pid_param.Roll_I,  pid_param.Roll_D);
		SendPacketHeader(PRINT, strlen(str));
		Serial_SendString(str);
}

/**
 * @brief ����PID����
 */
void save_PID(){
		float values[9];
		
		for(int i=0;i<9;i++){
			memcpy(params[i], &pid_param_buffer[i*4], sizeof(float));
			values[i] = *params[i];
		}
		
		uint32_t VM;
		ReadFlash(VM_ADDR, &VM, 1);
		
		Erase_page(PID_ADDR, 10);
		WriteFlash(PID_ADDR, (uint32_t*)values, 9);
		WriteFlash(VM_ADDR,(unsigned int *)&VM, 1);
		
		char str[100];
		sprintf(str, "save PID as %f %f %f %f %f %f %f %f %f\n",
						pid_param.Pitch_P, pid_param.Pitch_I, pid_param.Pitch_D,
						pid_param.Yaw_P,   pid_param.Yaw_I, 	pid_param.Yaw_D,
						pid_param.Roll_P,  pid_param.Roll_I,  pid_param.Roll_D);
		SendPacketHeader(PRINT, strlen(str));
		Serial_SendString(str);
}