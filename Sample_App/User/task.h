#ifndef __TASK_H
#define __TASK_H

#include "GY86.h"
#include "os.h"

#define StkSize 128

#define GY86_DELAY_TICKS 100u
#define GY86_MAX_COUNT 10	//此变量用于多少个gy86间隔上传
#define SERVO_DELAY_TICKS 1000u

extern struct GY86 gy86;
//extern OS_STK TaskGy86_Stk[StkSize];
//extern OS_STK TaskServo_Stk[StkSize];
//extern OS_STK TaskWifi_Stk[StkSize];

void TaskGetGy86(void* p_arg);
void TaskServo(void* p_arg);
void TaskWifi(void* p_arg);
void TaskInit(void);

#endif
