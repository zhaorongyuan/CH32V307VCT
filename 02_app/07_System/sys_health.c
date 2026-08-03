/**
 * @file sys_health.c
 * @brief 系统健康监控、全局错误码与看门狗管理实现文件 (精简版)
 * @details 提供确定性故障码记录、Fail-Safe 状态机切换与看门狗硬件服务。
 * @author zry
 * @date 2026-08-03
 * @version V1.5.0
 *
 * @note System HLR Traceability: [REQ-HLR-SYS-004]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "sys_health.h"

/* ============================================================================
 * C11 静态断言: 校验枚举界限与类型大小 (MISRA-C:2012 Compliance)
 * ============================================================================ */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert(sizeof(Sys_Status_t) == 4U, "Sys_Status_t size must be 32-bit");
_Static_assert(sizeof(Sys_Fault_Code_t) == 4U, "Sys_Fault_Code_t size must be 32-bit");
_Static_assert(SYS_FAULT_COUNT == 5U, "Fault code count mismatch");
#endif

/* ============================================================================
 * 模块内部静态变量 (静态内存分配, 无 Dynamic Heap)
 * ============================================================================ */

static volatile bool             g_is_safe_state_active = false;
static volatile Sys_Fault_Code_t g_last_fault_code      = SYS_FAULT_NONE;

/* ============================================================================
 * 底层硬件抽象私有函数 (HSI - Hardware Software Interface)
 * ============================================================================ */

/**
 * @brief  进入临界区 (保存并禁用中断)
 * @return uint32_t 保存的中断状态字
 */
static inline uint32_t HM_EnterCritical(void)
{
    uint32_t primask = 0U;
#if defined(__ARM_ARCH) || defined(__ARM_ARCH_7EM__) || defined(__TARGET_ARCH_ARM)
    __asm__ volatile("mrs %0, primask\n cpsid i" : "=r"(primask) :: "memory");
#elif defined(__riscv)
    __asm__ volatile("csrrc %0, mstatus, %1" : "=r"(primask) : "r"(8U) : "memory");
#else
#error "Critical section not implemented for target architecture!"
#endif
    return primask;
}

/**
 * @brief  退出临界区 (恢复中断状态)
 * @param[in] primask 保存的中断状态字
 */
static inline void HM_ExitCritical(const uint32_t primask)
{
#if defined(__ARM_ARCH) || defined(__ARM_ARCH_7EM__) || defined(__TARGET_ARCH_ARM)
    __asm__ volatile("msr primask, %0" ::"r"(primask) : "memory");
#elif defined(__riscv)
    __asm__ volatile("csrw mstatus, %0" :: "r"(primask) : "memory");
#else
#error "Critical section not implemented for target architecture!"
#endif
}

/**
 * @brief 硬件级切断所有危险执行机构输出 (如关闭 PWM/高压继电器)
 */
static void HM_Hw_DisableActuators(void)
{
    /* HSI: 将硬件安全控制 GPIO 拉低 */
}

/**
 * @brief 写入故障日志至非易失黑匣子
 */
static void HM_Hw_WriteBlackboxLog(const Sys_Fault_Code_t fault)
{
    (void)fault;
    /* HSI: NVM 写入逻辑 */
}

/**
 * @brief 底层硬件看门狗物理脉冲触发
 */
static void HM_Hw_KickHardwareWatchdog(void)
{
    /* HSI: 寄存器写入硬件喂狗指令 */
}

/* ============================================================================
 * 公开接口实现 (Public API Implementations)
 * ============================================================================ */

/**
 * @brief  初始化系统健康监控器与看门狗
 * @note LLR Traceability: [REQ-SW-HM-001]
 * @note Safety Criticality: DAL-A
 */
Sys_Status_t Sys_Health_Init(Sys_InitFaultMask_t *const out_init_mask)
{
    Sys_Status_t status = SYS_OK;
    Sys_InitFaultMask_t fault_mask = SYS_INIT_FAULT_NONE;

    const uint32_t primask = HM_EnterCritical();

    g_is_safe_state_active = false;
    g_last_fault_code      = SYS_FAULT_NONE;

    HM_ExitCritical(primask);

    /* 硬件自检与看门狗初始化 */
    /* 若检测失败: fault_mask |= SYS_INIT_FAULT_HEALTH_BIT; status = SYS_ERR_HW_FAIL; */

    if (out_init_mask != NULL)
    {
        *out_init_mask = fault_mask;
    }

    return status;
}

/**
 * @brief  系统强切入 Safe-State 安全状态
 * @note LLR Traceability: [REQ-SW-HM-002]
 * @note Safety Criticality: DAL-A
 */
void Sys_Health_EnterSafeState(const Sys_Fault_Code_t fault_code)
{
    Sys_Fault_Code_t actual_fault = fault_code;

    /* 防御性校验：若传入非法故障码或 SYS_FAULT_NONE，强制修正为 SYS_FAULT_HARDFAULT */
    if (!IS_SYS_FAULT_CODE_VALID(fault_code) || (fault_code == SYS_FAULT_NONE))
    {
        actual_fault = SYS_FAULT_HARDFAULT;
    }

    const uint32_t primask = HM_EnterCritical();

    g_is_safe_state_active = true;
    g_last_fault_code      = actual_fault;

    /* 物理级关闭执行机构 */
    HM_Hw_DisableActuators();

    /* 写入非易失黑匣子 */
    HM_Hw_WriteBlackboxLog(actual_fault);

    HM_ExitCritical(primask);
}

/**
 * @brief  Safe-State 挂起死循环
 * @note LLR Traceability: [REQ-SW-HM-003]
 * @note Safety Criticality: DAL-A | Non-returning
 */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Noreturn void Sys_Health_ExecSafeStateLoop(void)
#elif defined(__GNUC__) || defined(__clang__)
void Sys_Health_ExecSafeStateLoop(void)
#else
void Sys_Health_ExecSafeStateLoop(void)
#endif
{
    if (!Sys_Health_IsSafeStateActive())
    {
        Sys_Health_EnterSafeState(SYS_FAULT_HARDFAULT);
    }

    for (;;)
    {
        /* Safe-State 下持续刷新底层硬件看门狗，保持 Fail-Safe 维持状态 */
        HM_Hw_KickHardwareWatchdog();

#if defined(__ARM_ARCH) || defined(__ARM_ARCH_7EM__) || defined(__TARGET_ARCH_ARM)
        __asm__ volatile("wfi");
#elif defined(__riscv)
        __asm__ volatile("wfi");
#else
        for (volatile uint32_t delay = 0U; delay < 10000U; delay++)
        {
            /* Fallback delay */
        }
#endif
    }
}

/**
 * @brief  刷新/喂硬件看门狗 (正常业务模式)
 * @note LLR Traceability: [REQ-SW-HM-006]
 * @note Safety Criticality: DAL-A
 */
Sys_Status_t Sys_Health_ServiceWatchdog(void)
{
    Sys_Status_t status = SYS_OK;

    const uint32_t primask = HM_EnterCritical();

    /* 门禁：仅在非 Safe-State 下允许正常业务喂狗 */
    if (!g_is_safe_state_active)
    {
        HM_Hw_KickHardwareWatchdog();
        status = SYS_OK;
    }
    else
    {
        status = SYS_ERR_BUSY;
    }

    HM_ExitCritical(primask);

    return status;
}

/**
 * @brief  查询系统当前是否处于 Safe-State
 * @note LLR Traceability: [REQ-SW-HM-007]
 */
bool Sys_Health_IsSafeStateActive(void)
{
    const uint32_t primask = HM_EnterCritical();
    const bool active = g_is_safe_state_active;
    HM_ExitCritical(primask);

    return active;
}

/**
 * @brief  获取导致切入 Safe-State 的故障码
 * @note LLR Traceability: [REQ-SW-HM-008]
 */
Sys_Fault_Code_t Sys_Health_GetLastFault(void)
{
    const uint32_t primask = HM_EnterCritical();
    const Sys_Fault_Code_t fault = g_last_fault_code;
    HM_ExitCritical(primask);

    return fault;
}