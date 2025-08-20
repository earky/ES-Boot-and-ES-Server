#include "PPM.h"
#include "OLED.h"

void PPM_Init(void)
{
    // 开启各种时钟树
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 配置GPIOC4为上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 		
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource4);//PC4 连接到中断线4

    // 配置EXIT
    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // 配置NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;//外部中断2
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
    NVIC_Init(&NVIC_InitStructure);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;


    // 配置TIM3时基单元
    TIM_TimeBaseStructure.TIM_Period = 65536 - 1;               // 满量程计数
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;               // 标准频率为72M/72=1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 不分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // 通道可以不用配置 我们只需要时基单元就可以了
    // TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;                // 选择通道1
    // TIM_ICInitStructure.TIM_ICFilter = 0;                           // 不滤波
    // TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;     // 上升沿触发
    // TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;           // 不分频
    // TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; // 选择直连通道
    // TIM_ICInit(TIM3, &TIM_ICInitStructure);

//   TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);        // 选择从模式触发源为TI1FP1
//   TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);     // 选择从模式为Reset

    // 使能TIM3时钟
    TIM_Cmd(TIM3, ENABLE);
}

uint16_t PPM_Sample_Cnt=0;
u32		 PPM_Time=0;
uint16_t PPM_Okay=0;
uint16_t PPM_Databuf[10]={0};

//222 - 444
void EXTI4_IRQHandler(void)
{
	 if(EXTI_GetITStatus(EXTI_Line4)!=RESET)
	 {
        //PPM_Databuf[0] = TIM3->CNT;
        PPM_Time = TIM3->CNT;
        TIM3->CNT = 0;				//读取完之后清空，该值每1us加1
//				if(PPM_Sample_Cnt <= 9){
//					PPM_Databuf[PPM_Sample_Cnt++] = PPM_Time;
//				}
        if(PPM_Time>=4200){  // 过滤空波
            PPM_Sample_Cnt=0;
        }else{
            PPM_Databuf[PPM_Sample_Cnt++]=PPM_Time;
        }
    }	 
	 EXTI_ClearITPendingBit(EXTI_Line4);//清除LINE2上的中断标志位 
}
