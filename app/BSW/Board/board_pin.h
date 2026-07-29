/**
 * @file    board_pin.h
 * @brief   硬件-软件接口 (HSI) 引脚映射定义头文件
 * @details 对应硬件系统板级引脚分配表，提供外设 GPIO 端口、引脚及时钟宏定义。
 *          符合 DO-178C HSI Data 规范 (DO-178C Section 11.13)。
 * @author  Avionics Software Engineering Team / zry
 * @date    2026-07-29
 * @version V0.0.0
 *
 * @docref  HSI-SPEC-CH32V30X-REV2
 * @note    更改硬件底层引脚配置时，仅需修改本文件的物理映射，保持上层驱动不变。
 *
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_PIN_H
#define BOARD_PIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================ */
/*                             系统与头文件包含                                 */
/* ============================================================================ */
#include "ch32v30x.h"

/* ============================================================================ */
/* 1. 用户接口单元 (User Interface: Keys, Joystick, LEDs)                        */
/* ============================================================================ */

/* 按键 (Keys) */
#define KEY_WAKEUP_PIN                  GPIO_Pin_0
#define KEY_WAKEUP_PORT                 GPIOA
#define KEY_WAKEUP_CLK                  RCC_APB2Periph_GPIOA
#define KEY_WAKEUP_ACTIVE_LEVEL         1   /* 按下输入 1 */

#define KEY_SW1_PIN                     GPIO_Pin_4
#define KEY_SW1_PORT                    GPIOE
#define KEY_SW1_CLK                     RCC_APB2Periph_GPIOE
#define KEY_SW1_ACTIVE_LEVEL            0   /* 按下输入 0 */

#define KEY_SW2_PIN                     GPIO_Pin_5
#define KEY_SW2_PORT                    GPIOE
#define KEY_SW2_CLK                     RCC_APB2Periph_GPIOE
#define KEY_SW2_ACTIVE_LEVEL            0   /* 按下输入 0 */

/* 五向开关 (5-Way Switch) */
#define JOY_UP_PIN                      GPIO_Pin_1
#define JOY_UP_PORT                     GPIOE
#define JOY_UP_CLK                      RCC_APB2Periph_GPIOE

#define JOY_DOWN_PIN                    GPIO_Pin_2
#define JOY_DOWN_PORT                   GPIOE
#define JOY_DOWN_CLK                    RCC_APB2Periph_GPIOE

#define JOY_LEFT_PIN                    GPIO_Pin_6
#define JOY_LEFT_PORT                   GPIOD
#define JOY_LEFT_CLK                    RCC_APB2Periph_GPIOD

#define JOY_RIGHT_PIN                   GPIO_Pin_3
#define JOY_RIGHT_PORT                  GPIOE
#define JOY_RIGHT_CLK                   RCC_APB2Periph_GPIOE

#define JOY_SEL_PIN                     GPIO_Pin_13
#define JOY_SEL_PORT                    GPIOD
#define JOY_SEL_CLK                     RCC_APB2Periph_GPIOD

/* 指示灯 (LEDs) */
#define LED1_PIN                        GPIO_Pin_11
#define LED1_PORT                       GPIOE
#define LED1_CLK                        RCC_APB2Periph_GPIOE
#define LED1_ACTIVE_LEVEL               0   /* 输出 0 点亮 */

#define LED2_PIN                        GPIO_Pin_12
#define LED2_PORT                       GPIOE
#define LED2_CLK                        RCC_APB2Periph_GPIOE
#define LED2_ACTIVE_LEVEL               0   /* 输出 0 点亮 */


/* ============================================================================ */
/* 2. 通信接口单元 (Communications: UART, WiFi, BLE)                             */
/* ============================================================================ */

/* 串口 1 (UART1) - 警告: 与 DVP_D0/D1 硬件复用 */
#define UART1_TX_PIN                    GPIO_Pin_9
#define UART1_TX_PORT                   GPIOA
#define UART1_RX_PIN                    GPIO_Pin_10
#define UART1_RX_PORT                   GPIOA
#define UART1_CLK                       RCC_APB2Periph_USART1
#define UART1_GPIO_CLK                  RCC_APB2Periph_GPIOA
#define UART1_PERIPH                    USART1

/* 串口 2 (UART2) - 调试器连接选跳线 */
#define UART2_TX_PIN                    GPIO_Pin_2
#define UART2_TX_PORT                   GPIOA
#define UART2_RX_PIN                    GPIO_Pin_3
#define UART2_RX_PORT                   GPIOA
#define UART2_CLK                       RCC_APB1Periph_USART2
#define UART2_GPIO_CLK                  RCC_APB2Periph_GPIOA
#define UART2_PERIPH                    USART2

/* WiFi 接口 (UART6 / ESP8266) */
#define WIFI_UART_TX_PIN                GPIO_Pin_0
#define WIFI_UART_TX_PORT               GPIOC
#define WIFI_UART_RX_PIN                GPIO_Pin_1
#define WIFI_UART_RX_PORT               GPIOC
#define WIFI_UART_CLK                   RCC_APB1Periph_UART6
#define WIFI_UART_GPIO_CLK              RCC_APB2Periph_GPIOC
#define WIFI_UART_PERIPH                UART6

/* 蓝牙接口 (UART7 / CH9141) */
#define BLE_UART_TX_PIN                 GPIO_Pin_2
#define BLE_UART_TX_PORT                GPIOC
#define BLE_UART_RX_PIN                 GPIO_Pin_3
#define BLE_UART_RX_PORT                GPIOC
#define BLE_UART_CLK                    RCC_APB1Periph_UART7
#define BLE_UART_GPIO_CLK               RCC_APB2Periph_GPIOC
#define BLE_UART_PERIPH                 UART7

#define BLE_AT_PIN                      GPIO_Pin_7
#define BLE_AT_PORT                     GPIOA
#define BLE_AT_CLK                      RCC_APB2Periph_GPIOA

#define BLE_SLEEP_PIN                   GPIO_Pin_13
#define BLE_SLEEP_PORT                  GPIOC
#define BLE_SLEEP_CLK                   RCC_APB2Periph_GPIOC


/* ============================================================================ */
/* 3. 显示屏接口单元 (Display Interface: FSMC LCD)                               */
/* ============================================================================ */

#define LCD_FSMC_D0_PIN                 GPIO_Pin_14
#define LCD_FSMC_D0_PORT                GPIOD
#define LCD_FSMC_D1_PIN                 GPIO_Pin_15
#define LCD_FSMC_D1_PORT                GPIOD
#define LCD_FSMC_D2_PIN                 GPIO_Pin_0
#define LCD_FSMC_D2_PORT                GPIOD
#define LCD_FSMC_D3_PIN                 GPIO_Pin_1
#define LCD_FSMC_D3_PORT                GPIOD
#define LCD_FSMC_D4_PIN                 GPIO_Pin_7
#define LCD_FSMC_D4_PORT                GPIOE
#define LCD_FSMC_D5_PIN                 GPIO_Pin_8
#define LCD_FSMC_D5_PORT                GPIOE
#define LCD_FSMC_D6_PIN                 GPIO_Pin_9
#define LCD_FSMC_D6_PORT                GPIOE
#define LCD_FSMC_D7_PIN                 GPIO_Pin_10
#define LCD_FSMC_D7_PORT                GPIOE

#define LCD_FSMC_NOE_RD_PIN             GPIO_Pin_4
#define LCD_FSMC_NOE_RD_PORT            GPIOD
#define LCD_FSMC_NWE_WR_PIN             GPIO_Pin_5
#define LCD_FSMC_NWE_WR_PORT            GPIOD
#define LCD_FSMC_NE1_CS_PIN             GPIO_Pin_7
#define LCD_FSMC_NE1_CS_PORT            GPIOD
#define LCD_FSMC_A17_DC_PIN             GPIO_Pin_12
#define LCD_FSMC_A17_DC_PORT            GPIOD

#define LCD_BL_PIN                      GPIO_Pin_14
#define LCD_BL_PORT                     GPIOB
#define LCD_BL_CLK                      RCC_APB2Periph_GPIOB

#define LCD_TE_PIN                      GPIO_Pin_4
#define LCD_TE_PORT                     GPIOC
#define LCD_TE_CLK                      RCC_APB2Periph_GPIOC


/* ============================================================================ */
/* 4. 共享总线与外设 (Shared Bus: I2C2 / SCCB)                                  */
/* ============================================================================ */

/* 共享 I2C2 / SCCB 控制总线 (挂载: DVP, ES8388, MPU6050, AHT10, AP3216C) */
#define SHARED_I2C2_SCL_PIN             GPIO_Pin_10
#define SHARED_I2C2_SCL_PORT            GPIOB
#define SHARED_I2C2_SDA_PIN             GPIO_Pin_11
#define SHARED_I2C2_SDA_PORT            GPIOB
#define SHARED_I2C2_GPIO_CLK            RCC_APB2Periph_GPIOB
#define SHARED_I2C2_PERIPH              I2C2


/* ============================================================================ */
/* 5. 摄像头接口单元 (Camera DVP) - 警告: 与 SD卡 / UART1 存在物理复用          */
/* ============================================================================ */

#define DVP_D0_PIN                      GPIO_Pin_9     /* 复用 UART1_TX */
#define DVP_D0_PORT                     GPIOA
#define DVP_D1_PIN                      GPIO_Pin_10    /* 复用 UART1_RX */
#define DVP_D1_PORT                     GPIOA
#define DVP_D2_PIN                      GPIO_Pin_8     /* 复用 SD_D0 */
#define DVP_D2_PORT                     GPIOC
#define DVP_D3_PIN                      GPIO_Pin_9     /* 复用 SD_D1 */
#define DVP_D3_PORT                     GPIOC
#define DVP_D4_PIN                      GPIO_Pin_11    /* 复用 SD_D3 */
#define DVP_D4_PORT                     GPIOC
#define DVP_D5_PIN                      GPIO_Pin_6
#define DVP_D5_PORT                     GPIOB
#define DVP_D6_PIN                      GPIO_Pin_8
#define DVP_D6_PORT                     GPIOB
#define DVP_D7_PIN                      GPIO_Pin_9
#define DVP_D7_PORT                     GPIOB
#define DVP_D8_PIN                      GPIO_Pin_10    /* 复用 SD_D2 */
#define DVP_D8_PORT                     GPIOC
#define DVP_D9_PIN                      GPIO_Pin_12    /* 复用 SD_CLK */
#define DVP_D9_PORT                     GPIOC

#define DVP_RESET_PIN                   GPIO_Pin_7
#define DVP_RESET_PORT                  GPIOB
#define DVP_HSYNC_PIN                   GPIO_Pin_4
#define DVP_HSYNC_PORT                  GPIOA
#define DVP_VSYNC_PIN                   GPIO_Pin_5
#define DVP_VSYNC_PORT                  GPIOA
#define DVP_PCLK_PIN                    GPIO_Pin_6
#define DVP_PCLK_PORT                   GPIOA
#define DVP_PWDN_PIN                    GPIO_Pin_7
#define DVP_PWDN_PORT                   GPIOC


/* ============================================================================ */
/* 6. 音频接口单元 (Audio ES8388)                                               */
/* ============================================================================ */

#define AUDIO_I2S2_LRCK_PIN             GPIO_Pin_12
#define AUDIO_I2S2_LRCK_PORT            GPIOB
#define AUDIO_I2S2_SCLK_PIN             GPIO_Pin_13
#define AUDIO_I2S2_SCLK_PORT            GPIOB
#define AUDIO_I2S2_SD_PIN               GPIO_Pin_15
#define AUDIO_I2S2_SD_PORT              GPIOB
#define AUDIO_I2S2_MCLK_PIN             GPIO_Pin_6
#define AUDIO_I2S2_MCLK_PORT            GPIOC

#define AUDIO_CTL_PIN                   GPIO_Pin_8
#define AUDIO_CTL_PORT                  GPIOA
#define AUDIO_CTL_CLK                   RCC_APB2Periph_GPIOA
#define AUDIO_CTL_MODE_REC              1   /* 1: ES8388 -> MCU (录音) */
#define AUDIO_CTL_MODE_PLAY             0   /* 0: MCU -> ES8388 (播放) */


/* ============================================================================ */
/* 7. SD 卡/TF 卡接口单元 (SDIO) - 警告: 与 DVP 摄像头引脚物理复用               */
/* ============================================================================ */

#define SD_D0_PIN                       GPIO_Pin_8     /* 复用 DVP_D2 */
#define SD_D0_PORT                      GPIOC
#define SD_D1_PIN                       GPIO_Pin_9     /* 复用 DVP_D3 */
#define SD_D1_PORT                      GPIOC
#define SD_D2_PIN                       GPIO_Pin_10    /* 复用 DVP_D8 */
#define SD_D2_PORT                      GPIOC
#define SD_D3_PIN                       GPIO_Pin_11    /* 复用 DVP_D4 */
#define SD_D3_PORT                      GPIOC
#define SD_CLK_PIN                      GPIO_Pin_12    /* 复用 DVP_D9 */
#define SD_CLK_PORT                     GPIOC
#define SD_CMD_PIN                      GPIO_Pin_2
#define SD_CMD_PORT                     GPIOD


/* ============================================================================ */
/* 8. SPI Flash 存储单元 (SPI3)                                                 */
/* ============================================================================ */

#define FLASH_SPI3_CS_PIN               GPIO_Pin_15
#define FLASH_SPI3_CS_PORT              GPIOA
#define FLASH_SPI3_CLK_PIN              GPIO_Pin_3
#define FLASH_SPI3_CLK_PORT             GPIOB
#define FLASH_SPI3_MISO_PIN             GPIO_Pin_4
#define FLASH_SPI3_MISO_PORT            GPIOB
#define FLASH_SPI3_MOSI_PIN             GPIO_Pin_5
#define FLASH_SPI3_MOSI_PORT            GPIOB
#define FLASH_SPI3_PERIPH               SPI3


/* ============================================================================ */
/* 9. 传感器中断单元 (Sensors Interrupts)                                       */
/* ============================================================================ */

/* MPU6050 陀螺仪中断 */
#define MPU_INT_PIN                     GPIO_Pin_5
#define MPU_INT_PORT                    GPIOC
#define MPU_INT_CLK                     RCC_APB2Periph_GPIOC

/* AP3216C 环境光传感器中断 */
#define AP_INT_PIN                      GPIO_Pin_6
#define AP_INT_PORT                     GPIOE
#define AP_INT_CLK                      RCC_APB2Periph_GPIOE


/* ============================================================================ */
/* 10. USB 及调试接口 (USB & Debugger)                                          */
/* ============================================================================ */

#define USB1_DM_PIN                     GPIO_Pin_11
#define USB1_DM_PORT                    GPIOA
#define USB1_DP_PIN                     GPIO_Pin_12
#define USB1_DP_PORT                    GPIOA

#define SWDIO_PIN                       GPIO_Pin_13
#define SWDIO_PORT                      GPIOA
#define SWCLK_PIN                       GPIO_Pin_14
#define SWCLK_PORT                      GPIOA


/* ============================================================================ */
/* 11. 系统时钟与启动配置 (System Clock & Boot)                                 */
/* ============================================================================ */

#define OSC32_IN_PIN                    GPIO_Pin_14
#define OSC32_IN_PORT                   GPIOC
#define OSC32_OUT_PIN                   GPIO_Pin_15
#define OSC32_OUT_PORT                  GPIOC

#define BOOT1_PIN                       GPIO_Pin_2
#define BOOT1_PORT                      GPIOB


/* ============================================================================ */
/* 12. 扩展端口接口单元 (Expansion Ports)                                       */
/* ============================================================================ */

#define EXT_PB0_PIN                     GPIO_Pin_0
#define EXT_PB0_PORT                    GPIOB
#define EXT_PB1_PIN                     GPIO_Pin_1
#define EXT_PB1_PORT                    GPIOB
#define EXT_PA1_PIN                     GPIO_Pin_1
#define EXT_PA1_PORT                    GPIOA

#define EXT_PE13_PIN                    GPIO_Pin_13
#define EXT_PE13_PORT                   GPIOE
#define EXT_PE14_PIN                    GPIO_Pin_14
#define EXT_PE14_PORT                   GPIOE
#define EXT_PE15_PIN                    GPIO_Pin_15
#define EXT_PE15_PORT                   GPIOE

#define EXT_PD3_PIN                     GPIO_Pin_3
#define EXT_PD3_PORT                    GPIOD
#define EXT_PD8_PIN                     GPIO_Pin_8
#define EXT_PD8_PORT                    GPIOD
#define EXT_PD9_PIN                     GPIO_Pin_9
#define EXT_PD9_PORT                    GPIOD
#define EXT_PD10_PIN                    GPIO_Pin_10
#define EXT_PD10_PORT                   GPIOD
#define EXT_PD11_PIN                    GPIO_Pin_11
#define EXT_PD11_PORT                   GPIOD

#ifdef __cplusplus
}
#endif

#endif /* BOARD_PIN_H */