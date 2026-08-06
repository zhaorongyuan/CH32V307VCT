/**
 * @file board_uart_cfg.c
 * @brief UART Hardware Parameter Table Definition
 * @details Complete mapping of physical pins, DMA channels, and interrupts for all 4 UART channels.
 * @author zry
 * @date 2026-08-06
 * @version V1.0.0
 *
 * @note System HLR Traceability: [REQ-HLR-BSP-003]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "board_uart_cfg.h"
#include "board_cfg.h"

/**
 * @brief Master Data-Driven Hardware Parameter Table (Stored in Flash / Read-Only Section)
 * @note MISRA-C:2012 Rule 8.4, 8.9 & Rule 9.2 Compliant
 */
const UART_Config_t g_UartConfigTable[UART_ID_MAX] = {
    /* ------------------------------------------------------------------------ */
    /* Channel 0: USART1 (Master Debug Console)                                 */
    /* ------------------------------------------------------------------------ */
    [UART_ID_DEBUG1] = {
        .pInstance          = USART1,
        .eBusType           = BOARD_BUS_APB2,
        .ulUartClockMask    = RCC_APB2Periph_USART1,
        
        .ulBaudRate         = 115200UL,
        .usWordLength       = USART_WordLength_8b,
        .usStopBits         = USART_StopBits_1,
        .usParity           = USART_Parity_No,
        .usHwFlowCtl        = USART_HardwareFlowControl_None,

        .pTxPort            = UART1_TX_PORT,
        .usTxPin            = UART1_TX_PIN,
        .pRxPort            = UART1_RX_PORT,
        .usRxPin            = UART1_RX_PIN,

        .pDmaTxChannel      = DMA1_Channel4,
        .pDmaRxChannel      = DMA1_Channel5,
        .ulDmaClockMask     = RCC_AHBPeriph_DMA1,

        .eIrqNumber         = USART1_IRQn,
        .ucIrqPreemptPri    = 1U,
        .ucIrqSubPri        = 0U
    },

    /* ------------------------------------------------------------------------ */
    /* Channel 1: USART2 (Jumper Diagnostic Console)                            */
    /* ------------------------------------------------------------------------ */
    [UART_ID_DEBUG2] = {
        .pInstance          = USART2,
        .eBusType           = BOARD_BUS_APB1,
        .ulUartClockMask    = RCC_APB1Periph_USART2,
        
        .ulBaudRate         = 115200UL,
        .usWordLength       = USART_WordLength_8b,
        .usStopBits         = USART_StopBits_1,
        .usParity           = USART_Parity_No,
        .usHwFlowCtl        = USART_HardwareFlowControl_None,

        .pTxPort            = UART2_TX_PORT,
        .usTxPin            = UART2_TX_PIN,
        .pRxPort            = UART2_RX_PORT,
        .usRxPin            = UART2_RX_PIN,

        .pDmaTxChannel      = DMA1_Channel7,
        .pDmaRxChannel      = DMA1_Channel6,
        .ulDmaClockMask     = RCC_AHBPeriph_DMA1,

        .eIrqNumber         = USART2_IRQn,
        .ucIrqPreemptPri    = 2U,
        .ucIrqSubPri        = 0U
    },

    /* ------------------------------------------------------------------------ */
    /* Channel 2: UART6 (ESP8266 WiFi Module)                                   */
    /* ------------------------------------------------------------------------ */
    [UART_ID_WIFI] = {
        .pInstance          = UART6,
        .eBusType           = BOARD_BUS_APB1,
        .ulUartClockMask    = RCC_APB1Periph_UART6,
        
        .ulBaudRate         = 115200UL,
        .usWordLength       = USART_WordLength_8b,
        .usStopBits         = USART_StopBits_1,
        .usParity           = USART_Parity_No,
        .usHwFlowCtl        = USART_HardwareFlowControl_None,

        .pTxPort            = WIFI_UART_TX_PORT,
        .usTxPin            = WIFI_UART_TX_PIN,
        .pRxPort            = WIFI_UART_RX_PORT,
        .usRxPin            = WIFI_UART_RX_PIN,

        .pDmaTxChannel      = DMA2_Channel3,
        .pDmaRxChannel      = DMA2_Channel2,
        .ulDmaClockMask     = RCC_AHBPeriph_DMA2,

        .eIrqNumber         = UART6_IRQn,
        .ucIrqPreemptPri    = 2U,
        .ucIrqSubPri        = 1U
    },

    /* ------------------------------------------------------------------------ */
    /* Channel 3: UART7 (CH9141 BLE Wireless Module)                            */
    /* ------------------------------------------------------------------------ */
    [UART_ID_BLE] = {
        .pInstance          = UART7,
        .eBusType           = BOARD_BUS_APB1,
        .ulUartClockMask    = RCC_APB1Periph_UART7,
        
        .ulBaudRate         = 115200UL,
        .usWordLength       = USART_WordLength_8b,
        .usStopBits         = USART_StopBits_1,
        .usParity           = USART_Parity_No,
        .usHwFlowCtl        = USART_HardwareFlowControl_None,

        .pTxPort            = BLE_UART_TX_PORT,
        .usTxPin            = BLE_UART_TX_PIN,
        .pRxPort            = BLE_UART_RX_PORT,
        .usRxPin            = BLE_UART_RX_PIN,

        .pDmaTxChannel      = DMA2_Channel5,
        .pDmaRxChannel      = DMA2_Channel4,
        .ulDmaClockMask     = RCC_AHBPeriph_DMA2,

        .eIrqNumber         = UART7_IRQn,
        .ucIrqPreemptPri    = 2U,
        .ucIrqSubPri        = 2U
    }
};