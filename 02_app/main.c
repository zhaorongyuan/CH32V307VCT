/**
 * @file    main.c
 * @brief   系统 Master 主入口与时间触发调度器 (Time-Triggered Super Loop)
 * @details 负责硬件初始化校验、安全健康监控以及确定性周期任务调度，符合 RTCA DO-178C DAL-A/B 标准。
 * @author  zry
 * @date    2026-07-31
 * @version V1.0.0
 *
 * @note    系统 HLR 追溯: [REQ-HLR-SYS-001], [REQ-HLR-SYS-002]
 * @copyright (c) 2026 zry. All rights reserved.
 */

/* ============================================================================
 * Include Files (严格按照“5+2”分层访问控制)
 * ============================================================================ */
#include "app_cfg.h"         /* 08_Config: 任务周期定义与全局静态参数 */
#include "bsp_board.h"       /* 04_BSP: 板级硬件总初始化入口 */
#include "pdi_mgr.h"         /* 05_Middleware/PDI: 协议数据解析管理器 */
#include "rte_interface.h"   /* 05_Middleware/RTE: ASW/BSW 信号数据字典桥梁 */
#include "task_control.h"    /* 06_APP: 核心控制业务任务 */
#include "task_ota.h"        /* 06_APP: OTA 升级业务状态机 */
#include "sys_health.h"      /* 07_System: 健康监控、看门狗矩阵与 Fault 捕获 */
#include "debug.h"       /* 07_System: 条件编译诊断 Log 与 SYS_ASSERT */
// #include "sys_atomic.h"      /* 07_System: RISC-V 确定性临界区保护 */

/* ============================================================================
 * 私有函数声明
 * ============================================================================ */
static Sys_Status_t System_FailSafe_InitSequence(void);

/* ============================================================================
 * 主函数入口
 * ============================================================================ */
int main(void)
{
    Sys_Status_t init_status = SYS_ERR_PARAM;

    /* ------------------------------------------------------------------------
     * 1. 执行严格的 Fail-Safe 顺序初始化 (包含 Core, Board, RTE, APP)
     * ------------------------------------------------------------------------ */
    init_status = System_FailSafe_InitSequence();
    if (init_status != SYS_OK)
    {
        /* Fatal 初始化失败：切入安全状态挂起，等待看门狗复位或安全停机 */
        SYS_LOG_ERROR("[MAIN] System Init Failed! Code: %d", (int32_t)init_status);
        Sys_Health_EnterSafeState(SYS_FAULT_INIT_FAILURE);
        
        for (;;)
        {
            Sys_Health_ExecSafeStateLoop();
        }
    }

    SYS_LOG_INFO("[MAIN] Initialization Success. Starting Super Loop...");

    /* ------------------------------------------------------------------------
     * 2. 调度器时间戳初始化 (使用 BSP 封装接口，防止裸访问全局变量)
     * ------------------------------------------------------------------------ */
    uint32_t last_10ms_ticks   = BSP_GetTick();
    uint32_t last_100ms_ticks  = BSP_GetTick();
    uint32_t last_1000ms_ticks = BSP_GetTick();

    /* ------------------------------------------------------------------------
     * 3. 确定性主循环调度 (Super Loop)
     * ------------------------------------------------------------------------ */
    for (;;)
    {
        uint32_t current_ticks = BSP_GetTick();

        /* --------------------------------------------------------------------
         * 10ms 周期任务域：控制算法 + PDI 协议解析 + RTE 信号更新
         * -------------------------------------------------------------------- */
        if ((current_ticks - last_10ms_ticks) >= APP_TASK_10MS_PERIOD)
        {
            last_10ms_ticks = current_ticks; /* 赋值更新，防止追赶死循环 */

            /* A. 执行 10ms 核心控制算法 */
            Task_Control_Update_10ms();

            /* B. 处理 PDI 通信串口数据解包 */
            PDI_Mgr_Process();

            /* C. 更新 RTE 信号字典与故障位图 (如 FltFlag_Update) */
            RTE_Signal_Update();

            /* D. 向健康监控报到 (打卡) */
            Sys_Health_TaskReport(SYS_TASK_ID_10MS);
        }

        /* --------------------------------------------------------------------
         * 100ms 周期任务域：慢速监控与状态检查
         * -------------------------------------------------------------------- */
        if ((current_ticks - last_100ms_ticks) >= APP_TASK_100MS_PERIOD)
        {
            last_100ms_ticks = current_ticks;

            Task_Control_Update_100ms();
            Sys_Health_TaskReport(SYS_TASK_ID_100MS);
        }

        /* --------------------------------------------------------------------
         * 1000ms 周期任务域：系统心跳、OTA 检查与长周期状态机
         * -------------------------------------------------------------------- */
        if ((current_ticks - last_1000ms_ticks) >= APP_TASK_1000MS_PERIOD)
        {
            last_1000ms_ticks = current_ticks;

            Task_Control_Update_1000ms();
            Task_OTA_Update();

            Sys_Health_TaskReport(SYS_TASK_ID_1000MS);
        }

        /* --------------------------------------------------------------------
         * 安全喂狗：只有当所有 10ms/100ms/1000ms 任务全都正常按时打卡，才喂看门狗
         * -------------------------------------------------------------------- */
        if (Sys_Health_VerifyLivenessMatrix() == SYS_OK)
        {
            Sys_Health_ServiceWatchdog();
        }
    }
}

/**
 * @brief  系统顺序初始化函数 (带防御性返回值校验)
 * @return Sys_Status_t SYS_OK 代表全部初始化成功
 */
static Sys_Status_t System_FailSafe_InitSequence(void)
{
    /* 1. 核心与健康监控初始化 (Trap / 看门狗预置) */
    if (Sys_Health_Init() != SYS_OK)
    {
        return SYS_ERR_HEALTH_INIT;
    }

    /* 2. 板级硬件总初始化 (时钟、GPIO、UART DMA、Flash) */
    if (BSP_Board_Init() != BOARD_OK)
    {
        return SYS_ERR_HW_FAIL;
    }

    /* 3. RTE 信号数据字典初始化 */
    if (RTE_Signal_Init() != SYS_OK)
    {
        return SYS_ERR_RTE_INIT;
    }

    /* 4. PDI 协议管理器初始化 */
    if (PDI_Mgr_Init() != SYS_OK)
    {
        return SYS_ERR_PDI_INIT;
    }

    /* 5. 业务任务初始化 */
    if (Task_Control_Init() != SYS_OK)
    {
        return SYS_ERR_APP_INIT;
    }

    if (Task_OTA_Init() != SYS_OK)
    {
        return SYS_ERR_OTA_INIT;
    }

    /* 6. 开启全局中断 */
    SYS_ENTER_CRITICAL();
    __enable_irq();
    SYS_EXIT_CRITICAL();

    return SYS_OK;
}
// /**
//  * @file    main.c
//  * @brief   程序入口 — Super Loop 调度器
//  * @author  zry
//  * @date    2026-07-17
//  * @version V0.0.1
//  *
//  * @note    主函数入口，负责系统初始化和周期调度
//  * @copyright (c) 2026 zry. All rights reserved.
//  */


// /* ============================================================================
//  * Include Files (Strict Layer Access Control)
//  * ============================================================================ */
// #include "app_cfg.h"         /* 08_Config: Task periods and static system definitions */
// #include "pdi_mgr.h"         /* 05_Middleware/PDI: Protocol Data Interface */
// #include "rte_interface.h"   /* 05_Middleware/RTE: ASW/BSW Signal Exchange Bridge */
// #include "task_control.h"    /* 06_APP: Core control business task */
// #include "task_ota.h"        /* 06_APP: OTA update task with safety interlocks */
// #include "board.h"
// #include "ch32v30x_conf.h"
// #include "app_cfg.h"
// #include "task_control.h"
// #include "task_ota.h"
// #include <stdio.h>

// int main(void)
// {
//     /* 1. 核心初始化 */
//     System_Core_Init();

//     /* 2. 板级外设初始化 (通过 BSW 层) */
//     Board_Init();

//     /* 3. HAL 外设初始化 (通过 Interface 层) */
//     // API_Led_Init();
//     // API_Key_Init();
//     // API_Buzzer_Init();
//     // API_Wifi_Init();

//     /* 4. 业务任务初始化 */
//     // Task_Control_Init();
//     // Task_OTA_Init();

//     // printf("SystemClk:%u Hz\r\n", (unsigned int)SystemCoreClock);
//     // printf("ChipID:%08x\r\n", (unsigned int)DBGMCU_GetCHIPID());
//     // printf("FW: v%d.%d.%d\r\n",
//     //        APP_FW_VERSION_MAJOR, APP_FW_VERSION_MINOR, APP_FW_VERSION_PATCH);

//     /* 开启全局中断 */
//     __enable_irq();

//     /* 调度器时间戳初始化 */
//     uint32_t last_10ms_ticks   = Gc_SysTick_ms;
//     uint32_t last_100ms_ticks  = Gc_SysTick_ms;
//     uint32_t last_1000ms_ticks = Gc_SysTick_ms;

//     /* 5. 主循环调度 (Super Loop) */
//     for (;;)
//     {
//         uint32_t current_ticks = Gc_SysTick_ms;

//         /* Task Control: 10ms 周期执行 */
//         if ((current_ticks - last_10ms_ticks) >= APP_TASK_10MS_PERIOD) {
//             last_10ms_ticks += APP_TASK_10MS_PERIOD;
//             Task_Control_Update_10ms();
//         }

//         /* Task Control: 100ms 周期执行 */
//         if ((current_ticks - last_100ms_ticks) >= APP_TASK_100MS_PERIOD) {
//             last_100ms_ticks += APP_TASK_100MS_PERIOD;
//             Task_Control_Update_100ms();
//         }

//         /* Task Control + OTA: 1000ms 周期执行 */
//         if ((current_ticks - last_1000ms_ticks) >= APP_TASK_1000MS_PERIOD) {
//             last_1000ms_ticks += APP_TASK_1000MS_PERIOD;
//             Task_Control_Update_1000ms();
//             // Task_OTA_Update();
//         }

//         /* 可选：等待下一个中断唤醒，降低无任务时的 CPU 功耗 */
//         // __WFI(); 
//     }
// }
