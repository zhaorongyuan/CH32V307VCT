/**
 * @file board_uart_cfg.h
 * @brief Master UART Hardware Configuration Interface (Data-Driven Pattern)
 * @details Fully describes physical pins, peripheral register bases, DMA channels,
 *          and PFIC interrupt vectors for high-integrity UART interfaces.
 * @author zry
 * @date 2026-08-06
 * @version V1.0.0
 *
 * @note System HLR Traceability: [REQ-HLR-BSP-003]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_UART_CFG_H
#define BOARD_UART_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v30x.h"

/**
 * @brief Peripheral Bus Identification Enumeration
 */
typedef enum {
    BOARD_BUS_APB1 = 0U,
    BOARD_BUS_APB2 = 1U
} Board_BusType_t;

/**
 * @brief Logical UART Channel Enumeration ID
 */
typedef enum {
    UART_ID_DEBUG1 = 0U,    /* USART1 - Master Debug Serial (PA9/PA10) */
    UART_ID_DEBUG2,         /* USART2 - Jumper Debug Serial (PA2/PA3) */
    UART_ID_WIFI,           /* UART6  - ESP8266 WiFi Module (PC0/PC1) */
    UART_ID_BLE,            /* UART7  - CH9141 Bluetooth Module (PC2/PC3) */
    UART_ID_MAX
} UART_ID_t;

/**
 * @brief High-Integrity Data-Driven UART Driver Configuration Structure
 */
typedef struct {
    /* 1. Controller Base Address & Clocking */
    USART_TypeDef*         pInstance;          /* Hardware Register Base Address */
    Board_BusType_t        eBusType;           /* Peripheral Bus Assignment (APB1 / APB2) */
    uint32_t               ulUartClockMask;    /* Peripheral Clock Enable Mask */
    
    /* 2. Communication Line Parameters */
    uint32_t               ulBaudRate;         /* Baud Rate (e.g., 115200) */
    uint16_t               usWordLength;       /* Data Bits (8b/9b) */
    uint16_t               usStopBits;         /* Stop Bits (1 / 1.5 / 2) */
    uint16_t               usParity;           /* Parity Check (None / Even / Odd) */
    uint16_t               usHwFlowCtl;        /* Hardware Flow Control (None / RTS_CTS) */

    /* 3. Decoupled Physical Pin Mapping */
    GPIO_TypeDef*          pTxPort;            /* TX GPIO Port Base */
    uint16_t               usTxPin;            /* TX Pin Mask */
    GPIO_TypeDef*          pRxPort;            /* RX GPIO Port Base */
    uint16_t               usRxPin;            /* RX Pin Mask */

    /* 4. DMA Subsystem Configuration (DAL-A/B Requirement) */
    DMA_Channel_TypeDef*   pDmaTxChannel;      /* DMA TX Stream/Channel Base */
    DMA_Channel_TypeDef*   pDmaRxChannel;      /* DMA RX Stream/Channel Base */
    uint32_t               ulDmaClockMask;     /* DMA Controller Clock Mask */

    /* 5. PFIC Interrupt Vector Configuration */
    IRQn_Type              eIrqNumber;         /* Interrupt Vector ID */
    uint8_t                ucIrqPreemptPri;    /* Preemption Priority */
    uint8_t                ucIrqSubPri;        /* Sub Priority */
} UART_Config_t;

/* Global Read-Only Flash Table Export */
extern const UART_Config_t g_UartConfigTable[UART_ID_MAX];

#ifdef __cplusplus
}
#endif

#endif /* BOARD_UART_CFG_H */