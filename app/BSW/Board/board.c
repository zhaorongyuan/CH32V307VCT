/**
 * @file    board.c
 * @brief   板级初始化实现（数据驱动与分阶段 BSP 初始化框架）
 * @details 绝对无硬编码引脚，全表驱动遍历，具备空指针防御性校验。
 * @author  zry
 * @date    2026-07-30
 * @version V0.0.1
 *
 * @note    System HLR Traceability: [REQ-HLR-BSP-001], [REQ-HLR-BSP-002], [REQ-HLR-BSP-003]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "board.h"
#include <stddef.h>

/* 私有函数声明 */
static Board_Status_t Board_Clock_Init(void);

/**
 * @brief   系统基础 GPIO 外设时钟统一使能
 * @details 集中使能 GPIOA~GPIOE 及 AFIO 总线时钟。
 * 
 * @param   void
 * @return  Board_Status_t 执行状态码
 * @retval  BOARD_OK 操作成功
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0101]
 * @note    Safety Criticality: DAL-B / Deterministic execution
 */
static Board_Status_t Board_Clock_Init(void)
{
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
        RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO,
        ENABLE
    );
    
    return BOARD_OK;
}

/**
 * @brief   板级 GPIO 外设数据驱动统一初始化
 * @details 遍历 g_GpioConfigTable 配置表，依次锁存 Safe-State 并初始化引脚模式。
 * 
 * @param   void
 * @return  Board_Status_t 执行状态码
 * @retval  BOARD_OK                操作成功
 * @retval  BOARD_ERR_INVALID_PARAM 遇到非法配置项或 NULL 指针
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0102]
 * @note    Safety Criticality: DAL-B / Bounded Loop (Max iterations: g_GpioConfigTableSize)
 */
Board_Status_t Board_Gpio_Init(void)
{
    GPIO_InitTypeDef gpioInit = {0};
    uint8_t index = 0U;

    for (index = 0U; index < g_GpioConfigTableSize; index++)
    {
        const Board_GPIO_Config_t* const pCfg = &g_GpioConfigTable[index];

        /* 防御性编程：校验端口指针有效性 */
        if (pCfg->pPort == NULL)
        {
            return BOARD_ERR_INVALID_PARAM;
        }

        /* 若定义了安全初始电平，在模式配置前锁存，避免引脚初始化瞬间产生电气毛刺 */
        if (pCfg->ucApplyInitState != 0U)
        {
            GPIO_WriteBit(pCfg->pPort, pCfg->usPin, pCfg->eInitialState);
        }

        gpioInit.GPIO_Pin   = pCfg->usPin;
        gpioInit.GPIO_Mode  = pCfg->eMode;
        gpioInit.GPIO_Speed = pCfg->eSpeed;
        GPIO_Init(pCfg->pPort, &gpioInit);
    }

    return BOARD_OK;
}

/**
 * @brief   板级串口统一初始化 (数据驱动架构)
 * @details 遍历 g_UartConfigTable 静态配置表，完成全板 4 路串口引脚与外设初始化。
 * 
 * @param   void
 * @return  Board_Status_t 执行状态码
 * @retval  BOARD_OK                操作成功
 * @retval  BOARD_ERR_INVALID_PARAM 配置表指针或基地址为空
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0103]
 * @note    Safety Criticality: DAL-B / Bounded Loop (Max iterations: UART_ID_MAX)
 */
Board_Status_t Board_Uart_Init(void)
{
    GPIO_InitTypeDef  gpioInit = {0};
    USART_InitTypeDef usartInit = {0};
    uint8_t index = 0U;

    for (index = 0U; index < (uint8_t)UART_ID_MAX; index++)
    {
        const UART_Config_t* const pCfg = &g_UartConfigTable[index];

        /* 防御性编程：多重空指针校验 */
        if ((pCfg->pInstance == NULL) || (pCfg->pTxPort == NULL) || (pCfg->pRxPort == NULL))
        {
            return BOARD_ERR_INVALID_PARAM;
        }

        /* 1. 时钟使能 (依据 APB1/APB2 映射) */
        if (pCfg->ucIsAPB2 != 0U) {
            RCC_APB2PeriphClockCmd(pCfg->ulClockMask, ENABLE);
        } else {
            RCC_APB1PeriphClockCmd(pCfg->ulClockMask, ENABLE);
        }

        /* 2. TX 物理引脚配置 (数据驱动) */
        gpioInit.GPIO_Pin   = pCfg->usTxPin;
        gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
        gpioInit.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_Init(pCfg->pTxPort, &gpioInit);

        /* 3. RX 物理引脚配置 (数据驱动) */
        gpioInit.GPIO_Pin   = pCfg->usRxPin;
        gpioInit.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
        GPIO_Init(pCfg->pRxPort, &gpioInit);

        /* 4. 串口协议参数初始化 */
        usartInit.USART_BaudRate            = pCfg->ulBaudRate;
        usartInit.USART_WordLength          = pCfg->usWordLength;
        usartInit.USART_StopBits            = pCfg->usStopBits;
        usartInit.USART_Parity              = pCfg->usParity;
        usartInit.USART_HardwareFlowControl = pCfg->usHwFlowCtl;
        usartInit.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

        USART_Init(pCfg->pInstance, &usartInit);
        USART_Cmd(pCfg->pInstance, ENABLE);
    }

    return BOARD_OK;
}

/**
 * @brief   板级硬件总初始化入口 (Phase 1 BSP Driver Initialization)
 * @details 顺序调度 Clock、GPIO、UART 例程，实行故障拦截。
 * 
 * @param   void
 * @return  Board_Status_t 执行状态码
 * @retval  BOARD_OK               初始化全部成功
 * @retval  BOARD_ERR_INVALID_PARAM 初始化过程出现非法参数
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0101], [REQ-SW-BSP-0102], [REQ-SW-BSP-0103]
 * @note    Safety Criticality: DAL-B / Safe Safe Entry point
 */
Board_Status_t Board_Init(void)
{
    Board_Status_t status = BOARD_OK;

    /* 1. 外设时钟初始化 */
    status = Board_Clock_Init();
    if (status != BOARD_OK) {
        return status;
    }

    /* 2. 板载 GPIO 数据驱动初始化 */
    status = Board_Gpio_Init();
    if (status != BOARD_OK) {
        return status;
    }

    /* 3. 串口数据驱动初始化 */
    status = Board_Uart_Init();
    if (status != BOARD_OK) {
        return status;
    }

    return BOARD_OK;
}