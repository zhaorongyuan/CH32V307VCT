/**
 * @file    Interface.h
 * @brief   变量声明
 * @author  zry
 * @date    2026-07-17
 * @version V1.0.0
 *
 * @note    
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef _IINTERFACE_H
#define _IINTERFACE_H

#include <stdint.h>
#include "ch32v30x.h"
#include "core_riscv.h"

/* BSW */

extern volatile float Gs_CpuLoad_tick;
/* 全局系统心跳变量 */
extern volatile uint32_t Gc_SysTick_ms;

/* ASW */


#endif /*_IINTERFACE_H */