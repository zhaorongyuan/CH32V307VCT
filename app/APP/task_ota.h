/**
 * @file    task_ota.h
 * @brief   OTA 升级任务声明
 * @author  zry
 * @date    2026-07-17
 * @version V1.0.0
 *
 * @note    由 main() 中的 Super Loop 调度器按 1000ms 周期调用
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef TASK_OTA_H
#define TASK_OTA_H

#include <stdint.h>

/**
 * @brief  OTA 任务初始化
 */
void Task_OTA_Init(void);

/**
 * @brief  OTA 任务周期处理（1000ms）
 * @note   检查升级请求、接收固件数据、写入 Flash
 */
void Task_OTA_Update(void);

#endif /* TASK_OTA_H */
