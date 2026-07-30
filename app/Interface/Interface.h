/**
 * @file    Interface.h
 * @brief   统一 HAL 接口层 — 所有硬件 API 声明
 * @author  zry
 * @date    2026-07-17
 * @version V0.0.1
 *
 * @note    APP 层只能调用本文件声明的 API，不得直接访问 BSW 层
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef INTERFACE_H
#define INTERFACE_H

#include <stdint.h>
#include "ch32v30x.h"
#include "board_uart_cfg.h"

/* ================================================================
 *  HAL - LED
 * ================================================================ */

/**
 * @brief  初始化所有 LED 引脚
 */
void API_Led_Init(void);

/**
 * @brief  控制 LED 开关
 * @param  led_id: 0=状态灯(PA0), 1=网络灯(PA1)
 * @param  on:     1=亮, 0=灭
 */
void API_Led_Set(uint8_t led_id, uint8_t on);

/**
 * @brief  翻转 LED 状态
 * @param  led_id: 0=状态灯, 1=网络灯
 */
void API_Led_Toggle(uint8_t led_id);

/* ================================================================
 *  HAL - UART
 * ================================================================ */

/**
 * @brief  初始化指定 UART 通道
 * @param  uart_id: UART_ID_DEBUG / UART_ID_WIFI
 * @param  baud:    波特率
 */
void API_Uart_Init(UART_ID_t uart_id, uint32_t baud);

/**
 * @brief  通过指定 UART 发送数据
 * @param  uart_id: UART 通道
 * @param  data:    数据指针
 * @param  len:     数据长度
 */
void API_Uart_Send(UART_ID_t uart_id, const uint8_t *data, uint16_t len);

/* ================================================================
 *  HAL - Key (按键)
 * ================================================================ */

/**
 * @brief  初始化按键引脚
 */
void API_Key_Init(void);

/**
 * @brief  读取按键状态
 * @return 1=按下, 0=释放
 */
uint8_t API_Key_Read(void);

/* ================================================================
 *  HAL - Buzzer (蜂鸣器)
 * ================================================================ */

/**
 * @brief  初始化蜂鸣器引脚
 */
void API_Buzzer_Init(void);

/**
 * @brief  控制蜂鸣器开关
 * @param  on: 1=响, 0=停
 */
void API_Buzzer_Set(uint8_t on);

/* ================================================================
 *  HAL - WiFi (ESP8266)
 * ================================================================ */

/**
 * @brief  初始化 WiFi 模块（内部调用 UART 初始化）
 */
void API_Wifi_Init(void);

/**
 * @brief  向 WiFi 模块发送 AT 指令字符串
 * @param  cmd: AT 指令字符串（含 \r\n）
 * @return SET=成功, RESET=忙
 */
FlagStatus API_Wifi_SendCmd(const char *cmd);

/**
 * @brief  从 WiFi 模块接收数据
 * @param  buf: 接收缓冲区
 * @param  max_len: 缓冲区最大长度
 * @return 实际接收字节数
 */
uint16_t API_Wifi_Recv(char *buf, uint16_t max_len);

#endif /* INTERFACE_H */
