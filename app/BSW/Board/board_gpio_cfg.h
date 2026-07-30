/**
 * @file    board_gpio_cfg.h
 * @brief   GPIO 外设抽象配置描述头文件（数据驱动模式）
 * @details 定义离散量基础 GPIO 的数据驱动抽象结构体与配置项。
 * @author  zry
 * @date    2026-07-30
 * @version V0.0.1
 *
 * @note    System HLR Traceability: [REQ-HLR-BSP-002]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_GPIO_CFG_H
#define BOARD_GPIO_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v30x.h"

/**
 * @brief 单个/组 GPIO 引脚初始化配置描述结构体
 */
typedef struct {
    GPIO_TypeDef*      pPort;            /* GPIO 端口基地址 */
    uint16_t           usPin;            /* GPIO 引脚掩码 */
    GPIOMode_TypeDef   eMode;            /* 输入/输出模式 */
    GPIOSpeed_TypeDef  eSpeed;           /* 输出翻转速率 */
    BitAction          eInitialState;    /* 初始安全电平状态 */
    uint8_t            ucApplyInitState; /* 1: 配置模式前强置初始电平; 0: 不设置 */
} Board_GPIO_Config_t;

extern const Board_GPIO_Config_t g_GpioConfigTable[];
extern const uint8_t g_GpioConfigTableSize;

#ifdef __cplusplus
}
#endif

#endif /* BOARD_GPIO_CFG_H */