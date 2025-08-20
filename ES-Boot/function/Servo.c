#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"

void PWM_Init(void)
{
	//¿ªÆôÊ±ÖÓ
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	

	
	//³õÊ¼»¯GPIO
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10| GPIO_Pin_11;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	
	//ÉèÖÃGPIO¿ÚÎª¸´ÓÃ
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1); // PA8 -> TIM1_CH1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1); // PA9 -> TIM1_CH2
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_TIM1); // PA10 -> TIM1_CH3
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_TIM1); // PA11 -> TIM1_CH4
	
	//³õÊ¼»¯Ê±»ùµ¥Ôª
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Prescaler =  83; //Ô¤·ÖÆµÆ÷ÏµÊý£¬Ö÷ÆµÊÇ84MHzµÄ»°£¬84MHz/84 = 1000kHz
	TIM_TimeBaseInitStruct.TIM_Period =19999;  //ARR ×Ô¶¯ÖØ×°ÔØ¼Ä´æÆ÷£¬ÉÏÃæµÄ1000kHz£¬³ýÒÔ20000ºó£¬ÊÇ50Hz£¬Õâ¾ÍÊÇÊä³öµÄ×îºóµÄÆµÂÊ
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);
	
	//³õÊ¼»¯Êä³ö±È½ÏÍ¨µÀ
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1; // PWM Ä£Ê½ 1
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; // ÆôÓÃÊä³ö
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High; // Êä³ö¼«ÐÔ
	TIM_OCInitStruct.TIM_Pulse = 90; //CCR,ÉèÖÃÕ¼¿Õ±È£¬·¶Î§Îª0-ARR
	TIM_OC1Init(TIM1, &TIM_OCInitStruct);
	TIM_OC2Init(TIM1, &TIM_OCInitStruct);
	TIM_OC3Init(TIM1, &TIM_OCInitStruct);
	TIM_OC4Init(TIM1, &TIM_OCInitStruct);
	
	TIM_Cmd(TIM1, ENABLE); // Æô¶¯¶¨Ê±Æ÷
	TIM_CtrlPWMOutputs(TIM1, ENABLE); // ÆôÓÃ PWM Êä³ö£¬¸ß¼¶¶¨Ê±Æ÷²ÅÓÐ
}


//¿ØÖÆËÄ¸öµç»ú×ªËÙµÄº¯Êý£¬²ÎÊýÎªÕ¼¿Õ±È£¬0-100
void change_DutyCycle_Motor1(uint16_t DutyCycle)
{
	TIM_SetCompare1(TIM1,(uint32_t)(DutyCycle*200));
}

void change_DutyCycle_Motor2(uint16_t DutyCycle)
{
	TIM_SetCompare2(TIM1,(uint32_t)DutyCycle*200);
}

void change_DutyCycle_Motor3(uint16_t DutyCycle)
{
	TIM_SetCompare3(TIM1,(uint32_t)DutyCycle*200);
}

void change_DutyCycle_Motor4(uint16_t DutyCycle)
{
	TIM_SetCompare4(TIM1,(uint32_t)DutyCycle*200);
}
