/**
 * @file    board.c
 * @brief   板级初始化实现（数据驱动与分阶段 BSP 初始化框架）
 * @details 严格整合 board_pin.h 与 board_uart_cfg.h，实现全串口统一驱动配置。
 *          符合 DO-178C DAL-C 级代码标准。
 * @author  zry
 * @date    2026-07-29
 * @version V2.0.0
 *
 * @docref  BSP-DESIGN-SPEC-V2
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "board.h"
#include "ch32v30x_conf.h"
#include <stddef.h>

/* ============================================================================ */
/* 静态串口配置数据表 (数据驱动落地)                                            */
/* ============================================================================ */
static const UART_Config_t g_UartConfigTable[UART_ID_MAX] = {
    [UART_ID_DEBUG1] = {
        .pInstance    = DBG_UART1_PERIPH,
        .ulClockMask  = DBG_UART1_CLK,
        .ulBaudRate   = DBG_UART1_BAUD,
        .usWordLength = USART_WordLength_8b,
        .usStopBits   = USART_StopBits_1,
        .usParity     = USART_Parity_No,
        .usHwFlowCtl  = USART_HardwareFlowControl_None,
        .ucIsAPB2     = 1U
    },
    [UART_ID_DEBUG2] = {
        .pInstance    = DBG_UART2_PERIPH,
        .ulClockMask  = DBG_UART2_CLK,
        .ulBaudRate   = DBG_UART2_BAUD,
        .usWordLength = USART_WordLength_8b,
        .usStopBits   = USART_StopBits_1,
        .usParity     = USART_Parity_No,
        .usHwFlowCtl  = USART_HardwareFlowControl_None,
        .ucIsAPB2     = 0U
    },
    [UART_ID_WIFI] = {
        .pInstance    = WIFI_UART_PERIPH,
        .ulClockMask  = WIFI_UART_CLK,
        .ulBaudRate   = WIFI_UART_BAUD,
        .usWordLength = USART_WordLength_8b,
        .usStopBits   = USART_StopBits_1,
        .usParity     = USART_Parity_No,
        .usHwFlowCtl  = USART_HardwareFlowControl_None,
        .ucIsAPB2     = 0U
    },
    [UART_ID_BLE] = {
        .pInstance    = BLE_UART_PERIPH,
        .ulClockMask  = BLE_UART_CLK,
        .ulBaudRate   = BLE_UART_BAUD,
        .usWordLength = USART_WordLength_8b,
        .usStopBits   = USART_StopBits_1,
        .usParity     = USART_Parity_No,
        .usHwFlowCtl  = USART_HardwareFlowControl_None,
        .ucIsAPB2     = 0U
    }
};

/* 私有函数声明 */
static void Board_Clock_Init(void);

/**
 * @brief  系统基础 GPIO 外设时钟统一使能
 */
static void Board_Clock_Init(void)
{
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
        RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO,
        ENABLE
    );
}

/**
 * @brief  板级 GPIO 外设初始化
 */
void Board_Gpio_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    /* 1. LED 初始化 (默认上电不亮，输出高电平) */
    GPIO_SetBits(LED1_PORT, LED1_PIN);
    GPIO_SetBits(LED2_PORT, LED2_PIN);

    GPIO_InitStructure.GPIO_Pin   = LED1_PIN | LED2_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED1_PORT, &GPIO_InitStructure);

    /* 2. 按键初始化 */
    /* WakeUp (PA0) -> 下拉输入 */
    GPIO_InitStructure.GPIO_Pin  = KEY_WAKEUP_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(KEY_WAKEUP_PORT, &GPIO_InitStructure);

    /* SW1, SW2 (PE4, PE5) -> 上拉输入 */
    GPIO_InitStructure.GPIO_Pin  = KEY_SW1_PIN | KEY_SW2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_SW1_PORT, &GPIO_InitStructure);

    /* 五向开关 -> 上拉输入 */
    GPIO_InitStructure.GPIO_Pin  = JOY_UP_PIN | JOY_DOWN_PIN | JOY_RIGHT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin  = JOY_LEFT_PIN | JOY_SEL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* 3. 音频控制引脚初始化 (PA8) -> 默认播放模式 */
    GPIO_WriteBit(AUDIO_CTL_PORT, AUDIO_CTL_PIN, (BitAction)AUDIO_CTL_MODE_PLAY);
    GPIO_InitStructure.GPIO_Pin   = AUDIO_CTL_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(AUDIO_CTL_PORT, &GPIO_InitStructure);

    /* 4. 蓝牙控制引脚 (PA7, PC13) */
    GPIO_InitStructure.GPIO_Pin   = BLE_AT_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(BLE_AT_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = BLE_SLEEP_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(BLE_SLEEP_PORT, &GPIO_InitStructure);
}

/**
 * @brief  板级串口统一初始化 (数据驱动架构，覆盖全板 4 路串口)
 */
void Board_Uart_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    uint8_t i = 0U;

    /* ---------------- Step 1: 配置所有串口引脚 ---------------- */
    /* USART1 (PA9-TX, PA10-RX) */
    GPIO_InitStructure.GPIO_Pin   = UART1_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(UART1_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin  = UART1_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART1_RX_PORT, &GPIO_InitStructure);

    /* USART2 (PA2-TX, PA3-RX) */
    GPIO_InitStructure.GPIO_Pin   = UART2_TX_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(UART2_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin  = UART2_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART2_RX_PORT, &GPIO_InitStructure);

    /* UART6 (PC0-TX, PC1-RX) */
    GPIO_InitStructure.GPIO_Pin   = WIFI_UART_TX_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(WIFI_UART_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin  = WIFI_UART_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(WIFI_UART_RX_PORT, &GPIO_InitStructure);

    /* UART7 (PC2-TX, PC3-RX) */
    GPIO_InitStructure.GPIO_Pin   = BLE_UART_TX_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(BLE_UART_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin  = BLE_UART_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(BLE_UART_RX_PORT, &GPIO_InitStructure);

    /* ---------------- Step 2: 依据配置表逐个使能串口外设 ---------------- */
    for (i = 0U; i < (uint8_t)UART_ID_MAX; i++)
    {
        const UART_Config_t* pCfg = &g_UartConfigTable[i];

        if (pCfg->pInstance != NULL)
        {
            /* 使能外设总线时钟 */
            if (pCfg->ucIsAPB2 != 0U) {
                RCC_APB2PeriphClockCmd(pCfg->ulClockMask, ENABLE);
            } else {
                RCC_APB1PeriphClockCmd(pCfg->ulClockMask, ENABLE);
            }

            /* 配置 USART 控制器参数 */
            USART_InitStructure.USART_BaudRate            = pCfg->ulBaudRate;
            USART_InitStructure.USART_WordLength          = pCfg->usWordLength;
            USART_InitStructure.USART_StopBits            = pCfg->usStopBits;
            USART_InitStructure.USART_Parity              = pCfg->usParity;
            USART_InitStructure.USART_HardwareFlowControl = pCfg->usHwFlowCtl;
            USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

            USART_Init(pCfg->pInstance, &USART_InitStructure);
            USART_Cmd(pCfg->pInstance, ENABLE);
        }
    }
}

/**
 * @brief  板级硬件总初始化入口 (Phase 1 BSP Driver Initialization)
 */
void Board_Init(void)
{
    /* 1. 时钟初始化 */
    Board_Clock_Init();

    /* 2. GPIO 基础外设初始化 */
    Board_Gpio_Init();

    /* 3. 通信串口统一初始化 (数据驱动配置) */
    Board_Uart_Init();
}