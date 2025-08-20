#ifndef __OTA_H
#define __OTA_H

#define PageSize		FLASH_PAGE_SIZE			//1K

#define BootLoader_Size 		0x5000U			// BootLoader的大小
#define Application_Size		0xA000U			// 应用程序的大小

#define APP_ADDR_SUC					0x00004000UL	// 应用程序地址后缀
#define Application_1_Addr		0x08004000U		// 应用程序的首地址

#define MAX_FILE_SIZE   60000u

#define VM_Addr       0x0807FFFC
#define PID_ADDR      0x0807FFD8
/* 启动的步骤 */
#define Startup_Normol   0xFFFF	// 正常启动
#define Startup_Update   0xAAAA	// 升级完成

//#define Startup_Updating 0xBBBB	// 正在升级

#define Packet_Len 512

#define OTA_Device_ID	 "12jdasdx"

void Start_BootLoader(void);
uint16_t GetVersion(void);
uint8_t GetBigVersion(void);
uint8_t GetSmallVersion(void);


#endif
