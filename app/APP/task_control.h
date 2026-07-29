/**
 * @file    task_control.h
 * @brief   控制任务声明（10ms / 100ms / 1000ms 周期调度）
 * @author  zry
 * @date    2026-07-17
 * @version V1.0.0
 *
 * @note    由 main() 中的 Super Loop 调度器按周期调用
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef TASK_CONTROL_H
#define TASK_CONTROL_H

#include <stdint.h>

/**
 * @brief  控制任务初始化
 */
void Task_Control_Init(void);

/**
 * @brief  10ms 周期任务
 * @note   用于高频控制逻辑（如 PID 调节、输入采样）
 */
void Task_Control_Update_10ms(void);

/**
 * @brief  100ms 周期任务
 * @note   用于中频逻辑（如状态机、LED 闪烁）
 */
void Task_Control_Update_100ms(void);

/**
 * @brief  1000ms 周期任务
 * @note   用于低频逻辑（如传感器采集、心跳上报）
 */
void Task_Control_Update_1000ms(void);

#endif /* TASK_CONTROL_H */
