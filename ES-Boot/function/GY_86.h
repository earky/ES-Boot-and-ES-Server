#ifndef __GY_86_H__
#define __GY_86_H__

void MPU6050_Init();
void MPU6050_GetData(int16_t *data);
void GY_86_I2C_Init();
void MPU6050_WaitEvent(uint32_t I2C_EVENT);


#endif
