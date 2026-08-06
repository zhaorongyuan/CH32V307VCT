/**
 * @file bsp_board.h
 * @brief Master Board Support Package (BSP) Layer Abstraction Interface
 * @details Unified board-level status codes, hardware lifecycle API declarations,
 *          and DO-178C requirement traceability tags.
 * @author zry
 * @date 2026-08-06
 * @version V1.0.0
 *
 * @note System HLR Traceability: [REQ-HLR-BSP-001], [REQ-HLR-BSP-004]
 * @copyright (c) 2026 zry. All rights reserved.
 */

 #ifndef BSP_BOARD_H
 #define BSP_BOARD_H
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 #include "ch32v30x.h"
 #include "sys_health.h"      /* 07_System: Health Monitoring, Watchdog & Fault Capture */
 #include "board_gpio_cfg.h"
 #include "board_uart_cfg.h"
 
 /**
  * @brief Board Level Operation Return Status Enumeration
  */
 typedef enum {
     BOARD_OK                = 0x00U, /* Operation Succeeded */
     BOARD_ERR_INVALID_PARAM = 0x01U, /* Null pointer or out-of-bounds parameter */
     BOARD_ERR_HW_TIMEOUT    = 0x02U, /* Hardware clock / peripheral lock timeout */
     BOARD_ERR_VERIFY        = 0x03U  /* Hardware read-back mismatch / stuck-at fault */
 } Board_Status_t;
 
 /* ============================================================================ */
 /* Master Board Initialization API Declarations                                */
 /* ============================================================================ */
 
 /**
  * @brief Top-level System BSP Bridge Entry Point
  * @details Called by main.c to perform architecture and board initialization.
  * @return Sys_Status_t Unified System Error Code
  *
  * @note LLR Traceability: [REQ-SW-BSP-0100]
  * @note Safety Criticality: DAL-A / Fail-Safe Entry
  */
 Sys_Status_t BSP_Init(void);
 
 /**
  * @brief Initialize Core Microcontroller Low-Level Infrastructure
  * @details Configures NVIC Priority Grouping, System Clock variables, and SysTick interrupts.
  *
  * @note LLR Traceability: [REQ-SW-BSP-0101]
  * @note Safety Criticality: DAL-A / Single-threaded Execution
  */
 void System_Core_Init(void);
 
 /**
  * @brief Board Phase-1 Hardware Master Driver Initialization
  * @details Sequentially calls Clock, GPIO, and UART setup routines with error containment.
  *
  * @return Board_Status_t
  * @retval BOARD_OK Driver setup fully successful.
  * @retval BOARD_ERR_INVALID_PARAM Configuration table error or NULL pointer.
  * @retval BOARD_ERR_HW_TIMEOUT Clock lock timeout.
  * @retval BOARD_ERR_VERIFY Hardware read-back mismatch fault.
  *
  * @note LLR Traceability: [REQ-SW-BSP-0102]
  * @note Safety Criticality: DAL-A
  */
 Board_Status_t Board_Init(void);
 
 /**
  * @brief Data-Driven GPIO Initialization with Hardware Read-Back Verification
  * @return Board_Status_t
  *
  * @note LLR Traceability: [REQ-SW-BSP-0103]
  * @note Safety Criticality: DAL-A / Bounded Loop (Max iterations: g_GpioConfigTableSize)
  */
 Board_Status_t Board_Gpio_Init(void);
 
 /**
  * @brief Data-Driven Master UART Initialization
  * @return Board_Status_t
  *
  * @note LLR Traceability: [REQ-SW-BSP-0104]
  * @note Safety Criticality: DAL-B / Bounded Loop (Max iterations: UART_ID_MAX)
  */
 Board_Status_t Board_Uart_Init(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* BSP_BOARD_H */