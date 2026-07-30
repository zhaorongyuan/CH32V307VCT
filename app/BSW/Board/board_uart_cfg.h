/**
 * @file    board_uart_cfg.h
 * @brief   UART 通信接口参数配置头文件（纯数据与结构定义）
 * @details 包含 UART1, UART2, WiFi(UART6), BLE(UART7) 物理引脚与协议参数数据描述。
 * @author  zry
 * @date    2026-07-30
 * @version V2.2.0
 *
 * @note    System HLR Traceability: [REQ-HLR-BSP-003]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_UART_CFG_H
#define BOARD_UART_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v30x.h"

/**
 * @brief 串口逻辑通道枚举
 */
typedef enum {
    UART_ID_DEBUG1 = 0U,    /* USART1 - 主调试串口 (PA9/PA10) */
    UART_ID_DEBUG2,         /* USART2 - 跳线调试串口 (PA2/PA3) */
    UART_ID_WIFI,           /* UART6  - ESP8266 WiFi (PC0/PC1) */
    UART_ID_BLE,            /* UART7  - CH9141 蓝牙 (PC2/PC3) */
    UART_ID_MAX
} UART_ID_t;

/**
 * @brief 串口数据驱动完备描述结构体（解耦包含硬件物理引脚）
 */
typedef struct {
    USART_TypeDef*      pInstance;      /* 串口寄存器基地址 */
    uint32_t            ulClockMask;    /* 时钟使能掩码 */
    uint32_t            ulBaudRate;     /* 波特率 */
    uint16_t            usWordLength;   /* 数据位 */
    uint16_t            usStopBits;     /* 停止位 */
    uint16_t            usParity;       /* 校验位 */
    uint16_t            usHwFlowCtl;    /* 流控 */
    uint8_t             ucIsAPB2;       /* 1: APB2 总线; 0: APB1 总线 */
    
    /* 物理引脚解耦映射 (来自 board_pin.h) */
    GPIO_TypeDef*       pTxPort;        /* TX 引脚端口 */
    uint16_t            usTxPin;        /* TX 引脚掩码 */
    GPIO_TypeDef*       pRxPort;        /* RX 引脚端口 */
    uint16_t            usRxPin;        /* RX 引脚掩码 */
} UART_Config_t;

extern const UART_Config_t g_UartConfigTable[UART_ID_MAX];

#ifdef __cplusplus
}
#endif

#endif /* BOARD_UART_CFG_H */