/**
 * @file    main.c
 * @brief   程序入口 — Super Loop 调度器
 * @author  zry
 * @date    2026-07-17
 * @version V0.0.1
 *
 * @note    主函数入口，负责系统初始化和周期调度
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "ch32v30x_conf.h"
#include "app_cfg.h"
#include "app_global.h"
#include "board.h"
#include "Interface.h"
#include "task_control.h"
#include "task_ota.h"

/**
 * @brief
 */
void SysTick_Config(uint32_t ticks)
{
    SysTick->CTLR &= ~(1U << 0);

    SysTick->CNT = 0;
    SysTick->SR = 0;

    SysTick->CMP = ticks - 1;

    SysTick->CTLR |= (1U << 4) | (1U << 3) | (1U << 2);     
    SysTick->CTLR |= (1U << 5);
    SysTick->CTLR |= (1U << 1);
    SysTick->CTLR |= (1U << 0);

    NVIC_EnableIRQ(SysTick_IRQn);

}

/**
 * @brief  系统核心底层组件初始化
 */
static void System_Core_Init(void)
{
    /* 配置中断优先级分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* 更新 SystemCoreClock 变量 */
    SystemCoreClockUpdate();

    /* 配置 1ms 的 SysTick 定时器并开启中断 */
    SysTick_Config(SystemCoreClock / 1000U);

    /* 初始化延时组件与调试串口 (115200bps) */
    Delay_Init();
    USART_Printf_Init(115200);
}

int main(void)
{
    /* 1. 核心初始化 */
    System_Core_Init();

    /* 2. 板级外设初始化 (通过 BSW 层) */
    Board_Init();

    /* 3. HAL 外设初始化 (通过 Interface 层) */
    API_Led_Init();
    API_Key_Init();
    API_Buzzer_Init();
    API_Wifi_Init();

    /* 4. 业务任务初始化 */
    Task_Control_Init();
    Task_OTA_Init();

    /* 开启全局中断 */
    __enable_irq();

    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("FW: v%d.%d.%d\r\n",
           APP_FW_VERSION_MAJOR, APP_FW_VERSION_MINOR, APP_FW_VERSION_PATCH);

    /* 调度器时间戳 */
    uint32_t last_10ms_ticks   = 0;
    uint32_t last_100ms_ticks  = 0;
    uint32_t last_1000ms_ticks = 0;

    /* 5. 主循环调度 (Super Loop) */
    for (;;)
    {
        /* Task Control: 10ms 周期执行 */
        if ((Gc_SysTick_ms - last_10ms_ticks) >= APP_TASK_10MS_PERIOD) {
            last_10ms_ticks = Gc_SysTick_ms;
            Task_Control_Update_10ms();
        }

        /* Task Control: 100ms 周期执行 */
        if ((Gc_SysTick_ms - last_100ms_ticks) >= APP_TASK_100MS_PERIOD) {
            last_100ms_ticks = Gc_SysTick_ms;
            Task_Control_Update_100ms();
        }

        /* Task Control + OTA: 1000ms 周期执行 */
        if ((Gc_SysTick_ms - last_1000ms_ticks) >= APP_TASK_1000MS_PERIOD) {
            last_1000ms_ticks = Gc_SysTick_ms;
            Task_Control_Update_1000ms();
            // Task_OTA_Update();

            /* CPU 负载率（整数运算，无 FPU） */
            uint32_t systick_cnt = SysTick->CNT;
            Gs_CpuLoad_percent = (96000U - systick_cnt) * 100U / 96000U;
        }
    }
}
// /**
//  * @file    main.c
//  * @brief   程序入口 — Super Loop 调度器
//  * @author  zry
//  * @date    2026-07-17
//  * @version V0.0.1
//  *
//  * @note    主函数入口，负责系统初始化和周期调度
//  * @copyright (c) 2026 zry. All rights reserved.
//  */

// #include "ch32v30x_conf.h"
// #include "app_cfg.h"
// #include "app_global.h"
// #include "board.h"
// #include "Interface.h"
// #include "task_control.h"
// #include "task_ota.h"

// /**
//  * @brief  系统核心底层组件初始化
//  */
// static void System_Core_Init(void)
// {
//     /* 配置中断优先级分组 */
//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

//     /* 更新 SystemCoreClock 变量 */
//     SystemCoreClockUpdate();

//     /* 配置 1ms 的 SysTick 定时器并开启中断 */
//     SysTick_Config(SystemCoreClock / 1000U);

//     /* 初始化延时组件与调试串口 (115200bps) */
//     Delay_Init();
//     USART_Printf_Init(115200);
// }

// int main(void)
// {
//     /* 1. 核心初始化 */
//     System_Core_Init();

//     /* 2. 板级外设初始化 (通过 BSW 层) */
//     Board_Init();

//     /* 3. HAL 外设初始化 (通过 Interface 层) */
//     API_Led_Init();
//     API_Key_Init();
//     API_Buzzer_Init();
//     API_Wifi_Init();

//     /* 4. 业务任务初始化 */
//     Task_Control_Init();
//     Task_OTA_Init();

//     /* 开启全局中断 */
//     __enable_irq();

//     printf("SystemClk:%d\r\n", SystemCoreClock);
//     printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
//     printf("FW: v%d.%d.%d\r\n",
//            APP_FW_VERSION_MAJOR, APP_FW_VERSION_MINOR, APP_FW_VERSION_PATCH);

//     /* 调度器时间戳 */
//     uint32_t last_10ms_ticks   = 0;
//     uint32_t last_100ms_ticks  = 0;
//     uint32_t last_1000ms_ticks = 0;

//     /* 5. 主循环调度 (Super Loop) */
//     for (;;)
//     {
//         /* Task Control: 10ms 周期执行 */
//         if ((Gc_SysTick_ms - last_10ms_ticks) >= APP_TASK_10MS_PERIOD) {
//             last_10ms_ticks = Gc_SysTick_ms;
//             Task_Control_Update_10ms();
//         }

//         /* Task Control: 100ms 周期执行 */
//         if ((Gc_SysTick_ms - last_100ms_ticks) >= APP_TASK_100MS_PERIOD) {
//             last_100ms_ticks = Gc_SysTick_ms;
//             Task_Control_Update_100ms();
//         }

//         /* Task Control + OTA: 1000ms 周期执行 */
//         if ((Gc_SysTick_ms - last_1000ms_ticks) >= APP_TASK_1000MS_PERIOD) {
//             last_1000ms_ticks = Gc_SysTick_ms;
//             Task_Control_Update_1000ms();
//             Task_OTA_Update();

//             /* CPU 负载率（整数运算，无 FPU） */
//             uint32_t systick_cnt = SysTick->CNT;
//             Gs_CpuLoad_percent = (96000U - systick_cnt) * 100U / 96000U;
//         }
//     }
// }
// /**
//  * @file    main.c
//  * @brief   �������
//  * @author  zry
//  * @date    2026-07-17
//  * @version V0.0.1
//  *
//  * @note    �����������
//  * @copyright (c) 2026 zry. All rights reserved.
//  */

// #include "ch32v30x_conf.h"
// #include "Interface.h"
// #include "app_cfg.h"
// #include "debug.h"



// /**
//  * @brief  ���� SysTick ��ʱ�� (���� CH32V30x RISC-V �淶)
//  */
// void SysTick_Config(uint32_t ticks)
// {
//     SysTick->CTLR &= ~(1U << 0);

//     /* ��ռ��������жϱ�־λ */
//     SysTick->CNT = 0;
//     SysTick->SR = 0;

//     SysTick->CMP = ticks - 1;

//     SysTick->CTLR |= (1U << 4) | (1U << 3) | (1U << 2);     
//     SysTick->CTLR |= (1U << 5);
//     SysTick->CTLR |= (1U << 1);
//     SysTick->CTLR |= (1U << 0);

//     /* �� NVIC ��ʹ�� SysTick �� IRQ ͨ�� */
//     NVIC_EnableIRQ(SysTick_IRQn);

// }

// /**
//  * @brief  ϵͳ���ĵײ������ʼ��
//  */
// static void System_Core_Init(void)
// {
//     /* �����ж����ȼ����� */
//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
//     /* ���� SystemCoreClock ���� */
//     SystemCoreClockUpdate();
    
//     /* ���� 1ms �� SysTick ��ʱ���������ж� */
//     SysTick_Config(SystemCoreClock / 1000U);
    
//     /* ��ʼ����ʱ�������Դ��� (115200bps) */
//     Delay_Init();
//     USART_Printf_Init(115200);
// }

// void delay(volatile unsigned int count) {
// while (count--) {
// // ��ѭ��
// }
// }

// int main(void)
// {
//     /* 1. ���������Ӳ�������ʼ�� */
//     System_Core_Init();
    
//     // Board_UartFifo_Init();  /* ������ UART ֮ǰ��ʼ�� */

//     // /* 2. Ӳ�������ʼ�� (ͨ�� HAL �ӿ�) */
//     // API_Uart_Init(UART_ID_DEBUG, DBG_UART_BAUD);
//     // API_Led_Init();
//     // API_Button_Init();
//     // API_Buzzer_Init();

//     // /* 3. ҵ�������ʼ�� */
//     // Task_Control_Init();
//     // Task_OTA_Init();

//     /* ����ȫ���ж� */
//     __enable_irq();

//     /* ������ʱ������������ֱ�Ϊ��ͬ��ִ�����ڷ��������״̬�Ĵ��� */
//     uint32_t last_10ms_ticks   = 0;
//     uint32_t last_100ms_ticks  = 0;
//     uint32_t last_1000ms_ticks = 0;

//     /* 4. ��ѭ������ (Super Loop) */
//     for(;;)
//     {
//         delay(1000);

//         /* Task Control: 10ms ����ִ�� */
//         if ((Gc_SysTick_ms - last_10ms_ticks) >= APP_TASK_10MS_PERIOD) {
//             last_10ms_ticks = Gc_SysTick_ms;

//             // Task_Control_Update_10ms();
//         }

//         /* Task Control: 100ms ����ִ�� */
//         if ((Gc_SysTick_ms - last_100ms_ticks) >= APP_TASK_100MS_PERIOD) {
//             last_100ms_ticks = Gc_SysTick_ms;
            
//             // Task_Control_Update_100ms();
//         }

//         /* Task Control: 1000ms ����ִ�� */
//         if ((Gc_SysTick_ms - last_1000ms_ticks) >= APP_TASK_1000MS_PERIOD) {
//             last_1000ms_ticks = Gc_SysTick_ms;
            
//             // Task_Control_Update_1000ms();

//             // CPU ������
//             uint32_t sysstick_cnt = SysTick->CNT;
//             Gs_CpuLoad_tick = ((float)(96000U - sysstick_cnt) / 96000.0f) * 100.0f;
//             printf("Gs_CpuLoad_tick: %u ,\n", (uint32_t)Gs_CpuLoad_tick);
            
//         }
//     }
// }





// /* Global typedef */

// /* Global define */
// #define RXBUF_SIZE 1024 // DMA buffer size
// #define size(a)   (sizeof(a) / sizeof(*(a)))
// /* Global Variable */
// u8 TxBuffer[] = " ";
// u8 RxBuffer[RXBUF_SIZE]={0};                                         
// uint16_t rxBufferReadPos = 0;       //���ջ�������ָ��

// // ��ȡ����������������ջ�������
// static char uart_temp_buf[256] = {0};

// /*********************************************************************
//  * @fn      LED_Init
//  *
//  * @brief   
//  *
//  * @return  none
//  */
// void LED_Init()
// {
// 	GPIO_InitTypeDef LED_GPIO_InitTypeDef = {0};

// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
// 	LED_GPIO_InitTypeDef.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;
// 	LED_GPIO_InitTypeDef.GPIO_Mode = GPIO_Mode_Out_PP;
// 	LED_GPIO_InitTypeDef.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOE, &LED_GPIO_InitTypeDef);

// 	GPIO_WriteBit(GPIOE, GPIO_Pin_11, Bit_SET);
// 	GPIO_WriteBit(GPIOE, GPIO_Pin_12, Bit_SET);

// }

// /*********************************************************************
//  * @fn      WIFI8266_Init
//  *
//  * @brief   PC0 - UART6_TX - ESP8266_RX <���� ESP-01��ESP-01S WiFi ģ��>
//  *			PC1 - UART6_RX - ESP8266_TX <ʹ��ʱע�� WiFi ���߳������>
//  *
//  * @return  none
//  */
// void WIFI8266_Init()
// {
// 	GPIO_InitTypeDef GPIO_InitStructure = {0};
// 	USART_InitTypeDef USART_InitStructure = {0};

// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6, ENABLE);

// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOC, &GPIO_InitStructure);

// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
// 	//RX����������
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOC, &GPIO_InitStructure);

// 	USART_InitStructure.USART_BaudRate = 115200;
//     USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//     USART_InitStructure.USART_StopBits = USART_StopBits_1;
//     USART_InitStructure.USART_Parity = USART_Parity_No;
//     USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//     USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

// 	GPIO_WriteBit(GPIOC, GPIO_Pin_0, Bit_RESET);
// 	GPIO_WriteBit(GPIOC, GPIO_Pin_1, Bit_RESET);
	
// 	USART_Init(UART6, &USART_InitStructure);
// 	//�������� DMA
// 	DMA_Cmd(DMA2_Channel7, ENABLE);
//     USART_Cmd(UART6, ENABLE);

// }

// /*********************************************************************
//  * @fn      DMA2__Init
//  *
//  * @brief   Configures the DMA.
//  *
//  * @return  none
//  */
//  void DMA2__Init()
//  {
// 	DMA_InitTypeDef DMA_InitStructure ={0};
// 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

// 	// TX DMA ��ʼ��
// 	DMA_DeInit(DMA2_Channel6);
// 	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART6->DATAR);        // DMA �����ַ����ָ���Ӧ������
// 	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)TxBuffer;                   // DMA �ڴ��ַ��ָ���ͻ��������׵�ַ
// 	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      // ���� : ���� ��Ϊ �յ㣬�� �ڴ� ->  ����
// 	DMA_InitStructure.DMA_BufferSize = 0;                                   // ��������С,��ҪDMA���͵����ݳ���,Ŀǰû�����ݿɷ�
// 	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // �����ַ����������
// 	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // �ڴ��ַ����������
// 	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // ��������λ����8λ(Byte)
//  	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // �ڴ�����λ����8λ(Byte)
// 	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // ��ͨģʽ�������������ѭ������
// 	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 // ���ȼ����
// 	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            // M2P,����M2M
// 	DMA_Init(DMA2_Channel6, &DMA_InitStructure);

// 	// RX DMA ��ʼ�������λ������Զ�����
// 	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RxBuffer;                   // ���ջ�����
// 	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                      // ���� : ���� ��Ϊ Դ���� �ڴ� <- ����
// 	DMA_InitStructure.DMA_BufferSize = RXBUF_SIZE;                          // ����������Ϊ RXBUF_SIZE
// 	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         // ѭ��ģʽ�����ɻ��λ�����
// 	DMA_Init(DMA2_Channel7, &DMA_InitStructure);

//  }

// /*********************************************************************
//  * @fn      uartWriteWiFi
//  *
//  * @brief   �� WiFi ģ�鷢������ (������)
//  *
//  * @param   data - Ҫ���͵����ݵ��׵�ַ
//  *          num  - ���ݳ���
//  *
//  * @return  RESET - UART6 busy,failed to send
//  *          SET   - send success
//  */
// FlagStatus uartWriteWiFi(char * data , uint16_t num)
// {
//     //���ϴη���δ��ɣ�����
// 	if(DMA_GetCurrDataCounter(DMA2_Channel6) != 0){
// 		return RESET;
// 	}

//     DMA_ClearFlag(DMA2_FLAG_TC8);
// 	DMA_Cmd(DMA2_Channel6, DISABLE);           // �� DMA �����
// 	DMA2_Channel6->MADDR = (uint32_t)data;      // ���ͻ�����Ϊ data
// 	DMA_SetCurrDataCounter(DMA2_Channel6, num);  // ���û���������
// 	DMA_Cmd(DMA2_Channel6, ENABLE);             // �� DMA
// 	return SET;
// }

// /*********************************************************************
//  * @fn      uartWriteWiFiStr
//  *
//  * @brief   �� WiFi ģ�鷢���ַ���
//  *
//  * @param   str - string to send
//  *
//  * @return  RESET - UART busy,failed to send
//  *          SET   - send success
//  */
// FlagStatus uartWriteWiFiStr(char * str)
// {
//     uint16_t num = 0;
//     while(str[num])num++;           // �����ַ�������
//     return uartWriteWiFi(str, num);
// }

// /*********************************************************************
//  * @fn      uartReadWiFi
//  *
//  * @brief   �ӽ��ջ���������һ������
//  *
//  * @param   buffer - ������Ŷ������ݵĵ�ַ
//  *          num    - Ҫ�����ֽ���
//  *
//  * @return  uint32_t - ����ʵ�ʶ������ֽ���
//  */
// uint32_t uartReadWiFi(char * buffer , uint16_t num)
// {
//     uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7); //���� DMA ����β��λ��
//     uint16_t i = 0;
//     if (rxBufferReadPos == rxBufferEnd){
// 		// �����ݣ�����
//         return 0;
//     }

//     while (rxBufferReadPos!=rxBufferEnd && i < num){
//         buffer[i] = RxBuffer[rxBufferReadPos];
//         i++;
//         rxBufferReadPos++;
//         if(rxBufferReadPos >= RXBUF_SIZE){
//             // ����������������
//             rxBufferReadPos = 0;
//         }
//     }
//     return i;
// }

// /*********************************************************************
//  * @fn      uartReadByteWiFi
//  *
//  * @brief   �ӽ��ջ��������� 1 �ֽ�����
//  *
//  * @return  char - ���ض���������(������Ҳ����0)
//  */
// char uartReadByteWiFi(void)
// {
//     char ret;
//     uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7);
//     if (rxBufferReadPos == rxBufferEnd){
//         // �����ݣ�����
//         return 0;
//     }
//     ret = RxBuffer[rxBufferReadPos];
//     rxBufferReadPos++;
//     if(rxBufferReadPos >= RXBUF_SIZE){
//         // ����������������
//         rxBufferReadPos = 0;
//     }
//     return ret;
// }

// /*********************************************************************
//  * @fn      uartAvailableWiFi
//  *
//  * @brief   ��ȡ�������пɶ����ݵ�����
//  *
//  * @return  uint16_t - ���ؿɶ���������
//  */
// uint16_t uartAvailableWiFi(void)
// {
//     uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7);//���� DMA ����β��λ��
//     // ����ɶ��ֽ�
//     if (rxBufferReadPos <= rxBufferEnd){
//         return rxBufferEnd - rxBufferReadPos;
//     }else{
//         return rxBufferEnd +RXBUF_SIZE -rxBufferReadPos;
//     }
// }

// // ������������ջ��ν��ջ������ڻ��۵���������
// void uartFlushWiFi(void)
// {
//     uint16_t num = uartAvailableWiFi();
//     if(num > 0) {
//         char dump[512];
//         uint16_t read_len = (num > 512) ? 512 : num;
//         uartReadWiFi(dump, read_len);
//     }
// }

// /*********************************************************************
//  * @fn      main
//  *
//  * @brief   Main program.
//  *
//  * @return  none
//  */
// int main(void)
// {
// 	int num = 0; // ���ڽ��տɶ��ֽ���

// 	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
// 	SystemCoreClockUpdate();
// 	Delay_Init();
// 	USART_Printf_Init(115200);	
// 	printf("SystemClk:%d\r\n",SystemCoreClock);
// 	printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
// 	printf("This is printf example\r\n");
// 	printf("8266 WiFi TEST\r\n");

// 	LED_Init();
// 	DMA2__Init();    // ������� DMA ��ʼ��
// 	WIFI8266_Init(); // ������� UART6 ���ڳ�ʼ��
	
// 	// �ǳ���Ҫ������ UART6 �� DMA �շ�����
// 	USART_DMACmd(UART6, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);

//     Delay_Ms(1000);
    
//     /* --------------------------------------------------------
//      * ? �Ż��� 1���ϵ����ȶ�ģ����и�λ��������в������Ӻ�ģʽ
//      * -------------------------------------------------------- */
//     printf("1. ���ڸ�λ WiFi ģ��...\r\n");
//     while(uartWriteWiFiStr("AT+RST\r\n")==RESET);
//     Delay_Ms(3000); // ���� 3 ������ʱ��
//     uartFlushWiFi(); // �����λ�����������Ϣ��������ź�������

//     // ��ѯ WiFi ģ���Ƿ���������
//     while(uartWriteWiFiStr("AT\r\n")==RESET);
//     Delay_Ms(300);
    
//     // ��ѯ WiFi ģ��汾��Ϣ
//     while(uartWriteWiFiStr("AT+GMR\r\n")==RESET);
//     Delay_Ms(300);
    
//     // ��Ϊ Station ģʽ
//     while(uartWriteWiFiStr("AT+CWMODE=1\r\n")==RESET);
//     Delay_Ms(300);
    
//     uartFlushWiFi(); // �������Ļ�Ӧ��ѹ

//     /* --------------------------------------------------------
//      * ? �Ż��� 2��Wi-Fi �����߼��Ż�
//      * -------------------------------------------------------- */
//     printf("2. ��ʼ���� Wi-Fi �ȵ�...\r\n");
//     while(uartWriteWiFiStr("AT+CWJAP=\"HONOR V30\",\"12345678\"\r\n")==RESET);
    
//     // ѭ���������ȴ�ģ�鿪ʼ�������ݣ�˵�����ֿ�ʼ��
//     while(uartAvailableWiFi() == 0);
//     Delay_Ms(5000); // ���ֺͷ���IP��Ҫ 5 ��ʱ��

//     num = uartAvailableWiFi();
//     if (num > 0 ){
//         uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
//         uartReadWiFi(uart_temp_buf, len);
//         uart_temp_buf[len] = '\0';
//         printf("Received Wi-Fi status:\r\n%s", uart_temp_buf);
//     }
//     Delay_Ms(1000);

//     /* --------------------------------------------------------
//      * ? �Ż��� 3���ڽ��� TCP ǰ�������ѯ IP��ȷ���õ���0.0.0.0��IP�� [2]
//      * -------------------------------------------------------- */
//     printf("\r\n=================================\r\n");
//     printf("3. ��ѯ���䵽�� IP ��ַ...\r\n");
//     uartFlushWiFi(); 
//     while(uartWriteWiFiStr("AT+CIFSR\r\n")==RESET);
//     Delay_Ms(500);
//     num = uartAvailableWiFi();
//     if(num > 0){ 
//         uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
//         uartReadWiFi(uart_temp_buf, len);
//         uart_temp_buf[len] = '\0';
//         printf("%s", uart_temp_buf); 
//     }

//     /* --------------------------------------------------------
//      * 4. �������ӹ��� TCP ������
//      * -------------------------------------------------------- */
//     printf("\r\n=================================\r\n");
//     printf("4. �������� TCP ������...\r\n");
//     uartFlushWiFi(); 

//     while(uartWriteWiFiStr("AT+CIPSTART=\"TCP\",\"60.205.167.213\",3390\r\n")==RESET);
    
//     // ���� 3 ���ӣ��ȴ� OK �� CONNECT
//     for(int i=0; i<30; i++){
//         Delay_Ms(100);
//         num = uartAvailableWiFi();
//         if(num > 0){
//             uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
//             uartReadWiFi(uart_temp_buf, len);
//             uart_temp_buf[len] = '\0';
//             printf("%s", uart_temp_buf);
//         }
//     }

//     /* --------------------------------------------------------
//      * 5. ���Կ���͸��ģʽ
//      * -------------------------------------------------------- */
//     printf("\r\n=================================\r\n");
//     printf("5. ���Կ���͸��ģʽ...\r\n");
//     uartFlushWiFi(); 
//     while(uartWriteWiFiStr("AT+CIPMODE=1\r\n")==RESET);
//     Delay_Ms(500);
//     num = uartAvailableWiFi();
//     if(num > 0){ 
//         uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
//         uartReadWiFi(uart_temp_buf, len);
//         uart_temp_buf[len] = '\0';
//         printf("%s", uart_temp_buf); 
//     }

//     /* --------------------------------------------------------
//      * 6. ׼���������� (CIPSEND)
//      * -------------------------------------------------------- */
//     printf("\r\n=================================\r\n");
//     printf("6. ׼���������� (CIPSEND)...\r\n");
//     uartFlushWiFi(); 
//     while(uartWriteWiFiStr("AT+CIPSEND\r\n")==RESET);
//     Delay_Ms(500);
//     num = uartAvailableWiFi();
//     if(num > 0){ 
//         uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
//         uartReadWiFi(uart_temp_buf, len);
//         uart_temp_buf[len] = '\0';
//         printf("%s", uart_temp_buf); 
//     }

//     printf("\r\n=================================\r\n");
//     printf("��ʽ�����ѭ����\r\n");
//     uartFlushWiFi(); 

//     int count = 0;
// 	while(1)
//     {
// 		Delay_Ms(1000); // 1���ӷ�һ��
		
// 		// 1. ��Ƭ������������������������
//         char sendBuf[64];
//         sprintf(sendBuf, "Hello SSCOM! count = %d\r\n", count++);
//         uartWriteWiFiStr(sendBuf); 

// 		// 2. ���շ�����������Ƭ�������ݣ�����ӡ����Ƭ���Ĵ����ն�
//         int num = uartAvailableWiFi();
//         if (num > 0 ){
//             // ���ù���ȫ�ֻ����������Ƶ��ζ�ȡ���ȣ������ڴ氲ȫ
//             uint16_t len = (num > sizeof(uart_temp_buf)-1) ? sizeof(uart_temp_buf)-1 : num;
//             uartReadWiFi(uart_temp_buf, len);
//             uart_temp_buf[len] = '\0';
//             printf("SSCOM Says: %s", uart_temp_buf); 
//         }
// 	}
// }


