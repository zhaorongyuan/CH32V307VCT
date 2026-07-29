/**
 * @file    board.h
 * @brief   板级初始化接口声明
 * @author  zry
 * @date    2026-07-17
 * @version V1.0.0
 *
 * @note    板级总入口，上层只需调用 Board_Init()
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

/**
 * @brief  板级硬件初始化（时钟、GPIO、UART 等）
 * @note   在 main() 中 System_Core_Init() 之后调用
 */
void Board_Init(void);

/**
 * @brief  板级 UART 外设初始化
 * @note   内部根据 board_pin.h / board_uart_cfg.h 配置各 UART
 */
void Board_Uart_Init(void);

/**
 * @brief  板级 GPIO 外设初始化（LED、Key、Buzzer 等）
 * @note   内部根据 board_pin.h 配置各 GPIO
 */
void Board_Gpio_Init(void);

#endif /* BOARD_H */
