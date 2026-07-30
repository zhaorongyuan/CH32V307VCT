/**
 * @file    board.h
 * @brief   板级支持包 (BSP) 抽象接口头文件（DAL-A/B 级加固版）
 * @details 采用 static inline 强化类型安全，屏蔽电路电平极性。
 * @author  zry
 * @date    2026-07-30
 * @version V0.0.1
 *
 * @note    System HLR Traceability: [REQ-HLR-BSP-004]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_H
#define BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v30x.h"
#include "board_pin.h"
#include "board_gpio_cfg.h"
#include "board_uart_cfg.h"

/**
 * @brief BSP 操作状态枚举定义
 */
typedef enum {
    BOARD_OK = 0x00U,               /* 操作成功 */
    BOARD_ERR_INVALID_PARAM = 0x01U,/* 非法参数或空指针 */
    BOARD_ERR_HW_TIMEOUT = 0x02U    /* 硬件响应超时 */
} Board_Status_t;

/* ============================================================================ */
/* 1. LED 控制内联函数 (MISRA-C:2012 强类型合规)                                 */
/* ============================================================================ */

/**
 * @brief   点亮 LED1
 * @details 硬件电平驱动：输出低电平点亮。
 * @return  void
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0104]
 * @note    Safety Criticality: DAL-B / Bounded Execution
 */
static inline void Board_Led1_On(void)
{
    GPIO_ResetBits(LED1_PORT, LED1_PIN);
}

/**
 * @brief   熄灭 LED1
 * @details 硬件电平驱动：输出高电平熄灭。
 * @return  void
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0104]
 * @note    Safety Criticality: DAL-B / Bounded Execution
 */
static inline void Board_Led1_Off(void)
{
    GPIO_SetBits(LED1_PORT, LED1_PIN);
}

/**
 * @brief   翻转 LED1 状态
 * @details 读取当前输出锁存状态并取反翻转（显式与 Bit_RESET 校验）。
 * @return  void
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0104]
 * @note    Safety Criticality: DAL-B / Zero Ambiguity
 */
static inline void Board_Led1_Toggle(void)
{
    const uint8_t ucBitState = GPIO_ReadOutputDataBit(LED1_PORT, LED1_PIN);
    if (ucBitState != (uint8_t)Bit_RESET) {
        GPIO_ResetBits(LED1_PORT, LED1_PIN);
    } else {
        GPIO_SetBits(LED1_PORT, LED1_PIN);
    }
}

/* ============================================================================ */
/* 2. 板级初始化对外 API 声明                                                   */
/* ============================================================================ */
Board_Status_t Board_Init(void);
Board_Status_t Board_Gpio_Init(void);
Board_Status_t Board_Uart_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */