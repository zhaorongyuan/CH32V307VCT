/**
 * @file sys_health.h
 * @brief 系统健康监控、全局错误码与看门狗管理头文件 (精简版)
 * @details 本模块仅保留核心故障码定义、硬件看门狗喂狗与 Fail-Safe 状态机管控，去除了任务打卡矩阵。
 * @author zry
 * @date 2026-08-03
 * @version V1.5.0
 *
 * @note System HLR Traceability: [REQ-HLR-SYS-004]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef SYS_HEALTH_H_
#define SYS_HEALTH_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>  /* <--- 补上此头文件，提供 NULL 宏定义 */

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 全局系统状态与错误码枚举 (System Status & Error Codes)
 * ============================================================================ */

/**
 * @brief 系统 API 通用返回值枚举 (显式强制 32-bit 宽度)
 */
typedef enum
{
    SYS_OK                 = 0x00000000U, /**< 0: 操作成功 / Normal */
    SYS_ERR_PARAM          = 0x00000001U, /**< 1: 无效参数 / Invalid Param / Null Pointer */
    SYS_ERR_TIMEOUT        = 0x00000002U, /**< 2: 执行超时 / Hardware Timeout */
    SYS_ERR_BUSY           = 0x00000003U, /**< 3: 模块忙 / Safe-State 激活中拒绝对外服务 */
    SYS_ERR_HW_FAIL        = 0x00000004U, /**< 4: 硬件响应故障 / Hardware Failure */
    SYS_ERR_CORRUPTED      = 0x00000005U, /**< 5: 数据/内存损坏 (CRC/Parity Error) */

    SYS_STATUS_COUNT       = 0x00000006U, /**< 状态总数哨兵，用于防御性边界校验 */
    SYS_STATUS_FORCE_32BIT = 0x7FFFFFFFU  /**< 强制编译器将此枚举编译为 32-bit 宽度 */
} Sys_Status_t;

/**
 * @brief 系统 POST/BIT 初始化阶段故障位掩码 (Bitmask)
 */
typedef uint32_t Sys_InitFaultMask_t;

#define SYS_INIT_FAULT_NONE       (0x00000000U)
#define SYS_INIT_FAULT_HEALTH_BIT (1U << 0) /**< Bit 0: 健康监控器自身初始化失败 */
#define SYS_INIT_FAULT_BSP_BIT    (1U << 1) /**< Bit 1: 硬件 BSP/驱动初始化失败 */
#define SYS_INIT_FAULT_RTE_BIT    (1U << 2) /**< Bit 2: RTE 信号字典初始化失败 */
#define SYS_INIT_FAULT_PDI_BIT    (1U << 3) /**< Bit 3: PDI 总线协议管理器初始化失败 */
#define SYS_INIT_FAULT_APP_BIT    (1U << 4) /**< Bit 4: Task Control 业务任务初始化失败 */
#define SYS_INIT_FAULT_MAINT_BIT  (1U << 5) /**< Bit 5: Maintenance 模块初始化失败 */
#define SYS_INIT_FAULT_OTA_BIT    SYS_INIT_FAULT_MAINT_BIT /**< 兼容旧版名称 */

/* ============================================================================
 * 严重故障类型枚举
 * ============================================================================ */

/**
 * @brief 触发切入 Fail-Safe 安全状态的严重故障分类 (显式强制 32-bit 宽度)
 */
typedef enum
{
    SYS_FAULT_NONE         = 0x00000000U,
    SYS_FAULT_INIT_FAILURE = 0x00000001U, /**< 初始化序列失败 */
    SYS_FAULT_WATCHDOG     = 0x00000002U, /**< 看门狗超时 / 喂狗序列异常 */
    SYS_FAULT_HARDFAULT    = 0x00000003U, /**< 硬件 Exception Trap / NMI */
    SYS_FAULT_RAM_CORRUPT  = 0x00000004U, /**< 关键内存校验 (ECC/Parity) 错误 */

    SYS_FAULT_COUNT        = 0x00000005U, /**< 故障码边界上限哨兵 */
    SYS_FAULT_FORCE_32BIT  = 0x7FFFFFFFU  /**< 强制 32-bit 宽度 */
} Sys_Fault_Code_t;

/* ============================================================================
 * 防御性编程边界校验内联宏
 * ============================================================================ */

#define IS_SYS_STATUS_VALID(status)    ((uint32_t)(status) < (uint32_t)SYS_STATUS_COUNT)
#define IS_SYS_FAULT_CODE_VALID(fault) ((uint32_t)(fault) < (uint32_t)SYS_FAULT_COUNT)

/* ============================================================================
 * 系统健康服务接口声明
 * ============================================================================ */

/**
 * @brief  初始化系统健康监控器与看门狗硬件
 * @details 复位内部安全状态机，使能底层硬件看门狗。
 *
 * @param[out] out_init_mask 可选的初始化故障位掩码输出指针 (可传入 NULL)。
 *                           若非空，成功时写入 SYS_INIT_FAULT_NONE，失败时写入发生的故障 Bit 位。
 * @return Sys_Status_t
 * @retval SYS_OK           初始化成功
 * @retval SYS_ERR_HW_FAIL  硬件看门狗或自检配置失败
 *
 * @note LLR Traceability: [REQ-SW-HM-001]
 * @note Safety Criticality: DAL-A | Bounded Execution Time | No Dynamic Memory
 */
Sys_Status_t Sys_Health_Init(Sys_InitFaultMask_t *const out_init_mask);

/**
 * @brief  系统强切入 Safe-State 安全状态
 * @details 记录故障码至黑匣子/NVM，强行切断执行机构输出。若传入非法故障码，
 *          系统将强制矫正为 SYS_FAULT_HARDFAULT 并记录。
 *
 * @param[in] fault_code 导致切入安全状态的故障码
 *
 * @note LLR Traceability: [REQ-SW-HM-002]
 * @note Safety Criticality: DAL-A | Fail-Safe Entry | Side Effect: Disable Actuators
 */
void Sys_Health_EnterSafeState(const Sys_Fault_Code_t fault_code);

/**
 * @brief  Safe-State 挂起死循环
 * @details 仅允许执行最低限度的硬件看门狗刷新与维持逻辑，禁止任何业务指令响应。
 *
 * @note LLR Traceability: [REQ-SW-HM-003]
 * @note Safety Criticality: DAL-A | Infinite Safe Loop (Non-returning)
 */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Noreturn void Sys_Health_ExecSafeStateLoop(void);
#elif defined(__GNUC__) || defined(__clang__)
void Sys_Health_ExecSafeStateLoop(void) __attribute__((noreturn));
#else
void Sys_Health_ExecSafeStateLoop(void);
#endif

/**
 * @brief  刷新/喂硬件看门狗 (正常业务模式)
 * @details 仅在系统处于非 Safe-State 时允许正常刷新硬件看门狗。
 *
 * @return Sys_Status_t
 * @retval SYS_OK               看门狗刷新成功
 * @retval SYS_ERR_BUSY         系统已被强切入 Safe-State，拒绝正常业务喂狗
 *
 * @note LLR Traceability: [REQ-SW-HM-006]
 * @note Safety Criticality: DAL-A | Critical Hardware I/O
 */
Sys_Status_t Sys_Health_ServiceWatchdog(void);

/**
 * @brief  查询系统当前是否处于 Safe-State 安全状态
 *
 * @return true  系统处于 Safe-State
 * @return false 系统运行正常
 *
 * @note LLR Traceability: [REQ-SW-HM-007]
 */
bool Sys_Health_IsSafeStateActive(void);

/**
 * @brief  获取导致切入 Safe-State 的故障码
 *
 * @return Sys_Fault_Code_t 故障码
 *
 * @note LLR Traceability: [REQ-SW-HM-008]
 */
Sys_Fault_Code_t Sys_Health_GetLastFault(void);

#ifdef __cplusplus
}
#endif

#endif /* SYS_HEALTH_H_ */