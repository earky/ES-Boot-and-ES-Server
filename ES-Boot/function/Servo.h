#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f4xx.h"

void PWM_Init(void);

void change_DutyCycle_Motor1(uint16_t DutyCycle);
void change_DutyCycle_Motor2(uint16_t DutyCycle);
void change_DutyCycle_Motor3(uint16_t DutyCycle);
void change_DutyCycle_Motor4(uint16_t DutyCycle);


#endif
