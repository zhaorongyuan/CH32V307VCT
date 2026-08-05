// /**
//  * @file    board_uart_cfg.c
//  * @brief   UART 通信接口参数配置定义落地文件
//  * @details 映射完整硬件引脚与通信参数。
//  * @author  zry
//  * @date    2026-07-30
//  * @version V0.0.1
//  *
//  * @note    System HLR Traceability: [REQ-HLR-BSP-003]
//  * @copyright (c) 2026 zry. All rights reserved.
//  */

// // #include "ch32v30x.h"
// #include "board_uart_cfg.h"

// const UART_Config_t g_UartConfigTable[UART_ID_MAX] = {
//     [UART_ID_DEBUG1] = {
//         .pInstance    = USART1,
//         .ulClockMask  = RCC_APB2Periph_USART1,
//         .ulBaudRate   = 115200UL,
//         .usWordLength = USART_WordLength_8b,
//         .usStopBits   = USART_StopBits_1,
//         .usParity     = USART_Parity_No,
//         .usHwFlowCtl  = USART_HardwareFlowControl_None,
//         .ucIsAPB2     = 1U,
//         .pTxPort      = UART1_TX_PORT,
//         .usTxPin      = UART1_TX_PIN,
//         .pRxPort      = UART1_RX_PORT,
//         .usRxPin      = UART1_RX_PIN
//     },
//     [UART_ID_DEBUG2] = {
//         .pInstance    = USART2,
//         .ulClockMask  = RCC_APB1Periph_USART2,
//         .ulBaudRate   = 115200UL,
//         .usWordLength = USART_WordLength_8b,
//         .usStopBits   = USART_StopBits_1,
//         .usParity     = USART_Parity_No,
//         .usHwFlowCtl  = USART_HardwareFlowControl_None,
//         .ucIsAPB2     = 0U,
//         .pTxPort      = UART2_TX_PORT,
//         .usTxPin      = UART2_TX_PIN,
//         .pRxPort      = UART2_RX_PORT,
//         .usRxPin      = UART2_RX_PIN
//     },
//     [UART_ID_WIFI] = {
//         .pInstance    = UART6,
//         .ulClockMask  = RCC_APB1Periph_UART6,
//         .ulBaudRate   = 115200UL,
//         .usWordLength = USART_WordLength_8b,
//         .usStopBits   = USART_StopBits_1,
//         .usParity     = USART_Parity_No,
//         .usHwFlowCtl  = USART_HardwareFlowControl_None,
//         .ucIsAPB2     = 0U,
//         .pTxPort      = WIFI_UART_TX_PORT,
//         .usTxPin      = WIFI_UART_TX_PIN,
//         .pRxPort      = WIFI_UART_RX_PORT,
//         .usRxPin      = WIFI_UART_RX_PIN
//     },
//     [UART_ID_BLE] = {
//         .pInstance    = UART7,
//         .ulClockMask  = RCC_APB1Periph_UART7,
//         .ulBaudRate   = 115200UL,
//         .usWordLength = USART_WordLength_8b,
//         .usStopBits   = USART_StopBits_1,
//         .usParity     = USART_Parity_No,
//         .usHwFlowCtl  = USART_HardwareFlowControl_None,
//         .ucIsAPB2     = 0U,
//         .pTxPort      = BLE_UART_TX_PORT,
//         .usTxPin      = BLE_UART_TX_PIN,
//         .pRxPort      = BLE_UART_RX_PORT,
//         .usRxPin      = BLE_UART_RX_PIN
//     }
// };
