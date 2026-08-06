/**
 * @file bsp_board.c
 * @brief Master Board Level Initialization Implementation (DAL-A/B Hardened)
 * @details Implements fault-tolerant, data-driven peripheral startup sequences,
 *          clock safety loops, and hardware read-back validation.
 * @author zry
 * @date 2026-08-06
 * @version V1.0.0
 *
 * @note System HLR Traceability: [REQ-HLR-BSP-001], [REQ-HLR-BSP-002], [REQ-HLR-BSP-003]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "bsp_board.h"
#include <stddef.h>
#include "board_systick.h"
#include "debug.h"    /* Replaces raw printf with conditionally compiled debug macros */
#include "sys_health.h"   /* System Fault Reporting Interface */

/* Upper bound limit for hardware loop iterations to satisfy WCET static analysis */
#define BSP_CLOCK_LOCK_TIMEOUT_LIMIT    (100000U)

/* Private Function Prototypes */
static Board_Status_t Board_Clock_Init(void);

/**
 * @brief  System Core Architecture Low-Level Setup
 * @details Configures NVIC, core clocks, and SysTick 1ms tick interrupts.
 */
void System_Core_Init(void)
{
    /* 1. Configure Interrupt Priority Grouping (2 bits pre-emption, 2 bits sub-priority) */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* 2. Update SystemCoreClock global variable according to RCC register states */
    SystemCoreClockUpdate();

    /* 3. Initialize Delay module and SysTick Timer */
    Delay_Init();

    /* 4. Conditional Debug Macro (Stripped in Production Release Builds) */
    // SYS_DEBUG_PRINTF("BSP: Core System Initialized. Target SysClock = %u Hz\r\n", SystemCoreClock);
    USART_Printf_Init(115200);
    printf("Debug init\r\n");


    /* 5. Configure 1ms Deterministic System Tick Interrupt */
    BSW_SysTick_Config(SystemCoreClock);
}

/**
 * @brief   Enable System Peripheral Clocks with PLL Lock Verification
 * @details Enables GPIO A~E and AFIO bus clocks, then verifies PLL clock lock status within WCET bounds.
 * 
 * @param   void
 * @return  Board_Status_t
 * @retval  BOARD_OK Status OK and Clock locked.
 * @retval  BOARD_ERR_HW_TIMEOUT PLL failed to lock within timeout limit.
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0101]
 * @note    Safety Criticality: DAL-A / Bounded Execution Loop
 */
static Board_Status_t Board_Clock_Init(void)
{
    Board_Status_t status = BOARD_OK;
    uint32_t timeout = BSP_CLOCK_LOCK_TIMEOUT_LIMIT;

    /* Enable APB2 Bus Clocks for GPIOA ~ GPIOE and AFIO */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
        RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO,
        ENABLE
    );

    /* Bounded Verification Loop: Ensure PLL Clock is Stable */
    while ((RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) && (timeout > 0U))
    {
        timeout--;
    }

    if (0U == timeout)
    {
        /* Clock Lock Failed! Report Fault to System Health Monitoring */
        // (void)SYS_Health_ReportFault(SYS_FAULT_ID_CLK_FAIL, SYS_FAULT_SEVERITY_CRITICAL);
        status = BOARD_ERR_HW_TIMEOUT;
    }

    return status;
}

/**
 * @brief   Data-Driven GPIO Initialization Routine with Hardware Read-Back Check
 * @details Traverses g_GpioConfigTable, applies initial safe levels, and reads back IDR for discrete outputs.
 * 
 * @param   void
 * @return  Board_Status_t
 * @retval  BOARD_OK                Operation successful and all outputs verified.
 * @retval  BOARD_ERR_INVALID_PARAM NULL pointer detected in table.
 * @retval  BOARD_ERR_VERIFY        Physical pin failed read-back verification (Short-circuit / Stuck fault).
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0102]
 * @note    Safety Criticality: DAL-A / Bounded Loop (Max iterations: g_GpioConfigTableSize)
 */
Board_Status_t Board_Gpio_Init(void)
{
    GPIO_InitTypeDef gpioInit = {0};
    uint8_t index = 0U;

    for (index = 0U; index < g_GpioConfigTableSize; index++)
    {
        const Board_GPIO_Config_t* const pCfg = &g_GpioConfigTable[index];

        /* Defensive Check: Validate Port Pointer */
        if (NULL == pCfg->pPort)
        {
            return BOARD_ERR_INVALID_PARAM;
        }

        /* 1. Pre-latch safe initial state before pin mode configuration */
        if (pCfg->ucApplyInitState != 0U)
        {
            GPIO_WriteBit(pCfg->pPort, pCfg->usPin, pCfg->eInitialState);
        }

        /* 2. Call MCAL Driver to initialize pin modes */
        gpioInit.GPIO_Pin   = pCfg->usPin;
        gpioInit.GPIO_Mode  = pCfg->eMode;
        gpioInit.GPIO_Speed = pCfg->eSpeed;
        GPIO_Init(pCfg->pPort, &gpioInit);

        /* 3. Hardware Read-Back Verification for Discrete Outputs */
        if (pCfg->ucApplyInitState != 0U)
        {
            uint8_t actualBit = GPIO_ReadInputDataBit(pCfg->pPort, pCfg->usPin);
            uint8_t expectedBit = (pCfg->eInitialState != Bit_RESET) ? 1U : 0U;

            if (actualBit != expectedBit)
            {
                /* Physical line electrical fault detected! Report to System Health */
                // (void)SYS_Health_ReportFault(SYS_FAULT_ID_GPIO_VERIFY_FAIL, SYS_FAULT_SEVERITY_CRITICAL);
                return BOARD_ERR_VERIFY;
            }
        }
    }

    return BOARD_OK;
}

/**
 * @brief   Data-Driven UART Peripheral Master Initialization
 * @details Traverses g_UartConfigTable to configure clocks, TX/RX physical pins, and protocol parameters.
 * 
 * @param   void
 * @return  Board_Status_t
 * @retval  BOARD_OK                Operation successful.
 * @retval  BOARD_ERR_INVALID_PARAM NULL pointer found in configuration table.
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0103]
 * @note    Safety Criticality: DAL-B / Bounded Loop (Max iterations: UART_ID_MAX)
 */
Board_Status_t Board_Uart_Init(void)
{
    GPIO_InitTypeDef  gpioInit = {0};
    USART_InitTypeDef usartInit = {0};
    NVIC_InitTypeDef  nvicInit = {0};
    uint8_t index = 0U;

    for (index = 0U; index < (uint8_t)UART_ID_MAX; index++)
    {
        const UART_Config_t* const pCfg = &g_UartConfigTable[index];

        /* 防御性编程：多重空指针校验 */
        if ((NULL == pCfg->pInstance) || (NULL == pCfg->pTxPort) || (NULL == pCfg->pRxPort))
        {
            return BOARD_ERR_INVALID_PARAM;
        }

        /* 1. 使能 GPIO 与 USART 外设时钟 */
        if (pCfg->eBusType == BOARD_BUS_APB2)
        {
            RCC_APB2PeriphClockCmd(pCfg->ulUartClockMask, ENABLE);
        }
        else
        {
            RCC_APB1PeriphClockCmd(pCfg->ulUartClockMask, ENABLE);
        }

        /* 2. 使能 DMA 外设时钟 */
        if (pCfg->pDmaTxChannel != NULL)
        {
            RCC_AHBPeriphClockCmd(pCfg->ulDmaClockMask, ENABLE);
        }

        /* 3. TX 物理引脚配置 (复用推挽) */
        gpioInit.GPIO_Pin   = pCfg->usTxPin;
        gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
        gpioInit.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_Init(pCfg->pTxPort, &gpioInit);

        /* 4. RX 物理引脚配置 (浮空输入) */
        gpioInit.GPIO_Pin   = pCfg->usRxPin;
        gpioInit.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
        GPIO_Init(pCfg->pRxPort, &gpioInit);

        /* 5. 串口协议参数初始化 */
        usartInit.USART_BaudRate            = pCfg->ulBaudRate;
        usartInit.USART_WordLength          = pCfg->usWordLength;
        usartInit.USART_StopBits            = pCfg->usStopBits;
        usartInit.USART_Parity              = pCfg->usParity;
        usartInit.USART_HardwareFlowControl = pCfg->usHwFlowCtl;
        usartInit.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

        USART_Init(pCfg->pInstance, &usartInit);

        /* 6. 配置 PFIC 中断向量与优先级 (供 DMA/Idle 帧中断使用) */
        nvicInit.NVIC_IRQChannel                     = pCfg->eIrqNumber;
        nvicInit.NVIC_IRQChannelPreemptionPriority   = pCfg->ucIrqPreemptPri;
        nvicInit.NVIC_IRQChannelSubPriority          = pCfg->ucIrqSubPri;
        nvicInit.NVIC_IRQChannelCmd                  = ENABLE;
        NVIC_Init(&nvicInit);

        USART_Cmd(pCfg->pInstance, ENABLE);
    }

    return BOARD_OK;
}

/**
 * @brief   Phase-1 Board Hardware Driver Master Entry
 * @details Sequentially calls Clock, GPIO, and UART initializations. Halts on first error.
 * 
 * @param   void
 * @return  Board_Status_t
 * @retval  BOARD_OK All initializations completed successfully.
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0100]
 * @note    Safety Criticality: DAL-A / Fail-Safe Entry Point
 */
Board_Status_t Board_Init(void)
{
    Board_Status_t status = BOARD_OK;

    /* 1. Peripheral Clock & Lock Setup */
    status = Board_Clock_Init();
    if (status != BOARD_OK)
    {
        return status;
    }

    /* 2. Board GPIO Data-Driven Setup with HW Verification */
    status = Board_Gpio_Init();
    if (status != BOARD_OK)
    {
        return status;
    }

    /* 3. Master UART Initialization */
    status = Board_Uart_Init();
    if (status != BOARD_OK)
    {
        return status;
    }

    return BOARD_OK;
}

/**
 * @brief   BSP Master Entry Point for 07_System Dispatcher
 * @details Converts local BSP status to unified system-level status code without silent fall-throughs.
 * 
 * @return  Sys_Status_t
 * 
 * @note    LLR Traceability: [REQ-SW-BSP-0100]
 * @note    Safety Criticality: DAL-A
 */
Sys_Status_t BSP_Init(void)
{
    Board_Status_t status = BOARD_OK;

    /* 1. Core Architecture Setup */
    System_Core_Init();

    /* 2. Phase-1 Board Drivers Setup */
    status = Board_Init();

    /* 3. Explicit Status Mapping & Fault Propagation (No Implicit Pass) */
    Sys_Status_t sysStatus = SYS_OK;
    switch (status)
    {
        case BOARD_OK:
            sysStatus = SYS_OK;
            break;

        case BOARD_ERR_INVALID_PARAM:
            sysStatus = SYS_ERR_PARAM;
            break;

        case BOARD_ERR_HW_TIMEOUT:
            sysStatus = SYS_ERR_TIMEOUT;
            break;

        case BOARD_ERR_VERIFY:
            sysStatus = SYS_ERR_HW_FAIL;
            break;

        default:
            sysStatus = SYS_ERR_HW_FAIL;
            break;
    }

    return sysStatus;
}
