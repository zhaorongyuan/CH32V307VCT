/**
 * @file    board.c
 * @brief   板级初始化实现
 * @author  zry
 * @date    2026-07-17
 * @version V1.0.0
 *
 * @note    根据 board_pin.h / board_uart_cfg.h 完成所有板级外设初始化
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "board.h"
#include "board_pin.h"
#include "board_uart_cfg.h"
#include "ch32v30x_conf.h"

/**
 * @brief  板级 GPIO 外设初始化（LED、Key、Buzzer 等）
 */
void Board_Gpio_Init(void)
{
    /* TODO: 使能 GPIO 时钟 */

    /* TODO: 配置 LED 引脚 (PA0, PA1) - 推挽输出 */

    /* TODO: 配置按键引脚 (PB0) - 输入上拉 */

    /* TODO: 配置蜂鸣器引脚 (PB5) - 推挽输出 */
}

/**
 * @brief  板级 UART 外设初始化
 */
void Board_Uart_Init(void)
{
    /* TODO: 配置调试串口 USART1 (PA9-TX, PA10-RX) */

    /* TODO: 配置 WiFi 串口 USART3 (PB10-TX, PB11-RX) */
}

/**
 * @brief  板级硬件总初始化入口
 */
void Board_Init(void)
{
    Board_Gpio_Init();
    Board_Uart_Init();
}
