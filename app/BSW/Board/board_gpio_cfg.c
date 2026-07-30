/**
 * @file    board_gpio_cfg.c
 * @brief   GPIO 板级硬件引脚配置表落地实现
 * @details 集中维护板载基础离散量引脚（LED、Keys、Joystick、Control Pins）的安全初始状态。
 * @author  zry
 * @date    2026-07-30
 * @version V0.0.1
 *
 * @note    System HLR Traceability: [REQ-HLR-BSP-002]
 * @copyright (c) 2026 zry. All rights reserved.
 */

#include "board_gpio_cfg.h"
#include "board_pin.h"

/**
 * @brief 静态 GPIO 配置描述表（置于 Flash 只读段，防止运行期踩踏）
 */
const Board_GPIO_Config_t g_GpioConfigTable[] = {
    /* 1. LED1 指示灯 (默认熄灭: 高电平) */
    {
        .pPort            = LED1_PORT,
        .usPin            = LED1_PIN,
        .eMode            = GPIO_Mode_Out_PP,
        .eSpeed           = GPIO_Speed_50MHz,
        .eInitialState    = Bit_SET,
        .ucApplyInitState = 1U
    },
    /* 2. LED2 指示灯 (默认熄灭: 高电平) */
    {
        .pPort            = LED2_PORT,
        .usPin            = LED2_PIN,
        .eMode            = GPIO_Mode_Out_PP,
        .eSpeed           = GPIO_Speed_50MHz,
        .eInitialState    = Bit_SET,
        .ucApplyInitState = 1U
    },
    /* 3. WAKEUP 按键 (下拉输入) */
    {
        .pPort            = KEY_WAKEUP_PORT,
        .usPin            = KEY_WAKEUP_PIN,
        .eMode            = GPIO_Mode_IPD,
        .eSpeed           = GPIO_Speed_50MHz,
        .eInitialState    = Bit_RESET,
        .ucApplyInitState = 0U
    },
    /* 4. SW1 & SW2 按键 (上拉输入) */
    {
        .pPort            = KEY_SW1_PORT,
        .usPin            = KEY_SW1_PIN | KEY_SW2_PIN,
        .eMode            = GPIO_Mode_IPU,
        .eSpeed           = GPIO_Speed_50MHz,
        .eInitialState    = Bit_SET,
        .ucApplyInitState = 0U
    },
    /* 5. 五向开关 (UP/DOWN/RIGHT - GPIOE) */
    {
        .pPort            = GPIOE,
        .usPin            = JOY_UP_PIN | JOY_DOWN_PIN | JOY_RIGHT_PIN,
        .eMode            = GPIO_Mode_IPU,
        .eSpeed           = GPIO_Speed_50MHz,
        .eInitialState    = Bit_SET,
        .ucApplyInitState = 0U
    },
    /* 6. 五向开关 (LEFT/SEL - GPIOD) */
    {
        .pPort            = GPIOD,
        .usPin            = JOY_LEFT_PIN | JOY_SEL_PIN,
        .eMode            = GPIO_Mode_IPU,
        .eSpeed           = GPIO_Speed_50MHz,
        .eInitialState    = Bit_SET,
        .ucApplyInitState = 0U
    },
    /* 7. 音频控制引脚 (默认为播放模式: 0) */
    {
        .pPort            = AUDIO_CTL_PORT,
        .usPin            = AUDIO_CTL_PIN,
        .eMode            = GPIO_Mode_Out_PP,
        .eSpeed           = GPIO_Speed_50MHz,
        .eInitialState    = (BitAction)AUDIO_CTL_MODE_PLAY,
        .ucApplyInitState = 1U
    },
    /* 8. 蓝牙 AT 指令引脚 */
    {
        .pPort            = BLE_AT_PORT,
        .usPin            = BLE_AT_PIN,
        .eMode            = GPIO_Mode_Out_PP,
        .eSpeed           = GPIO_Speed_10MHz,
        .eInitialState    = Bit_RESET,
        .ucApplyInitState = 1U
    },
    /* 9. 蓝牙 SLEEP 控制引脚 */
    {
        .pPort            = BLE_SLEEP_PORT,
        .usPin            = BLE_SLEEP_PIN,
        .eMode            = GPIO_Mode_Out_PP,
        .eSpeed           = GPIO_Speed_10MHz,
        .eInitialState    = Bit_RESET,
        .ucApplyInitState = 1U
    },
    /* 10. LCD 液晶背光控制引脚 (高电平使能) */
    {
        .pPort            = LCD_BL_PORT,
        .usPin            = LCD_BL_PIN,
        .eMode            = GPIO_Mode_Out_PP,
        .eSpeed           = GPIO_Speed_50MHz,
        .eInitialState    = Bit_RESET,
        .ucApplyInitState = 1U
    }
};

const uint8_t g_GpioConfigTableSize = (uint8_t)(sizeof(g_GpioConfigTable) / sizeof(g_GpioConfigTable[0]));
