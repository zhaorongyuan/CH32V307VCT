/**
 * @file    app_global.c
 * @brief   全局共享变量定义
 * @author  zry
 * @date    2026-07-17
 * @version V1.0.0
 *
 * @note    所有跨模块共享的全局变量在此定义
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "app_global.h"

/* ========== 系统心跳 ========== */
volatile uint32_t Gc_SysTick_ms = 0;

/* ========== 系统状态 ========== */
volatile uint32_t Gs_CpuLoad_percent = 0;
