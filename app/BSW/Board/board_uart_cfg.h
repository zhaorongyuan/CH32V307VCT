/**
 * @file    board_uart_cfg.h
 * @brief   UART 参数配置（仅宏定义，不含逻辑）
 * @author  zry
 * @date    2026-07-17
 * @version V1.0.0
 *
 * @note    更换硬件平台时只需修改此文件
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_UART_CFG_H
#define BOARD_UART_CFG_H

#include "ch32v30x.h"

/* ========== 调试串口配置 (USART1) ========== */
#define DBG_UART_PERIPH         USART1
#define DBG_UART_BAUD           115200U
#define DBG_UART_WordLength     USART_WordLength_8b
#define DBG_UART_StopBits       USART_StopBits_1
#define DBG_UART_Parity         USART_Parity_No

/* ========== WiFi 串口配置 (USART3) ========== */
#define WIFI_UART_PERIPH        USART3
#define WIFI_UART_BAUD          115200U
#define WIFI_UART_WordLength    USART_WordLength_8b
#define WIFI_UART_StopBits      USART_StopBits_1
#define WIFI_UART_Parity        USART_Parity_No

/* ========== UART 通道枚举 ========== */
typedef enum {
    UART_ID_DEBUG = 0,      /* USART1 - 调试串口 */
    UART_ID_WIFI,           /* USART3 - ESP8266 WiFi */
    UART_ID_MAX
} UART_ID_t;

#endif /* BOARD_UART_CFG_H */
