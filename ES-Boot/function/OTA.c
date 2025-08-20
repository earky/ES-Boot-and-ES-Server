#include "stm32f4xx.h"
#include "stm32f4xx_flash.h"
#include "OTA.h"
#include "Serial.h"
#include <stdio.h>
#include "Log.h"

/*
	��VM_addr��:
	0x      FFFF             ABCD 
		  versionΪFFFF     start_modeΪV AB.CD
						(ǰ1���ֽ�Ϊ��汾����2���ֽ�ΪС�汾)
*/
uint16_t start_mode;
uint16_t version;
uint32_t file_length = 0;

/* ��ȡBootloader�汾�� */
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

/**
 * @brief ��version��mode��ʼ�����ҷ��͸�������
 */
void VMInit(void)
{
	uint32_t data = 0;
	ReadFlash(VM_Addr, &data, 1);
	start_mode    = data >> 16;
	version = 0x0000FFFF & data; 
	
	SendPacketHeader(0x03, 0);
	// ͨ��TCP��ȡ���°汾
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
 * @brief ���µ�version��mode���浽FLASH
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
 * @brief �ó�����ת��APP
 */
void JumpToApp(void)
{		
		uint32_t *boot_vector_table = (uint32_t*)(FLASH_BASE | APP_ADDR_SUC);
    __set_MSP(boot_vector_table[0]);
    
    // ��ת�� Bootloader ��λ������
    void (*boot_reset)(void) = (void (*)(void))(boot_vector_table[1]);
    boot_reset();
}

/**
 * @brief ���ز��Ҹ��³���
 */
void Update()
{
	/* ��ֹ�ļ�����bootloader������Erase_page */
	if(file_length > MAX_FILE_SIZE){
			file_length = 0;
	}
	/*1.����Ŀ�ĵ�ַ*/
	Log("[STM32] : Start erase des flash......\r\n");
	Erase_page(Application_1_Addr, file_length);
	Log("[STM32] : Erase des flash down......\r\n");
	
	/*2.��ʼ����*/	
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
 * @brief ����OTA�豸ID
 */
void SendOTADeviceID(void)
{
	SendPacketHeader(0x02, 8);
	Serial_SendBytes(OTA_Device_ID, 8);
	
}

/**
 * @brief ����BootLoader
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
