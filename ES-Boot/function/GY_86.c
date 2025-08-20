#include "stm32f4xx.h"                  // Device header
#include "GY_86_Reg.h"
#include "stm32f4xx_i2c.h"
#include "Delay.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "USART.h"

//#define I2C1 I2C1

void MPU6050_WaitEvent(uint32_t I2C_EVENT)
{
	uint32_t Timeout;
	Timeout = 10000;
	while (!I2C_CheckEvent(I2C1, I2C_EVENT))
	{
		Timeout --;
		if (Timeout == 0)
		{
			GPIO_SetBits(GPIOA,GPIO_Pin_0);
            break;
		}
	}
}


void GY_86_I2C_Init()
{   
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct;
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;         
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;       
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;     
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);    
    
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource8,GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource9,GPIO_AF_I2C1);
    
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,DISABLE);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
    
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,ENABLE);
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,DISABLE);
    }
    
    I2C_InitTypeDef I2C_InitStruct;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStruct.I2C_ClockSpeed = 100000;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_Init(I2C1,&I2C_InitStruct);
    
//    I2C_SoftwareResetCmd(I2C1,ENABLE); 
//    I2C_SoftwareResetCmd(I2C1,DISABLE); 
    
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,ENABLE);
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,DISABLE);
    }
      
    I2C_Cmd(I2C1,ENABLE);
}

void GY_86_WriteReg(uint8_t HardwareAddress,uint8_t RegAddress,uint8_t data)
{
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
//        I2C1->CR1 |= (1<<15);
//        I2C1->CR1 &= ~(1<<15);
    }
    
    I2C_GenerateSTART(I2C1,ENABLE);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT);
    
    I2C_Send7bitAddress(I2C1,HardwareAddress,I2C_Direction_Transmitter);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
    
    I2C_SendData(I2C1,RegAddress);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
    
    I2C_SendData(I2C1,data);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
    
    I2C_GenerateSTOP(I2C1,ENABLE);
}

uint8_t GY_86_ReadReg(uint8_t HardwareAddress,uint8_t RegAddress)
{
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,ENABLE);
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,DISABLE);
    }
    
    I2C_GenerateSTART(I2C1,ENABLE);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT);
    
    I2C_Send7bitAddress(I2C1,HardwareAddress,I2C_Direction_Transmitter);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
    
    I2C_SendData(I2C1,RegAddress);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
    
    I2C_GenerateSTART(I2C1,ENABLE);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT);
    
    I2C_Send7bitAddress(I2C1,HardwareAddress,I2C_Direction_Receiver);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
    
    I2C_AcknowledgeConfig(I2C1,DISABLE);
    I2C_GenerateSTOP(I2C1,ENABLE);
    
    MPU6050_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED);
    uint8_t data = I2C_ReceiveData(I2C1);
    
    I2C_AcknowledgeConfig(I2C1,ENABLE);
    
    return data;
}

void GY_86_Init(uint16_t* MS5611_PROM);

void MPU6050_Init()
{   
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,ENABLE);
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,DISABLE);
    }
    
    GY_86_WriteReg(MPU6050_ADDRESS,MPU6050_PWR_MGMT_1,0x01);// 0不复位 0解除睡眠 0不需要循环 0 0温度传感器不失能 001陀螺仪时钟
    GY_86_WriteReg(MPU6050_ADDRESS,MPU6050_PWR_MGMT_2,0x00);// 00循环模式唤醒频率 000000每个轴的待机位，这里设置6个轴均不待机   
    GY_86_WriteReg(MPU6050_ADDRESS,MPU6050_SMPLRT_DIV,0x09);// 采样率分频，越小越快，这里设置为10
    GY_86_WriteReg(MPU6050_ADDRESS,MPU6050_CONFIG,0x06);//000 00 110平滑滤波（11最大）
    GY_86_WriteReg(MPU6050_ADDRESS,MPU6050_GYRO_CONFIG,0x18);//000自测使能 11陀螺仪，满量程选择（根据实际）000
    GY_86_WriteReg(MPU6050_ADDRESS,MPU6050_ACCEL_CONFIG,0x18);
}
void MPU6050_GetData(int16_t *data)
{
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,ENABLE);
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1,DISABLE);
    }
    
    I2C_GenerateSTART(I2C1,ENABLE);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT);
    
    I2C_Send7bitAddress(I2C1,MPU6050_ADDRESS,I2C_Direction_Transmitter);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
    
    I2C_SendData(I2C1,MPU6050_ACCEL_XOUT_H);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING);
    
    I2C_GenerateSTART(I2C1,ENABLE);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT);
    
    I2C_Send7bitAddress(I2C1,MPU6050_ADDRESS,I2C_Direction_Receiver);
    MPU6050_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
    
    uint8_t datah,datal;
    for(int i=0;i<7;++i)
    {
        MPU6050_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED);
        datah = I2C_ReceiveData(I2C1);
        if(i==7) I2C_AcknowledgeConfig(I2C1,DISABLE);
        else I2C_AcknowledgeConfig(I2C1,ENABLE);
        MPU6050_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED);
        datal = I2C_ReceiveData(I2C1);
        I2C_AcknowledgeConfig(I2C1,ENABLE);
        data[i] = ((int16_t)datah<<8) | datal;
    }

    I2C_GenerateSTOP(I2C1,ENABLE);
    I2C_AcknowledgeConfig(I2C1,ENABLE);
}

void HMC5883L_Init();
void HMC5883_GetData(int16_t *data);

void MS5611_Init();
int32_t MS5611_ReadTempData();
int32_t MS5611_ReadAtmData();













