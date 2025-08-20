#ifndef __GY86_H
#define __GY86_H

#include "stm32f4xx.h"

enum ModuleType{
	MPU6050  = 0,
	MS5611   = 1,
	HMC5883L = 2
};

struct GY86
{
	int16_t AccX,AccY,AccZ,GyroX,GyroY,GyroZ,GaX,GaY,GaZ;
	//温度为当前摄氏度*100  气压为当前Mkpa*100
	float Pressure,Temperature;
};


void GY86Init(void);
void GetData(struct GY86* gy86);

#endif
