/**
 * @file    board_pin.h
 * @brief   板级引脚映射定义（仅宏定义，不含逻辑）
 * @author  zry
 * @date    2026-07-17
 * @version V1.0.0
 *
 * @note    更换硬件平台时只需修改此文件
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_PIN_H
#define BOARD_PIN_H

#include "ch32v30x.h"

/* ========== LED 引脚定义 ========== */
#define LED_STATUS_PIN      GPIO_Pin_0
#define LED_STATUS_PORT     GPIOA
#define LED_NET_PIN         GPIO_Pin_1
#define LED_NET_PORT        GPIOA

/* ========== 按键引脚定义 ========== */
#define KEY_PIN             GPIO_Pin_0
#define KEY_PORT            GPIOB

/* ========== 蜂鸣器引脚定义 ========== */
#define BUZZER_PIN          GPIO_Pin_5
#define BUZZER_PORT         GPIOB

/* ========== 调试串口引脚定义 (USART1) ========== */
#define DEBUG_UART_TX_PIN   GPIO_Pin_9
#define DEBUG_UART_RX_PIN   GPIO_Pin_10
#define DEBUG_UART_PORT     GPIOA
#define DEBUG_UART_PERIPH   USART1

/* ========== WiFi 串口引脚定义 (USART3) ========== */
#define WIFI_UART_TX_PIN    GPIO_Pin_10
#define WIFI_UART_RX_PIN    GPIO_Pin_11
#define WIFI_UART_PORT      GPIOB
#define WIFI_UART_PERIPH    USART3

#endif /* BOARD_PIN_H */
