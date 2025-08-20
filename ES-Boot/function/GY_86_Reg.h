#ifndef __GY_86_REG_H__
#define __GY_86_REG_H__

#define MPU6050_ADDRESS     0xD0
#define HMC5883L_ADDRESS    0x3C
#define MS5611_ADDRESS      0xEE

#define	MPU6050_SMPLRT_DIV		0x19
#define	MPU6050_CONFIG			0x1A
#define	MPU6050_GYRO_CONFIG		0x1B
#define	MPU6050_ACCEL_CONFIG	0x1C

#define	MPU6050_ACCEL_XOUT_H	0x3B
#define	MPU6050_ACCEL_XOUT_L	0x3C
#define	MPU6050_ACCEL_YOUT_H	0x3D
#define	MPU6050_ACCEL_YOUT_L	0x3E
#define	MPU6050_ACCEL_ZOUT_H	0x3F
#define	MPU6050_ACCEL_ZOUT_L	0x40
#define	MPU6050_TEMP_OUT_H		0x41
#define	MPU6050_TEMP_OUT_L		0x42
#define	MPU6050_GYRO_XOUT_H		0x43
#define	MPU6050_GYRO_XOUT_L		0x44
#define	MPU6050_GYRO_YOUT_H		0x45
#define	MPU6050_GYRO_YOUT_L		0x46
#define	MPU6050_GYRO_ZOUT_H		0x47
#define	MPU6050_GYRO_ZOUT_L		0x48

#define	MPU6050_PWR_MGMT_1		0x6B
#define	MPU6050_PWR_MGMT_2		0x6C
#define	MPU6050_WHO_AM_I		0x75

#define HMC5883L_READ_ADDRESS         0x3D
#define HMC5883L_WRITE_ADDRESS        0x3C
#define HMC5883L_REG_CONFIG_A         0x00
#define HMC5883L_REG_CONFIG_B         0x01
#define HMC5883L_REG_MODE             0x02
#define HMC5883L_REG_OUT_X_M          0x03
#define HMC5883L_REG_OUT_X_L          0x04
#define HMC5883L_REG_OUT_Z_M          0x05
#define HMC5883L_REG_OUT_Z_L          0x06
#define HMC5883L_REG_OUT_Y_M          0x07
#define HMC5883L_REG_OUT_Y_L          0x08
#define HMC5883L_REG_STATUS           0x09
#define HMC5883L_REG_IDENT_A          0x0A
#define HMC5883L_REG_IDENT_B          0x0B
#define HMC5883L_REG_IDENT_C          0x0C

#define MS5611_RESET                  0x1E
#define MS5611_ADC_READ               0x00 // ADC read command
#define MS5611_ATM_CONFIG             0x40 // ADC D1 conversion
#define MS5611_TEMP_CONFIG            0x50 // ADC D2 conversion
#define MS5611_CONFIG_256             0x00 // ADC OSR=256
#define MS5611_CONFIG_512             0x02 // ADC OSR=512
#define MS5611_CONFIG_1024            0x04 // ADC OSR=1024
#define MS5611_CONFIG_2048            0x06 // ADC OSR=2048
#define MS5611_CONFIG_4096            0x08 // ADC OSR=4096
#define MS5611_PROM_RD                0xA0 // Prom read command

#endif
