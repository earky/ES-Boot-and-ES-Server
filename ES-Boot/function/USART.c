#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "misc.h"


//void Serial_Init()
//{
//    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
//    
//    GPIO_InitTypeDef GPIO_InitStruct;
//    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
//    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
//    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
//    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA,&GPIO_InitStruct);
//    
//    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
//    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
//    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
//    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA,&GPIO_InitStruct);
//    
//    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
//    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
//    
//    USART_InitTypeDef USART_InitStruct;
//    USART_InitStruct.USART_BaudRate = 9600;
//    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//    USART_InitStruct.USART_Parity = USART_Parity_No;
//    USART_InitStruct.USART_StopBits = USART_StopBits_1;
//    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
//    USART_Init(USART1,&USART_InitStruct);
//    
//    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
//    
//    NVIC_InitTypeDef NVIC_InitStruct;
//    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
//    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
//    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
//    NVIC_Init(&NVIC_InitStruct);
//    
//    USART_Cmd(USART1,ENABLE);
//}

//void USART1_SendData(uint16_t data)
//{
//    USART_SendData(USART1,data);
//    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
//}
