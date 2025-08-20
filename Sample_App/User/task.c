#include "task.h"
#include "os.h"
#include "PPM.h"

struct GY86 gy86;
uint8_t gy86_Count = 0;
OS_STK TaskGy86_Stk[StkSize];
OS_STK TaskServo_Stk[StkSize];
OS_STK TaskWifi_Stk[StkSize];

OS_EVENT* sem;
void TaskInit(void){
		
		sem = OSSemCreate(1);
    OSTaskCreate(TaskGetGy86, (void *)0, &TaskGy86_Stk[StkSize-1] , 5);//创建其他任务App1Task
    OSTaskCreate(TaskServo  , (void *)0, &TaskServo_Stk[StkSize-1], 6);//创建其他任务App2Task
		OSTaskCreate(TaskWifi   , (void *)0, &TaskWifi_Stk[StkSize-1] , 7);//创建其他任务App3Task
}

void TaskGetGy86(void* p_arg){
	INT8U err;
	while(1){
		if(gy86_Count == 0){
				OSSemPend(sem, 0, &err);
		}
		GetData(&gy86);
		gy86_Count++;
		if(gy86_Count == GY86_MAX_COUNT){
				OSSemPost(sem);
				gy86_Count = 0;
		}
		OSTimeDly(SERVO_DELAY_TICKS);
	}
}

char servo_char[60];
void TaskServo(void* p_arg){
	while(1){
		sprintf(servo_char, "%s: %d %d %d %d %d %d %d %d\n","servo", PPM_Databuf[0], PPM_Databuf[1], PPM_Databuf[2], PPM_Databuf[3], PPM_Databuf[4], PPM_Databuf[5], PPM_Databuf[6], PPM_Databuf[7]);
		Serial_SendString(servo_char, strlen(servo_char));
		//Serial_SendBytes(PPM_Databuf, sizeof(PPM_Databuf));
		OSTimeDly(GY86_DELAY_TICKS);
	}
}

char gy86_char[60];
void TaskWifi(void* p_arg){
	INT8U err;
	
	while(1){
		OSSemPend(sem, 0, &err);
		sprintf(gy86_char, "%s %d %d %d %d %d %d %d %d %d %lf %lf\n", "gy86",gy86.AccX, gy86.AccY, gy86.AccZ, gy86.GaX, gy86.GaY, gy86.GaZ, gy86.GyroX, gy86.GyroY, gy86.GyroZ, gy86.Temperature, gy86.Pressure);
		Serial_SendString(gy86_char, strlen(gy86_char));
//		Serial_SendByte(0xFF);
//		Serial_SendByte(0xFE);
//		
//		Serial_SendBytes(&gy86, sizeof(gy86));
//		
//		Serial_SendByte(0xFE);
//		Serial_SendByte(0xFF);
		OSSemPost(sem);
	}
}
