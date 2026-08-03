/**
 * @file    sys_health.h
 * @brief   系统健康监控、全局错误码与看门狗打卡矩阵头文件
 * @author  zry
 * @date    2026-07-31
 * @version V1.0.0
 *
 * @note    系统 HLR 追溯: [REQ-HLR-SYS-004]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef SYS_HEALTH_H_
#define SYS_HEALTH_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 全局系统状态与错误码枚举 (System Status & Error Codes)
 * ============================================================================ */
typedef enum 
{
    SYS_OK                  = 0,   /**< 0: 操作成功 / Normal */
    SYS_ERR_PARAM           = 1,   /**< 1: 无效参数 / Invalid Param */
    SYS_ERR_HEALTH_INIT     = 2,   /**< 2: 健康监控初始化失败 */
    SYS_ERR_HW_FAIL         = 3,   /**< 3: 硬件 BSP 初始化失败 */
    SYS_ERR_RTE_INIT        = 4,   /**< 4: RTE 信号字典初始化失败 */
    SYS_ERR_PDI_INIT        = 5,   /**< 5: PDI 协议管理器初始化失败 */
    SYS_ERR_APP_INIT        = 6,   /**< 6: Task Control 业务任务初始化失败 */
    SYS_ERR_OTA_INIT        = 7,   /**< 7: Task OTA 任务初始化失败 */
    SYS_ERR_TIMEOUT         = 8,   /**< 8: 执行超时 */
    SYS_ERR_SAFE_STATE      = 9    /**< 9: 系统已强切入安全状态 */
} Sys_Status_t;

/* ============================================================================
 * 任务打卡 ID 枚举 (用于看门狗 Liveness Matrix 矩阵)
 * ============================================================================ */
typedef enum
{
    SYS_TASK_ID_1MS         = 0,   /**< 1ms 健康监控任务 */
    SYS_TASK_ID_10MS        = 1,   /**< 10ms 控制算法与 PDI 任务 */
    SYS_TASK_ID_100MS       = 2,   /**< 100ms 监控任务 */
    SYS_TASK_ID_1000MS      = 3,   /**< 1000ms 心跳/OTA 任务 */
    SYS_TASK_ID_MAX
} Sys_Task_ID_t;

/* ============================================================================
 * 严重故障类型枚举
 * ============================================================================ */
typedef enum
{
    SYS_FAULT_NONE          = 0,
    SYS_FAULT_INIT_FAILURE  = 1,   /**< 初始化序列失败 */
    SYS_FAULT_WATCHDOG      = 2,   /**< 任务死锁/看门狗超时 */
    SYS_FAULT_HARDFAULT     = 3    /**< RISC-V 硬件 Exception Trap */
} Sys_Fault_Code_t;

/* ============================================================================
 * 系统健康服务接口声明
 * ============================================================================ */

/**
 * @brief  初始化系统健康监控器与看门狗
 * @return Sys_Status_t SYS_OK 代表成功
 */
Sys_Status_t Sys_Health_Init(void);

/**
 * @brief  系统强切入 Safe-State 安全状态
 * @param  fault_code 导致切入安全状态的故障码
 */
void Sys_Health_EnterSafeState(Sys_Fault_Code_t fault_code);

/**
 * @brief  Safe-State 挂起循环 (仅允许执行安全打卡与停机逻辑)
 */
void Sys_Health_ExecSafeStateLoop(void);

/**
 * @brief  周期任务向健康监控器打卡
 * @param  task_id 任务 ID
 */
void Sys_Health_TaskReport(Sys_Task_ID_t task_id);

/**
 * @brief  校验所有任务的打卡矩阵状态
 * @return Sys_Status_t SYS_OK 代表所有任务运行正常
 */
Sys_Status_t Sys_Health_VerifyLivenessMatrix(void);

/**
 * @brief  刷新/喂硬件看门狗
 */
void Sys_Health_ServiceWatchdog(void);

#ifdef __cplusplus
}
#endif

#endif /* SYS_HEALTH_H_ */