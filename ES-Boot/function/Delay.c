#include "stm32f4xx.h"


void TIM_Delay_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    TIM_TimeBaseInitTypeDef TIM_InitStruct = {
        .TIM_Prescaler = 84 - 1,      // 84MHz / 84 = 1MHz (1us计数)
        .TIM_CounterMode = TIM_CounterMode_Up,
        .TIM_Period = 0xFFFF,         // 最大重载值（65535us）
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_RepetitionCounter = 0
    };
    TIM_TimeBaseInit(TIM2, &TIM_InitStruct);
    TIM_Cmd(TIM2, DISABLE);            // 初始化后先关闭定时器
}

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
//void Delay_us(uint32_t xus)
//{
////	SysTick->LOAD = 72 * xus;				//设置定时器重装值
////	SysTick->VAL = 0x00;					//清空当前计数值
////	SysTick->CTRL = 0x00000005;				//设置时钟源为HCLK，启动定时器
////	while(!(SysTick->CTRL & 0x00010000));	//等待计数到0
////	SysTick->CTRL = 0x00000004;				//关闭定时器
//		TIM_SetAutoreload(TIM2, xus - 1);  // 设置自动重载值
//    TIM_SetCounter(TIM2, 0);           // 清零计数器
//    TIM_Cmd(TIM2, ENABLE);             // 启动定时器
//    
//    while (!TIM_GetFlagStatus(TIM2, TIM_FLAG_Update)); // 等待更新事件
//    
//    TIM_ClearFlag(TIM2, TIM_FLAG_Update); // 清除标志位
//    TIM_Cmd(TIM2, DISABLE);            // 关闭定时器
//}

void Delay_us(u16 time)
{    
   u16 i=0;  
   while(time--)
   {
      i=10;  //自己定义
      while(i--) ;    
   }
 }
/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
	while(xms--)
	{
		Delay_us(1000);
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs--)
	{
		Delay_ms(1000);
	}
} 
