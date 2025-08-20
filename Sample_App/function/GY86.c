#include<stdio.h>
#include<stdlib.h>
#include "GY86.h"
#include "GY86_Reg.h"
#include "MyI2C.h"
#include "stm32f4xx.h"
#include "Delay.h"




uint8_t ADDRESS = MPU6050_ADDRESS;//当前操控模块的地址
uint8_t DATA_ADDRESS = MPU6050_ACCEL_XOUT_H;//当前操控模块需要数据所在的地址
uint8_t DATA_LENGTH = 7;//当前操控模块数据区域长度
uint16_t Cal_c[7];      //MS5611中PROM的校准数据


//更改当前操作的模块
void ChangeTarget(enum ModuleType module){
	//更改当前模块为6050
	if(module == MPU6050){
		ADDRESS = MPU6050_ADDRESS;
        DATA_ADDRESS = MPU6050_ACCEL_XOUT_H;
        DATA_LENGTH = 7;
	}else if(module == MS5611){
		ADDRESS = MS561101BA_ADDR;
		DATA_ADDRESS = MS561101BA_PROM_BASE_ADDR;
		DATA_LENGTH = 7;
	}else if(module == HMC5883L){
		ADDRESS = HMC5883L_WRITE_ADDRESS;
		DATA_ADDRESS = HMC5883L_REG_OUT_X_M;
		DATA_LENGTH = 3;
	}
}

//改变基地址
void ChangeDS(uint8_t RegAddress)
{
	MyI2C_Start();
	MyI2C_SendByte(ADDRESS);
	checkAck();
	MyI2C_SendByte(RegAddress);
	checkAck();
	MyI2C_Stop();
}

//向寄存器中写入单个数据
void WriteReg(uint8_t RegAddress,uint8_t Data)
{
	MyI2C_Start();
	MyI2C_SendByte(ADDRESS);
	checkAck();
	MyI2C_SendByte(RegAddress);
	checkAck();
	MyI2C_SendByte(Data);
	checkAck();
	MyI2C_Stop();
}


void MPUInit(){
	ChangeTarget(MPU6050);
	WriteReg(MPU6050_PWR_MGMT_1, 0x01);		//电源管理寄存器1，取消睡眠模式，选择时钟源为X轴陀螺仪
	WriteReg(MPU6050_PWR_MGMT_2, 0x00);		//电源管理寄存器2，保持默认值0，所有轴均不待机
	WriteReg(MPU6050_SMPLRT_DIV, 0x7F);		//采样率分频寄存器，配置采样率
	WriteReg(MPU6050_CONFIG, 0x06);			//配置寄存器，配置DLPF
	WriteReg(MPU6050_GYRO_CONFIG, 0x18);	//陀螺仪配置寄存器，选择满量程为±2000°/s
	WriteReg(MPU6050_ACCEL_CONFIG, 0x18);	//加速度计配置寄存器，选择满量程为±16g
}

//HMC5883需要通过MPU的寄存器才可以读取
//MPU直通模式
void MPUPassMode(){
	ChangeTarget(MPU6050);
	WriteReg(MPU6050_INT_PIN_CFG,0x02);// INT_PIN_CFG中 I2C_BYPASS_EN置1
	WriteReg(MPU6050_USER_CTRL,0x00);  //MPU放出I2C控制权
}
//MPU主模式
void MPUMasMode(){
	ChangeTarget(MPU6050);
	WriteReg(MPU6050_INT_PIN_CFG,0x00);// INT_PIN_CFG中 I2C_BYPASS_EN置1
	WriteReg(MPU6050_USER_CTRL,0x22);	
}

void HMC5883LInit(){
	MPUPassMode();
	ChangeTarget(HMC5883L);
	WriteReg(HMC5883L_REG_CONFIG_A, 0X1A); //CRA7清除 采样平均值取8 输出速率15MHZ XYZ轴默认0
	WriteReg(HMC5883L_REG_CONFIG_B, 0X80); //设置1090/高斯
	WriteReg(HMC5883L_REG_MODE, 0X00); 	   //连续测量模式
	MPUMasMode();
}

void GetHMCData(int16_t arr[])
{
	MPUPassMode();
	ChangeTarget(HMC5883L);
	ChangeDS(DATA_ADDRESS);
	uint16_t DataH,DataL;
	MyI2C_Start();
	MyI2C_SendByte(HMC5883L_READ_ADDRESS);
	checkAck();
	
	for(int i=0;i<DATA_LENGTH;i++){
		DataH = MyI2C_ReceiveByte();
		MyI2C_SendAck(0);
		DataL = MyI2C_ReceiveByte();
		MyI2C_SendAck((i == DATA_LENGTH - 1));//当数据传输完成时 就需要发送1表示不需要继续传输数据了
		
        arr[i] = (DataH << 8) | (DataL);
	}
	MyI2C_Stop();
	MPUMasMode();
}

void MS5611Init(){
	ChangeTarget(MS5611);
	MyI2C_Start();
	MyI2C_SendByte(ADDRESS);
	checkAck();
	MyI2C_SendByte(MS561101BA_RESET);
	checkAck();
	MyI2C_Stop();

	Delay_ms(100);
	//读取PROM的数据
	uint16_t DataH,DataL;
	
	for (int i=0;i<=MS561101BA_PROM_REG_COUNT;i++) 
	{
		ChangeDS(DATA_ADDRESS + (i * MS561101BA_PROM_REG_SIZE));
		MyI2C_Start();
		MyI2C_SendByte(ADDRESS | 0x01);
		checkAck();
		DataH = MyI2C_ReceiveByte();
		MyI2C_SendAck(0);
		DataL = MyI2C_ReceiveByte();
		MyI2C_SendAck(1);
        Cal_c[i] = (DataH << 8) | (DataL);
	}
	MyI2C_Stop();
}

void GetMPUData(int16_t arr[])
{
	ChangeTarget(MPU6050);
	ChangeDS(DATA_ADDRESS);
	uint16_t DataH,DataL;
    
	MyI2C_Start();
	MyI2C_SendByte(ADDRESS | 0x01);
	checkAck();

	for(int i=0;i<DATA_LENGTH;i++){
		DataH = MyI2C_ReceiveByte();
		MyI2C_SendAck(0);
		DataL = MyI2C_ReceiveByte();
		MyI2C_SendAck((i == DATA_LENGTH - 1));//当数据传输完成时 就需要发送1表示不需要继续传输数据了
		
    arr[i] = (DataH << 8) | (DataL);
	}
	MyI2C_Stop();
}

//转换数据
uint32_t MS5611Conversion(uint8_t command)
{
	uint32_t conversion;
	MyI2C_Start();
	MyI2C_SendByte(ADDRESS);
	checkAck();
	MyI2C_SendByte(command);
	checkAck();
	MyI2C_Stop();

	Delay_ms(10);//延时,去掉数据错误


	uint32_t data[3] = {0};
	MyI2C_Start();
	MyI2C_SendByte(ADDRESS);
	checkAck();
	MyI2C_SendByte(0);
	checkAck();
	MyI2C_Start();
	MyI2C_SendByte(ADDRESS | 0x01);
	checkAck();
	for(int i=0;i<3;i++){
		data[i] = MyI2C_ReceiveByte();
		MyI2C_SendAck(i == 2);
	}
	MyI2C_Stop();

	conversion=(data[0] << 16) | (data[1] << 8) | data[2];
	return conversion;
}

//温度为当前摄氏度*100  气压为当前Mkpa*100
void GetMS5611Data(float arr[]){
	ChangeTarget(MS5611);
	uint32_t D1_Pres=0;
	uint32_t D2_Temp=0;
	float Pressure=0;
	float dT=0;
	float Temperature=0;
	float Temperature2=0;
	double OFF;
	double SENS=0;
	double Aux=0;
	double OFF2=0;
	double SENS2=0;
	uint32_t ex_Pressure=0;
	uint8_t  exchange_num[8];
	
	//读取数字温度
	Temperature2 = MS5611Conversion(MS561101BA_D2_OSR_4096);	
	
 	Delay_ms(100);
 	dT=Temperature2 - (((uint32_t)Cal_c[5])<<8);
 	Temperature=2000+dT*((uint32_t)Cal_c[6])/8388608;	//算出温度值的100倍，2001表示20.01°

	D1_Pres = MS5611Conversion(MS561101BA_D1_OSR_4096);
	Delay_ms(100);

	OFF=(uint32_t)(Cal_c[2]<<16)+((uint32_t)Cal_c[4]*dT)/128.0;
	SENS=(uint32_t)(Cal_c[1]<<15)+((uint32_t)Cal_c[3]*dT)/256.0;
   //温度补偿
	if(Temperature < 2000)// second order temperature compensation when under 20 degrees C
	{
		Temperature2 = (dT*dT) / 0x80000000;
		Aux = (Temperature-2000)*(Temperature-2000);
		OFF2 = 2.5*Aux;
		SENS2 = 1.25*Aux;
		if(Temperature < -1500)
		{
			Aux = (Temperature+1500)*(Temperature+1500);
			OFF2 = OFF2 + 7*Aux;
			SENS2 = SENS + 5.5*Aux;
		}
	}else  //(Temperature > 2000)
	{
		Temperature2 = 0;
		OFF2 = 0;
		SENS2 = 0;
	}

	Temperature = Temperature - Temperature2;
	OFF = OFF - OFF2;
	SENS = SENS - SENS2;	

	Pressure=(D1_Pres*SENS/2097152.0-OFF)/32768.0;

	arr[0] = Pressure;
	arr[1] = Temperature;
}	


void GetHMC5883LData(int16_t arr[])
{
	ChangeTarget(HMC5883L);
	ChangeDS(DATA_ADDRESS);
	uint16_t DataH,DataL;
    
	MyI2C_Start();
	MyI2C_SendByte(ADDRESS | 0x01);
	checkAck();

	for(int i=0;i<DATA_LENGTH;i++){
		DataH = MyI2C_ReceiveByte();
		MyI2C_SendAck(0);
		DataL = MyI2C_ReceiveByte();
		MyI2C_SendAck((i == DATA_LENGTH - 1));//当数据传输完成时 就需要发送1表示不需要继续传输数据了
		
    arr[i] = (DataH << 8) | (DataL);
	}
	MyI2C_Stop();
}


void GetData(struct GY86* gy86)
{
	int16_t arr[7];
	float brr[2];
	GetMPUData(arr);
	gy86->AccX = arr[0];
	gy86->AccY = arr[1];
	gy86->AccZ = arr[2];
	gy86->GaX  = arr[4];
	gy86->GaY  = arr[5];
	gy86->GaZ  = arr[6];
	GetMS5611Data(brr);
	gy86->Pressure = brr[0];
	gy86->Temperature = brr[1];
	GetHMCData(arr);
	gy86->GyroX = arr[0];
	gy86->GyroZ = arr[1];
	gy86->GyroY = arr[2];
}

void GY86Init(){
	MPUInit();
	MS5611Init();
	HMC5883LInit();
}


