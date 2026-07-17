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

#include "debug.h"
#include "ch32v30x_conf.h"


/* Global typedef */

/* Global define */
#define RXBUF_SIZE 1024 // DMA buffer size
#define size(a)   (sizeof(a) / sizeof(*(a)))
/* Global Variable */
u8 TxBuffer[] = " ";
u8 RxBuffer[RXBUF_SIZE]={0};                                         
uint16_t rxBufferReadPos = 0;       //接收缓冲区读指针

// 提取公共缓冲区，避免栈溢出风险
static char uart_temp_buf[256] = {0};

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
 * @fn      DMA2__Init
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
 * @fn      uartWriteWiFi
 *
 * @brief   向 WiFi 模组发送数据 (非阻塞)
 *
 * @param   data - 要发送的数据的首地址
 *          num  - 数据长度
 *
 * @return  RESET - UART6 busy,failed to send
 *          SET   - send success
 */
FlagStatus uartWriteWiFi(char * data , uint16_t num)
{
    //如上次发送未完成，返回
	if(DMA_GetCurrDataCounter(DMA2_Channel6) != 0){
		return RESET;
	}

    DMA_ClearFlag(DMA2_FLAG_TC8);
	DMA_Cmd(DMA2_Channel6, DISABLE);           // 关 DMA 后操作
	DMA2_Channel6->MADDR = (uint32_t)data;      // 发送缓冲区为 data
	DMA_SetCurrDataCounter(DMA2_Channel6, num);  // 设置缓冲区长度
	DMA_Cmd(DMA2_Channel6, ENABLE);             // 开 DMA
	return SET;
}

/*********************************************************************
 * @fn      uartWriteWiFiStr
 *
 * @brief   向 WiFi 模组发送字符串
 *
 * @param   str - string to send
 *
 * @return  RESET - UART busy,failed to send
 *          SET   - send success
 */
FlagStatus uartWriteWiFiStr(char * str)
{
    uint16_t num = 0;
    while(str[num])num++;           // 计算字符串长度
    return uartWriteWiFi(str, num);
}

/*********************************************************************
 * @fn      uartReadWiFi
 *
 * @brief   从接收缓冲区读出一组数据
 *
 * @param   buffer - 用来存放读出数据的地址
 *          num    - 要读的字节数
 *
 * @return  uint32_t - 返回实际读出的字节数
 */
uint32_t uartReadWiFi(char * buffer , uint16_t num)
{
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7); //计算 DMA 数据尾的位置
    uint16_t i = 0;
    if (rxBufferReadPos == rxBufferEnd){
		// 无数据，返回
        return 0;
    }

    while (rxBufferReadPos!=rxBufferEnd && i < num){
        buffer[i] = RxBuffer[rxBufferReadPos];
        i++;
        rxBufferReadPos++;
        if(rxBufferReadPos >= RXBUF_SIZE){
            // 超出缓冲区，回零
            rxBufferReadPos = 0;
        }
    }
    return i;
}

/*********************************************************************
 * @fn      uartReadByteWiFi
 *
 * @brief   从接收缓冲区读出 1 字节数据
 *
 * @return  char - 返回读出的数据(无数据也返回0)
 */
char uartReadByteWiFi(void)
{
    char ret;
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7);
    if (rxBufferReadPos == rxBufferEnd){
        // 无数据，返回
        return 0;
    }
    ret = RxBuffer[rxBufferReadPos];
    rxBufferReadPos++;
    if(rxBufferReadPos >= RXBUF_SIZE){
        // 超出缓冲区，回零
        rxBufferReadPos = 0;
    }
    return ret;
}

/*********************************************************************
 * @fn      uartAvailableWiFi
 *
 * @brief   获取缓冲区中可读数据的数量
 *
 * @return  uint16_t - 返回可读数据数量
 */
uint16_t uartAvailableWiFi(void)
{
    uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7);//计算 DMA 数据尾的位置
    // 计算可读字节
    if (rxBufferReadPos <= rxBufferEnd){
        return rxBufferEnd - rxBufferReadPos;
    }else{
        return rxBufferEnd +RXBUF_SIZE -rxBufferReadPos;
    }
}

// 辅助函数：清空环形接收缓冲区内积累的垃圾数据
void uartFlushWiFi(void)
{
    uint16_t num = uartAvailableWiFi();
    if(num > 0) {
        char dump[512];
        uint16_t read_len = (num > 512) ? 512 : num;
        uartReadWiFi(dump, read_len);
    }
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
	int num = 0; // 用于接收可读字节数

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SystemCoreClockUpdate();
	Delay_Init();
	USART_Printf_Init(115200);	
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
	printf("This is printf example\r\n");
	printf("8266 WiFi TEST\r\n");

	LED_Init();
	DMA2__Init();    // 调用你的 DMA 初始化
	WIFI8266_Init(); // 调用你的 UART6 串口初始化
	
	// 非常重要：开启 UART6 的 DMA 收发请求
	USART_DMACmd(UART6, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);

    Delay_Ms(1000);
    
    /* --------------------------------------------------------
     * ? 优化点 1：上电首先对模块进行复位，清空所有残留连接和模式
     * -------------------------------------------------------- */
    printf("1. 正在复位 WiFi 模块...\r\n");
    while(uartWriteWiFiStr("AT+RST\r\n")==RESET);
    Delay_Ms(3000); // 给足 3 秒启动时间
    uartFlushWiFi(); // 清除复位输出的启动信息，避免干扰后续解析

    // 查询 WiFi 模块是否正常工作
    while(uartWriteWiFiStr("AT\r\n")==RESET);
    Delay_Ms(300);
    
    // 查询 WiFi 模块版本信息
    while(uartWriteWiFiStr("AT+GMR\r\n")==RESET);
    Delay_Ms(300);
    
    // 设为 Station 模式
    while(uartWriteWiFiStr("AT+CWMODE=1\r\n")==RESET);
    Delay_Ms(300);
    
    uartFlushWiFi(); // 清除上面的回应积压

    /* --------------------------------------------------------
     * ? 优化点 2：Wi-Fi 连接逻辑优化
     * -------------------------------------------------------- */
    printf("2. 开始连接 Wi-Fi 热点...\r\n");
    while(uartWriteWiFiStr("AT+CWJAP=\"HONOR V30\",\"12345678\"\r\n")==RESET);
    
    // 循环阻塞，等待模块开始返回数据（说明握手开始）
    while(uartAvailableWiFi() == 0);
    Delay_Ms(5000); // 握手和分配IP需要 5 秒时间

    num = uartAvailableWiFi();
    if (num > 0 ){
        uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
        uartReadWiFi(uart_temp_buf, len);
        uart_temp_buf[len] = '\0';
        printf("Received Wi-Fi status:\r\n%s", uart_temp_buf);
    }
    Delay_Ms(1000);

    /* --------------------------------------------------------
     * ? 优化点 3：在建立 TCP 前，必须查询 IP（确保拿到非0.0.0.0的IP） [2]
     * -------------------------------------------------------- */
    printf("\r\n=================================\r\n");
    printf("3. 查询分配到的 IP 地址...\r\n");
    uartFlushWiFi(); 
    while(uartWriteWiFiStr("AT+CIFSR\r\n")==RESET);
    Delay_Ms(500);
    num = uartAvailableWiFi();
    if(num > 0){ 
        uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
        uartReadWiFi(uart_temp_buf, len);
        uart_temp_buf[len] = '\0';
        printf("%s", uart_temp_buf); 
    }

    /* --------------------------------------------------------
     * 4. 尝试连接公网 TCP 服务器
     * -------------------------------------------------------- */
    printf("\r\n=================================\r\n");
    printf("4. 尝试连接 TCP 服务器...\r\n");
    uartFlushWiFi(); 

    while(uartWriteWiFiStr("AT+CIPSTART=\"TCP\",\"60.205.167.213\",3390\r\n")==RESET);
    
    // 监听 3 秒钟，等待 OK 或 CONNECT
    for(int i=0; i<30; i++){
        Delay_Ms(100);
        num = uartAvailableWiFi();
        if(num > 0){
            uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
            uartReadWiFi(uart_temp_buf, len);
            uart_temp_buf[len] = '\0';
            printf("%s", uart_temp_buf);
        }
    }

    /* --------------------------------------------------------
     * 5. 尝试开启透传模式
     * -------------------------------------------------------- */
    printf("\r\n=================================\r\n");
    printf("5. 尝试开启透传模式...\r\n");
    uartFlushWiFi(); 
    while(uartWriteWiFiStr("AT+CIPMODE=1\r\n")==RESET);
    Delay_Ms(500);
    num = uartAvailableWiFi();
    if(num > 0){ 
        uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
        uartReadWiFi(uart_temp_buf, len);
        uart_temp_buf[len] = '\0';
        printf("%s", uart_temp_buf); 
    }

    /* --------------------------------------------------------
     * 6. 准备发送数据 (CIPSEND)
     * -------------------------------------------------------- */
    printf("\r\n=================================\r\n");
    printf("6. 准备发送数据 (CIPSEND)...\r\n");
    uartFlushWiFi(); 
    while(uartWriteWiFiStr("AT+CIPSEND\r\n")==RESET);
    Delay_Ms(500);
    num = uartAvailableWiFi();
    if(num > 0){ 
        uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
        uartReadWiFi(uart_temp_buf, len);
        uart_temp_buf[len] = '\0';
        printf("%s", uart_temp_buf); 
    }

    printf("\r\n=================================\r\n");
    printf("正式进入大循环！\r\n");
    uartFlushWiFi(); 

    int count = 0;
	while(1)
    {
		Delay_Ms(1000); // 1秒钟发一次
		
		// 1. 单片机主动向公网服务器发送数据
        char sendBuf[64];
        sprintf(sendBuf, "Hello SSCOM! count = %d\r\n", count++);
        uartWriteWiFiStr(sendBuf); 

		// 2. 接收服务器发给单片机的数据，并打印到单片机的串口终端
        int num = uartAvailableWiFi();
        if (num > 0 ){
            // 采用公共全局缓冲区，限制单次读取长度，保障内存安全
            uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
            uartReadWiFi(uart_temp_buf, len);
            uart_temp_buf[len] = '\0';
            printf("SSCOM Says: %s", uart_temp_buf); 
        }
	}
}
