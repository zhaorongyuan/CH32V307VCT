# CH32V30x 嵌入式分层软件架构 — 完整方案（定稿版）

---

## 一、架构总览

### 1.1 目录结构

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
│   │   │   ├── ch32v30x_usart.h
│   │   │   ├── ch32v30x_flash.h
│   │   │   ├── ch32v30x_rcc.h
│   │   │   └── ...
│   │   └── src/
│   │       ├── ch32v30x_gpio.c
│   │       ├── ch32v30x_usart.c
│   │       ├── ch32v30x_flash.c
│   │       ├── ch32v30x_rcc.c
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
│   ├── hal_gpio.h
│   ├── hal_gpio.c
│   ├── hal_uart.h
│   ├── hal_uart.c
│   ├── hal_network.h
│   └── hal_network.c
│
├── Middleware/                              # 中间件（纯软件，当前预留）
│   └── .gitkeep
│
├── APP/                                    # 应用业务逻辑
│   ├── task_control.h
│   ├── task_control.c
│   ├── task_ota.h
│   └── task_ota.c
│
└── User/                                   # 顶层入口
    ├── main.c
    └── app_cfg.h
```

### 1.2 依赖关系图

```
                         ┌─────────────────┐
                         │     APP 层       │
                         │  task_control    │
                         │    task_ota      │
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
     │    MCAL      │              │   Device 层      │
     │ (芯片官方库)  │              │   esp8266        │
     └──────┬──────┘              └────────┬─────────┘
            │                              │
            │         ┌────────────────────┘
            │         │ (从 FIFO 读取接收数据)
            │         ▼
            │  ┌─────────────────┐         ┌──────────────────┐
            │  │   Board 层       │         │   Services 层     │
            │  │ board_uart_fifo  │         │ drv_flash_eeprom │
            │  │       ▲          │         │   (片内复杂驱动)  │
            │  │       │          │         └────────┬─────────┘
            │  │ ch32v30x_it.c   │                  │
            │  │  (ISR 写 FIFO)  │                  │
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
- 零向上依赖：BSW 不引用任何 HAL 头文件
- 零直接水平耦合：ISR 和 Device 之间只有 FIFO 数据接力
- 按需设层：简单外设一跳到 MCAL，复杂驱动走 Services

### 1.3 配置窗口

| 文件 | 归属 | 职责 |
|---|---|---|
| `board_cfg.h` | BSW/Board | 硬件级：引脚映射、时钟、UART 通道、FIFO 大小 |
| `app_cfg.h` | User | 应用级：版本号、WiFi SSID、服务器地址等业务参数 |

换板子只改 `board_cfg.h`。改业务参数只改 `app_cfg.h`。

---

## 二、全部源文件

### 2.1 `User/app_cfg.h`

```c
/**
 * @file    app_cfg.h
 * @brief   应用级全局配置
 * @note    改业务参数只改这一个文件
 */

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
#define APP_TASK_100MS_PERIOD   100
#define APP_TASK_1000MS_PERIOD  1000

#endif /* APP_CFG_H */
```

---

### 2.2 `BSW/Core/ringbuffer.h`

```c
/**
 * @file    ringbuffer.h
 * @brief   环形缓冲 — 纯数据结构，无硬件依赖，所有层均可使用
 * @note    单生产者（ISR）单消费者（主循环），无需锁
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>

typedef struct {
    volatile uint8_t  *buf;     /* 数据区指针（外部提供） */
    volatile uint16_t  head;    /* 写指针 */
    volatile uint16_t  tail;    /* 读指针 */
    uint16_t           size;    /* 容量，必须为 2 的幂 */
} RingBuf;

void     RingBuf_Init(RingBuf *rb, uint8_t *buf, uint16_t size);
int      RingBuf_Put(RingBuf *rb, uint8_t byte);     /* 0=成功, -1=满 */
int      RingBuf_Get(RingBuf *rb);                    /* >=0=数据, -1=空 */
uint16_t RingBuf_Count(const RingBuf *rb);
void     RingBuf_Flush(RingBuf *rb);

#endif /* RINGBUFFER_H */
```

### 2.3 `BSW/Core/ringbuffer.c`

```c
/**
 * @file    ringbuffer.c
 * @brief   环形缓冲实现
 */

#include "ringbuffer.h"

void RingBuf_Init(RingBuf *rb, uint8_t *buf, uint16_t size)
{
    rb->buf  = buf;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
}

int RingBuf_Put(RingBuf *rb, uint8_t byte)
{
    uint16_t next = (rb->head + 1) & (rb->size - 1);
    if (next == rb->tail) return -1;
    rb->buf[rb->head] = byte;
    rb->head = next;
    return 0;
}

int RingBuf_Get(RingBuf *rb)
{
    if (rb->head == rb->tail) return -1;
    uint8_t byte = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) & (rb->size - 1);
    return (int)byte;
}

uint16_t RingBuf_Count(const RingBuf *rb)
{
    return (rb->head - rb->tail) & (rb->size - 1);
}

void RingBuf_Flush(RingBuf *rb)
{
    rb->head = 0;
    rb->tail = 0;
}
```

---

### 2.4 `BSW/Board/board_cfg.h`

```c
/**
 * @file    board_cfg.h
 * @brief   板级硬件配置 — 换板只改这一个文件
 */

#ifndef BOARD_CFG_H
#define BOARD_CFG_H

#include "ch32v30x.h"

/* ======== LED ======== */
#define LED_STATUS_PORT             GPIOA
#define LED_STATUS_PIN              GPIO_Pin_0
#define LED_STATUS_CLK              RCC_APB2Periph_GPIOA

#define LED_NET_PORT                GPIOA
#define LED_NET_PIN                 GPIO_Pin_1
#define LED_NET_CLK                 RCC_APB2Periph_GPIOA

/* ======== 按键 ======== */
#define BTN_KEY1_PORT               GPIOB
#define BTN_KEY1_PIN                GPIO_Pin_0
#define BTN_KEY1_CLK                RCC_APB2Periph_GPIOB
#define BTN_KEY1_ACTIVE_LOW         1

/* ======== 蜂鸣器 ======== */
#define BUZZER_PORT                 GPIOB
#define BUZZER_PIN                  GPIO_Pin_5
#define BUZZER_CLK                  RCC_APB2Periph_GPIOB
#define BUZZER_ACTIVE_HIGH          1

/* ======== 调试串口 (USART1) ======== */
#define DBG_UART                    USART1
#define DBG_UART_CLK                RCC_APB2Periph_USART1
#define DBG_UART_IS_APB2            1
#define DBG_UART_TX_PORT            GPIOA
#define DBG_UART_TX_PIN             GPIO_Pin_9
#define DBG_UART_RX_PORT            GPIOA
#define DBG_UART_RX_PIN             GPIO_Pin_10
#define DBG_UART_BAUD               115200

/* ======== ESP8266 串口 (USART3) ======== */
#define ESP_UART                    USART3
#define ESP_UART_CLK                RCC_APB1Periph_USART3
#define ESP_UART_IS_APB2            0
#define ESP_UART_TX_PORT            GPIOB
#define ESP_UART_TX_PIN             GPIO_Pin_10
#define ESP_UART_RX_PORT            GPIOB
#define ESP_UART_RX_PIN             GPIO_Pin_11
#define ESP_UART_BAUD               115200

/* ======== 串口 FIFO 大小（策略配置，必须为 2 的幂） ======== */
#define BOARD_UART1_RX_FIFO_SIZE    256
#define BOARD_UART3_RX_FIFO_SIZE    1024

/* ======== 系统时钟 ======== */
#define SYS_CLK_HZ                  144000000UL

#endif /* BOARD_CFG_H */
```

### 2.5 `BSW/Board/board_init.h`

```c
/**
 * @file    board_init.h
 * @brief   板级硬件初始化接口
 */

#ifndef BOARD_INIT_H
#define BOARD_INIT_H

#include <stdint.h>

void Board_InitDebugUart(uint32_t baudrate);
void Board_InitEspUart(uint32_t baudrate);
void Board_InitLeds(void);
void Board_InitButtons(void);
void Board_InitBuzzer(void);

#endif /* BOARD_INIT_H */
```

### 2.6 `BSW/Board/board_init.c`

```c
/**
 * @file    board_init.c
 * @brief   板级硬件初始化实现
 */

#include "board_init.h"
#include "board_cfg.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_usart.h"
#include "ch32v30x_rcc.h"

void Board_InitDebugUart(uint32_t baudrate)
{
    GPIO_InitTypeDef  gpio;
    USART_InitTypeDef uart;

    RCC_APB2PeriphClockCmd(DBG_UART_CLK, ENABLE);

    gpio.GPIO_Pin   = DBG_UART_TX_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DBG_UART_TX_PORT, &gpio);

    gpio.GPIO_Pin  = DBG_UART_RX_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(DBG_UART_RX_PORT, &gpio);

    uart.USART_BaudRate            = baudrate;
    uart.USART_WordLength          = USART_WordLength_8b;
    uart.USART_StopBits            = USART_StopBits_1;
    uart.USART_Parity              = USART_Parity_No;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(DBG_UART, &uart);
    USART_Cmd(DBG_UART, ENABLE);
}

void Board_InitEspUart(uint32_t baudrate)
{
    GPIO_InitTypeDef  gpio;
    USART_InitTypeDef uart;
    NVIC_InitTypeDef  nvic;

    if (ESP_UART_IS_APB2) {
        RCC_APB2PeriphClockCmd(ESP_UART_CLK, ENABLE);
    } else {
        RCC_APB1PeriphClockCmd(ESP_UART_CLK, ENABLE);
    }
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    gpio.GPIO_Pin   = ESP_UART_TX_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ESP_UART_TX_PORT, &gpio);

    gpio.GPIO_Pin  = ESP_UART_RX_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(ESP_UART_RX_PORT, &gpio);

    uart.USART_BaudRate            = baudrate;
    uart.USART_WordLength          = USART_WordLength_8b;
    uart.USART_StopBits            = USART_StopBits_1;
    uart.USART_Parity              = USART_Parity_No;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(ESP_UART, &uart);

    USART_ITConfig(ESP_UART, USART_IT_RXNE, ENABLE);

    nvic.NVIC_IRQChannel                   = USART3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    USART_Cmd(ESP_UART, ENABLE);
}

void Board_InitLeds(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(LED_STATUS_CLK | LED_NET_CLK, ENABLE);

    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    gpio.GPIO_Pin = LED_STATUS_PIN;
    GPIO_Init(LED_STATUS_PORT, &gpio);
    GPIO_WriteBit(LED_STATUS_PORT, LED_STATUS_PIN, Bit_RESET);

    gpio.GPIO_Pin = LED_NET_PIN;
    GPIO_Init(LED_NET_PORT, &gpio);
    GPIO_WriteBit(LED_NET_PORT, LED_NET_PIN, Bit_RESET);
}

void Board_InitButtons(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(BTN_KEY1_CLK, ENABLE);

    gpio.GPIO_Pin  = BTN_KEY1_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(BTN_KEY1_PORT, &gpio);
}

void Board_InitBuzzer(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(BUZZER_CLK, ENABLE);

    gpio.GPIO_Pin   = BUZZER_PIN;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZER_PORT, &gpio);
    GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN,
                  BUZZER_ACTIVE_HIGH ? Bit_RESET : Bit_SET);
}
```

### 2.7 `BSW/Board/board_uart_fifo.h`

```c
/**
 * @file    board_uart_fifo.h
 * @brief   板载串口 FIFO — ISR 写入端，Device/HAL 读取端
 * @note    纯引擎，缓冲区大小由 board_cfg.h 决定
 */

#ifndef BOARD_UART_FIFO_H
#define BOARD_UART_FIFO_H

#include <stdint.h>

typedef enum {
    UART_FIFO_DEBUG = 0,       /* USART1 */
    UART_FIFO_ESP,             /* USART3 */
    UART_FIFO_COUNT
} UartFifoId;

void     Board_UartFifo_Init(void);
void     Board_UartFifo_Push(UartFifoId id, uint8_t byte);  /* ISR 调用 */
int      Board_UartFifo_Get(UartFifoId id);                 /* -1=空 */
uint16_t Board_UartFifo_Count(UartFifoId id);
void     Board_UartFifo_Flush(UartFifoId id);

#endif /* BOARD_UART_FIFO_H */
```

### 2.8 `BSW/Board/board_uart_fifo.c`

```c
/**
 * @file    board_uart_fifo.c
 * @brief   板载串口 FIFO 实现
 */

#include "board_uart_fifo.h"
#include "board_cfg.h"
#include "ringbuffer.h"

static uint8_t  s_buf1[BOARD_UART1_RX_FIFO_SIZE];
static uint8_t  s_buf3[BOARD_UART3_RX_FIFO_SIZE];
static RingBuf  s_fifos[UART_FIFO_COUNT];

void Board_UartFifo_Init(void)
{
    RingBuf_Init(&s_fifos[UART_FIFO_DEBUG], s_buf1, BOARD_UART1_RX_FIFO_SIZE);
    RingBuf_Init(&s_fifos[UART_FIFO_ESP],   s_buf3, BOARD_UART3_RX_FIFO_SIZE);
}

void Board_UartFifo_Push(UartFifoId id, uint8_t byte)
{
    if (id < UART_FIFO_COUNT) {
        RingBuf_Put(&s_fifos[id], byte);
    }
}

int Board_UartFifo_Get(UartFifoId id)
{
    if (id >= UART_FIFO_COUNT) return -1;
    return RingBuf_Get(&s_fifos[id]);
}

uint16_t Board_UartFifo_Count(UartFifoId id)
{
    if (id >= UART_FIFO_COUNT) return 0;
    return RingBuf_Count(&s_fifos[id]);
}

void Board_UartFifo_Flush(UartFifoId id)
{
    if (id < UART_FIFO_COUNT) {
        RingBuf_Flush(&s_fifos[id]);
    }
}
```

### 2.9 `BSW/Board/ch32v30x_it.c`

```c
/**
 * @file    ch32v30x_it.c
 * @brief   中断服务 — 只往 Board_UartFifo 推数据
 * @note    完全不知道 ESP8266 的存在；换通信模组时此文件零修改
 */

#include "ch32v30x.h"
#include "board_uart_fifo.h"

extern volatile uint32_t sys_tick_ms;

void NMI_Handler(void)       { }
void HardFault_Handler(void) { while (1); }

void SysTick_Handler(void)
{
    sys_tick_ms++;
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        Board_UartFifo_Push(UART_FIFO_DEBUG,
                            (uint8_t)USART_ReceiveData(USART1));
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        Board_UartFifo_Push(UART_FIFO_ESP,
                            (uint8_t)USART_ReceiveData(USART3));
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}
```

---

### 2.10 `BSW/Services/drv_flash_eeprom.h`

```c
/**
 * @file    drv_flash_eeprom.h
 * @brief   片内 Flash 模拟 EEPROM
 * @note    归属 BSW/Services — 非厂商库，非板级，非外部器件
 */

#ifndef DRV_FLASH_EEPROM_H
#define DRV_FLASH_EEPROM_H

#include <stdint.h>
#include "hal_common.h"

HalStatus FlashEEPROM_Init(void);
HalStatus FlashEEPROM_Read(uint16_t key, uint8_t *data, uint16_t len);
HalStatus FlashEEPROM_Write(uint16_t key, const uint8_t *data, uint16_t len);
HalStatus FlashEEPROM_Format(void);

#endif /* DRV_FLASH_EEPROM_H */
```

### 2.11 `BSW/Services/drv_flash_eeprom.c`

```c
/**
 * @file    drv_flash_eeprom.c
 * @brief   片内 Flash 模拟 EEPROM — 直接调用 MCAL Flash API
 */

#include "drv_flash_eeprom.h"
#include "ch32v30x_flash.h"
#include <string.h>

#define EEPROM_PAGE_ADDR    0x0801F000
#define EEPROM_PAGE_SIZE    4096
#define ITEM_MAX_DATA_LEN   32
#define ITEM_MAGIC          0xA5
#define ITEM_INVALID        0xFF

#pragma pack(push, 1)
typedef struct {
    uint8_t  magic;
    uint16_t key;
    uint16_t len;
    uint8_t  data[ITEM_MAX_DATA_LEN];
    uint8_t  checksum;
} EepromItem;
#pragma pack(pop)

static uint32_t s_writeOffset = 0;

static uint8_t CalcChecksum(const EepromItem *item)
{
    uint8_t sum = 0;
    const uint8_t *p = (const uint8_t *)item;
    for (uint16_t i = 0; i < offsetof(EepromItem, checksum); i++) {
        sum ^= p[i];
    }
    return sum;
}

HalStatus FlashEEPROM_Init(void)
{
    s_writeOffset = 0;
    const EepromItem *p = (const EepromItem *)EEPROM_PAGE_ADDR;
    uint32_t maxItems = EEPROM_PAGE_SIZE / sizeof(EepromItem);

    for (uint32_t i = 0; i < maxItems; i++) {
        if (p[i].magic != ITEM_MAGIC) break;
        if (CalcChecksum(&p[i]) != p[i].checksum) break;
        s_writeOffset = (i + 1) * sizeof(EepromItem);
    }
    return HAL_OK;
}

HalStatus FlashEEPROM_Read(uint16_t key, uint8_t *data, uint16_t len)
{
    const EepromItem *page = (const EepromItem *)EEPROM_PAGE_ADDR;
    uint32_t maxItems = s_writeOffset / sizeof(EepromItem);
    int found = -1;

    for (uint32_t i = 0; i < maxItems; i++) {
        if (page[i].magic == ITEM_MAGIC &&
            page[i].key == key &&
            CalcChecksum(&page[i]) == page[i].checksum) {
            found = (int)i;
        }
    }

    if (found < 0) return HAL_ERROR;

    uint16_t copyLen = page[found].len;
    if (copyLen > len) copyLen = len;
    memcpy(data, page[found].data, copyLen);
    return HAL_OK;
}

HalStatus FlashEEPROM_Write(uint16_t key, const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0 || len > ITEM_MAX_DATA_LEN)
        return HAL_INVALID_PARAM;

    if ((s_writeOffset + sizeof(EepromItem)) > EEPROM_PAGE_SIZE) {
        /* 页面满：需擦除 + 搬移有效数据（此处简化为直接擦除） */
        FLASH_Unlock();
        FLASH_ErasePage(EEPROM_PAGE_ADDR);
        FLASH_Lock();
        s_writeOffset = 0;
    }

    EepromItem item;
    memset(&item, 0xFF, sizeof(item));
    item.magic = ITEM_MAGIC;
    item.key   = key;
    item.len   = len;
    memcpy(item.data, data, len);
    item.checksum = CalcChecksum(&item);

    FLASH_Unlock();

    uint32_t addr = EEPROM_PAGE_ADDR + s_writeOffset;
    const uint16_t *src = (const uint16_t *)&item;
    for (uint16_t i = 0; i < sizeof(EepromItem) / 2; i++) {
        FLASH_ProgramHalfWord(addr + i * 2, src[i]);
    }

    FLASH_Lock();

    s_writeOffset += sizeof(EepromItem);
    return HAL_OK;
}

HalStatus FlashEEPROM_Format(void)
{
    FLASH_Unlock();
    FLASH_ErasePage(EEPROM_PAGE_ADDR);
    FLASH_Lock();
    s_writeOffset = 0;
    return HAL_OK;
}
```

---

### 2.12 `BSW/Device/esp8266/esp8266.h`

```c
/**
 * @file    esp8266.h
 * @brief   ESP8266 AT 指令驱动
 * @note    归属 BSW/Device — 从 Board_UartFifo 读取，直接调 MCAL 发送
 */

#ifndef ESP8266_H
#define ESP8266_H

#include <stdint.h>
#include "hal_common.h"

HalStatus ESP8266_Init(void);
HalStatus ESP8266_ConnectAP(const char *ssid, const char *password,
                            uint32_t timeout_ms);
HalStatus ESP8266_TCPConnect(const char *ip, uint16_t port,
                             uint32_t timeout_ms);
HalStatus ESP8266_TCPSend(const uint8_t *data, uint16_t len,
                          uint32_t timeout_ms);
uint8_t   ESP8266_RxAvailable(void);
uint16_t  ESP8266_Read(uint8_t *buf, uint16_t max_len);

#endif /* ESP8266_H */
```

### 2.13 `BSW/Device/esp8266/esp8266.c`

```c
/**
 * @file    esp8266.c
 * @brief   ESP8266 AT 指令驱动实现
 * @note    接收：从 Board_UartFifo 拉取（不直接碰 ISR）
 *          发送：直接操作 MCAL USART 寄存器（最短路径）
 */

#include "esp8266.h"
#include "board_cfg.h"
#include "board_uart_fifo.h"
#include "ch32v30x_usart.h"
#include <string.h>
#include <stdio.h>

extern volatile uint32_t sys_tick_ms;

/*──────────────────────────────────────────
 *  接收：从 FIFO 拉取
 *──────────────────────────────────────────*/
static int RxRead(void)
{
    return Board_UartFifo_Get(UART_FIFO_ESP);
}

/*──────────────────────────────────────────
 *  发送：直接操作 MCAL
 *──────────────────────────────────────────*/
static void TxByte(uint8_t byte)
{
    while (USART_GetFlagStatus(ESP_UART, USART_FLAG_TXE) == RESET)
        ;
    USART_SendData(ESP_UART, byte);
}

static void TxString(const char *s)
{
    while (*s) TxByte((uint8_t)*s++);
}

static void TxData(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) TxByte(data[i]);
}

static void TxFlush(void)
{
    while (USART_GetFlagStatus(ESP_UART, USART_FLAG_TC) == RESET)
        ;
}

/*──────────────────────────────────────────
 *  等待指定响应字符串
 *──────────────────────────────────────────*/
static HalStatus WaitFor(const char *expected, uint32_t timeout_ms)
{
    uint32_t start = sys_tick_ms;
    char     tmp[128];
    uint16_t idx = 0;
    memset(tmp, 0, sizeof(tmp));

    while ((sys_tick_ms - start) < timeout_ms) {
        int ch = RxRead();
        if (ch >= 0) {
            if (idx < sizeof(tmp) - 1) {
                tmp[idx++] = (char)ch;
                tmp[idx]  = '\0';
            }
            if (strstr(tmp, expected) != NULL) {
                return HAL_OK;
            }
        }
    }
    return HAL_TIMEOUT;
}

static void SendCmd(const char *cmd)
{
    TxString(cmd);
    TxFlush();
}

/*──────────────────────────────────────────
 *  公共 API
 *──────────────────────────────────────────*/
HalStatus ESP8266_Init(void)
{
    /* FIFO 已在 Board_UartFifo_Init 中初始化 */

    SendCmd("AT\r\n");
    if (WaitFor("OK", 2000) != HAL_OK) return HAL_ERROR;

    SendCmd("AT+CWMODE=1\r\n");
    if (WaitFor("OK", 2000) != HAL_OK) return HAL_ERROR;

    return HAL_OK;
}

HalStatus ESP8266_ConnectAP(const char *ssid, const char *password,
                            uint32_t timeout_ms)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);
    SendCmd(cmd);
    return WaitFor("OK", timeout_ms);
}

HalStatus ESP8266_TCPConnect(const char *ip, uint16_t port,
                             uint32_t timeout_ms)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%u\r\n", ip, port);
    SendCmd(cmd);
    return WaitFor("OK", timeout_ms);
}

HalStatus ESP8266_TCPSend(const uint8_t *data, uint16_t len,
                          uint32_t timeout_ms)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u\r\n", len);
    SendCmd(cmd);
    if (WaitFor(">", 3000) != HAL_OK) return HAL_ERROR;

    TxData(data, len);
    TxFlush();
    return WaitFor("SEND OK", timeout_ms);
}

uint8_t ESP8266_RxAvailable(void)
{
    return (Board_UartFifo_Count(UART_FIFO_ESP) > 0) ? 1U : 0U;
}

uint16_t ESP8266_Read(uint8_t *buf, uint16_t max_len)
{
    uint16_t count = 0;
    while (count < max_len) {
        int ch = RxRead();
        if (ch < 0) break;
        buf[count++] = (uint8_t)ch;
    }
    return count;
}
```

---

### 2.14 `HAL/hal_common.h`

```c
/**
 * @file    hal_common.h
 * @brief   HAL 层公共类型定义
 */

#ifndef HAL_COMMON_H
#define HAL_COMMON_H

#include <stdint.h>

typedef enum {
    HAL_OK            = 0x00U,
    HAL_ERROR         = 0x01U,
    HAL_BUSY          = 0x02U,
    HAL_TIMEOUT       = 0x03U,
    HAL_INVALID_PARAM = 0x04U,
    HAL_NOT_INIT      = 0x05U
} HalStatus;

#endif /* HAL_COMMON_H */
```

### 2.15 `HAL/hal_gpio.h`

```c
/**
 * @file    hal_gpio.h
 * @brief   GPIO 接口 — 面向业务语义的 LED、按键、蜂鸣器
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "hal_common.h"

/* ======== LED ======== */
typedef enum {
    LED_STATUS = 0,
    LED_NET,
    LED_COUNT
} LedId;

typedef enum {
    LED_OFF = 0,
    LED_ON
} LedState;

HalStatus API_Led_Init(void);
HalStatus API_Led_SetStatus(LedId id, LedState state);
HalStatus API_Led_Toggle(LedId id);
LedState  API_Led_GetStatus(LedId id);

/* ======== 按键 ======== */
typedef enum {
    BTN_KEY1 = 0,
    BTN_COUNT
} ButtonId;

typedef enum {
    BTN_RELEASED = 0,
    BTN_PRESSED
} ButtonState;

HalStatus API_Button_Init(void);
HalStatus API_Button_GetState(ButtonId id, ButtonState *state);

/* ======== 蜂鸣器 ======== */
HalStatus API_Buzzer_Init(void);
HalStatus API_Buzzer_On(void);
HalStatus API_Buzzer_Off(void);
HalStatus API_Buzzer_Beep(uint16_t duration_ms);
void      API_Buzzer_Update(void);   /* 1ms 周期调用 */

#endif /* HAL_GPIO_H */
```

### 2.16 `HAL/hal_gpio.c`

```c
/**
 * @file    hal_gpio.c
 * @brief   GPIO 接口实现 — 直接调用 MCAL，无中间 Driver
 */

#include "hal_gpio.h"
#include "board_cfg.h"
#include "board_init.h"
#include "ch32v30x_gpio.h"

/*──────────────────────────────────────────
 *  引脚映射
 *──────────────────────────────────────────*/
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
} PinEntry;

static const PinEntry s_ledPin[LED_COUNT] = {
    [LED_STATUS] = { LED_STATUS_PORT, LED_STATUS_PIN },
    [LED_NET]    = { LED_NET_PORT,    LED_NET_PIN },
};

static const PinEntry s_btnPin[BTN_COUNT] = {
    [BTN_KEY1] = { BTN_KEY1_PORT, BTN_KEY1_PIN },
};

/*──────────────────────────────────────────
 *  内部状态
 *──────────────────────────────────────────*/
static LedState s_ledState[LED_COUNT];
static uint8_t  s_ledInited  = 0;
static uint8_t  s_btnInited  = 0;

#define BTN_DEBOUNCE_MS 20
static uint8_t s_btnLast[BTN_COUNT];
static uint8_t s_btnStable[BTN_COUNT];
static uint8_t s_btnCnt[BTN_COUNT];

static uint8_t  s_buzzInited = 0;
static uint8_t  s_buzzOn     = 0;
static uint16_t s_buzzTimer  = 0;

/*──────────────────────────────────────────
 *  LED
 *──────────────────────────────────────────*/
HalStatus API_Led_Init(void)
{
    Board_InitLeds();
    for (int i = 0; i < LED_COUNT; i++) s_ledState[i] = LED_OFF;
    s_ledInited = 1;
    return HAL_OK;
}

HalStatus API_Led_SetStatus(LedId id, LedState state)
{
    if (!s_ledInited)                              return HAL_NOT_INIT;
    if (id >= LED_COUNT)                           return HAL_INVALID_PARAM;
    if (state != LED_OFF && state != LED_ON)       return HAL_INVALID_PARAM;

    GPIO_WriteBit(s_ledPin[id].port, s_ledPin[id].pin,
                  (state == LED_ON) ? Bit_SET : Bit_RESET);
    s_ledState[id] = state;
    return HAL_OK;
}

HalStatus API_Led_Toggle(LedId id)
{
    if (!s_ledInited)    return HAL_NOT_INIT;
    if (id >= LED_COUNT) return HAL_INVALID_PARAM;

    if (GPIO_ReadOutputDataBit(s_ledPin[id].port, s_ledPin[id].pin) == Bit_SET) {
        GPIO_WriteBit(s_ledPin[id].port, s_ledPin[id].pin, Bit_RESET);
    } else {
        GPIO_WriteBit(s_ledPin[id].port, s_ledPin[id].pin, Bit_SET);
    }
    s_ledState[id] = (s_ledState[id] == LED_ON) ? LED_OFF : LED_ON;
    return HAL_OK;
}

LedState API_Led_GetStatus(LedId id)
{
    if (id >= LED_COUNT) return LED_OFF;
    return s_ledState[id];
}

/*──────────────────────────────────────────
 *  按键
 *──────────────────────────────────────────*/
HalStatus API_Button_Init(void)
{
    Board_InitButtons();
    for (int i = 0; i < BTN_COUNT; i++) {
        s_btnLast[i]   = 0;
        s_btnStable[i] = 0;
        s_btnCnt[i]    = 0;
    }
    s_btnInited = 1;
    return HAL_OK;
}

HalStatus API_Button_GetState(ButtonId id, ButtonState *state)
{
    if (!s_btnInited)     return HAL_NOT_INIT;
    if (id >= BTN_COUNT)  return HAL_INVALID_PARAM;
    if (state == NULL)    return HAL_INVALID_PARAM;

    uint8_t raw = (GPIO_ReadInputDataBit(s_btnPin[id].port,
                                          s_btnPin[id].pin) == Bit_SET) ? 1U : 0U;

    if (id == BTN_KEY1 && BTN_KEY1_ACTIVE_LOW) {
        raw = !raw;
    }

    if (raw != s_btnLast[id]) {
        s_btnCnt[id] = 0;
        s_btnLast[id] = raw;
    } else {
        if (s_btnCnt[id] < BTN_DEBOUNCE_MS) {
            s_btnCnt[id]++;
        } else {
            s_btnStable[id] = raw;
        }
    }

    *state = s_btnStable[id] ? BTN_PRESSED : BTN_RELEASED;
    return HAL_OK;
}

/*──────────────────────────────────────────
 *  蜂鸣器
 *──────────────────────────────────────────*/
HalStatus API_Buzzer_Init(void)
{
    Board_InitBuzzer();
    s_buzzInited = 1;
    s_buzzOn     = 0;
    s_buzzTimer  = 0;
    return HAL_OK;
}

HalStatus API_Buzzer_On(void)
{
    if (!s_buzzInited) return HAL_NOT_INIT;
    GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN,
                  BUZZER_ACTIVE_HIGH ? Bit_SET : Bit_RESET);
    s_buzzOn = 1;
    return HAL_OK;
}

HalStatus API_Buzzer_Off(void)
{
    if (!s_buzzInited) return HAL_NOT_INIT;
    GPIO_WriteBit(BUZZER_PORT, BUZZER_PIN,
                  BUZZER_ACTIVE_HIGH ? Bit_RESET : Bit_SET);
    s_buzzOn    = 0;
    s_buzzTimer = 0;
    return HAL_OK;
}

HalStatus API_Buzzer_Beep(uint16_t duration_ms)
{
    if (!s_buzzInited)     return HAL_NOT_INIT;
    if (duration_ms == 0)  return HAL_INVALID_PARAM;
    API_Buzzer_On();
    s_buzzTimer = duration_ms;
    return HAL_OK;
}

void API_Buzzer_Update(void)
{
    if (!s_buzzOn || s_buzzTimer == 0) return;
    s_buzzTimer--;
    if (s_buzzTimer == 0) API_Buzzer_Off();
}
```

### 2.17 `HAL/hal_uart.h`

```c
/**
 * @file    hal_uart.h
 * @brief   UART 接口 — 调试串口抽象
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include "hal_common.h"
#include <stdint.h>

typedef enum {
    UART_ID_DEBUG = 0,
    UART_ID_COUNT
} UartId;

HalStatus API_Uart_Init(UartId id, uint32_t baudrate);
HalStatus API_Uart_Send(UartId id, const uint8_t *data, uint16_t len);
HalStatus API_Uart_Printf(UartId id, const char *fmt, ...);

#endif /* HAL_UART_H */
```

### 2.18 `HAL/hal_uart.c`

```c
/**
 * @file    hal_uart.c
 * @brief   调试串口实现 — 直接调用 MCAL
 */

#include "hal_uart.h"
#include "board_cfg.h"
#include "board_init.h"
#include "ch32v30x_usart.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define TX_TIMEOUT_MS  1000

extern volatile uint32_t sys_tick_ms;

static uint8_t s_inited[UART_ID_COUNT] = {0};

static USART_TypeDef * const s_hw[UART_ID_COUNT] = {
    [UART_ID_DEBUG] = DBG_UART,
};

HalStatus API_Uart_Init(UartId id, uint32_t baudrate)
{
    if (id >= UART_ID_COUNT) return HAL_INVALID_PARAM;

    if (id == UART_ID_DEBUG) {
        Board_InitDebugUart(baudrate);
    }
    s_inited[id] = 1;
    return HAL_OK;
}

HalStatus API_Uart_Send(UartId id, const uint8_t *data, uint16_t len)
{
    if (id >= UART_ID_COUNT)       return HAL_INVALID_PARAM;
    if (!s_inited[id])             return HAL_NOT_INIT;
    if (data == NULL || len == 0)  return HAL_INVALID_PARAM;

    USART_TypeDef *uart = s_hw[id];
    uint32_t start = sys_tick_ms;

    for (uint16_t i = 0; i < len; i++) {
        while (USART_GetFlagStatus(uart, USART_FLAG_TXE) == RESET) {
            if ((sys_tick_ms - start) >= TX_TIMEOUT_MS) return HAL_TIMEOUT;
        }
        USART_SendData(uart, data[i]);
    }

    while (USART_GetFlagStatus(uart, USART_FLAG_TC) == RESET) {
        if ((sys_tick_ms - start) >= TX_TIMEOUT_MS) return HAL_TIMEOUT;
    }

    return HAL_OK;
}

HalStatus API_Uart_Printf(UartId id, const char *fmt, ...)
{
    if (id >= UART_ID_COUNT)  return HAL_INVALID_PARAM;
    if (!s_inited[id])        return HAL_NOT_INIT;

    char    buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    return API_Uart_Send(id, (const uint8_t *)buf, (uint16_t)strlen(buf));
}
```

### 2.19 `HAL/hal_network.h`

```c
/**
 * @file    hal_network.h
 * @brief   网络抽象接口
 * @note    屏蔽底层传输介质（WiFi/4G/以太网）
 */

#ifndef HAL_NETWORK_H
#define HAL_NETWORK_H

#include "hal_common.h"
#include <stdint.h>

typedef enum {
    NET_IDLE = 0,
    NET_READY,
    NET_TCP_OPEN,
    NET_ERROR
} NetState;

HalStatus API_Net_Init(void);
HalStatus API_Net_Connect(const char *ssid, const char *password,
                          uint32_t timeout_ms);
HalStatus API_Net_ConnectTCP(const char *ip, uint16_t port,
                             uint32_t timeout_ms);
HalStatus API_Net_Send(const uint8_t *data, uint16_t len);
HalStatus API_Net_RxAvailable(uint8_t *available);
HalStatus API_Net_Read(uint8_t *buf, uint16_t max_len, uint16_t *actual_len);
HalStatus API_Net_GetState(NetState *state);

#endif /* HAL_NETWORK_H */
```

### 2.20 `HAL/hal_network.c`

```c
/**
 * @file    hal_network.c
 * @brief   网络抽象实现 — 内部绑定 BSW/Device/esp8266
 * @note    换 4G/以太网时只改这一个文件，hal_network.h 和 APP 不动
 */

#include "hal_network.h"
#include "esp8266.h"            /* 调用 Device 层（向下依赖） */
#include "board_init.h"
#include "app_cfg.h"

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

HalStatus API_Net_Connect(const char *ssid, const char *password,
                          uint32_t timeout_ms)
{
    if (s_state < NET_READY) return HAL_NOT_INIT;
    if (ssid == NULL)        return HAL_INVALID_PARAM;

    HalStatus ret = ESP8266_ConnectAP(ssid, password, timeout_ms);
    if (ret != HAL_OK) s_state = NET_ERROR;
    return ret;
}

HalStatus API_Net_ConnectTCP(const char *ip, uint16_t port,
                             uint32_t timeout_ms)
{
    if (s_state < NET_READY) return HAL_ERROR;
    if (ip == NULL)          return HAL_INVALID_PARAM;

    HalStatus ret = ESP8266_TCPConnect(ip, port, timeout_ms);
    if (ret == HAL_OK) s_state = NET_TCP_OPEN;
    return ret;
}

HalStatus API_Net_Send(const uint8_t *data, uint16_t len)
{
    if (s_state != NET_TCP_OPEN)  return HAL_ERROR;
    if (data == NULL || len == 0) return HAL_INVALID_PARAM;

    return ESP8266_TCPSend(data, len, 5000);
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

---

### 2.21 `APP/task_control.h`

```c
/**
 * @file    task_control.h
 * @brief   主控制任务
 */

#ifndef TASK_CONTROL_H
#define TASK_CONTROL_H

void Task_Control_Init(void);
void Task_Control_1ms(void);
void Task_Control_100ms(void);
void Task_Control_1000ms(void);

#endif /* TASK_CONTROL_H */
```

### 2.22 `APP/task_control.c`

```c
/**
 * @file    task_control.c
 * @brief   主控制任务 — 只调用 HAL API，零硬件依赖
 */

#include "task_control.h"
#include "hal_gpio.h"
#include "hal_uart.h"
#include "hal_network.h"
#include "app_cfg.h"
#include <string.h>

static uint8_t s_ready = 0;

void Task_Control_Init(void)
{
    /* 调试串口 */
    API_Uart_Init(UART_ID_DEBUG, DBG_UART_BAUD);
    API_Uart_Printf(UART_ID_DEBUG, "\r\n[SYS] v%s booting...\r\n", APP_FW_VERSION);

    /* LED */
    API_Led_Init();
    API_Led_SetStatus(LED_STATUS, LED_ON);

    /* 按键 */
    API_Button_Init();

    /* 蜂鸣器 */
    API_Buzzer_Init();
    API_Buzzer_Beep(100);

    /* 网络 */
    API_Uart_Printf(UART_ID_DEBUG, "[SYS] WiFi init...\r\n");
    if (API_Net_Init() == HAL_OK) {
        API_Uart_Printf(UART_ID_DEBUG, "[SYS] WiFi module OK\r\n");

        if (API_Net_Connect(APP_WIFI_SSID, APP_WIFI_PASSWORD, 15000) == HAL_OK) {
            API_Uart_Printf(UART_ID_DEBUG, "[SYS] WiFi connected\r\n");
            API_Led_SetStatus(LED_NET, LED_ON);

            if (API_Net_ConnectTCP(APP_SERVER_IP, APP_SERVER_PORT, 10000) == HAL_OK) {
                API_Uart_Printf(UART_ID_DEBUG, "[SYS] TCP connected\r\n");
            } else {
                API_Uart_Printf(UART_ID_DEBUG, "[SYS] TCP failed\r\n");
            }
        } else {
            API_Uart_Printf(UART_ID_DEBUG, "[SYS] WiFi failed\r\n");
            API_Led_SetStatus(LED_NET, LED_OFF);
        }
    } else {
        API_Uart_Printf(UART_ID_DEBUG, "[SYS] WiFi init failed\r\n");
    }

    s_ready = 1;
    API_Uart_Printf(UART_ID_DEBUG, "[SYS] ready\r\n");
}

void Task_Control_1ms(void)
{
    API_Buzzer_Update();
}

void Task_Control_100ms(void)
{
    ButtonState btn;
    if (API_Button_GetState(BTN_KEY1, &btn) == HAL_OK) {
        if (btn == BTN_PRESSED) {
            API_Led_Toggle(LED_STATUS);
            API_Buzzer_Beep(50);
            API_Uart_Printf(UART_ID_DEBUG, "[BTN] KEY1\r\n");
        }
    }
}

void Task_Control_1000ms(void)
{
    /* 心跳 */
    NetState ns;
    if (API_Net_GetState(&ns) == HAL_OK && ns == NET_TCP_OPEN) {
        const char *hb = "{\"type\":\"heartbeat\"}\n";
        API_Net_Send((const uint8_t *)hb, (uint16_t)strlen(hb));
        API_Led_Toggle(LED_NET);
    }

    /* 接收检查 */
    uint8_t avail = 0;
    if (API_Net_RxAvailable(&avail) == HAL_OK && avail) {
        uint8_t  buf[128];
        uint16_t len = 0;
        API_Net_Read(buf, sizeof(buf), &len);
        if (len > 0) {
            API_Uart_Printf(UART_ID_DEBUG, "[NET] rx %u bytes\r\n", len);
        }
    }
}
```

### 2.23 `APP/task_ota.h`

```c
/**
 * @file    task_ota.h
 * @brief   OTA 升级任务
 */

#ifndef TASK_OTA_H
#define TASK_OTA_H

#include "hal_common.h"

typedef enum {
    OTA_IDLE = 0,
    OTA_CHECKING,
    OTA_DOWNLOADING,
    OTA_VALIDATING,
    OTA_APPLYING,
    OTA_DONE,
    OTA_ERROR
} OtaState;

HalStatus API_OTA_Init(void);
HalStatus API_OTA_CheckUpdate(void);
HalStatus API_OTA_GetState(OtaState *state);
HalStatus API_OTA_Process(void);

#endif /* TASK_OTA_H */
```

### 2.24 `APP/task_ota.c`

```c
/**
 * @file    task_ota.c
 * @brief   OTA 任务实现 — 只调用 HAL
 */

#include "task_ota.h"
#include "hal_uart.h"
#include "hal_network.h"
#include "app_cfg.h"
#include <string.h>

static OtaState s_state = OTA_IDLE;

HalStatus API_OTA_Init(void)
{
    s_state = OTA_IDLE;
    return HAL_OK;
}

HalStatus API_OTA_CheckUpdate(void)
{
    if (s_state != OTA_IDLE) return HAL_BUSY;
    s_state = OTA_CHECKING;

    char query[128];
    snprintf(query, sizeof(query),
             "{\"cmd\":\"check\",\"ver\":\"%s\"}\n", APP_FW_VERSION);

    HalStatus ret = API_Net_Send((const uint8_t *)query, (uint16_t)strlen(query));
    if (ret != HAL_OK) s_state = OTA_ERROR;
    return ret;
}

HalStatus API_OTA_GetState(OtaState *state)
{
    if (state == NULL) return HAL_INVALID_PARAM;
    *state = s_state;
    return HAL_OK;
}

HalStatus API_OTA_Process(void)
{
    if (s_state == OTA_CHECKING) {
        uint8_t avail = 0;
        if (API_Net_RxAvailable(&avail) == HAL_OK && avail) {
            uint8_t  buf[256];
            uint16_t len = 0;
            API_Net_Read(buf, sizeof(buf), &len);

            API_Uart_Printf(UART_ID_DEBUG, "[OTA] response %u bytes\r\n", len);

            /* TODO: 解析应答，进入 DOWNLOADING */

            s_state = OTA_IDLE;
        }
    }
    return HAL_OK;
}
```

---

### 2.25 `User/main.c`

```c
/**
 * @file    main.c
 * @brief   系统入口 — 只做初始化和调度
 */

#include "ch32v30x.h"
#include "board_cfg.h"
#include "board_uart_fifo.h"
#include "task_control.h"
#include "task_ota.h"
#include "app_cfg.h"

volatile uint32_t sys_tick_ms = 0;

int main(void)
{
    /* 内核初始化 */
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SysTick_Config(SYS_CLK_HZ / 1000);

    /* Board 层（FIFO 在 ISR 开启前就绪） */
    Board_UartFifo_Init();

    /* APP 层初始化 */
    Task_Control_Init();
    API_OTA_Init();

    /* 主循环 */
    uint32_t last100  = 0;
    uint32_t last1000 = 0;

    while (1) {
        uint32_t now = sys_tick_ms;

        if ((now - last100) >= APP_TASK_100MS_PERIOD) {
            last100 = now;
            Task_Control_100ms();
        }

        if ((now - last1000) >= APP_TASK_1000MS_PERIOD) {
            last1000 = now;
            Task_Control_1000ms();
        }

        API_OTA_Process();
    }
}
```

---

## 三、换场景改动范围验证

| 场景 | 要改的 | 不动的 |
|---|---|---|
| **换板子（引脚变了）** | `board_cfg.h`, `board_init.c` | HAL, APP, Device, Core, Services |
| **换芯片（CH32V307→STM32F4）** | `Startup/`, `MCAL/`, `Board/` 全部, `ch32v30x_it.c` | HAL, APP, Device/esp8266 (MCAL USART 接口类似), Core/ringbuffer |
| **WiFi 换 4G（EC20）** | 新建 `Device/ec20/`, 改 `hal_network.c` 内部绑定 | `hal_network.h`, APP 全部, Board 全部, ISR 零改动 |
| **加 DMA UART 接收** | 新建 `Services/drv_dma_uart.c`, 改 `board_init.c` 使能 DMA | ISR 可保持不变（DMA+IDLE 中断仍推 FIFO）, Device 零改动 |
| **业务逻辑改版** | `APP/` | 其他全部 |
| **加 FreeRTOS** | `main.c` 改为 RTOS 启动, APP 改为 RTOS Task | HAL, BSW 全部不变 |

---

## 四、编译依赖关系（Makefile 视角）

```
APP/          →  HAL/hal_*.h, app_cfg.h
HAL/          →  BSW/Device/esp8266/esp8266.h
              →  BSW/Board/board_init.h, board_cfg.h
              →  BSW/MCAL/inc/*.h
              →  hal_common.h
BSW/Device/   →  BSW/Board/board_uart_fifo.h, board_cfg.h
              →  BSW/MCAL/inc/ch32v30x_usart.h
              →  hal_common.h
BSW/Board/    →  BSW/Core/ringbuffer.h, board_cfg.h
              →  BSW/MCAL/inc/*.h
BSW/Services/ →  BSW/MCAL/inc/ch32v30x_flash.h
              →  hal_common.h
BSW/Core/     →  <stdint.h> （零外部依赖）
```

**无环。所有箭头向下或同级（Device → Board 的 FIFO 接口）。**

---

## 五、文件清单汇总（共 24 个文件）

| # | 文件 | 行数 | 职责 |
|---|---|---|---|
| 1 | `BSW/Core/ringbuffer.h` | 25 | 纯数据结构声明 |
| 2 | `BSW/Core/ringbuffer.c` | 40 | 环形缓冲实现 |
| 3 | `BSW/Board/board_cfg.h` | 60 | 所有硬件参数的唯一配置窗口 |
| 4 | `BSW/Board/board_init.h` | 14 | 板级初始化声明 |
| 5 | `BSW/Board/board_init.c` | 110 | 外设初始化实现 |
| 6 | `BSW/Board/board_uart_fifo.h` | 22 | 串口 FIFO 引擎声明 |
| 7 | `BSW/Board/board_uart_fifo.c` | 38 | FIFO 引擎实现 |
| 8 | `BSW/Board/ch32v30x_it.c` | 35 | 中断服务（只推 FIFO） |
| 9 | `BSW/Services/drv_flash_eeprom.h` | 18 | 片内 Flash EEPROM 声明 |
| 10 | `BSW/Services/drv_flash_eeprom.c` | 100 | Flash EEPROM 实现 |
| 11 | `BSW/Device/esp8266/esp8266.h` | 22 | ESP8266 接口声明 |
| 12 | `BSW/Device/esp8266/esp8266.c` | 130 | AT 指令驱动实现 |
| 13 | `HAL/hal_common.h` | 20 | 公共类型定义 |
| 14 | `HAL/hal_gpio.h` | 45 | GPIO 接口声明 |
| 15 | `HAL/hal_gpio.c` | 150 | GPIO 接口实现（直调 MCAL） |
| 16 | `HAL/hal_uart.h` | 20 | UART 接口声明 |
| 17 | `HAL/hal_uart.c` | 75 | UART 实现（直调 MCAL） |
| 18 | `HAL/hal_network.h` | 28 | 网络接口声明 |
| 19 | `HAL/hal_network.c` | 80 | 网络实现（绑定 esp8266） |
| 20 | `APP/task_control.h` | 14 | 控制任务声明 |
| 21 | `APP/task_control.c` | 95 | 控制任务实现 |
| 22 | `APP/task_ota.h` | 28 | OTA 任务声明 |
| 23 | `APP/task_ota.c` | 60 | OTA 任务实现 |
| 24 | `User/main.c` | 40 | 系统入口 |
| 25 | `User/app_cfg.h` | 22 | 应用级配置 |

---

以上为最终定稿版本。所有代码可以直接复制到工程中编译。如果有特定模块需要进一步展开（如 DMA UART 驱动、RTOS 适配、Bootloader 方案），可以继续。