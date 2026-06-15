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
#include "ch32v30x_conf.h"


/* Global typedef */

/* Global define */
#define RXBUF_SIZE 1024 // DMA buffer size
#define size(a)   (sizeof(a) / sizeof(*(a)))
/* Global Variable */
u8 TxBuffer[] = " ";
u8 RxBuffer[RXBUF_SIZE]={0};                                         


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
 * @fn      WIFI8266_Init
 *
 * @brief   PC0 - UART6_TX - ESP8266_RX <兼容 ESP-01，ESP-01S WiFi 模块>
 *			PC1 - UART6_RX - ESP8266_TX <使用时注意 WiFi 天线朝向板外>
 *
 * @return  none
 */
void WIFI8266_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	USART_InitTypeDef USART_InitStructure = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	//RX，输入上拉
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;


	GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET);
	GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_RESET);
	
	USART_Init(UART6, &USART_InitStructure);
	//开启接收 DMA
	DMA_Cmd(DMA2_Channel7, ENABLE);
    USART_Cmd(UART6, ENABLE);

}

/*********************************************************************
 * @fn      DMA_Init
 *
 * @brief   Configures the DMA.
 *
 * @return  none
 */
 void DMA2__Init()
 {
	DMA_InitTypeDef DMA_InitStructure ={0};
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

	// TX DMA 初始化
	DMA_DeInit(DMA2_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART6->DATAR);        // DMA 外设基址，需指向对应的外设
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)TxBuffer;                   // DMA 内存基址，指向发送缓冲区的首地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      // 方向 : 外设 作为 终点，即 内存 ->  外设
	DMA_InitStructure.DMA_BufferSize = 0;                                   // 缓冲区大小,即要DMA发送的数据长度,目前没有数据可发
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // 外设地址自增，禁用
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // 内存地址自增，启用
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据位宽，8位(Byte)
 	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // 内存数据位宽，8位(Byte)
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // 普通模式，发完结束，不循环发送
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 // 优先级最高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            // M2P,禁用M2M
	DMA_Init(DMA2_Channel6, &DMA_InitStructure);

	// RX DMA 初始化，环形缓冲区自动接收
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RxBuffer;                   // 接收缓冲区
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                      // 方向 : 外设 作为 源，即 内存 <- 外设
	DMA_InitStructure.DMA_BufferSize = RXBUF_SIZE;                          // 缓冲区长度为 RXBUF_SIZE
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         // 循环模式，构成环形缓冲区
	DMA_Init(DMA2_Channel7, &DMA_InitStructure);

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





