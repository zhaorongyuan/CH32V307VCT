/**
 * @file    bsp_board.h
 * @brief   板级支持包 (BSP) 抽象接口头文件（DAL-A/B 级加固版）
 * @details 采用 static inline 强化类型安全，屏蔽电路电平极性。
 * @author  zry
 * @date    2026-07-30
 * @version V0.0.1
 *
 * @note    System HLR Traceability: [REQ-HLR-BSP-004]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BSP_BOARD_H
#define BSP_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v30x.h"
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
/* 2. 板级初始化对外 API 声明                                                   */
/* ============================================================================ */
void System_Core_Init(void);
Board_Status_t Board_Init(void);
Board_Status_t Board_Gpio_Init(void);
Board_Status_t Board_Uart_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BOARD_H */