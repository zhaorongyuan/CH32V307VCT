

# CH32V30x 嵌入式分层软件架构 — 完整方案（定稿版）

## 一、架构总览

### 1.1 目录结构

Plaintext

```
app_project/
│
├── BSW/                                    # 基础软件层
│   ├── Startup/                            # 启动汇编
│   ├── Core/                               # 内核支持 + 公共数据结构
│   │   ├── ringbuffer.h                    # 环形缓冲（无层属性，所有层可用）
│   │   └── ringbuffer.c
│   │
│   ├── MCAL/                               # 微控制器抽象层（芯片官方库，只读不改）
│   │   ├── inc/
│   │   │   ├── ch32v30x.h
│   │   │   ├── ch32v30x_gpio.h
│   │   │   └── ...
│   │   └── src/
│   │       ├── ch32v30x_gpio.c
│   │       └── ...
│   │
│   ├── Services/                           # 片内复杂驱动（非厂商标准库）
│   │   ├── drv_flash_eeprom.h
│   │   └── drv_flash_eeprom.c
│   │
│   ├── Board/                              # 板级：配置 + 初始化 + ISR + FIFO
│   │   ├── board_cfg.h                     # 唯一硬件配置窗口
│   │   ├── board_init.h
│   │   ├── board_init.c
│   │   ├── board_uart_fifo.h               # 通用串口 FIFO 引擎
│   │   ├── board_uart_fifo.c
│   │   └── ch32v30x_it.c                   # 中断服务（只推 FIFO，不碰 Device）
│   │
│   └── Device/                             # 板载外部器件驱动
│       └── esp8266/
│           ├── esp8266.h
│           └── esp8266.c
│
├── HAL/                                    # 硬件抽象接口层（APP 的唯一依赖）
│   ├── hal_common.h                        # 公共类型（HalStatus 等）
│   ├── hal_gpio.h & .c
│   ├── hal_uart.h & .c
│   └── hal_network.h & .c
│
├── Middleware/                             # 中间件（纯软件，当前预留 RTOS/协议栈）
│   └── .gitkeep
│
├── APP/                                    # 应用业务逻辑
│   ├── task_control.h & .c                 # 本地控制逻辑（按键、LED）
│   └── task_ota.h & .c                     # 远程升级逻辑
│
└── User/                                   # 顶层入口
    ├── main.c                              # 主循环与时间片轮询
    └── app_cfg.h                           # 业务参数配置
```

### 1.2 依赖关系图（三条硬规则设计）

Plaintext

```
                         ┌─────────────────┐
                         │     APP 层       │
                         │  task_control   │
                         │    task_ota     │
                         └────────┬─────────┘
                                  │ 只调用 API_*()
                                  ▼
                         ┌─────────────────┐
                         │     HAL 层       │
                         │   hal_gpio ──────────→ MCAL (GPIO)
                         │   hal_uart ──────────→ MCAL (USART)
                         │   hal_network ───┐
                         └──────────────────┤
                                            │
              ┌─────────────────────────────┤
              ▼                             ▼
     ┌─────────────┐              ┌─────────────────┐
     │    MCAL     │              │   Device 层      │
     │ (芯片官方库)  │              │   esp8266        │
     └──────┬──────┘              └────────┬─────────┘
            │                              │
            │         ┌────────────────────┘
            │         │ (从 FIFO 读取接收数据)
            │         ▼
            │  ┌─────────────────┐         ┌──────────────────┐
            │  │    Board 层      │         │   Services 层     │
            │  │ board_uart_fifo  │         │ drv_flash_eeprom │
            │  │       ▲          │         │   (片内复杂驱动)  │
            │  │       │          │         └────────┬─────────┘
            │  │ ch32v30x_it.c    │                  │
            │  │  (ISR 写 FIFO)   │                  │
            │  └─────────────────┘                  │
            │                                       │
            └───────────────────────────────────────┘
                               │
                       ┌───────┴───────┐
                       │ BSW/Core      │
                       │ ringbuffer    │ ← 纯数据结构，无层属性
                       └───────────────┘
```

**三条硬规则：**

1. **零向上依赖：** BSW 不引用任何 HAL 头文件。
2. **零直接水平耦合：** ISR 和 Device 之间只有 FIFO 数据接力。
3. **按需设层：** 简单外设一跳到 MCAL，复杂驱动走 Services，外部器件走 Device。

### 1.3 配置窗口

| **文件**      | **归属**  | **职责**                                                     |
| ------------- | --------- | ------------------------------------------------------------ |
| `board_cfg.h` | BSW/Board | 硬件级：引脚映射、时钟、UART 通道、FIFO 大小。换板子只改此文件。 |
| `app_cfg.h`   | User      | 应用级：版本号、WiFi SSID、服务器地址、周期参数。改业务只改此文件。 |

## 二、配置与基础核心 (Config & Core)

### 2.1 `User/app_cfg.h` (业务配置)

C

```
#ifndef APP_CFG_H
#define APP_CFG_H

/* 固件版本 */
#define APP_FW_VERSION          "1.0.0"

/* WiFi 配置 */
#define APP_WIFI_SSID           "MySSID"
#define APP_WIFI_PASSWORD       "MyPassword"

/* 服务器配置 */
#define APP_SERVER_IP           "192.168.1.100"
#define APP_SERVER_PORT         8080

/* 任务周期 (ms) */
#define APP_TASK_CTRL_PERIOD    100
#define APP_TASK_OTA_PERIOD     1000

#endif /* APP_CFG_H */
```

### 2.2 `BSW/Board/board_cfg.h` (硬件映射)

C

```
#ifndef BOARD_CFG_H
#define BOARD_CFG_H

#include "ch32v30x.h"

/* ======== LED ======== */
#define LED_STATUS_PORT         GPIOA
#define LED_STATUS_PIN          GPIO_Pin_0
#define LED_STATUS_CLK          RCC_APB2Periph_GPIOA

#define LED_NET_PORT            GPIOA
#define LED_NET_PIN             GPIO_Pin_1
#define LED_NET_CLK             RCC_APB2Periph_GPIOA

/* ======== 按键 ======== */
#define BTN_KEY1_PORT           GPIOB
#define BTN_KEY1_PIN            GPIO_Pin_0
#define BTN_KEY1_CLK            RCC_APB2Periph_GPIOB
#define BTN_KEY1_ACTIVE_LOW     1

/* ======== 蜂鸣器 ======== */
#define BUZZER_PORT             GPIOB
#define BUZZER_PIN              GPIO_Pin_5
#define BUZZER_CLK              RCC_APB2Periph_GPIOB
#define BUZZER_ACTIVE_HIGH      1

/* ======== 调试串口 (USART1) ======== */
#define DBG_UART                USART1
#define DBG_UART_CLK            RCC_APB2Periph_USART1
#define DBG_UART_IS_APB2        1
#define DBG_UART_TX_PORT        GPIOA
#define DBG_UART_TX_PIN         GPIO_Pin_9
#define DBG_UART_RX_PORT        GPIOA
#define DBG_UART_RX_PIN         GPIO_Pin_10
#define DBG_UART_BAUD           115200

/* ======== ESP8266 串口 (USART3) ======== */
#define ESP_UART                USART3
#define ESP_UART_CLK            RCC_APB1Periph_USART3
#define ESP_UART_IS_APB2        0
#define ESP_UART_TX_PORT        GPIOB
#define ESP_UART_TX_PIN         GPIO_Pin_10
#define ESP_UART_RX_PORT        GPIOB
#define ESP_UART_RX_PIN         GPIO_Pin_11
#define ESP_UART_BAUD           115200

/* ======== 串口 FIFO 大小（必须为 2 的幂） ======== */
#define BOARD_UART1_RX_FIFO_SIZE    256
#define BOARD_UART3_RX_FIFO_SIZE    1024

/* ======== 系统时钟 ======== */
#define SYS_CLK_HZ                  144000000UL

#endif /* BOARD_CFG_H */
```

### 2.3 `BSW/Core/ringbuffer.h`

*(注：保持原版实现，为单生产者单消费者的无锁环形缓冲区，在此省略具体源码，沿用原版即可。)*

## 三、硬件抽象层与设备驱动 (HAL & Device)

### 3.1 修复与补全：`HAL/hal_network.c`

原版在此文件结尾处截断。我们将其补全，让 APP 彻底摆脱对 `esp8266.h` 的依赖。

C

```
/**
 * @file    hal_network.c
 * @brief   网络抽象实现 — 内部绑定 BSW/Device/esp8266
 * @note    换 4G/以太网时只改这一个文件，APP 逻辑零修改
 */

#include "hal_network.h"
#include "esp8266.h"            /* 调用 Device 层 */
#include "board_init.h"
#include "board_cfg.h"

static NetState s_state = NET_IDLE;

HalStatus API_Net_Init(void)
{
    Board_InitEspUart(ESP_UART_BAUD);

    if (ESP8266_Init() != HAL_OK) {
        s_state = NET_ERROR;
        return HAL_ERROR;
    }
    s_state = NET_READY;
    return HAL_OK;
}

HalStatus API_Net_Connect(const char *ssid, const char *password, uint32_t timeout_ms)
{
    if (s_state == NET_ERROR) return HAL_ERROR;
    
    if (ESP8266_ConnectAP(ssid, password, timeout_ms) == HAL_OK) {
        return HAL_OK;
    }
    return HAL_ERROR;
}

HalStatus API_Net_ConnectTCP(const char *ip, uint16_t port, uint32_t timeout_ms)
{
    if (ESP8266_TCPConnect(ip, port, timeout_ms) == HAL_OK) {
        s_state = NET_TCP_OPEN;
        return HAL_OK;
    }
    return HAL_ERROR;
}

HalStatus API_Net_Send(const uint8_t *data, uint16_t len)
{
    if (s_state != NET_TCP_OPEN) return HAL_ERROR;
    return ESP8266_TCPSend(data, len, 3000);
}

HalStatus API_Net_RxAvailable(uint8_t *available)
{
    if (available == NULL) return HAL_INVALID_PARAM;
    *available = ESP8266_RxAvailable();
    return HAL_OK;
}

HalStatus API_Net_Read(uint8_t *buf, uint16_t max_len, uint16_t *actual_len)
{
    if (buf == NULL || actual_len == NULL) return HAL_INVALID_PARAM;
    *actual_len = ESP8266_Read(buf, max_len);
    return HAL_OK;
}

HalStatus API_Net_GetState(NetState *state)
{
    if (state == NULL) return HAL_INVALID_PARAM;
    *state = s_state;
    return HAL_OK;
}
```

*(注：`hal_gpio.c`, `hal_uart.c`, `board_init.c`, `ch32v30x_it.c`, `esp8266.c`, `drv_flash_eeprom.c` 完全沿用你原版设计的优秀实现。)*

## 四、应用层 (APP)

通过 `APP` 层，我们来验证整个架构的解耦能力。你会发现在这里，**完全看不到任何寄存器、GPIO Port 或是特定芯片的宏**。

### 4.1 `APP/task_control.h`

C

```
#ifndef TASK_CONTROL_H
#define TASK_CONTROL_H

void Task_Control_Init(void);
void Task_Control_Update(void);

#endif /* TASK_CONTROL_H */
```

### 4.2 `APP/task_control.c`

C

```
/**
 * @file    task_control.c
 * @brief   本地控制任务：负责按键读取、状态指示、蜂鸣器反馈
 * @note    纯业务逻辑，只依赖 HAL 层
 */

#include "task_control.h"
#include "hal_gpio.h"
#include "hal_uart.h"

static ButtonState s_lastBtnState = BTN_RELEASED;

void Task_Control_Init(void)
{
    /* 硬件初始化已在 main.c 中由 HAL 层统一完成，这里可做业务变量初始化 */
    s_lastBtnState = BTN_RELEASED;
    API_Uart_Printf(UART_ID_DEBUG, "[Task Ctrl] Initialized.\r\n");
}

void Task_Control_Update(void)
{
    ButtonState currentState;
    
    /* 1. 读取按键 */
    if (API_Button_GetState(BTN_KEY1, &currentState) == HAL_OK) {
        /* 检测到按键按下（下降沿触发业务） */
        if (currentState == BTN_PRESSED && s_lastBtnState == BTN_RELEASED) {
            API_Uart_Printf(UART_ID_DEBUG, "[Task Ctrl] Key Pressed!\r\n");
            
            /* 业务执行：反转状态 LED 并发出 100ms 短鸣 */
            API_Led_Toggle(LED_STATUS);
            API_Buzzer_Beep(100);
        }
        s_lastBtnState = currentState;
    }
}
```

### 4.3 `APP/task_ota.h`

C

```
#ifndef TASK_OTA_H
#define TASK_OTA_H

void Task_OTA_Init(void);
void Task_OTA_Update(void);

#endif /* TASK_OTA_H */
```

### 4.4 `APP/task_ota.c` (网络与远程业务状态机)

C

```
/**
 * @file    task_ota.c
 * @brief   网络与远程任务：负责连接 WiFi、连接服务器及维持心跳
 * @note    采用非阻塞状态机设计，避免死等网络超时阻塞主循环
 */

#include "task_ota.h"
#include "hal_network.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "app_cfg.h"

typedef enum {
    OTA_STATE_INIT,
    OTA_STATE_CONNECT_WIFI,
    OTA_STATE_CONNECT_SERVER,
    OTA_STATE_IDLE
} OtaState;

static OtaState s_otaState = OTA_STATE_INIT;

void Task_OTA_Init(void)
{
    s_otaState = OTA_STATE_INIT;
}

void Task_OTA_Update(void)
{
    switch (s_otaState) {
        case OTA_STATE_INIT:
            API_Uart_Printf(UART_ID_DEBUG, "[Task OTA] Init Network...\r\n");
            if (API_Net_Init() == HAL_OK) {
                s_otaState = OTA_STATE_CONNECT_WIFI;
            } else {
                API_Buzzer_Beep(500); /* 错误警报 */
            }
            break;

        case OTA_STATE_CONNECT_WIFI:
            API_Uart_Printf(UART_ID_DEBUG, "[Task OTA] Connecting to %s...\r\n", APP_WIFI_SSID);
            if (API_Net_Connect(APP_WIFI_SSID, APP_WIFI_PASSWORD, 5000) == HAL_OK) {
                API_Led_SetStatus(LED_NET, LED_ON);
                s_otaState = OTA_STATE_CONNECT_SERVER;
            }
            break;

        case OTA_STATE_CONNECT_SERVER:
            API_Uart_Printf(UART_ID_DEBUG, "[Task OTA] Connecting TCP Server...\r\n");
            if (API_Net_ConnectTCP(APP_SERVER_IP, APP_SERVER_PORT, 3000) == HAL_OK) {
                API_Uart_Printf(UART_ID_DEBUG, "[Task OTA] Server Connected.\r\n");
                s_otaState = OTA_STATE_IDLE;
            } else {
                /* 连接失败，退回上一步或重试 */
                s_otaState = OTA_STATE_CONNECT_WIFI; 
            }
            break;

        case OTA_STATE_IDLE:
            /* 在此处理接收服务器数据或发送心跳包 */
            {
                uint8_t hasData = 0;
                API_Net_RxAvailable(&hasData);
                if (hasData) {
                    uint8_t rxBuf[64];
                    uint16_t len;
                    API_Net_Read(rxBuf, sizeof(rxBuf)-1, &len);
                    rxBuf[len] = '\0';
                    API_Uart_Printf(UART_ID_DEBUG, "[Task OTA] Rx: %s\r\n", rxBuf);
                }
            }
            break;
            
        default:
            s_otaState = OTA_STATE_INIT;
            break;
    }
}
```

## 五、顶层入口 (User)

### 5.1 `User/main.c`

我们采用轻量级的 **前后台（SysTick + 轮询）** 架构。所有 1ms 级别的紧急任务（如蜂鸣器倒计时更新）放在主循环最外侧，而业务模块通过“时间戳相减”实现时间片调度。

C

```
/**
 * @file    main.c
 * @brief   主入口与轮询调度器
 * @note    严禁在此文件中出现寄存器操作，全权交由 HAL 接管
 */

#include "ch32v30x.h"
#include "board_uart_fifo.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "app_cfg.h"

#include "task_control.h"
#include "task_ota.h"

/* 全局系统心跳滴答，由 SysTick_Handler 累加 (见 ch32v30x_it.c) */
volatile uint32_t sys_tick_ms = 0;

static void System_Core_Init(void)
{
    /* 配置中断优先级分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    /* 更新 SystemCoreClock 变量 */
    SystemCoreClockUpdate();
    /* 配置 1ms 的 SysTick 定时器 */
    SysTick_Config(SystemCoreClock / 1000);
}

int main(void)
{
    /* 1. 核心与基础内存组件初始化 */
    System_Core_Init();
    Board_UartFifo_Init();  /* 必须在 UART 之前初始化 */

    /* 2. 硬件外设初始化 (通过 HAL 接口) */
    API_Uart_Init(UART_ID_DEBUG, DBG_UART_BAUD);
    API_Led_Init();
    API_Button_Init();
    API_Buzzer_Init();

    API_Uart_Printf(UART_ID_DEBUG, "\r\n=============================\r\n");
    API_Uart_Printf(UART_ID_DEBUG, "System Booting... FW: %s\r\n", APP_FW_VERSION);
    API_Uart_Printf(UART_ID_DEBUG, "SYS CLK: %u Hz\r\n", SystemCoreClock);
    API_Uart_Printf(UART_ID_DEBUG, "=============================\r\n");

    /* 3. 业务任务初始化 */
    Task_Control_Init();
    Task_OTA_Init();

    /* 调度器时间戳 */
    uint32_t last_ctrl_ms = 0;
    uint32_t last_ota_ms = 0;

    /* 4. 主循环调度 (Super Loop) */
    while (1)
    {
        /* 高频紧急更新 (1ms 响应级别) */
        API_Buzzer_Update();

        /* Task Control: 100ms 周期执行 */
        if ((sys_tick_ms - last_ctrl_ms) >= APP_TASK_CTRL_PERIOD) {
            last_ctrl_ms = sys_tick_ms;
            Task_Control_Update();
        }

        /* Task OTA: 1000ms 周期执行 */
        if ((sys_tick_ms - last_ota_ms) >= APP_TASK_OTA_PERIOD) {
            last_ota_ms = sys_tick_ms;
            Task_OTA_Update();
        }
    }
}
```
