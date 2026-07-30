/**
 * @file    Interface.c
 * @brief   统一 HAL 接口层 — 所有硬件 API 实现
 * @author  zry
 * @date    2026-07-17
 * @version V0.0.1
 *
 * @note    内部调用 BSW 层完成实际操作，对外仅暴露 API_* 接口
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "Interface.h"
#include "board_pin.h"
#include "board_uart_cfg.h"
#include "ch32v30x_conf.h"

/* ================================================================
 *  HAL - LED
 * ================================================================ */

void API_Led_Init(void)
{
    /* TODO: 使能 GPIOA 时钟 */
    /* TODO: 配置 PA0, PA1 为推挽输出 */
}

void API_Led_Set(uint8_t led_id, uint8_t on)
{
    /* TODO: 根据 led_id 控制对应 LED 引脚电平 */
    (void)led_id;
    (void)on;
}

void API_Led_Toggle(uint8_t led_id)
{
    /* TODO: 翻转对应 LED 引脚电平 */
    (void)led_id;
}

/* ================================================================
 *  HAL - UART
 * ================================================================ */

void API_Uart_Init(UART_ID_t uart_id, uint32_t baud)
{
    /* TODO: 根据 uart_id 选择对应 USART，配置 GPIO + UART 参数 */
    (void)uart_id;
    (void)baud;
}

void API_Uart_Send(UART_ID_t uart_id, const uint8_t *data, uint16_t len)
{
    /* TODO: 通过对应 USART 发送数据 */
    (void)uart_id;
    (void)data;
    (void)len;
}

/* ================================================================
 *  HAL - Key (按键)
 * ================================================================ */

void API_Key_Init(void)
{
    /* TODO: 使能 GPIOB 时钟，配置 PB0 为上拉输入 */
}

uint8_t API_Key_Read(void)
{
    /* TODO: 读取 PB0 电平，返回按键状态 */
    return 0;
}

/* ================================================================
 *  HAL - Buzzer (蜂鸣器)
 * ================================================================ */

void API_Buzzer_Init(void)
{
    /* TODO: 使能 GPIOB 时钟，配置 PB5 为推挽输出 */
}

void API_Buzzer_Set(uint8_t on)
{
    /* TODO: 控制 PB5 电平 */
    (void)on;
}

/* ================================================================
 *  HAL - WiFi (ESP8266)
 * ================================================================ */

void API_Wifi_Init(void)
{
    /* TODO: 初始化 WiFi UART，复位 WiFi 模块 */
}

FlagStatus API_Wifi_SendCmd(const char *cmd)
{
    /* TODO: 通过 USART3 发送 AT 指令 */
    (void)cmd;
    return SET;
}

uint16_t API_Wifi_Recv(char *buf, uint16_t max_len)
{
    /* TODO: 从 USART3 接收缓冲区读取数据 */
    (void)buf;
    (void)max_len;
    return 0;
}

/* ================================================================
 *  INTERFACE - BSW
 * ================================================================ */

volatile float Gs_CpuLoad_tick = 0.0;
// volatile uint32_t Gc_SysTick_ms = 0;

/* ================================================================
 *  INTERFACE - APP
 * ================================================================ */

/* ASW */





