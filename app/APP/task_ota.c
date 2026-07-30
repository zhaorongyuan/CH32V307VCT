/**
 * @file    task_ota.c
 * @brief   OTA 升级任务实现
 * @author  zry
 * @date    2026-07-17
 * @version V0.0.1
 *
 * @note    仅调用 Interface 层 API，不直接访问 BSW
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "task_ota.h"
#include "Interface.h"
#include "app_global.h"

/**
 * @brief  OTA 任务初始化
 */
void Task_OTA_Init(void)
{
    /* TODO: OTA 相关变量/状态初始化 */
}

/**
 * @brief  OTA 任务周期处理（1000ms）
 */
void Task_OTA_Update(void)
{
    /* TODO: 检查是否有 OTA 升级请求 */

    /* TODO: 接收固件数据包 */

    /* TODO: 写入 Flash */
}
