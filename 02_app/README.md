# CH32V30x README

---

## 一、Architecture

### 1 Architecture Tree

```
app/
├── 00_Doc/                  # [文档域] DO-178C 适航认证生命周期过程文档体系
│   ├── Certification/       # 适航计划文档 (PSAC, SDP, SVP, SQAP, SCMP)
│   ├── ICD/                 # 接口控制文档 (Hardware, Software, Communication Protocols)
│   ├── Traceability/        # 双向追溯矩阵 (SysREQ <-> HLR <-> LLR <-> Code <-> Test Case)
│   └── Safety/              # 针对系统安全分析的衍生需求 (FHA, PASA, FMEA, FMEDA, Hazard Log)
│
├── 01_Build/                # [构建域] 编译控制、静态分析规范与内存分区映射
│   ├── Link.ld              # RISC-V 链接脚本 (Dual-Bank Flash, ISR Vector, Stack Guard, DTCM/ITCM)
│   ├── toolchain.cmake      # CMake 编译工具链 (固定警告级别, 禁用隐式转换, 强制 WCET 导出)
│   └── static_analysis/     # MISRA-C:2012 & CERT-C 静态检查规则集与偏差说明 (Deviations)
│
├── 02_Core/                 # [Core 域] RISC-V (QingKe V4F) 内核启动与底层控制
│   ├── startup_ch32v30x_D8C.S # 汇编启动文件 (中断向量表表项映射、HW FPU 上下文初始化、Stack 初始化)
│   ├── core_riscv.c         # RISC-V 架构 API (PFIC 中断控制器配置、SysTick 驱动、CSR 读写封装)
│   └── core_riscv.h         # 内核底层寄存器映射 (mstatus, mepc, mcause, mtvec, pmp)
│
├── 03_MCAL/                 # [MCAL 域] 芯片厂商底层外设标准库 (零修改 Vendor Read-Only Library)
│   ├── inc/                 # ch32v30x_adc.h, ch32v30x_gpio.h, ch32v30x_usart.h, ch32v30x_flash.h...
│   └── src/                 # ch32v30x_adc.c, ch32v30x_gpio.c, ch32v30x_usart.c, ch32v30x_flash.c...
│
├── 04_BSP/                  # [BSP 域] 板级支持包 (硬件相关驱动，对外暴露统一防错 API)
│   ├── inc/
│   │   ├── bsp_board.h      # 板级上电自检 (POST) 与总初始化入口 (BSP_Board_Init)
│   │   ├── bsp_gpio.h       # 离散 IO、继电器及指示灯安全抽象 (带状态读回校验)
│   │   ├── bsp_uart.h       # 串口 DMA + 环形缓冲区 (硬件 Overrun / Parity 故障隔离)
│   │   ├── bsp_flash.h      # 内部 Flash 读写安全封装 (带 Dual-Bank 擦写锁、超时校验与 ECC)
│   │   ├── bsp_adc.h        # 多路 ADC 采样抽象驱动 (含 DMA 双缓冲区与 HW 超时监控)
│   │   └── bsp_wdg.h        # 看门狗底层驱动 (窗口看门狗 WWDG / 独立看门狗 IWDG 硬件触发)
│   └── src/
│       ├── bsp_board.c
│       ├── bsp_gpio.c
│       ├── bsp_uart.c
│       ├── bsp_flash.c
│       ├── bsp_adc.c
│       └── bsp_wdg.c
│
├── 05_Middleware/           # [中间件域] 硬件无关纯逻辑服务层 (支持 Host PC 100% MC/DC 白盒测试)
│   ├── PDI/                 # 协议数据接口 (Protocol Data Interface)
│   │   ├── pdi_def.h        # 帧结构、Command ID、网络状态码与校验算法宏
│   │   ├── pdi_mgr.h        # 报文解包/组包状态机 API (流式状态机，防缓冲区溢出)
│   │   ├── pdi_mgr.c
│   │   ├── pdi_default.h    # 异常应答与 Ack/Nack 模版响应生成器
│   │   └── pdi_default.c
│   ├── OTA/                 # 固件在线升级中间件
│   │   ├── ota_mgr.h        # Dual-Bank 镜像 Header 校验 (RSA/CRC32)、防倒滚计数器、Boot 标志管理
│   │   └── ota_mgr.c
│   └── RTE/                 # 运行时环境 / 信号数据字典桥接层 (实现 ASW 与 BSW 解耦)
│       ├── rte_signal.h     # 全局数据字典类型定义 (带 Validity/Quality 字段的物理量结构体)
│       ├── rte_signal.c     # 信号读写互锁保护与更新逻辑 (含死区限制、限幅与状态标记)
│       ├── rte_interface.h  # 控制层抽象访问接口 Getter / Setter API
│       └── rte_interface.c
│
├── 06_APP/                  # [应用域 / ASW] 纯控制与业务算法层 (严格禁止引用任何 03/04 硬件头文件)
│   ├── task_control.h       # 周期性核心控制算法 (1kHz/100Hz 确定性闭环控制逻辑)
│   ├── task_control.c
│   ├── task_ota.h           # OTA 业务控制状态机 (含 Safe Interlock 飞行/控制状态安全互锁)
│   ├── task_ota.c
│   ├── task_health.h        # 应用级健康状态汇报与降级逻辑
│   └── task_health.c
│
├── 07_System/               # [系统与服务域] 系统级服务、确定性调度与安全监控
│   ├── system_ch32v30x.h    # 系统主时钟及总线时钟配置头文件
│   ├── system_ch32v30x.c    # SystemInit() 配置 (含 HSE 锁定超时降级至 HSI 机制)
│   ├── sys_isr.h            # 中断服务入口声明与优先级映射表
│   ├── sys_isr.c            # 统一 C 中断入口 (仅执行快速 Ack 和硬件数据入队，无复杂逻辑)
│   ├── sys_health.h         # DO-178C 适航级健康监控 (HM)、调度心跳矩阵监控、 Safe-State 切换
│   ├── sys_health.c         # CPU Exception/Trap 捕获、NVRAM 故障日志持久化
│   ├── sys_scheduler.h      # 确定性静态速率单调调度器 (Deterministic Static RMS Scheduler)
│   ├── sys_scheduler.c
│   ├── sys_atomic.h         # RISC-V 临界区保护 (全局中断开关与 Restore 机制，防止嵌套破环)
│   ├── sys_debug.h          # 条件编译诊断工具 / 断言 SYS_ASSERT (RELEASE 构建彻底剔除)
│   └── sys_debug.c
│
├── 08_Config/               # [配置域] 全局静态配置文件 (彻底切断层间头文件交叉依赖)
│   ├── app_cfg.h            # 应用层参数宏 (任务执行周期、算法限制边界、缓冲区容量)
│   ├── board_cfg.h          # 板级硬件物理引脚与映射抽象 (仅引用标准 C 类型，禁止包含 MCAL)
│   ├── mcal_cfg.h           # MCAL 裁剪使能与参数映射 (仅供 BSW/MCAL 使用)
│   └── rte_cfg.h            # RTE 信号池规模及范围约束配置
│
└── main.c                   # 系统总入口 (系统硬件初始化 -> POST 自检 -> 启动确定性主调度循环)
```

---

### 二、 架构各层级访问权限与依赖禁令矩阵 (Inclusion & Access Rules)

为确保静态分析（如 Axivion / Polyspace）和 DO-178C 体系评审通过，严格执行以下 **依赖禁令（Prohibition Rules）**：

| 所在目录层级 | 允许 `#include` 的头文件范围 | 严格禁止包含的内容（Violation Rule） | 核心职责 |
| :--- | :--- | :--- | :--- |
| **06_APP / ASW** | `08_Config/app_cfg.h`<br>`05_Middleware/RTE/rte_interface.h`<br>`05_Middleware/PDI/pdi_mgr.h`<br>`05_Middleware/OTA/ota_mgr.h` | **绝对禁止包含** `03_MCAL`、`04_BSP` 以及 `ch32v30x.h`！<br>**绝对禁止** 直接读写底层硬件寄存器。 | 纯控制算法、业务逻辑、状态机。 |
| **05_Middleware** | `08_Config/app_cfg.h`<br>`05_Middleware/` 内部头文件<br>`07_System/sys_atomic.h` | **禁止包含** `03_MCAL` 寄存器头文件。<br>**禁止直接** 包含 `04_BSP` 具体驱动（仅通过抽象 Handler 交互）。 | 协议解析、数据打包、RTE 信号字典、OTA 校验。 |
| **04_BSP** | `08_Config/board_cfg.h`<br>`08_Config/mcal_cfg.h`<br>`03_MCAL/inc/*.h`<br>`07_System/sys_atomic.h` | **禁止包含** `06_APP` 应用头文件。<br>**禁止包含** 业务逻辑状态机。 | 硬件驱动抽象、物理接口收发、DMA/Flash 安全隔离。 |
| **03_MCAL** | `02_Core/core_riscv.h`<br>`08_Config/mcal_cfg.h` | **禁止修改** 该目录下任何代码。<br>**禁止包含** BSP、Middleware 或 APP 的任何头文件。 | 厂商寄存器级基础库。 |
| **07_System** | `08_Config/*.h`<br>`02_Core/*.h`<br>`03_MCAL/inc/*.h`<br>`04_BSP/inc/*.h` | **禁止** 在 ISR（中断服务函数）中编写超过受限时间（WCET）的长任务。 | 系统初始化、中断路由、Health Monitor、Trap 捕获。 |
| **08_Config** | **只包含基础标准库** `<stdint.h>`, `<stdbool.h>` | **`app_cfg.h` 严禁 include `board_cfg.h` 或任何 MCAL 头文件**，防止上游污染！ | 全局静态配置参数集中管理。 |

---

### 三、 架构对关键场景的处理机制 (Key Architectural Workflows)

#### 1. 信号字典与故障映射流 (承接原 S32K314 项目的 `Interface.c` 功能)
$$
\begin{array}{rcc}
\text{04\_BSP/bsp\_adc.c (物理采样)} & \longrightarrow & \text{写入 raw 变量} \\
\text{07\_System/sys\_health.c (诊断报警)} & \longrightarrow & \text{产生 \texttt{GbFDL\_*}} 
\end{array}
\xrightarrow{\text{调用}} \mathbf{05\_Middleware/RTE/rte\_signal.c} \left[\text{执行 } \texttt{FltFlag\_Update()}\right] \xrightarrow{\text{输出}} 
\begin{cases}
\mathbf{WltFaultNow.Bit} & \rightarrow \text{05\_Middleware/PDI (CAN/串口上报)} \\
\mathbf{rte\_interface.h} & \rightarrow \text{06\_APP/task\_control (控制降级决策)}
\end{cases}
$$

* **优势**：把原先乱串的全局变量全部封装在 `RTE` 模块内部，应用层仅通过 `rte_interface.h` 提供的 **Getter 函数** 访问信号，使得数据耦合（Data Coupling）清晰可追踪，满足 DO-178C 审计需求。

#### 2. 串口/CAN 通信解析流 (彻底剥离后的单向数据链)
```text
[ Hardware IRQ ]
       │
       ▼
07_System/sys_isr.c            --> 仅仅清除硬件中断标志，压入字节至 RingBuffer
       │
       ▼
04_BSP/bsp_uart.c              --> 提供安全的 Lock-Free RingBuffer API
       │
       ▼
05_Middleware/PDI/pdi_mgr.c    --> 轮询/解析 RingBuffer，校验 CRC，生成 PDI_Command 结构体
       │
       ▼
06_APP/task_control.c          --> 接收结构体，执行业务响应
```

#### 3. 适航级故障与看门狗处理链 (Health Monitor Thread)
```text
周期任务 (Task A, Task B, Task C) 
       │ (定时在周期内打卡，更新 Liveness Bitmap)
       ▼
07_System/sys_health.c (Health Monitor)
       │
       ├──> [检查 1]：所有任务运行正常？ ---> 喂硬件看门狗 (WWDG)
       │
       └──> [检查 2]：任务死锁 / 内存越界 Trap？ ---> 拒绝喂狗 -> 记录 NVRAM Log -> 触发 Safe-State -> 强制系统 Safe Reset
```

---

### 四、 总结与实施指引

通过这份全新的 **Refined Master Architecture Tree**：
1. **解决通信混乱**：通信解析全部落入 `05_Middleware/PDI`，驱动落入 `04_BSP/bsp_uart`。
2. **继承老项目经验**：老项目 `Interface.c` 里的信号映射和故障打包（如 `FltFlag_Update()`）顺畅升级为 **`05_Middleware/RTE`**，既保留了 MBD/数据字典的思想，又消除了全局变量污染。
3. **满足 DO-178C 适航**：建立了严格的编译屏障与依赖禁令，增加了 `sys_health` 和 `sys_atomic`，可以顺畅开展 MC/DC 单元测试与静态代码分析。

我们可以正式开始编写 `08_Config/` 下的隔离头文件以及 `07_System/main.c` 核心主入口模版代码。