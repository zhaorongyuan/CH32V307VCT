/**
 * @file main.c
 * @brief 系统 Master 主入口与时间触发调度器 (Time-Triggered Super Loop)
 * @details 负责硬件初始化校验、安全健康监控以及确定性周期任务调度，符合 RTCA DO-178C DAL-A/B 标准。
 * @author zry
 * @date 2026-08-03
 * @version V1.3.0
 *
 * @note 系统 HLR 追溯: [REQ-HLR-SYS-001], [REQ-HLR-SYS-002]
 * @copyright (c) 2026 zry. All rights reserved.
 */

/* ============================================================================
 * Include Files (严格按照“5+2”分层访问控制)
 * ============================================================================ */
#include <stddef.h>          /* 提供 NULL 宏定义 */
#include <stdint.h>
#include <stdbool.h>

#include "app_cfg.h"         /* 08_Config: 任务周期定义与全局静态参数 */
#include "bsp_board.h"       /* 04_BSP: 板级硬件总初始化入口 */
#include "pdi_mgr.h"         /* 05_Middleware/PDI: 协议数据解析管理器 */
#include "rte_interface.h"   /* 05_Middleware/RTE: ASW/BSW 信号数据字典桥梁 */
#include "task_control.h"    /* 06_APP: 核心控制业务任务 */
#include "task_ota.h"        /* 06_APP: OTA 升级业务状态机 */
#include "sys_health.h"      /* 07_System: 健康监控、看门狗与 Fault 捕获 */
#include "debug.h"           /* 07_System: 条件编译诊断 Log 与 SYS_ASSERT */

/* ============================================================================
 * 私有函数声明
 * ============================================================================ */
static Sys_Status_t System_FailSafe_InitSequence(Sys_InitFaultMask_t *const out_fault_mask);

/* ============================================================================
 * 主函数入口
 * ============================================================================ */

/**
 * @brief  系统主入口函数
 * @details 按照阶段初始化->自检失败拦截->时间触发调度超循环的顺序运行。
 *
 * @return int 遵循 ISO C 标准返回类型
 *
 * @note LLR Traceability: [REQ-SW-MAIN-001]
 * @note Safety Criticality: DAL-A | Time-Triggered Super Loop
 */
int main(void)
{
    Sys_Status_t init_status = SYS_ERR_PARAM;
    Sys_InitFaultMask_t init_faults = SYS_INIT_FAULT_NONE;

    /* ------------------------------------------------------------------------
     * 1. 执行严格的 Fail-Safe 顺序初始化与全模块故障掩码收集
     * ------------------------------------------------------------------------ */
    init_status = System_FailSafe_InitSequence(&init_faults);
    if (init_status != SYS_OK)
    {
        /* Fatal 初始化失败：记录多模块故障掩码 (init_faults)，切入 Safe-State 挂起 */
        SYS_LOG_ERROR("[MAIN] Init Failed! Status: %d, FaultMask: 0x%08X", 
                      (int32_t)init_status, (uint32_t)init_faults);

        Sys_Health_EnterSafeState(SYS_FAULT_INIT_FAILURE);
        Sys_Health_ExecSafeStateLoop(); /* 不可逆挂起死循环 */
    }

    SYS_LOG_INFO("[MAIN] Initialization Success. Starting Time-Triggered Super Loop...");

    /* ------------------------------------------------------------------------
     * 2. 调度器时间戳基准初始化 (原子读取单次起点，消除不同时间戳间的微小时间差)
     * ------------------------------------------------------------------------ */
    const uint32_t start_ticks = BSP_GetTick();
    uint32_t last_10ms_ticks   = start_ticks;
    uint32_t last_100ms_ticks  = start_ticks;
    uint32_t last_1000ms_ticks = start_ticks;

    /* ------------------------------------------------------------------------
     * 3. 确定性主循环调度 (Super Loop)
     * ------------------------------------------------------------------------ */
    while (!Sys_Health_IsSafeStateActive())
    {
        const uint32_t current_ticks = BSP_GetTick();

        /* --------------------------------------------------------------------
         * 10ms 周期任务域：核心控制算法 + PDI 协议解析 + RTE 信号更新
         * -------------------------------------------------------------------- */
        if ((current_ticks - last_10ms_ticks) >= APP_TASK_10MS_PERIOD)
        {
            /* 相位加法补偿，消除时钟累积漂移 (Timing Drift Fix) */
            last_10ms_ticks += APP_TASK_10MS_PERIOD;

            /* A. 执行 10ms 核心控制算法 */
            Task_Control_Update_10ms();

            /* B. 处理 PDI 通信串口数据解包 */
            PDI_Mgr_Process();

            /* C. 更新 RTE 信号字典与故障位图 */
            RTE_Signal_Update();

            /* 即时熔断检查：若 10ms 控制算法触发了 Safe-State，瞬间跳出循环 */
            if (Sys_Health_IsSafeStateActive())
            {
                break;
            }
        }

        /* --------------------------------------------------------------------
         * 100ms 周期任务域：慢速监控、状态检查与 100ms 周期受控喂狗
         * -------------------------------------------------------------------- */
        if ((current_ticks - last_100ms_ticks) >= APP_TASK_100MS_PERIOD)
        {
            last_100ms_ticks += APP_TASK_100MS_PERIOD;

            Task_Control_Update_100ms();

            /* 受控喂狗：定频在 100ms 周期末尾刷新硬件看门狗，防止 CPU 空转高频喂狗 */
            const Sys_Status_t wd_status = Sys_Health_ServiceWatchdog();
            if (wd_status != SYS_OK)
            {
                /* 喂狗被拒绝 (已处于 Safe-State)，立即打破主循环 */
                break;
            }
        }

        /* --------------------------------------------------------------------
         * 1000ms 周期任务域：系统心跳、OTA 检查与长周期状态机
         * -------------------------------------------------------------------- */
        if ((current_ticks - last_1000ms_ticks) >= APP_TASK_1000MS_PERIOD)
        {
            last_1000ms_ticks += APP_TASK_1000MS_PERIOD;

            Task_Control_Update_1000ms();
            Task_OTA_Update();
        }
    }

    /* ------------------------------------------------------------------------
     * 4. 运行期异常汇聚点 (Safe-State 挂起死循环)
     * ------------------------------------------------------------------------ */
    Sys_Health_ExecSafeStateLoop();

    /* MISRA-C:2012 Rule 2.1 Justification:
     * Unreachable code retained strictly to satisfy ISO C main() int return type requirement. */
    return 0;
}

/* ============================================================================
 * 私有函数实现
 * ============================================================================ */

/**
 * @brief  系统 Fail-Safe 初始化序列
 * @details 顺序初始化 BSP、健康监控器、中间件与 APP，并完整收集失败掩码。
 *          包含安全门禁机制：硬件 BSP 严重损坏时自动跳过后续依赖驱动。
 *
 * @param[out] out_fault_mask 初始化故障位掩码输出指针 (不能为 NULL)
 * @return Sys_Status_t
 * @retval SYS_OK           全部模块初始化成功
 * @retval SYS_ERR_PARAM    out_fault_mask 为空指针
 * @retval SYS_ERR_HW_FAIL  存在至少一个模块初始化失败
 *
 * @note LLR Traceability: [REQ-SW-MAIN-002]
 * @note Safety Criticality: DAL-A | Safe-Gated Init Pattern
 */
static Sys_Status_t System_FailSafe_InitSequence(Sys_InitFaultMask_t *const out_fault_mask)
{
    Sys_Status_t status = SYS_OK;
    Sys_InitFaultMask_t init_faults = SYS_INIT_FAULT_NONE;

    if (out_fault_mask == NULL)
    {
        status = SYS_ERR_PARAM;
    }
    else
    {
        /* 1. 板级硬件 BSP 初始化 */
        if (BSP_Init() != SYS_OK)
        {
            init_faults |= SYS_INIT_FAULT_BSP_BIT;
            status = SYS_ERR_HW_FAIL;
        }

        /* 2. 健康监控器与看门狗初始化 (由内部直接累加写回 init_faults) */
        if (Sys_Health_Init(&init_faults) != SYS_OK)
        {
            status = SYS_ERR_HW_FAIL;
        }

        /* 安全门禁检查：仅在 BSP 硬件基础正常时才初始化依赖外设的中间件与 APP */
        if ((init_faults & SYS_INIT_FAULT_BSP_BIT) == 0U)
        {
            /* 3. 中间件 RTE 初始化 */
            if (RTE_Init() != SYS_OK)
            {
                init_faults |= SYS_INIT_FAULT_RTE_BIT;
                status = SYS_ERR_HW_FAIL;
            }

            /* 4. 中间件 PDI 协议管理器初始化 */
            if (PDI_Mgr_Init() != SYS_OK)
            {
                init_faults |= SYS_INIT_FAULT_PDI_BIT;
                status = SYS_ERR_HW_FAIL;
            }

            /* 5. APP 核心控制任务初始化 */
            if (Task_Control_Init() != SYS_OK)
            {
                init_faults |= SYS_INIT_FAULT_APP_BIT;
                status = SYS_ERR_HW_FAIL;
            }

            /* 6. APP OTA 升级任务初始化 */
            if (Task_OTA_Init() != SYS_OK)
            {
                init_faults |= SYS_INIT_FAULT_MAINT_BIT;
                status = SYS_ERR_HW_FAIL;
            }
        }

        /* 输出累加收集到的完整故障位掩码 */
        *out_fault_mask = init_faults;
    }

    return status;
}
