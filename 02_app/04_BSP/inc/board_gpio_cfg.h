/**
 * @file board_gpio_cfg.h
 * @brief Discrete GPIO Configuration Data Structure and Logical Channel Definitions
 * @details Fully defines logical GPIO IDs, table structure, and read-only configuration interfaces.
 * @author zry
 * @date 2026-08-06
 * @version V1.0.0
 *
 * @note System HLR Traceability: [REQ-HLR-BSP-002]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_GPIO_CFG_H
#define BOARD_GPIO_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v30x.h"

/**
 * @brief Logical Discrete GPIO Output Channel Enumeration
 * @details Used for safety-critical per-channel control and fault reporting.
 */
typedef enum {
    BOARD_GPIO_OUT_LED1 = 0U,       /* LED1 Indicator (Active Low / Default High) */
    BOARD_GPIO_OUT_LED2,            /* LED2 Indicator (Active Low / Default High) */
    BOARD_GPIO_OUT_AUDIO_CTL,       /* Audio Control Pin (Play Mode) */
    BOARD_GPIO_OUT_BLE_AT,          /* Bluetooth AT Command Mode Pin */
    BOARD_GPIO_OUT_BLE_SLEEP,       /* Bluetooth Sleep Mode Pin */
    BOARD_GPIO_OUT_LCD_BL,          /* LCD Backlight Control (Active High) */
    BOARD_GPIO_OUT_MAX_COUNT
} Board_GPIO_OutChannel_t;

/**
 * @brief GPIO Pin Configuration Structure (Stored in Flash / Read-Only Section)
 */
typedef struct {
    GPIO_TypeDef*      pPort;            /* Physical GPIO Port Base Address */
    uint16_t           usPin;            /* Physical GPIO Pin Mask */
    GPIOMode_TypeDef   eMode;            /* Pin Mode (Out_PP, IPU, IPD, etc.) */
    GPIOSpeed_TypeDef  eSpeed;           /* Output Slew Rate */
    BitAction          eInitialState;    /* Pre-latch Safe State (Bit_SET / Bit_RESET) */
    uint8_t            ucApplyInitState; /* 1: Latch state before init + Read-back verify; 0: Pure input */
} Board_GPIO_Config_t;

/* Global Read-Only Configuration Table References */
extern const Board_GPIO_Config_t g_GpioConfigTable[];
extern const uint8_t g_GpioConfigTableSize;

#ifdef __cplusplus
}
#endif

#endif /* BOARD_GPIO_CFG_H */