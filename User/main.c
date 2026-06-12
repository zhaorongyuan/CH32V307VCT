/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 *@Note
 USART Print debugging routine:
 USART1_Tx(PA9).
 This example demonstrates using USART1(PA9) as a print debug port output.

*/

#include "debug.h"
#include "ch32v30x_gpio.h"


/* Global typedef */

/* Global define */

/* Global Variable */

/*********************************************************************
 * @fn      LED_Init
 *
 * @brief   
 *
 * @return  none
 */
void LED_Init()
{
	GPIO_InitTypeDef LED_GPIO_InitTypeDef = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	LED_GPIO_InitTypeDef.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;
	LED_GPIO_InitTypeDef.GPIO_Mode = GPIO_Mode_Out_PP;
	LED_GPIO_InitTypeDef.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &LED_GPIO_InitTypeDef);

	GPIO_WriteBit(GPIOE, GPIO_Pin_11, Bit_SET);
	GPIO_WriteBit(GPIOE, GPIO_Pin_12, Bit_SET);

}

/*********************************************************************
 * @fn      WIFI_Init
 *
 * @brief   
 *
 * @return  none
 */
void WIFI_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	USART_InitTypeDef USART_InitStructure = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
	
	GPIO_WriteBit(GPIOE, GPIO_Pin_11, Bit_SET);
	GPIO_WriteBit(GPIOE, GPIO_Pin_12, Bit_SET);

	USART_Init(UART6, &USART_InitStructure);
    USART_Cmd(UART6, ENABLE);

}


/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SystemCoreClockUpdate();
	Delay_Init();
	USART_Printf_Init(115200);	
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
	printf("This is printf example\r\n");

	LED_Init();


	while(1)
    {
		Delay_Ms(100);
		GPIO_WriteBit(GPIOE, GPIO_Pin_11, Bit_RESET);
		Delay_Ms(100);
		GPIO_WriteBit(GPIOE, GPIO_Pin_11, Bit_SET);

	}
}

