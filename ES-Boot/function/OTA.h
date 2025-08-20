#ifndef __OTA_H
#define __OTA_H

#define PageSize		FLASH_PAGE_SIZE			//1K

#define BootLoader_Size 		0x5000U			// BootLoader�Ĵ�С
#define Application_Size		0xA000U			// Ӧ�ó���Ĵ�С

#define APP_ADDR_SUC					0x00004000UL	// Ӧ�ó����ַ��׺
#define Application_1_Addr		0x08004000U		// Ӧ�ó�����׵�ַ

#define MAX_FILE_SIZE   60000u

#define VM_Addr       0x0807FFFC
#define PID_ADDR      0x0807FFD8
/* �����Ĳ��� */
#define Startup_Normol   0xFFFF	// ��������
#define Startup_Update   0xAAAA	// �������

//#define Startup_Updating 0xBBBB	// ��������

#define Packet_Len 512

#define OTA_Device_ID	 "12jdasdx"

void Start_BootLoader(void);
uint16_t GetVersion(void);
uint8_t GetBigVersion(void);
uint8_t GetSmallVersion(void);


#endif
