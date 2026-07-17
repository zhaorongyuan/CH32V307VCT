# 基于 openCH 赤菟 开发板 v1.0.3 进行 bootloader 和 app 开发

## 简介

    本项目基于 openCH 赤菟 开发板 v1.0.3 进行 bootloader 和 app 开发 

## 规划
# CH32V30x 完整 HAL 接口层方案

以下是基于前面讨论的完整落地方案，所有代码可直接用于 CH32V30x 项目。

---

## 一、文件结构

```
app_project/
│
├── BSW/                              # 基础软件层
│   ├── Startup/                      # 启动文件
│   ├── Core/                         # 内核相关
│   ├── Peripheral/                   # 芯片官方外设库
│   │   ├── inc/
│   │   │   └── ch32v30x_gpio.h
│   │   │   └── ch32v30x_usart.h
│   │   └── src/
│   │       └── ch32v30x_gpio.c
│   │       └── ch32v30x_usart.c
│   │
│   ├── Board/                        # 板级配置
│   │   ├── board_cfg.h               # 引脚映射、时钟配置
│   │   └── board_cfg.c
│   │
│   └── Driver/                       # 物理驱动
│       ├── drv_gpio.h
│       ├── drv_gpio.c
│       ├── drv_uart.h
│       ├── drv_uart.c
│       ├── drv_esp8266.h
│       └── drv_esp8266.c
│
├── HAL/                              # 接口层（ASW 与 BSW 的分界线）
│   ├── hal_common.h                  # 公共类型定义
│   ├── hal_gpio.h
│   ├── hal_gpio.c
│   ├── hal_uart.h
│   ├── hal_uart.c
│   ├── hal_wifi.h
│   └── hal_wifi.c
│
├── Task/                             # 应用软件层（只调用 HAL）
│   ├── task_control.h
│   ├── task_control.c
│   ├── task_ota.h
│   └── task_ota.c
│
└── User/                             # 用户入口
    ├── main.c
    └── ch32v30x_it.c
```

---

## 二、HAL 公共类型

### `HAL/hal_common.h`

```c
/**
 * @file    hal_common.h
 * @brief   HAL 层公共类型定义
 * @note    所有 HAL 模块共用的返回值类型和基础枚举
 */

#ifndef HAL_COMMON_H
#define HAL_COMMON_H

#include <stdint.h>

/*============================================================================
 *  通用返回状态 — 所有 HAL API 统一使用
 *============================================================================*/
typedef enum {
    HAL_OK             = 0x00U,   /* 操作成功 */
    HAL_ERROR          = 0x01U,   /* 通用错误 */
    HAL_BUSY           = 0x02U,   /* 资源忙 */
    HAL_TIMEOUT        = 0x03U,   /* 超时 */
    HAL_INVALID_PARAM  = 0x04U,   /* 参数非法 */
    HAL_NOT_INIT       = 0x05U    /* 模块未初始化 */
} HalStatus;

/*============================================================================
 *  通用布尔状态
 *============================================================================*/
typedef enum {
    STATE_FALSE = 0U,
    STATE_TRUE  = 1U
} BoolState;

/*============================================================================
 *  GPIO 电平（Driver 层使用，HAL 层不直接暴露）
 *============================================================================*/
typedef enum {
    PIN_LOW  = 0U,
    PIN_HIGH = 1U
} PinLevel;

#endif /* HAL_COMMON_H */
```

---

## 三、板级配置

### `BSW/Board/board_cfg.h`

```c
/**
 * @file    board_cfg.h
 * @brief   板级硬件配置 — 引脚映射、时钟参数
 * @note    换板子时只改这一个文件
 */

#ifndef BOARD_CFG_H
#define BOARD_CFG_H

#include "ch32v30x.h"

/*============================================================================
 *  LED 引脚定义
 *============================================================================*/
#define LED_STATUS_GPIO_PORT    GPIOA
#define LED_STATUS_GPIO_PIN     GPIO_Pin_0
#define LED_STATUS_GPIO_CLK     RCC_APB2Periph_GPIOA

#define LED_NET_GPIO_PORT       GPIOA
#define LED_NET_GPIO_PIN        GPIO_Pin_1
#define LED_NET_GPIO_CLK        RCC_APB2Periph_GPIOA

/*============================================================================
 *  按键引脚定义
 *============================================================================*/
#define BTN_KEY1_GPIO_PORT      GPIOB
#define BTN_KEY1_GPIO_PIN       GPIO_Pin_0
#define BTN_KEY1_GPIO_CLK       RCC_APB2Periph_GPIOB
#define BTN_KEY1_ACTIVE_LOW     1      /* 1 = 低电平有效 */

/*============================================================================
 *  蜂鸣器引脚定义
 *============================================================================*/
#define BUZZER_GPIO_PORT        GPIOB
#define BUZZER_GPIO_PIN         GPIO_Pin_5
#define BUZZER_GPIO_CLK         RCC_APB2Periph_GPIOB
#define BUZZER_ACTIVE_HIGH      1      /* 1 = 高电平响 */

/*============================================================================
 *  UART 端口定义
 *============================================================================*/
#define UART_DEBUG              USART1   /* 调试串口 */
#define UART_DEBUG_CLK          RCC_APB2Periph_USART1
#define UART_DEBUG_TX_PIN       GPIO_Pin_9
#define UART_DEBUG_TX_PORT      GPIOA
#define UART_DEBUG_RX_PIN       GPIO_Pin_10
#define UART_DEBUG_RX_PORT      GPIOA

#define UART_ESP                USART3   /* ESP8266 串口 */
#define UART_ESP_CLK            RCC_APB1Periph_USART3
#define UART_ESP_TX_PIN         GPIO_Pin_10
#define UART_ESP_TX_PORT        GPIOB
#define UART_ESP_RX_PIN         GPIO_Pin_11
#define UART_ESP_RX_PORT        GPIOB

/*============================================================================
 *  系统时钟
 *============================================================================*/
#define SYSTEM_CORE_CLOCK_HZ    144000000UL

#endif /* BOARD_CFG_H */
```

---

## 四、驱动层（Driver）

### 4.1 GPIO 驱动

#### `BSW/Driver/drv_gpio.h`

```c
/**
 * @file    drv_gpio.h
 * @brief   GPIO 物理驱动 — 封装寄存器操作，面向引脚
 */

#ifndef DRV_GPIO_H
#define DRV_GPIO_H

#include <stdint.h>
#include "hal_common.h"

/* 初始化某个 GPIO 引脚为推挽输出 */
void Drv_GPIO_InitOutput(GPIO_TypeDef *port, uint16_t pin, uint32_t clock);

/* 初始化某个 GPIO 引脚为浮空输入 */
void Drv_GPIO_InitInput(GPIO_TypeDef *port, uint16_t pin, uint32_t clock);

/* 设置引脚电平 */
void Drv_GPIO_SetPin(GPIO_TypeDef *port, uint16_t pin, PinLevel level);

/* 翻转引脚电平 */
void Drv_GPIO_TogglePin(GPIO_TypeDef *port, uint16_t pin);

/* 读取引脚电平 */
PinLevel Drv_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin);

#endif /* DRV_GPIO_H */
```

#### `BSW/Driver/drv_gpio.c`

```c
/**
 * @file    drv_gpio.c
 * @brief   GPIO 物理驱动实现
 */

#include "drv_gpio.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_rcc.h"

void Drv_GPIO_InitOutput(GPIO_TypeDef *port, uint16_t pin, uint32_t clock)
{
    GPIO_InitTypeDef gpio_init;

    RCC_APB2PeriphClockCmd(clock, ENABLE);

    gpio_init.GPIO_Pin   = pin;
    gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(port, &gpio_init);
}

void Drv_GPIO_InitInput(GPIO_TypeDef *port, uint16_t pin, uint32_t clock)
{
    GPIO_InitTypeDef gpio_init;

    RCC_APB2PeriphClockCmd(clock, ENABLE);

    gpio_init.GPIO_Pin  = pin;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(port, &gpio_init);
}

void Drv_GPIO_SetPin(GPIO_TypeDef *port, uint16_t pin, PinLevel level)
{
    GPIO_WriteBit(port, pin, (level == PIN_HIGH) ? Bit_SET : Bit_RESET);
}

void Drv_GPIO_TogglePin(GPIO_TypeDef *port, uint16_t pin)
{
    if (GPIO_ReadOutputDataBit(port, pin) == Bit_SET) {
        GPIO_WriteBit(port, pin, Bit_RESET);
    } else {
        GPIO_WriteBit(port, pin, Bit_SET);
    }
}

PinLevel Drv_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin)
{
    return (GPIO_ReadInputDataBit(port, pin) == Bit_SET) ? PIN_HIGH : PIN_LOW;
}
```

---

### 4.2 UART 驱动

#### `BSW/Driver/drv_uart.h`

```c
/**
 * @file    drv_uart.h
 * @brief   UART 物理驱动 — 封装 USART 外设操作
 */

#ifndef DRV_UART_H
#define DRV_UART_H

#include <stdint.h>
#include "hal_common.h"
#include "ch32v30x.h"

/* 初始化 USART */
void Drv_UART_Init(USART_TypeDef *uart, uint32_t baudrate,
                   GPIO_TypeDef *tx_port, uint16_t tx_pin,
                   GPIO_TypeDef *rx_port, uint16_t rx_pin,
                   uint32_t periph_clock, uint8_t is_apb2);

/* 发送单字节 */
void Drv_UART_SendByte(USART_TypeDef *uart, uint8_t byte);

/* 发送数据块 */
HalStatus Drv_UART_SendData(USART_TypeDef *uart,
                            const uint8_t *data, uint16_t len,
                            uint32_t timeout_ms);

/* 检查接收缓冲区是否有数据 */
uint8_t Drv_UART_RxReady(USART_TypeDef *uart);

/* 读取单字节（调用前须确认有数据） */
uint8_t Drv_UART_ReadByte(USART_TypeDef *uart);

#endif /* DRV_UART_H */
```

#### `BSW/Driver/drv_uart.c`

```c
/**
 * @file    drv_uart.c
 * @brief   UART 物理驱动实现
 */

#include "drv_uart.h"
#include "ch32v30x_usart.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_rcc.h"

/* 简易毫秒计数（假设 SysTick 已配置） */
extern volatile uint32_t sys_tick_ms;

static uint32_t Drv_GetTick(void)
{
    return sys_tick_ms;
}

void Drv_UART_Init(USART_TypeDef *uart, uint32_t baudrate,
                   GPIO_TypeDef *tx_port, uint16_t tx_pin,
                   GPIO_TypeDef *rx_port, uint16_t rx_pin,
                   uint32_t periph_clock, uint8_t is_apb2)
{
    GPIO_InitTypeDef  gpio_init;
    USART_InitTypeDef uart_init;

    /* 使能时钟 */
    if (is_apb2) {
        RCC_APB2PeriphClockCmd(periph_clock, ENABLE);
        /* USART1 挂在 APB2, TX/RX GPIO 时钟也要开 */
    } else {
        RCC_APB1PeriphClockCmd(periph_clock, ENABLE);
    }

    /* TX 引脚 */
    gpio_init.GPIO_Pin   = tx_pin;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(tx_port, &gpio_init);

    /* RX 引脚 */
    gpio_init.GPIO_Pin  = rx_pin;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(rx_port, &gpio_init);

    /* USART 参数 */
    uart_init.USART_BaudRate            = baudrate;
    uart_init.USART_WordLength          = USART_WordLength_8b;
    uart_init.USART_StopBits            = USART_StopBits_1;
    uart_init.USART_Parity              = USART_Parity_No;
    uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart_init.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(uart, &uart_init);

    USART_Cmd(uart, ENABLE);
}

void Drv_UART_SendByte(USART_TypeDef *uart, uint8_t byte)
{
    while (USART_GetFlagStatus(uart, USART_FLAG_TXE) == RESET)
        ;
    USART_SendData(uart, byte);
}

HalStatus Drv_UART_SendData(USART_TypeDef *uart,
                            const uint8_t *data, uint16_t len,
                            uint32_t timeout_ms)
{
    uint32_t start = Drv_GetTick();

    for (uint16_t i = 0; i < len; i++) {
        while (USART_GetFlagStatus(uart, USART_FLAG_TXE) == RESET) {
            if ((Drv_GetTick() - start) >= timeout_ms) {
                return HAL_TIMEOUT;
            }
        }
        USART_SendData(uart, data[i]);
    }

    /* 等待最后一个字节发完 */
    while (USART_GetFlagStatus(uart, USART_FLAG_TC) == RESET) {
        if ((Drv_GetTick() - start) >= timeout_ms) {
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

uint8_t Drv_UART_RxReady(USART_TypeDef *uart)
{
    return (USART_GetFlagStatus(uart, USART_FLAG_RXNE) != RESET) ? 1U : 0U;
}

uint8_t Drv_UART_ReadByte(USART_TypeDef *uart)
{
    return (uint8_t)USART_ReceiveData(uart);
}
```

---

### 4.3 ESP8266 驱动（AT 指令）

#### `BSW/Driver/drv_esp8266.h`

```c
/**
 * @file    drv_esp8266.h
 * @brief   ESP8266 AT 指令驱动
 */

#ifndef DRV_ESP8266_H
#define DRV_ESP8266_H

#include <stdint.h>
#include "hal_common.h"

/* 初始化 ESP8266（发送 AT 测试 + 设置 Station 模式） */
HalStatus Drv_ESP8266_Init(USART_TypeDef *uart);

/* 连接 WiFi 热点 */
HalStatus Drv_ESP8266_ConnectAP(const char *ssid, const char *password,
                                uint32_t timeout_ms);

/* 建立 TCP 连接 */
HalStatus Drv_ESP8266_TCPConnect(const char *ip, uint16_t port,
                                 uint32_t timeout_ms);

/* 通过已建立的 TCP 连接发送数据 */
HalStatus Drv_ESP8266_TCPSend(const uint8_t *data, uint16_t len,
                              uint32_t timeout_ms);

/* 检查是否有接收到的数据 */
uint8_t Drv_ESP8266_RxAvailable(void);

/* 读取接收缓冲区（返回读取长度） */
uint16_t Drv_ESP8266_Read(uint8_t *buf, uint16_t max_len);

#endif /* DRV_ESP8266_H */
```

#### `BSW/Driver/drv_esp8266.c`

```c
/**
 * @file    drv_esp8266.c
 * @brief   ESP8266 AT 指令驱动实现
 */

#include "drv_esp8266.h"
#include "drv_uart.h"
#include "board_cfg.h"
#include <string.h>

static USART_TypeDef *s_esp_uart = NULL;

/* 简易环形接收缓冲 */
#define ESP_RX_BUF_SIZE  512
static uint8_t  s_rx_buf[ESP_RX_BUF_SIZE];
static uint16_t s_rx_head = 0;
static uint16_t s_rx_tail = 0;

static void ESP_FlushRxBuf(void)
{
    s_rx_head = 0;
    s_rx_tail = 0;
}

static void ESP_PushRxByte(uint8_t byte)
{
    uint16_t next = (s_rx_head + 1) % ESP_RX_BUF_SIZE;
    if (next != s_rx_tail) {
        s_rx_buf[s_rx_head] = byte;
        s_rx_head = next;
    }
}

/* 从缓冲区拉取（供中断或轮询调用） */
void Drv_ESP8266_PollRx(void)
{
    if (s_esp_uart == NULL) return;
    while (Drv_UART_RxReady(s_esp_uart)) {
        ESP_PushRxByte(Drv_UART_ReadByte(s_esp_uart));
    }
}

static void ESP_SendCmd(const char *cmd)
{
    Drv_UART_SendData(s_esp_uart, (const uint8_t *)cmd,
                      (uint16_t)strlen(cmd), 1000);
}

/* 等待特定响应字符串，返回 HAL_OK 或 HAL_TIMEOUT */
static HalStatus ESP_WaitFor(const char *expected, uint32_t timeout_ms)
{
    extern volatile uint32_t sys_tick_ms;
    uint32_t start = sys_tick_ms;
    char     tmp[128];
    uint16_t idx = 0;

    memset(tmp, 0, sizeof(tmp));

    while ((sys_tick_ms - start) < timeout_ms) {
        Drv_ESP8266_PollRx();
        while (s_rx_tail != s_rx_head) {
            tmp[idx++] = s_rx_buf[s_rx_tail];
            s_rx_tail = (s_rx_tail + 1) % ESP_RX_BUF_SIZE;
            if (idx >= sizeof(tmp) - 1) idx = sizeof(tmp) - 2;
            tmp[idx] = '\0';
            if (strstr(tmp, expected) != NULL) {
                return HAL_OK;
            }
        }
    }
    return HAL_TIMEOUT;
}

HalStatus Drv_ESP8266_Init(USART_TypeDef *uart)
{
    s_esp_uart = uart;
    ESP_FlushRxBuf();

    /* 发送 AT 测试 */
    ESP_SendCmd("AT\r\n");
    if (ESP_WaitFor("OK", 2000) != HAL_OK) {
        return HAL_ERROR;
    }

    /* 设置 Station 模式 */
    ESP_SendCmd("AT+CWMODE=1\r\n");
    if (ESP_WaitFor("OK", 2000) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HalStatus Drv_ESP8266_ConnectAP(const char *ssid, const char *password,
                                uint32_t timeout_ms)
{
    char cmd[128];

    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);
    ESP_SendCmd(cmd);

    return ESP_WaitFor("OK", timeout_ms);
}

HalStatus Drv_ESP8266_TCPConnect(const char *ip, uint16_t port,
                                 uint32_t timeout_ms)
{
    char cmd[128];

    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%u\r\n", ip, port);
    ESP_SendCmd(cmd);

    return ESP_WaitFor("OK", timeout_ms);
}

HalStatus Drv_ESP8266_TCPSend(const uint8_t *data, uint16_t len,
                              uint32_t timeout_ms)
{
    char cmd[32];

    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u\r\n", len);
    ESP_SendCmd(cmd);
    if (ESP_WaitFor(">", 3000) != HAL_OK) {
        return HAL_ERROR;
    }

    return Drv_UART_SendData(s_esp_uart, data, len, timeout_ms);
}

uint8_t Drv_ESP8266_RxAvailable(void)
{
    Drv_ESP8266_PollRx();
    return (s_rx_head != s_rx_tail) ? 1U : 0U;
}

uint16_t Drv_ESP8266_Read(uint8_t *buf, uint16_t max_len)
{
    uint16_t count = 0;
    Drv_ESP8266_PollRx();
    while (s_rx_tail != s_rx_head && count < max_len) {
        buf[count++] = s_rx_buf[s_rx_tail];
        s_rx_tail = (s_rx_tail + 1) % ESP_RX_BUF_SIZE;
    }
    return count;
}
```

---

## 五、HAL 接口层

### 5.1 GPIO HAL

#### `HAL/hal_gpio.h`

```c
/**
 * @file    hal_gpio.h
 * @brief   GPIO 接口层 — 面向业务语义的 LED、按键、蜂鸣器抽象
 * @note    ASW 只 include 这个头文件，不接触任何芯片库
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "hal_common.h"

/*============================================================================
 *  LED
 *============================================================================*/
typedef enum {
    LED_STATUS = 0,       /* 系统状态灯 */
    LED_NET,              /* 网络状态灯 */
    LED_COUNT             /* 哨兵值 — 用于数组/边界检查 */
} LedId;

typedef enum {
    LED_OFF = 0,
    LED_ON
} LedState;

/* 初始化所有 LED 引脚 */
HalStatus API_Led_Init(void);

/* 设置 LED 状态 */
HalStatus API_Led_SetStatus(LedId id, LedState state);

/* 翻转 LED */
HalStatus API_Led_Toggle(LedId id);

/* 获取 LED 当前状态 */
LedState  API_Led_GetStatus(LedId id);

/*============================================================================
 *  按键
 *============================================================================*/
typedef enum {
    BTN_KEY1 = 0,
    BTN_COUNT
} ButtonId;

typedef enum {
    BTN_RELEASED = 0,
    BTN_PRESSED
} ButtonState;

/* 初始化所有按键引脚 */
HalStatus API_Button_Init(void);

/* 获取按键当前状态（带消抖） */
HalStatus API_Button_GetState(ButtonId id, ButtonState *state);

/*============================================================================
 *  蜂鸣器
 *============================================================================*/

/* 初始化蜂鸣器引脚 */
HalStatus API_Buzzer_Init(void);

/* 蜂鸣器开 */
HalStatus API_Buzzer_On(void);

/* 蜂鸣器关 */
HalStatus API_Buzzer_Off(void);

/* 蜂鸣器响指定毫秒（非阻塞，需在周期任务中调用 API_Buzzer_Update） */
HalStatus API_Buzzer_Beep(uint16_t duration_ms);

/* 蜂鸣器非阻塞更新 — 在 1ms 周期任务中调用 */
void API_Buzzer_Update(void);

#endif /* HAL_GPIO_H */
```

#### `HAL/hal_gpio.c`

```c
/**
 * @file    hal_gpio.c
 * @brief   GPIO 接口层实现
 */

#include "hal_gpio.h"
#include "drv_gpio.h"
#include "board_cfg.h"

/*============================================================================
 *  内部状态
 *============================================================================*/
static uint8_t    s_led_initialized = 0;
static LedState   s_led_state[LED_COUNT];

static uint8_t    s_btn_initialized = 0;
static uint8_t    s_btn_last_raw[BTN_COUNT];
static uint8_t    s_btn_stable[BTN_COUNT];
static uint8_t    s_btn_debounce_cnt[BTN_COUNT];
#define BTN_DEBOUNCE_TICKS  20   /* 20ms 消抖 */

static uint8_t    s_buzz_initialized = 0;
static uint8_t    s_buzz_on = 0;
static uint16_t   s_buzz_timer = 0;

/*============================================================================
 *  引脚查找表 — 由 board_cfg.h 定义的宏组成
 *============================================================================*/
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint32_t      clk;
} GpioMapping;

static const GpioMapping s_led_map[LED_COUNT] = {
    [LED_STATUS] = { LED_STATUS_GPIO_PORT, LED_STATUS_GPIO_PIN, LED_STATUS_GPIO_CLK },
    [LED_NET]    = { LED_NET_GPIO_PORT,    LED_NET_GPIO_PIN,    LED_NET_GPIO_CLK },
};

static const GpioMapping s_btn_map[BTN_COUNT] = {
    [BTN_KEY1] = { BTN_KEY1_GPIO_PORT, BTN_KEY1_GPIO_PIN, BTN_KEY1_GPIO_CLK },
};

static inline PinLevel LedStateToPinLevel(LedState state)
{
    return (state == LED_ON) ? PIN_HIGH : PIN_LOW;
}

/*============================================================================
 *  LED 实现
 *============================================================================*/
HalStatus API_Led_Init(void)
{
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        Drv_GPIO_InitOutput(s_led_map[i].port,
                            s_led_map[i].pin,
                            s_led_map[i].clk);
        Drv_GPIO_SetPin(s_led_map[i].port,
                        s_led_map[i].pin,
                        PIN_LOW);
        s_led_state[i] = LED_OFF;
    }
    s_led_initialized = 1;
    return HAL_OK;
}

HalStatus API_Led_SetStatus(LedId id, LedState state)
{
    if (!s_led_initialized)         return HAL_NOT_INIT;
    if (id >= LED_COUNT)            return HAL_INVALID_PARAM;
    if (state != LED_OFF && state != LED_ON) return HAL_INVALID_PARAM;

    Drv_GPIO_SetPin(s_led_map[id].port,
                    s_led_map[id].pin,
                    LedStateToPinLevel(state));
    s_led_state[id] = state;
    return HAL_OK;
}

HalStatus API_Led_Toggle(LedId id)
{
    if (!s_led_initialized)  return HAL_NOT_INIT;
    if (id >= LED_COUNT)     return HAL_INVALID_PARAM;

    Drv_GPIO_TogglePin(s_led_map[id].port, s_led_map[id].pin);
    s_led_state[id] = (s_led_state[id] == LED_ON) ? LED_OFF : LED_ON;
    return HAL_OK;
}

LedState API_Led_GetStatus(LedId id)
{
    if (id >= LED_COUNT) return LED_OFF;
    return s_led_state[id];
}

/*============================================================================
 *  按键实现
 *============================================================================*/
HalStatus API_Button_Init(void)
{
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        Drv_GPIO_InitInput(s_btn_map[i].port,
                           s_btn_map[i].pin,
                           s_btn_map[i].clk);
        s_btn_last_raw[i]    = 0;
        s_btn_stable[i]      = 0;
        s_btn_debounce_cnt[i] = 0;
    }
    s_btn_initialized = 1;
    return HAL_OK;
}

HalStatus API_Button_GetState(ButtonId id, ButtonState *state)
{
    if (!s_btn_initialized)  return HAL_NOT_INIT;
    if (id >= BTN_COUNT)     return HAL_INVALID_PARAM;
    if (state == NULL)       return HAL_INVALID_PARAM;

    PinLevel raw = Drv_GPIO_ReadPin(s_btn_map[id].port, s_btn_map[id].pin);

    /* 低电平有效则反转 */
    #if BTN_KEY1_ACTIVE_LOW
    if (id == BTN_KEY1) {
        raw = (raw == PIN_LOW) ? PIN_HIGH : PIN_LOW;
    }
    #endif

    uint8_t pressed = (raw == PIN_HIGH) ? 1U : 0U;

    if (pressed != s_btn_last_raw[id]) {
        s_btn_debounce_cnt[id] = 0;
        s_btn_last_raw[id] = pressed;
    } else {
        if (s_btn_debounce_cnt[id] < BTN_DEBOUNCE_TICKS) {
            s_btn_debounce_cnt[id]++;
        } else {
            s_btn_stable[id] = pressed;
        }
    }

    *state = s_btn_stable[id] ? BTN_PRESSED : BTN_RELEASED;
    return HAL_OK;
}

/*============================================================================
 *  蜂鸣器实现
 *============================================================================*/
HalStatus API_Buzzer_Init(void)
{
    Drv_GPIO_InitOutput(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, BUZZER_GPIO_CLK);
    Drv_GPIO_SetPin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, PIN_LOW);
    s_buzz_initialized = 1;
    s_buzz_on = 0;
    s_buzz_timer = 0;
    return HAL_OK;
}

HalStatus API_Buzzer_On(void)
{
    if (!s_buzz_initialized) return HAL_NOT_INIT;
    #if BUZZER_ACTIVE_HIGH
    Drv_GPIO_SetPin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, PIN_HIGH);
    #else
    Drv_GPIO_SetPin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, PIN_LOW);
    #endif
    s_buzz_on = 1;
    return HAL_OK;
}

HalStatus API_Buzzer_Off(void)
{
    if (!s_buzz_initialized) return HAL_NOT_INIT;
    #if BUZZER_ACTIVE_HIGH
    Drv_GPIO_SetPin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, PIN_LOW);
    #else
    Drv_GPIO_SetPin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, PIN_HIGH);
    #endif
    s_buzz_on = 0;
    s_buzz_timer = 0;
    return HAL_OK;
}

HalStatus API_Buzzer_Beep(uint16_t duration_ms)
{
    if (!s_buzz_initialized) return HAL_NOT_INIT;
    if (duration_ms == 0)    return HAL_INVALID_PARAM;

    API_Buzzer_On();
    s_buzz_timer = duration_ms;
    return HAL_OK;
}

void API_Buzzer_Update(void)
{
    if (!s_buzz_initialized) return;
    if (s_buzz_on && s_buzz_timer > 0) {
        s_buzz_timer--;
        if (s_buzz_timer == 0) {
            API_Buzzer_Off();
        }
    }
}
```

---

### 5.2 UART HAL

#### `HAL/hal_uart.h`

```c
/**
 * @file    hal_uart.h
 * @brief   UART 接口层 — 面向通信通道的抽象
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include "hal_common.h"
#include <stdint.h>

typedef enum {
    UART_ID_DEBUG = 0,   /* 调试串口 (USART1) */
    UART_ID_ESP,         /* ESP8266 串口 (USART3) */
    UART_ID_COUNT
} UartId;

/* 初始化指定 UART */
HalStatus API_Uart_Init(UartId id, uint32_t baudrate);

/* 发送数据（阻塞，带超时） */
HalStatus API_Uart_Send(UartId id, const uint8_t *data, uint16_t len);

/* 发送格式化字符串（调试用，内部缓冲 256 字节） */
HalStatus API_Uart_Printf(UartId id, const char *fmt, ...);

/* 检查是否有可读数据 */
HalStatus API_Uart_RxReady(UartId id, uint8_t *ready);

/* 读取一字节 */
HalStatus API_Uart_ReadByte(UartId id, uint8_t *byte);

#endif /* HAL_UART_H */
```

#### `HAL/hal_uart.c`

```c
/**
 * @file    hal_uart.c
 * @brief   UART 接口层实现
 */

#include "hal_uart.h"
#include "drv_uart.h"
#include "board_cfg.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define UART_TX_TIMEOUT_MS   1000

static uint8_t s_uart_initialized[UART_ID_COUNT] = {0};

/* UART ID 到硬件参数的映射 */
typedef struct {
    USART_TypeDef *uart;
    uint32_t       periph_clk;
    uint8_t        is_apb2;       /* 1 = APB2, 0 = APB1 */
    GPIO_TypeDef  *tx_port;
    uint16_t       tx_pin;
    GPIO_TypeDef  *rx_port;
    uint16_t       rx_pin;
} UartHwConfig;

static const UartHwConfig s_uart_cfg[UART_ID_COUNT] = {
    [UART_ID_DEBUG] = {
        .uart        = UART_DEBUG,
        .periph_clk  = UART_DEBUG_CLK,
        .is_apb2     = 1,
        .tx_port     = UART_DEBUG_TX_PORT,
        .tx_pin      = UART_DEBUG_TX_PIN,
        .rx_port     = UART_DEBUG_RX_PORT,
        .rx_pin      = UART_DEBUG_RX_PIN,
    },
    [UART_ID_ESP] = {
        .uart        = UART_ESP,
        .periph_clk  = UART_ESP_CLK,
        .is_apb2     = 0,
        .tx_port     = UART_ESP_TX_PORT,
        .tx_pin      = UART_ESP_TX_PIN,
        .rx_port     = UART_ESP_RX_PORT,
        .rx_pin      = UART_ESP_RX_PIN,
    },
};

HalStatus API_Uart_Init(UartId id, uint32_t baudrate)
{
    if (id >= UART_ID_COUNT) return HAL_INVALID_PARAM;

    const UartHwConfig *cfg = &s_uart_cfg[id];

    Drv_UART_Init(cfg->uart, baudrate,
                  cfg->tx_port, cfg->tx_pin,
                  cfg->rx_port, cfg->rx_pin,
                  cfg->periph_clk, cfg->is_apb2);

    s_uart_initialized[id] = 1;
    return HAL_OK;
}

HalStatus API_Uart_Send(UartId id, const uint8_t *data, uint16_t len)
{
    if (id >= UART_ID_COUNT)          return HAL_INVALID_PARAM;
    if (!s_uart_initialized[id])      return HAL_NOT_INIT;
    if (data == NULL || len == 0)     return HAL_INVALID_PARAM;

    return Drv_UART_SendData(s_uart_cfg[id].uart, data, len, UART_TX_TIMEOUT_MS);
}

HalStatus API_Uart_Printf(UartId id, const char *fmt, ...)
{
    if (id >= UART_ID_COUNT)       return HAL_INVALID_PARAM;
    if (!s_uart_initialized[id])   return HAL_NOT_INIT;

    char    buf[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    uint16_t len = (uint16_t)strlen(buf);
    return Drv_UART_SendData(s_uart_cfg[id].uart,
                             (const uint8_t *)buf, len,
                             UART_TX_TIMEOUT_MS);
}

HalStatus API_Uart_RxReady(UartId id, uint8_t *ready)
{
    if (id >= UART_ID_COUNT)       return HAL_INVALID_PARAM;
    if (!s_uart_initialized[id])   return HAL_NOT_INIT;
    if (ready == NULL)             return HAL_INVALID_PARAM;

    *ready = Drv_UART_RxReady(s_uart_cfg[id].uart) ? 1U : 0U;
    return HAL_OK;
}

HalStatus API_Uart_ReadByte(UartId id, uint8_t *byte)
{
    if (id >= UART_ID_COUNT)       return HAL_INVALID_PARAM;
    if (!s_uart_initialized[id])   return HAL_NOT_INIT;
    if (byte == NULL)              return HAL_INVALID_PARAM;

    *byte = Drv_UART_ReadByte(s_uart_cfg[id].uart);
    return HAL_OK;
}
```

---

### 5.3 WiFi HAL

#### `HAL/hal_wifi.h`

```c
/**
 * @file    hal_wifi.h
 * @brief   WiFi 接口层 — 面向网络连接的抽象
 * @note    ASW 只关心"连网"和"发数据"，不关心 ESP8266 AT 指令
 */

#ifndef HAL_WIFI_H
#define HAL_WIFI_H

#include "hal_common.h"
#include <stdint.h>

typedef enum {
    WIFI_STATE_IDLE = 0,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_TCP_OPEN,
    WIFI_STATE_ERROR
} WifiState;

/* 初始化 WiFi 模块（含 UART 初始化 + AT 测试） */
HalStatus API_Wifi_Init(void);

/* 连接 WiFi 热点（阻塞，超时毫秒） */
HalStatus API_Wifi_ConnectAP(const char *ssid, const char *password,
                             uint32_t timeout_ms);

/* 建立 TCP 连接 */
HalStatus API_Wifi_ConnectTCP(const char *server_ip, uint16_t port,
                              uint32_t timeout_ms);

/* 发送数据（通过已建立的 TCP 连接） */
HalStatus API_Wifi_SendData(const uint8_t *data, uint16_t len);

/* 获取当前连接状态 */
HalStatus API_Wifi_GetState(WifiState *state);

/* 检查是否有接收到的数据 */
HalStatus API_Wifi_RxAvailable(uint8_t *available);

/* 读取接收到的数据 */
HalStatus API_Wifi_ReadData(uint8_t *buf, uint16_t max_len,
                            uint16_t *actual_len);

#endif /* HAL_WIFI_H */
```

#### `HAL/hal_wifi.c`

```c
/**
 * @file    hal_wifi.c
 * @brief   WiFi 接口层实现
 */

#include "hal_wifi.h"
#include "drv_esp8266.h"
#include "hal_uart.h"
#include "board_cfg.h"

static WifiState s_wifi_state = WIFI_STATE_IDLE;
static uint8_t   s_wifi_initialized = 0;

HalStatus API_Wifi_Init(void)
{
    /* 先初始化 ESP8266 使用的 UART */
    HalStatus ret = API_Uart_Init(UART_ID_ESP, 115200);
    if (ret != HAL_OK) return ret;

    /* ESP8266 AT 初始化 */
    ret = Drv_ESP8266_Init(UART_ESP);
    if (ret != HAL_OK) {
        s_wifi_state = WIFI_STATE_ERROR;
        return ret;
    }

    s_wifi_initialized = 1;
    s_wifi_state = WIFI_STATE_IDLE;
    return HAL_OK;
}

HalStatus API_Wifi_ConnectAP(const char *ssid, const char *password,
                             uint32_t timeout_ms)
{
    if (!s_wifi_initialized)  return HAL_NOT_INIT;
    if (ssid == NULL)         return HAL_INVALID_PARAM;

    HalStatus ret = Drv_ESP8266_ConnectAP(ssid, password, timeout_ms);
    if (ret == HAL_OK) {
        s_wifi_state = WIFI_STATE_CONNECTED;
    } else {
        s_wifi_state = WIFI_STATE_ERROR;
    }
    return ret;
}

HalStatus API_Wifi_ConnectTCP(const char *server_ip, uint16_t port,
                              uint32_t timeout_ms)
{
    if (!s_wifi_initialized)  return HAL_NOT_INIT;
    if (server_ip == NULL)    return HAL_INVALID_PARAM;
    if (s_wifi_state != WIFI_STATE_CONNECTED) return HAL_ERROR;

    HalStatus ret = Drv_ESP8266_TCPConnect(server_ip, port, timeout_ms);
    if (ret == HAL_OK) {
        s_wifi_state = WIFI_STATE_TCP_OPEN;
    }
    return ret;
}

HalStatus API_Wifi_SendData(const uint8_t *data, uint16_t len)
{
    if (!s_wifi_initialized)       return HAL_NOT_INIT;
    if (s_wifi_state != WIFI_STATE_TCP_OPEN) return HAL_ERROR;
    if (data == NULL || len == 0)  return HAL_INVALID_PARAM;

    return Drv_ESP8266_TCPSend(data, len, 5000);
}

HalStatus API_Wifi_GetState(WifiState *state)
{
    if (state == NULL) return HAL_INVALID_PARAM;
    *state = s_wifi_state;
    return HAL_OK;
}

HalStatus API_Wifi_RxAvailable(uint8_t *available)
{
    if (!s_wifi_initialized) return HAL_NOT_INIT;
    if (available == NULL)   return HAL_INVALID_PARAM;

    *available = Drv_ESP8266_RxAvailable();
    return HAL_OK;
}

HalStatus API_Wifi_ReadData(uint8_t *buf, uint16_t max_len,
                            uint16_t *actual_len)
{
    if (!s_wifi_initialized)  return HAL_NOT_INIT;
    if (buf == NULL || actual_len == NULL) return HAL_INVALID_PARAM;

    *actual_len = Drv_ESP8266_Read(buf, max_len);
    return HAL_OK;
}
```

---

## 六、应用层（Task）— 只调用 HAL

### `Task/task_control.h`

```c
/**
 * @file    task_control.h
 * @brief   主控制任务
 */

#ifndef TASK_CONTROL_H
#define TASK_CONTROL_H

/* 初始化控制任务（初始化所有外设） */
void Task_Control_Init(void);

/* 1ms 周期任务（在 SysTick 中断或主循环中调用） */
void Task_Control_1ms(void);

/* 100ms 周期任务 */
void Task_Control_100ms(void);

/* 1000ms 周期任务 */
void Task_Control_1000ms(void);

#endif /* TASK_CONTROL_H */
```

### `Task/task_control.c`

```c
/**
 * @file    task_control.c
 * @brief   主控制任务实现
 * @note    此文件只调用 API_* 函数，不接触任何 Driver 或芯片库
 */

#include "task_control.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "hal_wifi.h"

/*============================================================================
 *  内部计数器
 *============================================================================*/
static uint16_t s_cnt_100ms = 0;
static uint16_t s_cnt_1000ms = 0;
static uint8_t  s_system_ready = 0;

/*============================================================================
 *  初始化
 *============================================================================*/
void Task_Control_Init(void)
{
    /* 调试串口 */
    API_Uart_Init(UART_ID_DEBUG, 115200);
    API_Uart_Printf(UART_ID_DEBUG, "\r\n[SYS] System booting...\r\n");

    /* LED */
    API_Led_Init();
    API_Led_SetStatus(LED_STATUS, LED_ON);

    /* 按键 */
    API_Button_Init();

    /* 蜂鸣器 */
    API_Buzzer_Init();
    API_Buzzer_Beep(100);   /* 开机提示音 */

    /* WiFi */
    API_Uart_Printf(UART_ID_DEBUG, "[SYS] Initializing WiFi...\r\n");
    if (API_Wifi_Init() == HAL_OK) {
        API_Uart_Printf(UART_ID_DEBUG, "[SYS] WiFi module ready\r\n");
        API_Led_SetStatus(LED_NET, LED_ON);

        if (API_Wifi_ConnectAP("MySSID", "MyPassword", 15000) == HAL_OK) {
            API_Uart_Printf(UART_ID_DEBUG, "[SYS] WiFi connected\r\n");
            API_Led_SetStatus(LED_NET, LED_ON);

            if (API_Wifi_ConnectTCP("192.168.1.100", 8080, 10000) == HAL_OK) {
                API_Uart_Printf(UART_ID_DEBUG, "[SYS] TCP connected\r\n");
            }
        } else {
            API_Uart_Printf(UART_ID_DEBUG, "[SYS] WiFi connect failed\r\n");
            API_Led_SetStatus(LED_NET, LED_OFF);
        }
    } else {
        API_Uart_Printf(UART_ID_DEBUG, "[SYS] WiFi init failed\r\n");
    }

    s_system_ready = 1;
    API_Uart_Printf(UART_ID_DEBUG, "[SYS] System ready\r\n");
}

/*============================================================================
 *  1ms 周期
 *============================================================================*/
void Task_Control_1ms(void)
{
    /* 蜂鸣器非阻塞更新 */
    API_Buzzer_Update();

    /* 计数分频 */
    s_cnt_100ms++;
}

/*============================================================================
 *  100ms 周期
 *============================================================================*/
void Task_Control_100ms(void)
{
    ButtonState btn_state;

    /* 按键检测 */
    if (API_Button_GetState(BTN_KEY1, &btn_state) == HAL_OK) {
        if (btn_state == BTN_PRESSED) {
            API_Led_Toggle(LED_STATUS);
            API_Buzzer_Beep(50);
            API_Uart_Printf(UART_ID_DEBUG, "[BTN] KEY1 pressed\r\n");
        }
    }
}

/*============================================================================
 *  1000ms 周期
 *============================================================================*/
void Task_Control_1000ms(void)
{
    /* 网络心跳 — 每秒发一次状态 */
    WifiState ws;
    if (API_Wifi_GetState(&ws) == HAL_OK && ws == WIFI_STATE_TCP_OPEN) {
        const char *heartbeat = "{\"type\":\"heartbeat\"}\n";
        API_Wifi_SendData((const uint8_t *)heartbeat, 22);
        API_Led_Toggle(LED_NET);
    }

    /* 检查是否有服务器下发数据 */
    uint8_t rx_avail = 0;
    if (API_Wifi_RxAvailable(&rx_avail) == HAL_OK && rx_avail) {
        uint8_t  buf[128];
        uint16_t actual = 0;
        API_Wifi_ReadData(buf, sizeof(buf), &actual);
        if (actual > 0) {
            API_Uart_Printf(UART_ID_DEBUG,
                            "[NET] Received %u bytes\r\n", actual);
        }
    }
}
```

---

### `Task/task_ota.h` & `Task/task_ota.c`

```c
/**
 * @file    task_ota.h
 * @brief   OTA 升级任务
 */

#ifndef TASK_OTA_H
#define TASK_OTA_H

#include "hal_common.h"

typedef enum {
    OTA_STATE_IDLE = 0,
    OTA_STATE_CHECKING,
    OTA_STATE_DOWNLOADING,
    OTA_STATE_VALIDATING,
    OTA_STATE_APPLYING,
    OTA_STATE_DONE,
    OTA_STATE_ERROR
} OtaState;

HalStatus API_OTA_Init(void);
HalStatus API_OTA_CheckUpdate(void);
HalStatus API_OTA_GetState(OtaState *state);
HalStatus API_OTA_Process(void);   /* 在主循环中调用 */

#endif /* TASK_OTA_H */
```

```c
/**
 * @file    task_ota.c
 * @brief   OTA 升级任务实现
 * @note    只调用 HAL 接口，不接触任何 Flash 寄存器或 ESP8266 指令
 */

#include "task_ota.h"
#include "hal_uart.h"
#include "hal_wifi.h"

static OtaState s_ota_state = OTA_STATE_IDLE;

HalStatus API_OTA_Init(void)
{
    s_ota_state = OTA_STATE_IDLE;
    return HAL_OK;
}

HalStatus API_OTA_CheckUpdate(void)
{
    if (s_ota_state != OTA_STATE_IDLE) return HAL_BUSY;

    s_ota_state = OTA_STATE_CHECKING;

    /* 向服务器发送版本查询 */
    const char *query = "{\"cmd\":\"check_update\",\"ver\":\"1.0.0\"}\n";
    HalStatus ret = API_Wifi_SendData((const uint8_t *)query, 30);
    if (ret != HAL_OK) {
        s_ota_state = OTA_STATE_ERROR;
    }

    return ret;
}

HalStatus API_OTA_GetState(OtaState *state)
{
    if (state == NULL) return HAL_INVALID_PARAM;
    *state = s_ota_state;
    return HAL_OK;
}

HalStatus API_OTA_Process(void)
{
    /* 实际下载/校验/刷写的复杂状态机后续扩展 */
    /* 此处只检查是否有服务器应答 */
    if (s_ota_state == OTA_STATE_CHECKING) {
        uint8_t avail = 0;
        if (API_Wifi_RxAvailable(&avail) == HAL_OK && avail) {
            uint8_t  buf[256];
            uint16_t len = 0;
            API_Wifi_ReadData(buf, sizeof(buf), &len);

            API_Uart_Printf(UART_ID_DEBUG,
                            "[OTA] Server response %u bytes\r\n", len);
            /* TODO: 解析应答，进入 DOWNLOADING 状态 */
            s_ota_state = OTA_STATE_IDLE;
        }
    }

    return HAL_OK;
}
```

---

## 七、系统入口

### `User/main.c`

```c
/**
 * @file    main.c
 * @brief   系统入口
 */

#include "ch32v30x.h"
#include "task_control.h"
#include "task_ota.h"

/*============================================================================
 *  全局毫秒计数
 *============================================================================*/
volatile uint32_t sys_tick_ms = 0;

/*============================================================================
 *  SysTick 中断处理
 *============================================================================*/
void SysTick_Handler(void)
{
    sys_tick_ms++;
    Task_Control_1ms();
}

/*============================================================================
 *  主函数
 *============================================================================*/
int main(void)
{
    /* 系统时钟初始化（144MHz） */
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* SysTick 配置为 1ms */
    SysTick_Config(SYSTEM_CORE_CLOCK_HZ / 1000);

    /* 初始化所有任务 */
    Task_Control_Init();
    API_OTA_Init();

    /* 主循环 — 轮询非实时任务 */
    uint32_t last_100ms  = 0;
    uint32_t last_1000ms = 0;

    while (1) {
        uint32_t now = sys_tick_ms;

        if ((now - last_100ms) >= 100) {
            last_100ms = now;
            Task_Control_100ms();
        }

        if ((now - last_1000ms) >= 1000) {
            last_1000ms = now;
            Task_Control_1000ms();
        }

        /* OTA 状态机轮询 */
        API_OTA_Process();
    }
}
```

---

## 八、架构总览图

```
┌─────────────────────────────────────────────────────────┐
│                    Task (ASW)                            │
│                                                          │
│  task_control.c        task_ota.c                        │
│  ├─ 按键 → 翻灯        ├─ OTA 版本查询                    │
│  ├─ 蜂鸣器提示          ├─ 固件下载                        │
│  └─ 心跳上报            └─ 校验刷写                        │
│                                                          │
│         只调用 API_*() 函数，不接触硬件                     │
├─────────────────────────────────────────────────────────┤
│                    HAL (接口层)                            │
│                                                          │
│  hal_gpio.c         hal_uart.c        hal_wifi.c         │
│  ├─ API_Led_*       ├─ API_Uart_*     ├─ API_Wifi_*      │
│  ├─ API_Button_*    └─ 调用 drv_uart   └─ 调用 drv_esp    │
│  └─ API_Buzzer_*                                           │
│                                                          │
│         消费 Driver 层，产出业务语义                        │
├─────────────────────────────────────────────────────────┤
│                    BSW (基础软件)                          │
│                                                          │
│  Driver/              Board/             Peripheral/      │
│  ├─ drv_gpio.c        ├─ board_cfg.h     ├─ ch32v30x_*.c  │
│  ├─ drv_uart.c        └─ 引脚映射宏       └─ 芯片官方库     │
│  └─ drv_esp8266.c                                           │
│                                                          │
│         操作寄存器，面向硬件                                │
└─────────────────────────────────────────────────────────┘
```

---

## 九、换板/换芯时的改动范围

| 场景 | 要改的文件 | 不需要动的 |
|---|---|---|
| **换一块板（引脚不同）** | `board_cfg.h` | HAL 全部、Task 全部、Driver 逻辑 |
| **换芯片（如 CH32V20x）** | `Peripheral/`、`board_cfg.h`、Driver 中寄存器调用 | HAL 全部、Task 全部 |
| **换 WiFi 模块（如 ESP32）** | `drv_esp8266.c` 内部实现 | `hal_wifi.h`、Task 全部 |
| **业务逻辑改版** | `Task/` 下的文件 | BSW 和 HAL 全部 |

这就是分层的价值：**每一层都是一个"变更防火墙"**。

---

以上就是完整方案。如果需要我补充某个模块的细节（比如 Flash 驱动的 HAL 封装、中断驱动的 UART 接收缓冲、或者 FreeRTOS 适配版本），可以直接说。

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
