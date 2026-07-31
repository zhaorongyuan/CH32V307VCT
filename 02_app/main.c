/**
 * @file    main.c
 * @brief   程序入口 — Super Loop 调度器
 * @author  zry
 * @date    2026-07-17
 * @version V0.0.1
 *
 * @note    主函数入口，负责系统初始化和周期调度
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "ch32v30x_conf.h"
#include "app_cfg.h"
#include "board.h"
#include "Interface.h"
#include "task_control.h"
#include "task_ota.h"
#include "debug.h"

/* SysTick 1ms 重载值 */
static uint32_t s_systick_reload_val = 0;

/**
 * @brief  配置 SysTick 为 1ms 周期中断
 */
void SysTick_Config_1ms(uint32_t sys_clk)
{
    s_systick_reload_val = sys_clk / 1000U;

    SysTick->CTLR &= ~(1U << 0);    /* 关闭 SysTick */
    SysTick->CNT = 0;
    SysTick->SR = 0;
    SysTick->CMP = s_systick_reload_val - 1U;

    /* 配置 CTLR 寄存器 (依据 CH32V30x 手册) */
    SysTick->CTLR |= (1U << 4) | (1U << 3) | (1U << 2);     
    SysTick->CTLR |= (1U << 1);     /* 开启中断 */
    SysTick->CTLR |= (1U << 0);     /* 开启计数器 */

    NVIC_EnableIRQ(SysTick_IRQn);
}

/**
 * @brief  系统核心底层组件初始化
 */
static void System_Core_Init(void)
{
    /* 配置中断优先级分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* 更新 SystemCoreClock 变量 */
    SystemCoreClockUpdate();

    /* 注意：如果 Delay_Init 使用了 SysTick，请确认其不会关闭中断。
       建议优先初始化 Delay，再配置 SysTick 心跳中断 */
    Delay_Init();
    USART_Printf_Init(115200);

    /* 配置 1ms 的 SysTick 心跳中断 */
    SysTick_Config_1ms(SystemCoreClock);
}

int main(void)
{
    /* 1. 核心初始化 */
    System_Core_Init();

    /* 2. 板级外设初始化 (通过 BSW 层) */
    Board_Init();

    /* 3. HAL 外设初始化 (通过 Interface 层) */
    API_Led_Init();
    API_Key_Init();
    API_Buzzer_Init();
    API_Wifi_Init();

    /* 4. 业务任务初始化 */
    Task_Control_Init();
    Task_OTA_Init();

    printf("SystemClk:%u Hz\r\n", (unsigned int)SystemCoreClock);
    printf("ChipID:%08x\r\n", (unsigned int)DBGMCU_GetCHIPID());
    printf("FW: v%d.%d.%d\r\n",
           APP_FW_VERSION_MAJOR, APP_FW_VERSION_MINOR, APP_FW_VERSION_PATCH);

    /* 开启全局中断 */
    __enable_irq();

    /* 调度器时间戳初始化 */
    uint32_t last_10ms_ticks   = Gc_SysTick_ms;
    uint32_t last_100ms_ticks  = Gc_SysTick_ms;
    uint32_t last_1000ms_ticks = Gc_SysTick_ms;

    /* 5. 主循环调度 (Super Loop) */
    for (;;)
    {
        uint32_t current_ticks = Gc_SysTick_ms;

        /* Task Control: 10ms 周期执行 */
        if ((current_ticks - last_10ms_ticks) >= APP_TASK_10MS_PERIOD) {
            last_10ms_ticks += APP_TASK_10MS_PERIOD;
            Task_Control_Update_10ms();
        }

        /* Task Control: 100ms 周期执行 */
        if ((current_ticks - last_100ms_ticks) >= APP_TASK_100MS_PERIOD) {
            last_100ms_ticks += APP_TASK_100MS_PERIOD;
            Task_Control_Update_100ms();
        }

        /* Task Control + OTA: 1000ms 周期执行 */
        if ((current_ticks - last_1000ms_ticks) >= APP_TASK_1000MS_PERIOD) {
            last_1000ms_ticks += APP_TASK_1000MS_PERIOD;
            Task_Control_Update_1000ms();
            // Task_OTA_Update();
        }

        /* 可选：等待下一个中断唤醒，降低无任务时的 CPU 功耗 */
        // __WFI(); 
    }
}
