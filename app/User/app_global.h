/**
 * @file    app_global.h
 * @brief   全局共享变量声明（跨模块/跨层使用）
 * @author  zry
 * @date    2026-07-17
 * @version V1.0.0
 *
 * @note    中断与主循环共享的变量需加 volatile 修饰
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef APP_GLOBAL_H
#define APP_GLOBAL_H

#include <stdint.h>

/* ========== 系统心跳（SysTick 中断中递增） ========== */

/** 系统毫秒计数器，1ms 递增，由 SysTick_Handler 维护 */
extern volatile uint32_t Gc_SysTick_ms;

/* ========== 系统状态 ========== */

/** CPU 负载率（百分比，整数 0~100） */
extern volatile uint32_t Gs_CpuLoad_percent;

#endif /* APP_GLOBAL_H */
