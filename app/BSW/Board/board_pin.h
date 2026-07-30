/**
 * @file    board_pin.h
 * @brief   硬件-软件接口 (HSI) 完备引脚映射定义头文件
 * @details 对应硬件系统板级引脚分配表 (完全展开版，包含全部 58 个引脚定义)。
 *          符合 DO-178C HSI Data 规范 (DO-178C Section 11.13)。
 * @author  zry
 * @date    2026-07-29
 * @version V0.0.1
 *
 * @note    System HLR Traceability: [REQ-HLR-BSP-002]
 * @docref  HSI-SPEC-CH32V30X-REV2
 * @copyright (c) 2026 zry. All rights reserved.
 */

#ifndef BOARD_PIN_H
#define BOARD_PIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v30x.h"

/* ============================================================================ */
/* 1. 按键与五向开关单元 (Buttons & Joystick)                                   */
/* ============================================================================ */

/* 按键 (Wake_Up / SW1 / SW2) */
#define KEY_WAKEUP_PIN                  GPIO_Pin_0
#define KEY_WAKEUP_PORT                 GPIOA
#define KEY_WAKEUP_CLK                  RCC_APB2Periph_GPIOA
#define KEY_WAKEUP_ACTIVE_LEVEL         1U  /* 按下输入 1 */

#define KEY_SW1_PIN                     GPIO_Pin_4
#define KEY_SW1_PORT                    GPIOE
#define KEY_SW1_CLK                     RCC_APB2Periph_GPIOE
#define KEY_SW1_ACTIVE_LEVEL            0U  /* 按下输入 0 */

#define KEY_SW2_PIN                     GPIO_Pin_5
#define KEY_SW2_PORT                    GPIOE
#define KEY_SW2_CLK                     RCC_APB2Periph_GPIOE
#define KEY_SW2_ACTIVE_LEVEL            0U  /* 按下输入 0 */

/* 五向开关 (Joystick) */
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

#define JOY_ACTIVE_LEVEL                0U  /* 按下输入 0 */


/* ============================================================================ */
/* 2. LED 指示灯单元 (LEDs)                                                    */
/* ============================================================================ */
#define LED1_PIN                        GPIO_Pin_11
#define LED1_PORT                       GPIOE
#define LED1_CLK                        RCC_APB2Periph_GPIOE
#define LED1_ACTIVE_LEVEL               0U  /* 输出 0 点亮 */

#define LED2_PIN                        GPIO_Pin_12
#define LED2_PORT                       GPIOE
#define LED2_CLK                        RCC_APB2Periph_GPIOE
#define LED2_ACTIVE_LEVEL               0U  /* 输出 0 点亮 */


/* ============================================================================ */
/* 3. 板载串口通信单元 (UART1 / UART2 / WiFi / BLE)                             */
/* ============================================================================ */

/* 串口 1 (UART1) - 物理复用 DVP_D0/D1 */
#define UART1_TX_PIN                    GPIO_Pin_9
#define UART1_TX_PORT                   GPIOA
#define UART1_RX_PIN                    GPIO_Pin_10
#define UART1_RX_PORT                   GPIOA

/* 串口 2 (UART2) - 调试器选择跳线帽连接 */
#define UART2_TX_PIN                    GPIO_Pin_2
#define UART2_TX_PORT                   GPIOA
#define UART2_RX_PIN                    GPIO_Pin_3
#define UART2_RX_PORT                   GPIOA

/* WiFi 接口 (UART6 / ESP8266) */
#define WIFI_UART_TX_PIN                GPIO_Pin_0
#define WIFI_UART_TX_PORT               GPIOC
#define WIFI_UART_RX_PIN                GPIO_Pin_1
#define WIFI_UART_RX_PORT               GPIOC

/* 蓝牙接口 (UART7 / CH9141) */
#define BLE_UART_TX_PIN                 GPIO_Pin_2
#define BLE_UART_TX_PORT                GPIOC
#define BLE_UART_RX_PIN                 GPIO_Pin_3
#define BLE_UART_RX_PORT                GPIOC

#define BLE_AT_PIN                      GPIO_Pin_7     /* 0: AT模式, 1: 透传模式 */
#define BLE_AT_PORT                     GPIOA
#define BLE_SLEEP_PIN                   GPIO_Pin_13    /* 低电平有效，低功耗模式 */
#define BLE_SLEEP_PORT                  GPIOC


/* ============================================================================ */
/* 4. 液晶屏 LCD 单元 (FSMC Parallel Interface)                                 */
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

#define LCD_FSMC_NOE_RD_PIN             GPIO_Pin_4     /* LCD_RD (读使能) */
#define LCD_FSMC_NOE_RD_PORT            GPIOD
#define LCD_FSMC_NWE_WR_PIN             GPIO_Pin_5     /* LCD_WR (写使能) */
#define LCD_FSMC_NWE_WR_PORT            GPIOD
#define LCD_FSMC_NE1_CS_PIN             GPIO_Pin_7     /* LCD_CS (片选) */
#define LCD_FSMC_NE1_CS_PORT            GPIOD
#define LCD_FSMC_A17_DC_PIN             GPIO_Pin_12    /* LCD_DC (命令/数据选择) */
#define LCD_FSMC_A17_DC_PORT            GPIOD

#define LCD_BL_PIN                      GPIO_Pin_14    /* 液晶背光，高电平有效 */
#define LCD_BL_PORT                     GPIOB
#define LCD_TE_PIN                      GPIO_Pin_4     /* Tearing Effect 帧同步 */
#define LCD_TE_PORT                     GPIOC


/* ============================================================================ */
/* 5. 摄像头 DVP 单元 (Digital Video Port) - [包含物理复用警告]                  */
/* ============================================================================ */
#define DVP_D0_PIN                      GPIO_Pin_9     /* 复用: UART1_TX */
#define DVP_D0_PORT                     GPIOA
#define DVP_D1_PIN                      GPIO_Pin_10    /* 复用: UART1_RX */
#define DVP_D1_PORT                     GPIOA
#define DVP_D2_PIN                      GPIO_Pin_8     /* 复用: TF卡 SD_D0 */
#define DVP_D2_PORT                     GPIOC
#define DVP_D3_PIN                      GPIO_Pin_9     /* 复用: TF卡 SD_D1 */
#define DVP_D3_PORT                     GPIOC
#define DVP_D4_PIN                      GPIO_Pin_11    /* 复用: TF卡 SD_D3 */
#define DVP_D4_PORT                     GPIOC
#define DVP_D5_PIN                      GPIO_Pin_6
#define DVP_D5_PORT                     GPIOB
#define DVP_D6_PIN                      GPIO_Pin_8
#define DVP_D6_PORT                     GPIOB
#define DVP_D7_PIN                      GPIO_Pin_9
#define DVP_D7_PORT                     GPIOB
#define DVP_D8_PIN                      GPIO_Pin_10    /* 复用: TF卡 SD_D2 */
#define DVP_D8_PORT                     GPIOC
#define DVP_D9_PIN                      GPIO_Pin_12    /* 复用: TF卡 SD_CLK */
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
/* 6. 音频编解码单元 (Audio Codec ES8388)                                       */
/* ============================================================================ */
#define AUDIO_I2S2_LRCK_PIN             GPIO_Pin_12
#define AUDIO_I2S2_LRCK_PORT            GPIOB
#define AUDIO_I2S2_SCLK_PIN             GPIO_Pin_13
#define AUDIO_I2S2_SCLK_PORT            GPIOB
#define AUDIO_I2S2_SD_PIN               GPIO_Pin_15
#define AUDIO_I2S2_SD_PORT              GPIOB
#define AUDIO_I2S2_MCLK_PIN             GPIO_Pin_6
#define AUDIO_I2S2_MCLK_PORT            GPIOC

#define AUDIO_CTL_PIN                   GPIO_Pin_8     /* 方向控制: 1录音, 0播放 */
#define AUDIO_CTL_PORT                  GPIOA
#define AUDIO_CTL_CLK                   RCC_APB2Periph_GPIOA
#define AUDIO_CTL_MODE_REC              1U
#define AUDIO_CTL_MODE_PLAY             0U


/* ============================================================================ */
/* 7. TF 卡 / SD 卡存储单元 (SDIO) - [复用 DVP 摄像头]                           */
/* ============================================================================ */
#define SD_D0_PIN                       GPIO_Pin_8     /* 复用: DVP_D2 */
#define SD_D0_PORT                      GPIOC
#define SD_D1_PIN                       GPIO_Pin_9     /* 复用: DVP_D3 */
#define SD_D1_PORT                      GPIOC
#define SD_D2_PIN                       GPIO_Pin_10    /* 复用: DVP_D8 */
#define SD_D2_PORT                      GPIOC
#define SD_D3_PIN                       GPIO_Pin_11    /* 复用: DVP_D4 */
#define SD_D3_PORT                      GPIOC
#define SD_CLK_PIN                      GPIO_Pin_12    /* 复用: DVP_D9 */
#define SD_CLK_PORT                     GPIOC
#define SD_CMD_PIN                      GPIO_Pin_2
#define SD_CMD_PORT                     GPIOD


/* ============================================================================ */
/* 8. SPI Flash 存储单元 (SPI3 W25Qxx)                                         */
/* ============================================================================ */
#define FLASH_SPI3_CS_PIN               GPIO_Pin_15
#define FLASH_SPI3_CS_PORT              GPIOA
#define FLASH_SPI3_CLK_PIN              GPIO_Pin_3
#define FLASH_SPI3_CLK_PORT             GPIOB
#define FLASH_SPI3_MISO_PIN             GPIO_Pin_4
#define FLASH_SPI3_MISO_PORT            GPIOB
#define FLASH_SPI3_MOSI_PIN             GPIO_Pin_5
#define FLASH_SPI3_MOSI_PORT            GPIOB


/* ============================================================================ */
/* 9. 共享 I2C2 / SCCB 外设总线 (DVP, ES8388, MPU6050, AHT10, AP3216C)           */
/* ============================================================================ */
#define SHARED_I2C2_SCL_PIN             GPIO_Pin_10
#define SHARED_I2C2_SCL_PORT            GPIOB
#define SHARED_I2C2_SDA_PIN             GPIO_Pin_11
#define SHARED_I2C2_SDA_PORT            GPIOB
#define SHARED_I2C2_PERIPH              I2C2

/* 传感器中断引脚 (Sensor Interrupt Pins) */
#define MPU6050_INT_PIN                 GPIO_Pin_5     /* 陀螺仪中断 */
#define MPU6050_INT_PORT                GPIOC
#define AP3216C_INT_PIN                 GPIO_Pin_6     /* 环境光传感器中断 */
#define AP3216C_INT_PORT                GPIOE


/* ============================================================================ */
/* 10. USB 接口单元 (USB1)                                                      */
/* ============================================================================ */
#define USB1_DM_PIN                     GPIO_Pin_11
#define USB1_DM_PORT                    GPIOA
#define USB1_DP_PIN                     GPIO_Pin_12
#define USB1_DP_PORT                    GPIOA


/* ============================================================================ */
/* 11. 调试器与晶振控制 (Debugger SWD & Oscillator)                             */
/* ============================================================================ */
#define SWDIO_PIN                       GPIO_Pin_13    /* 调试器 SWDIO */
#define SWDIO_PORT                      GPIOA
#define SWCLK_PIN                       GPIO_Pin_14    /* 调试器 SWCLK */
#define SWCLK_PORT                      GPIOA

#define OSC32_IN_PIN                    GPIO_Pin_14    /* 32.768KHz 晶振输入 */
#define OSC32_IN_PORT                   GPIOC
#define OSC32_OUT_PIN                   GPIO_Pin_15    /* 32.768KHz 晶振输出 */
#define OSC32_OUT_PORT                  GPIOC

#define BOOT1_PIN                       GPIO_Pin_2     /* BOOT1 选择位 */
#define BOOT1_PORT                      GPIOB


/* ============================================================================ */
/* 12. 扩展端口接口单元 (Expansion Ports & Alternate Functions)                  */
/* ============================================================================ */
#define EXT_PB0_PIN                     GPIO_Pin_0     /* ADC_IN8 / TIM3_CH3 / TIM8_CH2N / OPA1_CH1P */
#define EXT_PB0_PORT                    GPIOB
#define EXT_PB1_PIN                     GPIO_Pin_1     /* ADC_IN9 / TIM3_CH4 / TIM8_CH3N / OPA4_CH0N */
#define EXT_PB1_PORT                    GPIOB
#define EXT_PA1_PIN                     GPIO_Pin_1     /* ADC_IN1 / TIM5_CH2 / TIM2_CH2 / OPA3_OUT0 */
#define EXT_PA1_PORT                    GPIOA

#define EXT_PE13_PIN                    GPIO_Pin_13    /* FSMC_D10 / TIM1_CH3 / UART7_RX */
#define EXT_PE13_PORT                   GPIOE
#define EXT_PE14_PIN                    GPIO_Pin_14    /* FSMC_D11 / TIM1_CH4 / UART8_TX */
#define EXT_PE14_PORT                   GPIOE
#define EXT_PE15_PIN                    GPIO_Pin_15    /* FSMC_D12 / TIM1_BKIN / UART8_RX */
#define EXT_PE15_PORT                   GPIOE

#define EXT_PD3_PIN                     GPIO_Pin_3     /* FSMC_CLK / USART2_CTS / TIM10_CH2 */
#define EXT_PD3_PORT                    GPIOD
#define EXT_PD8_PIN                     GPIO_Pin_8     /* FSMC_D13 / USART3_TX / TIM9_CH1N */
#define EXT_PD8_PORT                    GPIOD
#define EXT_PD9_PIN                     GPIO_Pin_9     /* FSMC_D14 / USART3_RX / TIM9_CH1 / TIM9_ETR */
#define EXT_PD9_PORT                    GPIOD
#define EXT_PD10_PIN                    GPIO_Pin_10    /* FSMC_D15 / USART3_CK / TIM9_CH2N */
#define EXT_PD10_PORT                   GPIOD
#define EXT_PD11_PIN                    GPIO_Pin_11    /* FSMC_A16 / USART3_CTS / TIM9_CH2 */
#define EXT_PD11_PORT                   GPIOD

#ifdef __cplusplus
}
#endif

#endif /* BOARD_PIN_H */