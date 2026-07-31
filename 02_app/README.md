# CH32V30x README

---

## 一、Architecture

### 1 Architecture Tree

```
app/
├── 00_Doc/                  # [文档域] DO-178C 适航认证体系文档
│   ├── ICD/                 # 接口控制文档 (Interface Control Document)
│   ├── Traceability/        # 双向追溯表 (System REQ <-> HLR <-> LLR <-> Code <-> Test Case)
│   └── Safety/              # FMEA / FHA / Hazard Log (危险源与失效模式分析)
│
├── 01_Build/                # [构建域] 编译控制与内存分区映射
│   ├── Link.ld              # RISC-V 链接脚本 (映射 Boot/APP Dual-Bank, ISR Vector, Stack Guard Page)
│   └── toolchain.cmake      # 编译器选项 (开启 MISRA-C 警告、禁用隐式转换、优化级别固定)
│
├── 02_Core/                 # [Core 域] CPU 内核级启动与 CSR 寄存器控制 (QingKe V4F)
│   ├── startup_ch32v30x_D8C.S # 汇编启动文件 (向量表注册、Stack/Heap 指针初始化)
│   ├── core_riscv.c         # RISC-V 架构 API (PFIC 中断控制器、SysTick、CSR 读写)
│   └── core_riscv.h         # 内核底层寄存器映射
│
├── 03_MCAL/                 # [MCAL 域] 芯片厂商外设标准库 (零修改 Vendor Library)
│   ├── inc/                 # ch32v30x_adc.h, ch32v30x_gpio.h, ch32v30x_usart.h...
│   └── src/                 # ch32v30x_adc.c, ch32v30x_gpio.c, ch32v30x_usart.c...
│
├── 04_BSP/                  # [BSP 域] 板级支持包 (硬件相关，业务无关，只被 System/Middleware 调用)
│   ├── inc/
│   │   ├── bsp_board.h      # 板级总初始化入口 (BSP_Board_Init)
│   │   ├── bsp_gpio.h       # 板级 GPIO 抽象驱动 (LED, Switch, Relay)
│   │   ├── bsp_uart.h       # 串口 DMA + Safe RingBuffer 收发驱动
│   │   ├── bsp_flash.h      # 内部 Flash 擦写/读取安全封装 (带 Timeout & Boundary Check)
│   │   └── bsp_adc.h        # 模拟量采集抽象驱动 (电流、电压、温度 AD 原生值)
│   └── src/
│       ├── bsp_board.c
│       ├── bsp_gpio.c
│       ├── bsp_uart.c
│       ├── bsp_flash.c
│       └── bsp_adc.c
│
├── 05_Middleware/           # [中间件域] 纯逻辑服务层 (纯 C 编写，硬件完全无关，支持 PC 端 100% MC/DC 测试)
│   ├── PDI/                 # 协议数据接口 (Protocol Data Interface)
│   │   ├── pdi_def.h        # PDI 帧头/尾、Command ID、状态码
│   │   ├── pdi_mgr.h        # 解包/组包/CRC 校验状态机 API
│   │   ├── pdi_mgr.c
│   │   └── pdi_default.c    # 异常应答与 Ack/Nack 生成
│   ├── OTA/                 # 固件升级中间件
│   │   ├── ota_mgr.h        # Flash Dual-Bank 镜像 Header 校验、Boot 标记管理
│   │   └── ota_mgr.c
│   └── RTE/                 # [重构承接] 运行时环境 / 信号数据字典桥接层 (原 Interface.c 信号归宿)
│       ├── rte_signal.h     # BSW <-> ASW 共享数据字典定义 (GbBSW_*, GsSPH_*, WLT_FAULT 结构体)
│       ├── rte_signal.c     # 信号映射与状态打包函数 (如 FltFlag_Update() 逻辑)
│       └── rte_interface.h  # 控制层访问 RTE 信号的 Getter / Setter 接口定义
│
├── 06_APP/                  # [应用域 / ASW] 纯控制与业务算法层 (严格禁止包含任何 03_MCAL / 04_BSP 硬件头文件)
│   ├── task_control.h       # 核心控制周期任务 (电机/算法/系统状态机)
│   ├── task_control.c
│   ├── task_ota.h           # OTA 业务状态机 (含 Safety Interlock 飞行/控制状态安全互锁)
│   └── task_ota.c
│
├── 07_System/               # [系统与服务域] 系统初始化、中断路由、确定性安全监控
│   ├── system_ch32v30x.h    # 时钟树配置头文件
│   ├── system_ch32v30x.c    # SystemInit() 系统时钟配置
│   ├── sys_isr.h            # 物理向量中断服务函数映射头文件
│   ├── sys_isr.c            # 统一 C 语言 ISR 入口 (只做 Quick Ack 与 数据压入 BSP Buffer)
│   ├── sys_health.h         # 适航级健康监控 (HM)、看门狗 Liveness 矩阵、Safe-State 切换
│   ├── sys_health.c         # CPU Trap 捕获、NVRAM 异常日志记录
│   ├── sys_atomic.h         # RISC-V 确定性临界区 (CSR / PFIC 中断开关) 保护抽象
│   ├── sys_debug.h          # 条件编译诊断 Log / SYS_ASSERT (Release 状态彻底剥离)
│   └── sys_debug.c
│
├── 08_Config/               # [配置域] 全局静态配置文件 (彻底切断依赖污染)
│   ├── app_cfg.h            # 纯应用层配置 (任务周期、缓冲区大小，无任何 include)
│   ├── board_cfg.h          # 板级物理引脚抽象映射 (仅使用标准 C 类型，严禁包含 MCAL 头文件)
│   └── mcal_cfg.h           # 芯片外设库裁剪使能 (原 ch32v30x_conf.h，仅供 BSW/MCAL 使用)
│
└── main.c                   # 系统总入口 (BSP 初始化 -> System Health 初始化 -> 启动主调度循环)
```
