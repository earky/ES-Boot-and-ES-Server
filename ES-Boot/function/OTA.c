#include "stm32f4xx.h"
#include "stm32f4xx_flash.h"
#include "OTA.h"
#include "Serial.h"
#include <stdio.h>
#include "Log.h"

/*
	在VM_addr处:
	0x      FFFF             ABCD 
		  version为FFFF     start_mode为V AB.CD
						(前1个字节为大版本，第2个字节为小版本)
*/
uint16_t start_mode;
uint16_t version;
uint32_t file_length = 0;

/* 获取Bootloader版本号 */
uint16_t GetVersion(void)
{
	return version;
}

uint8_t GetBigVersion(void)
{
	return (version >> 8);
}

uint8_t GetSmallVersion(void)
{
	return (version & 0x00FF);
}


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

/**
 * @brief 将version和mode初始化并且发送给服务器
 */
void VMInit(void)
{
	uint32_t data = 0;
	ReadFlash(VM_Addr, &data, 1);
	start_mode    = data >> 16;
	version = 0x0000FFFF & data; 
	
	SendPacketHeader(0x03, 0);
	// 通过TCP获取最新版本
	char str[6];

	getSocketData(str, 6);
	
	uint16_t new_Version = str[0] << 8 | str[1];
	file_length = str[2] << 24 | str[3] << 16 |  str[4] << 8 | str[5];
	Log("> new version is %d\n", new_Version);

	if(new_Version != version){
		Log("> Old Version is V%d.%d, New Version is V%hu.%hu, File Length is %d\r\n", 
														GetBigVersion(), GetSmallVersion(),new_Version >> 8, new_Version & 0x00FF, file_length);
		start_mode = Startup_Update;
		version = new_Version;
	}else{
		Log("> Device Version is V%hu.%hu\r\n", version >> 8, version & 0x00FF);
		start_mode = Startup_Normol;
	}

}

/**
 * @brief 将新的version和mode保存到FLASH
 */
void VMUpdate(void)
{	
	float values[9];;
	ReadFlash(PID_ADDR, (uint32_t*)values, 9);
	
	Erase_page(PID_ADDR, 10);
	uint32_t x = (version) | (start_mode << 16);
	
	WriteFlash(PID_ADDR, (uint32_t*)values, 9);
	WriteFlash(VM_Addr,(unsigned int *) &x, 1);
}


/**
 * @brief 让程序跳转到APP
 */
void JumpToApp(void)
{		
		uint32_t *boot_vector_table = (uint32_t*)(FLASH_BASE | APP_ADDR_SUC);
    __set_MSP(boot_vector_table[0]);
    
    // 跳转到 Bootloader 复位处理函数
    void (*boot_reset)(void) = (void (*)(void))(boot_vector_table[1]);
    boot_reset();
}

/**
 * @brief 下载并且更新程序
 */
void Update()
{
	/* 防止文件过大，bootloader卡死在Erase_page */
	if(file_length > MAX_FILE_SIZE){
			file_length = 0;
	}
	/*1.擦除目的地址*/
	Log("[STM32] : Start erase des flash......\r\n");
	Erase_page(Application_1_Addr, file_length);
	Log("[STM32] : Erase des flash down......\r\n");
	
	/*2.开始拷贝*/	
	char str[Packet_Len + 10];
	uint32_t i = 0, len = 0;
	Log("[STM32] : Start copy......\r\n");

	while(file_length)
	{		
			len = file_length >= Packet_Len ? Packet_Len : file_length;
			SendPacketHeader(0x01,0);
				
			getSocketData(str, len);
			WriteFlash((Application_1_Addr + i*Packet_Len), (unsigned int *) str, len/4);
			
			i++;
			file_length -= len;		
	}
		
	Log("[STM32] : Copy down......\r\n");
}

/**
 * @brief 发送OTA设备ID
 */
void SendOTADeviceID(void)
{
	SendPacketHeader(0x02, 8);
	Serial_SendBytes(OTA_Device_ID, 8);
	
}

/**
 * @brief 启动BootLoader
 */
void Start_BootLoader(void)
{

		SendOTADeviceID();
		VMInit();
	
    switch(start_mode) {
        case Startup_Normol:
            Log("[STM32] : Normal start...\r\n");
            break;
            

				case Startup_Update:
					 Log("[STM32] : Start update...\r\n");
						Update();
						VMUpdate();
            Log("[STM32] : Update complete\r\n");
            break;
       
        default:
            Log("[STM32] : ERROR: 0x%X\r\n", start_mode);
            return;
    }
    
    Log("[STM32] : Starting application...\r\n\r\n");

		JumpToApp();
}
