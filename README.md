# 基于 openCH 赤菟 开发板 v1.0.3 进行 bootloader 和 app 开发

## 简介

    本项目基于 openCH 赤菟 开发板 v1.0.3 进行 bootloader 和 app 开发 

## 规划
# CH32V30x 完整 HAL 接口层方案

以下是基于前面讨论的完整落地方案，所有代码可直接用于 CH32V30x 项目。

---

## 一、文件结构

```
02_app
├─ .cproject
├─ .project
├─ .template
├─ CH32V307VCT.launch
├─ CH32V307VCT.wvproj
├─ main.c
├─ README - 副本.md
├─ README.md
├─ 09_TEST
│  ├─ debug.c
│  └─ debug.h
├─ 08_Config
│  ├─ app_cfg.h
│  ├─ board_cfg.h
│  └─ mcal_cfg.h
├─ 07_System
│  ├─ system_ch32v30x.c
│  ├─ system_ch32v30x.h
│  └─ system_isr.c
├─ 06_APP
│  ├─ task_control.c
│  ├─ task_control.h
│  ├─ task_ota.c
│  └─ task_ota.h
├─ 05_Middleware
│  ├─ RTE
│  │  ├─ rte_interface.h
│  │  ├─ rte_signal.c
│  │  └─ rte_signal.h
│  └─ PDI
│     ├─ pdi_def.h
│     ├─ pdi_default.c
│     ├─ pdi_mgr.c
│     └─ pdi_mgr.h
├─ 04_BSP
│  └─ src
│     ├─ board.c
│     ├─ board.h
│     ├─ board_gpio_cfg.c
│     ├─ board_gpio_cfg.h
│     ├─ board_pin.h
│     ├─ board_uart_cfg.c
│     └─ board_uart_cfg.h
├─ 03_MCAL
│  ├─ src
│  │  ├─ ch32v30x_adc.c
│  │  ├─ ch32v30x_bkp.c
│  │  ├─ ch32v30x_can.c
│  │  ├─ ch32v30x_crc.c
│  │  ├─ ch32v30x_dac.c
│  │  ├─ ch32v30x_dbgmcu.c
│  │  ├─ ch32v30x_dma.c
│  │  ├─ ch32v30x_dvp.c
│  │  ├─ ch32v30x_eth.c
│  │  ├─ ch32v30x_exti.c
│  │  ├─ ch32v30x_flash.c
│  │  ├─ ch32v30x_fsmc.c
│  │  ├─ ch32v30x_gpio.c
│  │  ├─ ch32v30x_i2c.c
│  │  ├─ ch32v30x_iwdg.c
│  │  ├─ ch32v30x_misc.c
│  │  ├─ ch32v30x_opa.c
│  │  ├─ ch32v30x_pwr.c
│  │  ├─ ch32v30x_rcc.c
│  │  ├─ ch32v30x_rng.c
│  │  ├─ ch32v30x_rtc.c
│  │  ├─ ch32v30x_sdio.c
│  │  ├─ ch32v30x_spi.c
│  │  ├─ ch32v30x_tim.c
│  │  ├─ ch32v30x_usart.c
│  │  └─ ch32v30x_wwdg.c
│  └─ inc
│     ├─ ch32v30x.h
│     ├─ ch32v30x_adc.h
│     ├─ ch32v30x_bkp.h
│     ├─ ch32v30x_can.h
│     ├─ ch32v30x_conf.h
│     ├─ ch32v30x_crc.h
│     ├─ ch32v30x_dac.h
│     ├─ ch32v30x_dbgmcu.h
│     ├─ ch32v30x_dma.h
│     ├─ ch32v30x_dvp.h
│     ├─ ch32v30x_eth.h
│     ├─ ch32v30x_exti.h
│     ├─ ch32v30x_flash.h
│     ├─ ch32v30x_fsmc.h
│     ├─ ch32v30x_gpio.h
│     ├─ ch32v30x_i2c.h
│     ├─ ch32v30x_iwdg.h
│     ├─ ch32v30x_misc.h
│     ├─ ch32v30x_opa.h
│     ├─ ch32v30x_pwr.h
│     ├─ ch32v30x_rcc.h
│     ├─ ch32v30x_rng.h
│     ├─ ch32v30x_rtc.h
│     ├─ ch32v30x_sdio.h
│     ├─ ch32v30x_spi.h
│     ├─ ch32v30x_tim.h
│     ├─ ch32v30x_usart.h
│     ├─ ch32v30x_usb.h
│     └─ ch32v30x_wwdg.h
├─ 02_Core
│  ├─ core_riscv.c
│  ├─ core_riscv.h
│  ├─ startup_ch32v30x_D8.S
│  └─ startup_ch32v30x_D8C.S
├─ 01_Build
│  └─ Link.ld
└─ .mrs
   ├─ 1d57fbca0f50a7b8309678a867876416-.snapshot
   ├─ b1ce2fc5d92f14790d5d4b98a0010c69-.snapshot
   ├─ CH32V307VCT.mrs-workspace
   ├─ launch.json
   └─ preferredEncoding.json

```

---

## 引脚分配

| 端口功能            | 引脚名  | 功能       | 备注                                                              |
| ------------------- | ------- | ---------- | ----------------------------------------------------------------- |
| 按键                | PA0     | Wake_Up    | 按下输入1                                                         |
|                     | PE4     | SW1        | 按下输入0                                                         |
|                     | PE5     | SW2        | 按下输入0                                                         |
| 五向开关            | PE1     | JOY_UP     | 按下输入0                                                         |
|                     | PE2     | JOY_DOWN   | 按下输入0                                                         |
|                     | PD6     | JOY_LEFT   | 按下输入0                                                         |
|                     | PE3     | JOY_RIGHT  | 按下输入0                                                         |
|                     | PD13    | JOY_SEL    | 按下输入0                                                         |
| LED                 | PE11    | LED1       | 输出0点亮                                                         |
|                     | PE12    | LED2       | 输出0点亮                                                         |
| 串口1               | PA9     | UART1_TX   | 复用：DVP_D0                                                      |
|                     | PA10    | UART1_RX   | 复用：DVP_D1                                                      |
| 串口2               | PA2     | UART2_TX   | 通过跳线帽选择调试器串口连接UART1或者UART2                        |
|                     | PA3     | UART2_RX   | 通过跳线帽选择调试器串口连接UART1或者UART2                        |
| WiFi 接口           | PC0     | UART6_TX   | ESP8266_RX <兼容 ESP-01，ESP-01S WiFi 模块>                       |
|                     | PC1     | UART6_RX   | ESP8266_TX <使用时注意 WiFi 天线朝向板外>                         |
| 蓝牙 CH9141         | PC2     | UART7_TX   | CH9141_RX                                                         |
|                     | PC3     | UART7_RX   | CH9141_TX                                                         |
|                     | PA7     | BLE_AT     | BLE控制管脚 0为AT模式，1为透传模式                                |
|                     | PC13    | BLE_SLEEP  | 低电平有效，低功耗模式                                            |
| 液晶屏LCD           | PD14    | FSMC_D0    | 液晶  LCD 数据口D0                                                |
|                     | PD15    | FSMC_D1    | 液晶  LCD 数据口D1                                                |
|                     | PD0     | FSMC_D2    | 液晶  LCD 数据口D2                                                |
|                     | PD1     | FSMC_D3    | 液晶  LCD 数据口D3                                                |
|                     | PE7     | FSMC_D4    | 液晶  LCD 数据口D4                                                |
|                     | PE8     | FSMC_D5    | 液晶  LCD 数据口D5                                                |
|                     | PE9     | FSMC_D6    | 液晶  LCD 数据口D6                                                |
|                     | PE10    | FSMC_D7    | 液晶  LCD 数据口D7                                                |
|                     | PD4     | FSMC_NOE   | 液晶  LCD_RD                                                      |
|                     | PD5     | FSMC_NWE   | 液晶  LCD_WR                                                      |
|                     | PD7     | FSMC_NE1   | 液晶  LCD_CS                                                      |
|                     | PD12    | FSMC_A17   | 液晶  LCD_DC                                                      |
|                     | RST     | 复位       | 液晶  LCD_RESET                                                   |
|                     | PB14    | LCD_BL     | 液晶背光开关，高电平有效                                          |
|                     | PC4     | LCD_TE     | 液晶 Tearing Effect 输出（帧同步）                                |
| 摄像头DVP           | PA9     | DVP_D0     | 复用：UART1_TX                                                    |
|                     | PA10    | DVP_D1     | 复用：UART1_RX                                                    |
|                     | PC8     | DVP_D2     | 复用：TF卡  D0                                                    |
|                     | PC9     | DVP_D3     | 复用：TF卡  D1                                                    |
|                     | PC11    | DVP_D4     | 复用：TF卡  D3                                                    |
|                     | PB6     | DVP_D5     |                                                                   |
|                     | PB8     | DVP_D6     |                                                                   |
|                     | PB9     | DVP_D7     |                                                                   |
|                     | PC10    | DVP_D8     | 复用：TF卡  D2                                                    |
|                     | PC12    | DVP_D9     | 复用：TF卡  CLK                                                   |
|                     | PB7     | DVP_RESSET |                                                                   |
|                     | PA4     | DVP_HSYN   |                                                                   |
|                     | PA5     | DVP_VSYNC  |                                                                   |
|                     | PA6     | DVP_PCLK   |                                                                   |
|                     | PC7     | DVP_PWDN   |                                                                   |
|                     | PB10    | SCCB_SCL   | 复用： DVP MPU6050 ES8388 AHT10 AP3216C                           |
|                     | PB11    | SCCB_SDA   | 复用： DVP MPU6050 ES8388 AHT10 AP3216C                           |
| MP3 ES8388          | PB12    | I2S2_LRCK  |                                                                   |
|                     | PB13    | I2S2_SCLK  |                                                                   |
|                     | PB15    | I2S2_SD    |                                                                   |
|                     | PC6     | I2S2_MCLK  |                                                                   |
|                     | PA8     | AUDIO_CTL  | I2S数据方向控制；1 : ES8388 -> MCU，录音；0 : MCU -> ES8388，播放 |
|                     | PB10    | I2C2_SCL   | 复用： DVP MPU6050 ES8388 AHT10 AP3216C                           |
|                     | PB11    | I2C2_SDA   | 复用： DVP MPU6050 ES8388 AHT10 AP3216C                           |
| TF卡                | PC8     | SD_D0      | 复用：DVP                                                         |
|                     | PC9     | SD_D1      | 复用：DVP                                                         |
|                     | PC10    | SD_D2      | 复用：DVP                                                         |
|                     | PC11    | SD_D3      | 复用：DVP                                                         |
|                     | PC12    | SD_CLK     | 复用：DVP                                                         |
|                     | PD2     | SD_CMD     |                                                                   |
| FLASH               | PA15    | SPI3_CS    |                                                                   |
|                     | PB3     | SPI3_CLK   |                                                                   |
|                     | PB4     | SPI3_MISO  |                                                                   |
|                     | PB5     | SPI3_MOSI  |                                                                   |
| 陀螺仪MPU6050       | PB10    | I2C2_SCL   | 复用： DVP MPU6050 ES8388 AHT10 AP3216C                           |
|                     | PB11    | I2C2_SDA   | 复用： DVP MPU6050 ES8388 AHT10 AP3216C                           |
|                     | PC5     | MUP_INT    |                                                                   |
| 温湿度AHT10         | PB10    | I2C2_SCL   | 复用： DVP MPU6050 ES8388 AHT10 AP3216C                           |
|                     | PB11    | I2C2_SDA   | 复用： DVP MPU6050 ES8388 AHT10 AP3216C                           |
| 环境光传感器AP3216C | PB10    | I2C2_SCL   | 复用： DVP MPU6050 ES8388 AHT10 AP3216C                           |
|                     | PB11    | I2C2_SDA   | 复用： DVP MPU6050 ES8388 AHT10 AP3216C                           |
|                     | PE6     | AP_INT     |                                                                   |
| USB                 | PA11    | USB1_D-    |                                                                   |
|                     | PA12    | USB1_D+    |                                                                   |
| 调试器接口          | PA13    | SWDIO      | 调试器专用                                                        |
|                     | PA14    | SWCLK      | 调试器专用                                                        |
| 外部晶振            | PC14    | OSC32_IN   | 32.768KHz 专用                                                    |
|                     | PC15    | OSC32_OUT  | 32.768KHz 专用                                                    |
|                     | OSC_IN  |            | 外部晶振 8MHz                                                     |
|                     | OSC_OUT |            | 外部晶振 8MHz                                                     |
| BOOT                | BOOT0   |            | 默认为0 。 短接跳线焊盘后为1                                      |
|                     | PB2     | BOOT1      | 默认为0 。 短接跳线焊盘后为1                                      |
| 扩展口              | PB0     | ADC_IN8    | 可用作：TIM3_CH3 / TIM8_CH2N / OPA1_CH1P等                        |
|                     | PB1     | ADC_IN9    | 可用作：TIM3_CH4 / TIM8_CH3N / OPA4_CH0N 等                       |
|                     | PA1     | ADC_IN1    | 可用作：TIM5_CH2 / TIM2_CH2 / OPA3_OUT0 等                        |
|                     | PE13    | FSMC_D10   | 重映射功能：TIM1_CH3/UART7_RX                                     |
|                     | PE14    | FSMC_D11   | 重映射功能：TIM1_CH4/UART8_TX                                     |
|                     | PE15    | FSMC_D12   | 重映射功能：TIM1_BKIN/UART8_RX                                    |
|                     | PD3     | FSMC_CLK   | 重映射功能：USART2_CTS TIM10_CH2                                  |
|                     | PD8     | FSMC_D13   | 重映射功能：USART3_TX/TIM9_CH1N                                   |
|                     | PD9     | FSMC_D14   | 重映射功能：USART3_RX TIM9_CH1/TIM9_ETR                           |
|                     | PD10    | FSMC_D15   | 重映射功能：USART3_CK/TIM9_CH2N                                   |
|                     | PD11    | FSMC_A16   | 重映射功能：USART3_CTS/TIM9_CH2                                   |

## 开发资源

- [***openCH 赤菟开发板原理图***](./doc/SCH_openCH_CH32V307_Board.pdf)
- [***openCH 赤菟开发板尺寸及元件位号图***](./doc/Dimension_openCH_CH32V307_Board.pdf)
- [***CH32V307 介绍页面 (WCH)***](http://www.wch.cn/products/CH32V307.html)
- [***CH32V307 沁恒官方例程（用于赤菟开发板时需修改）***](http://www.wch.cn/downloads/CH32V307EVT_ZIP.html)  
- [***CH32V307 芯片手册 (WCH)***](http://www.wch.cn/downloads/CH32V20x_30xDS0_PDF.html)
- [***CH32V307 参考手册 (WCH)***](http://www.wch.cn/downloads/CH32FV2x_V3xRM_PDF.html)
