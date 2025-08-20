#include "stm32f4xx.h"                  // Device header
#include "Delay.h"
#include "GY86.h"
#include "MyI2C.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include <stdlib.h>
#include <string.h> 
#include "Serial.h"
#include "Log.h"
#include "PID.h"

/**
 * @brief 跳转到Bootloader
 */
void JumpToBootloader(void){

			uint32_t *boot_vector_table = (uint32_t*)(FLASH_BASE);
			__set_MSP(boot_vector_table[0]);
			
			// 跳转到 Bootloader 复位处理函数
			void (*boot_reset)(void) = (void (*)(void))(boot_vector_table[1]);
			boot_reset();

}

int main(){
	// 更改中断向量表位置
	SCB->VTOR = FLASH_BASE | 0x00004000UL;
	__enable_irq();	//允许中断
	
	Serial_Init();
	log_init_PID();
	char msg[512];
	
	// 用于测试无人机姿态
	float pitch = 0, yaw = 0, roll = 0;
	float dt = 0.5;
	
	int servo_arr[8] = {113, 3276, 457, 4235, 346, 7658, 4572, 3265};
	while(1){
			
			sprintf(msg, "%d %d %d %d %d %d %d %d %d %f %f %f %f %f",
							0, 0, 0, 0, 0, 0, 0, 0, 0, 1000.00, 2000.00,
							pitch, yaw, roll);
			
			pitch += dt;
			yaw   += dt;
			roll  += dt;
		
			if(pitch > 45){
				dt = -dt;
			}else if(pitch == 0){
				dt = -dt;
			}
			
			// 发送GY86类型数据
			SendPacketHeader(GY86_DATA, strlen(msg));
			Serial_SendString(msg);
			Log("[STM32] : msg is %s\n", msg);
			
			// 发送舵机类型数据
			sprintf(msg, "%d %d %d %d %d %d %d %d",
								servo_arr[0], servo_arr[1], servo_arr[2], servo_arr[3],
								servo_arr[4], servo_arr[5], servo_arr[6], servo_arr[7]);
			SendPacketHeader(SERVO_DATA, strlen(msg));
			Serial_SendString(msg);
			
			
			// 状态机处理
			if(Serial_GetRxFlag()){
				switch(packet_type){
					case RESET_TYPE:
						JumpToBootloader();
						break;
					case UPDATE_PID:
						update_PID();
						break;
					case SAVE_PID:
						save_PID();
						break;
					case INIT_PID:
						init_PID();
					default:
						break;
				}
				packet_type = 0;
			}
			
			Delay_ms(1000);
	}
}


