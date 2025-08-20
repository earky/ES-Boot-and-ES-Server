#include "PID.h"
#include <stdlib.h>
#include <string.h>

PID_param pid_param;
char pid_param_buffer[128];

/**
 * @brief 获取FLASH的区块号
 * @param addr       转换地址
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
    return FLASH_Sector_7; // 默认返回最后扇区
}

/**
 * @brief 擦除指定范围的Flash
 * @param start_addr 起始地址
 * @param byte_size  要擦除的字节数
 * @return 1:成功 0:失败
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
            return 0; // 擦除失败
        }
    }
    
    FLASH_Lock();
    return 1; // 成功
}

/**
 * @brief 写数据到Flash
 * @param addr       写入地址
 * @param buff       数据缓冲区
 * @param word_size  要写入的字数(32位)
 */
static void WriteFlash(uint32_t addr, uint32_t *buff, int word_size)
{   
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                   FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    
    for(int i = 0; i < word_size; i++) {
        if(FLASH_ProgramWord(addr + 4*i, buff[i]) != FLASH_COMPLETE) {
						Log("> Flass Write Failed\r\n");
            break; // 写入失败
        }
    }
    
    FLASH_Lock();
}

/**
 * @brief 从Flash读取数据
 * @param addr       读取地址
 * @param buff       数据缓冲区
 * @param word_size  要读取的字数(32位)
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
 * @brief 用于测试进入APP的PID数据
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
 * @brief 给后端发送初始化PID的数据
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
 * @brief 更新PID数据
 */
void update_PID(){
	
		for(int i=0;i<9;i++){
			memcpy(params[i], &pid_param_buffer[i*4], sizeof(float));
		}

		/* 这里不能用Log,vsnprintf拼接浮点数改为大端显示了？ */
		/* 打印提示信息 */
		char str[100];
		sprintf(str, "update PID to %f %f %f %f %f %f %f %f %f\n",
						pid_param.Pitch_P, pid_param.Pitch_I, pid_param.Pitch_D,
						pid_param.Yaw_P,   pid_param.Yaw_I, 	pid_param.Yaw_D,
						pid_param.Roll_P,  pid_param.Roll_I,  pid_param.Roll_D);
		SendPacketHeader(PRINT, strlen(str));
		Serial_SendString(str);
}

/**
 * @brief 保存PID数据
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