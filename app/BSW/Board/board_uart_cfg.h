/**
 * @file    board_uart_cfg.h
 * @brief   UART 通信接口参数配置头文件（纯数据与宏定义）
 * @details 统一管理 UART1, UART2, UART6(WiFi), UART7(BLE) 的物理通道与协议参数。
 *          符合 DO-178C Section 11.13 HSI Configuration Data 规范。
 * @author  zry
 * @date    2026-07-29
 * @version V2.0.0
 *
 * @docref  HSI-SPEC-CH32V30X-REV2
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_UART_CFG_H
#define BOARD_UART_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v30x.h"

/* ============================================================================ */
/* 1. 串口逻辑通道枚举                                                          */
/* ============================================================================ */
typedef enum {
    UART_ID_DEBUG1 = 0,     /* USART1 - 主调试串口 (PA9/PA10) */
    UART_ID_DEBUG2,         /* USART2 - 跳线调试串口 (PA2/PA3) */
    UART_ID_WIFI,           /* UART6  - ESP8266 WiFi (PC0/PC1) */
    UART_ID_BLE,            /* UART7  - CH9141 蓝牙 (PC2/PC3) */
    UART_ID_MAX
} UART_ID_t;


/* ============================================================================ */
/* 2. 各串口硬件基地址与波特率定义                                              */
/* ============================================================================ */
#define DBG_UART1_PERIPH            USART1
#define DBG_UART1_CLK               RCC_APB2Periph_USART1
#define DBG_UART1_BAUD              115200U

#define DBG_UART2_PERIPH            USART2
#define DBG_UART2_CLK               RCC_APB1Periph_USART2
#define DBG_UART2_BAUD              115200U

#define WIFI_UART_PERIPH            UART6
#define WIFI_UART_CLK               RCC_APB1Periph_UART6
#define WIFI_UART_BAUD              115200U

#define BLE_UART_PERIPH             UART7
#define BLE_UART_CLK                RCC_APB1Periph_UART7
#define BLE_UART_BAUD               115200U


/* ============================================================================ */
/* 3. 串口数据驱动表结构体定义                                                  */
/* ============================================================================ */
typedef struct {
    USART_TypeDef*      pInstance;          /* 串口寄存器基地址 */
    uint32_t            ulClockMask;        /* 时钟使能掩码 */
    uint32_t            ulBaudRate;         /* 波特率 */
    uint16_t            usWordLength;       /* 数据位 */
    uint16_t            usStopBits;         /* 停止位 */
    uint16_t            usParity;           /* 校验位 */
    uint16_t            usHwFlowCtl;        /* 流控 */
    uint8_t             ucIsAPB2;           /* 1: APB2 总线; 0: APB1 总线 */
} UART_Config_t;

#ifdef __cplusplus
}
#endif

#endif /* BOARD_UART_CFG_H */