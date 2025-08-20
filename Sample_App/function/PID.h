#ifndef __PID_H
#define __PID_H

#include "Serial.h"
#include <stdlib.h>

//#define PID_ADDR		 0x08040000
#define PID_ADDR      0x0807FFD8
#define VM_ADDR       0x0807FFFC

#define PITCH_P_ADDR (PID_ADDR + 0x00)		//0x0807FFD8
#define PITCH_I_ADDR (PID_ADDR + 0x04)		//0x0807FFDC
#define PITCH_D_ADDR (PID_ADDR + 0x08)		//0x0807FFE0
		                                
#define YAW_P_ADDR   (PID_ADDR + 0x0C)		//0x0807FFE4
#define YAW_I_ADDR   (PID_ADDR + 0x10)		//0x0807FFE8
#define YAW_D_ADDR   (PID_ADDR + 0x14)		//0x0807FFEC
		                                      
#define ROLL_P_ADDR  (PID_ADDR + 0x18)		//0x0807FFF0
#define ROLL_I_ADDR  (PID_ADDR + 0x1C)		//0x0807FFF4
#define ROLL_D_ADDR  (PID_ADDR + 0x20)		//0x0807FFF8

#define PRINT        0x04
#define UPDATE_PID   0x07
#define SAVE_PID     0x08
#define INIT_PID     0x09
#define GY86_DATA		 0x10
#define SERVO_DATA   0x11
typedef struct{
	float Pitch_P;
	float Pitch_I;
	float Pitch_D;
	
	float Yaw_P;
	float Yaw_I;
	float Yaw_D;
	
	float Roll_P;
	float Roll_I;
	float Roll_D;
}PID_param;

extern PID_param pid_param;
extern char pid_param_buffer[128];

void update_PID(void);
void save_PID(void);
void init_PID(void);
void log_init_PID(void);

#endif

