#ifndef __HEADER_H
#define __HEADER_H

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "Delay.h"
#include "OLED.h"
#include "GY86.h"
#include "MyI2C.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include <stdlib.h>
#include <string.h> 
#include "Serial.h"
#include "Servo.h"
#include "PPM.h"
#include "Delay.h"

void HardwareInit(void);

#endif
