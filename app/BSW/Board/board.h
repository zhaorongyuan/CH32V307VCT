/**
 * @file    board.h
 * @brief   板级支持包 (BSP) 抽象接口头文件
 * @details 屏蔽硬件低电平/高电平有效差异，向上层提供一致的控制与读取接口。
 * @author  zry
 * @date    2026-07-29
 * @version V2.0.0
 *
 * @docref  BSP-INTERFACE-SPEC-V2
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_H
#define BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v30x.h"
#include "board_pin.h"
#include "board_uart_cfg.h"

/* ============================================================================ */
/* 1. LED 控制功能宏 (屏蔽输出 0 点亮逻辑)                                      */
/* ============================================================================ */
#define BOARD_LED1_ON()         GPIO_ResetBits(LED1_PORT, LED1_PIN)
#define BOARD_LED1_OFF()        GPIO_SetBits(LED1_PORT, LED1_PIN)
#define BOARD_LED1_TOGGLE()     ((GPIO_ReadOutputDataBit(LED1_PORT, LED1_PIN) != 0U) ? \
                                 GPIO_ResetBits(LED1_PORT, LED1_PIN) : \
                                 GPIO_SetBits(LED1_PORT, LED1_PIN))

#define BOARD_LED2_ON()         GPIO_ResetBits(LED2_PORT, LED2_PIN)
#define BOARD_LED2_OFF()        GPIO_SetBits(LED2_PORT, LED2_PIN)
#define BOARD_LED2_TOGGLE()     ((GPIO_ReadOutputDataBit(LED2_PORT, LED2_PIN) != 0U) ? \
                                 GPIO_ResetBits(LED2_PORT, LED2_PIN) : \
                                 GPIO_SetBits(LED2_PORT, LED2_PIN))

/* ============================================================================ */
/* 2. 按键与开关读取宏 (触发统一返回 1，未触发返回 0)                            */
/* ============================================================================ */
#define BOARD_KEY_WAKEUP_READ() (GPIO_ReadInputDataBit(KEY_WAKEUP_PORT, KEY_WAKEUP_PIN) == Bit_SET)
#define BOARD_KEY_SW1_READ()    (GPIO_ReadInputDataBit(KEY_SW1_PORT, KEY_SW1_PIN) == Bit_RESET)
#define BOARD_KEY_SW2_READ()    (GPIO_ReadInputDataBit(KEY_SW2_PORT, KEY_SW2_PIN) == Bit_RESET)

#define BOARD_JOY_UP_READ()     (GPIO_ReadInputDataBit(JOY_UP_PORT, JOY_UP_PIN) == Bit_RESET)
#define BOARD_JOY_DOWN_READ()   (GPIO_ReadInputDataBit(JOY_DOWN_PORT, JOY_DOWN_PIN) == Bit_RESET)
#define BOARD_JOY_LEFT_READ()   (GPIO_ReadInputDataBit(JOY_LEFT_PORT, JOY_LEFT_PIN) == Bit_RESET)
#define BOARD_JOY_RIGHT_READ()  (GPIO_ReadInputDataBit(JOY_RIGHT_PORT, JOY_RIGHT_PIN) == Bit_RESET)
#define BOARD_JOY_SEL_READ()    (GPIO_ReadInputDataBit(JOY_SEL_PORT, JOY_SEL_PIN) == Bit_RESET)

/* ============================================================================ */
/* 3. 音频通道切换宏                                                             */
/* ============================================================================ */
#define BOARD_AUDIO_SET_REC()   GPIO_SetBits(AUDIO_CTL_PORT, AUDIO_CTL_PIN)
#define BOARD_AUDIO_SET_PLAY()  GPIO_ResetBits(AUDIO_CTL_PORT, AUDIO_CTL_PIN)

/* ============================================================================ */
/* 4. 对外导出的 API 声明                                                       */
/* ============================================================================ */
void Board_Init(void);
void Board_Gpio_Init(void);
void Board_Uart_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */