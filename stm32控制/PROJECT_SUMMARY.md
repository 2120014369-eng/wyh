# STM32 智能小车固件项目总览

> **芯片**: STM32F103C8 (Cortex-M3, 72 MHz)  
> **IDE**: Keil MDK v5 (ARM Compiler V6.16 / ARMCLANG)  
> **外设**: K210 视觉模块 + HC-05/08 蓝牙 + Qt 上位机  
> **项目文件**: `neo.uvprojx` | **输出**: `Objects/neo.hex` (~8 KB)

---

## 目录结构

```
D:\MDK\DOC\
├── neo.uvprojx                 # Keil 工程文件 (UV5)
├── neo.uvoptx                  # 工程选项配置
├── neo.uvguix.jixiegeming      # 用户界面布局
├── CLAUDE.md                   # 项目说明 (AI 辅助开发用)
├── PROJECT_OVERVIEW.md         # 快速参考文档
│
├── User/                       # ★ 用户应用程序 (核心固件)
│   ├── main.c                  #   主循环、状态机、命令解析
│   ├── myusart.c / .h          #   USART3 蓝牙串口驱动
│   ├── mycar.c / .h            #   L298N 电机驱动 (PWM + 方向)
│   ├── ultrasonic.c / .h       #   HC-SR04 超声波测距
│   ├── track.c / .h            #   红外循迹 (三传感器)
│   ├── stm32f10x_it.c / .h     #   中断服务 (SysTick, USART3)
│   └── stm32f10x_conf.h        #   SPL 库配置文件
│
├── Startup/                    # 启动代码
│   ├── startup_stm32f10x_md.s  #   向量表 (中密度)
│   └── system_stm32f10x.c      #   SystemInit, 72MHz 时钟配置
│
├── Lib/                        # 标准外设库 (SPL v3.x)
│   ├── inc/                    #   头文件 (24 个)
│   └── src/                    #   源文件 (24 个)
│       ├── stm32f10x_gpio.c    #   实际使用的: GPIO
│       ├── stm32f10x_rcc.c     #   实际使用的: RCC
│       ├── stm32f10x_usart.c   #   实际使用的: USART
│       ├── stm32f10x_tim.c     #   实际使用的: TIM
│       ├── stm32f10x_iwdg.c    #   实际使用的: IWDG
│       ├── stm32f10x_exti.c    #   实际使用的: EXTI
│       └── misc.c              #   实际使用的: NVIC / SysTick
│
├── RTE/                        # Run-Time Environment 配置
│   └── _Target_1/
│       └── RTE_Components.h    #   CMSIS 设备头文件定义
│
├── Objects/                    # 编译输出
│   ├── neo.hex                 #   ★ 烧录文件
│   ├── neo.axf                 #   ELF 调试文件
│   └── *.o / *.d               #   目标文件与依赖
│
├── Listings/                   # 链接映射与构建日志
│   ├── neo.map                 #   内存映射
│   ├── codex_build.log         #   构建日志
│   └── codex_rebuild.log       #   全量重建日志
│
├── DebugConfig/                # 调试器配置
│   └── Target_1_STM32F103C8_*.dbgconf
│
├── k210/                       # ★ K210 视觉模块固件
│   ├── boot.py                 #   主脚本 (MaixPy/MicroPython)
│   ├── test_k210_usb.py        #   USB 串口通信测试
│   ├── upload_k210.py          #   上传脚本
│   └── boot.zip                #   固件包
│
├── test/                       # ★ 测试工具
│   ├── virtual_stm32.py        #   TCP 虚拟 STM32 (无需硬件)
│   ├── test_serial.py          #   串口/TCP 自动化测试套件
│   └── test_harness.exe        #   测试工具
│
├── tools/                      # 辅助工具
│   └── update_robot_report.py  #   Word 报告自动更新脚本
│
├── _backup_extract/            # 历史备份 (仅供参考)
│   └── DOC/User/
│       ├── k210_uart.c / .h    #   历史: STM32-K210 串口直连
│       ├── mybt.c / .h         #   历史: USART2 蓝牙驱动
│       └── main.c.bak          #   历史: 旧版主程序
│
├── _report_unpack/             # 实验报告 Word 解包缓存
│
└── .vscode/
    └── settings.json           # VS Code 设置
```

---

## 一、系统架构

### 1.1 整体框图

```
┌─────────────────────────────────────────────────────────────────┐
│                        PC / Qt 上位机                           │
│  ┌──────────────────┐          ┌──────────────────────────────┐ │
│  │  蓝牙串口控制面板 │          │      K210 视觉面板            │ │
│  │  (前进/后退/转向) │          │  (触发拍照 / 显示图片)         │ │
│  │  (循迹/速度/状态) │          │                              │ │
│  └────────┬─────────┘          └──────────────┬───────────────┘ │
└───────────┼────────────────────────────────────┼────────────────┘
            │ HC-05/08 Bluetooth                 │ USB CDC
            │ 115200-8N1                         │ (虚拟串口)
            ▼                                    ▼
┌───────────────────────┐          ┌──────────────────────────────┐
│   STM32F103C8 (主控)   │          │   K210 (视觉模块)             │
│                       │          │                              │
│  ┌──── USART3 ──────┐ │          │  ┌── camera (RGB565/QVGA) ─┐│
│  │ PB10 TX, PB11 RX  │ │          │  │  JPEG 压缩, Base64 分包  ││
│  │ 128B 环形缓冲区 IRQ│ │          │  │  协议: IMG:BEGIN/DATA/END││
│  └───────────────────┘ │          │  └──────────────────────────┘│
│                       │          │  ┌── LCD 实时预览 ──────────┐│
│  ┌── 电机驱动 ───────┐ │          │  │  (可选, 15MHz SPI)       ││
│  │ L298N H 桥         │ │          │  └──────────────────────────┘│
│  │ TIM4 CH3/CH4, 1kHz │ │          └──────────────────────────────┘
│  │ PB8/PB9 PWM        │ │
│  │ PB12-PB15 方向控制 │ │
│  └───────────────────┘ │
│                       │
│  ┌── 传感器 ─────────┐ │
│  │ HC-SR04 (PA11/12)  │ │
│  │ 红外循迹 ×3        │ │
│  │ (PA4/PA5/PB1)      │ │
│  └───────────────────┘ │
│                       │
│  ┌── 其他 ───────────┐ │
│  │ PC13 LED 指示灯    │ │
│  │ IWDG 独立看门狗    │ │
│  │ USART1 预留 ISP    │ │
│  └───────────────────┘ │
└─────────────────────────┘
```

### 1.2 工作流程

```
开机 ──→ ISP 烧录兼容延时 ──→ 系统初始化 ──→ 等待蓝牙指令
                                    │
                    ┌───────────────┼───────────────┐
                    ▼               ▼               ▼
              手动模式 (MANUAL)  循迹模式 (TRACKING)  状态查询
                    │               │
                    │                ├── 遇障 (≤180mm ×3) ──→ OBSTACLE_STOP
                    │                │                        │
                    │                │        障碍清除 (≥230mm ×2) ──→ 恢复循迹
                    │                │
                    │                └── 三黑线停靠 (111 ≥100ms) ──→ WAIT_USER
                    │                                             │
                    │                    continue ──→ 恢复循迹    │
                    │                    hold ──────→ 保持等待     │
                    │                    方向命令 ──→ 切回 MANUAL  │
                    │
                    任何非 track 命令 ──→ 切回手动模式
```

---

## 二、硬件引脚定义

### 2.1 总表

| 引脚 | 功能 | 说明 |
|------|------|------|
| **PA9** | USART1_TX | 预留给 ISP 系统串口烧录 |
| **PA10** | USART1_RX | 预留给 ISP 系统串口烧录 |
| **PB10** | USART3_TX | → 蓝牙模块 RXD |
| **PB11** | USART3_RX | ← 蓝牙模块 TXD |
| **PA11** | HC-SR04 TRIG | 超声波触发输出 |
| **PA12** | HC-SR04 ECHO | 超声波回波输入 (需 3.3V 兼容) |
| **PA4** | 循迹传感器左 | 红外光电 |
| **PA5** | 循迹传感器中 | 红外光电 |
| **PB1** | 循迹传感器右 | 红外光电 |
| **PB8** | TIM4_CH3 右电机使能 | L298N ENB, 1kHz PWM |
| **PB9** | TIM4_CH4 左电机使能 | L298N ENA, 1kHz PWM |
| **PB12** | L298N IN3 右电机方向 A | |
| **PB13** | L298N IN4 右电机方向 B | |
| **PB14** | L298N IN1 左电机方向 A | |
| **PB15** | L298N IN2 左电机方向 B | |
| **PC13** | LED 指示灯 | 低电平亮 (有源低) |

### 2.2 L298N 电机驱动逻辑

| 动作 | PB14 (左IN1) | PB15 (左IN2) | PB12 (右IN3) | PB13 (右IN4) |
|------|:---:|:---:|:---:|:---:|
| 前进 | 0 | 1 | 0 | 1 |
| 后退 | 1 | 0 | 1 | 0 |
| 左转 | 1 | 0 | 0 | 1 |
| 右转 | 0 | 1 | 1 | 0 |
| 停止 | 0 | 0 | 0 | 0 |

---

## 三、固件模块详解

### 3.1 `main.c` — 主控状态机

**职责**: 系统初始化、蓝牙命令解析、模式状态机流转、LED 指示

**关键常量**:

| 常量 | 值 | 含义 |
|------|:---:|------|
| `OBSTACLE_STOP_DISTANCE_MM` | 180 mm | 遇障停车阈值 |
| `OBSTACLE_CLEAR_DISTANCE_MM` | 230 mm | 障碍清除恢复阈值 |
| `OBSTACLE_CONFIRM_COUNT` | 3 | 遇障连续确认次数 |
| `OBSTACLE_CLEAR_COUNT` | 2 | 清除连续确认次数 |
| `ULTRASONIC_CHECK_INTERVAL_MS` | 120 ms | 超声波检测间隔 |
| `DISTANCE_REPORT_DELTA_MM` | 10 mm | 距离变化最小上报差值 |

**模式状态机** (`CarMode`):

```
                    ┌──────────────┐
                    │   MANUAL     │ ←── 默认模式，所有非 track 指令均回到此
                    └──────┬───────┘
                           │ 接收 "track" 命令
                           ▼
                    ┌──────────────┐
             ┌─────│  TRACKING    │──────┐
             │     └──────┬───────┘      │
             │            │              │ 红外111持续100ms
             │      距离 ≤180mm ×3       │
             ▼            ▼              ▼
    ┌────────────┐ ┌────────────┐ ┌──────────────┐
    │ OBSTACLE   │ │ WAIT_USER  │ │  继续循迹    │
    │ STOP       │ │ DECISION   │ │              │
    └─────┬──────┘ └──────┬─────┘ └──────────────┘
          │               │
    距离≥230mm ×2    "continue" 恢复
          │               │
          └───────┬───────┘
                  ▼
            TRACKING 恢复
```

**LED 指示逻辑**:

| 状态 | LED 行为 |
|------|----------|
| 电机停止 | 常亮 (低电平) |
| 电机运行 | 500ms 慢闪 |
| 收到蓝牙指令 | 100ms 快闪 3 次 |

**启动流程**:

1. ISP 兼容延时 (~500000 空转周期) — 确保 FlyMcu 握手机制完成
2. `watchdog_init()` — IWDG 预分频 256, 重载 0x0FFF
3. `NVIC_PriorityGroupConfig()` — 抢占优先级 2 位
4. `motor_hw_init()` — L298N PWM + GPIO 初始化
5. `LED_Init()` — PC13 推挽输出
6. `ultrasonic_init()` + `track_init()` — 传感器初始化
7. `usart_init()` — USART3 蓝牙串口
8. `SysTick_Config()` — 1ms 定时中断
9. 发送启动状态 → 进入 `while(1)` 主循环

---

### 3.2 `myusart.c` — 蓝牙串口驱动 (USART3)

**引脚**: PB10 (TX), PB11 (RX)  
**参数**: 115200-8N1

**核心设计**:

- **环形缓冲区**: 128 字节, `bt_rx_head` / `bt_rx_tail` 指针
- **中断接收**: `USART3_IRQHandler` → `my_usart_irq_handler()` 处理 RXNE 和 ORE 中断
- **阻塞发送**: `my_send()` 带 50ms 超时防死锁
- **溢出计数**: 缓冲区满时递增 `bt_rx_overflow_count`, 可通过 `status` 查询

**API**:

```c
void usart_init(void);                  // 初始化 USART3
void my_send(char ch);                  // 单字节发送 (带超时)
void my_send_string(const char *str, int num);  // 字符串发送
int my_usart_read_byte(char *ch);       // 非阻塞读取, 返回 1/0
void my_usart_irq_handler(void);        // 中断处理 (由 it.c 调用)
uint32_t my_usart_rx_overflow_count(void);  // 溢出计数
```

---

### 3.3 `mycar.c` — 电机驱动 (L298N)

**PWM 配置**:

- TIM4, 72 MHz / 72 / 1000 = **1 kHz**
- CH3 (PB8) → 右电机, CH4 (PB9) → 左电机
- 默认速度: **40%**
- 独立左右通道调速: `car_set_speed_lr(left%, right%)`

**API**:

```c
void mycar_init(void);                         // 初始化 GPIO + TIM4 PWM
void car_set_speed(unsigned char percent);     // 设置速度 0-100%
void car_set_speed_lr(unsigned char l, unsigned char r);  // 独立左右调速
unsigned char car_get_speed(void);             // 获取当前速度
unsigned char car_get_default_speed(void);     // 获取默认速度 (40%)
void go(void);               // 前进 (全速)
void stop(void);             // 停车 (PWM=0, IN全低)
void car_forward(void);      // 前进 (go 的别名)
void car_backward(void);     // 后退
void car_left(void);         // 原地左转 (差速)
void car_right(void);        // 原地右转 (差速)
```

---

### 3.4 `ultrasonic.c` — 超声波测距 (HC-SR04)

**引脚**: PA11 (TRIG), PA12 (ECHO)

**测距时序**:

1. TRIG 拉低 2µs → 拉高 12µs → 拉低
2. 等待 ECHO 变高 (30ms 超时, 超时返回 0)
3. 测量 ECHO 高电平脉宽 (最大 25000µs, 超时返回 0)
4. 距离公式: `脉宽(µs) × 17 / 100` → 毫米

**安全机制**:

- 30ms 起始等待超时 — 防止传感器无响应死循环
- 25000µs 最大脉宽限制 — 对应约 4.25m 最大量程
- 基于 SysTick 的微秒级延时 (`ultrasonic_delay_us()`)

---

### 3.5 `track.c` — 红外循迹

**传感器**: PA4 (左), PA5 (中), PB1 (右)  
**有效电平**: 高电平 = 黑线 (`TRACK_ACTIVE_LEVEL = 1`)  
**左右交换**: `TRACK_SWAP_LEFT_RIGHT = 1` (物理安装方向补偿)  
**消抖**: 3 次采样一致才采纳

**循迹策略** (无 PID, 固定比例差速):

| 模式 | 传感器 (左中右) | 动作 |
|:----:|:-----:|:-----|
| 010 | `0 1 0` | 直行, 左右全速 |
| 101 | `1 0 1` | 直行 (中心间隙/十字) |
| 110 | `1 1 0` | 轻左偏: 左 60% / 右 100% |
| 100 | `1 0 0` | 强左偏: 左 25% / 右 100% |
| 011 | `0 1 1` | 轻右偏: 左 100% / 右 60% |
| 001 | `0 0 1` | 强右偏: 左 100% / 右 25% |
| 111 | `1 1 1` | **三黑线 → 保持 100ms → 停车等待用户** |
| 000 | `0 0 0` | **立即停车** |

**三黑线保持机制**: 当检测到 111 模式后, 启动 `track_hold_start_ms` 计时, 持续 100ms 仍为 111 则触发 `WAIT_USER_DECISION` 模式, 通过蓝牙回传等待状态。上位机发送 `continue` 后可恢复循迹。

---

### 3.6 `stm32f10x_it.c` — 中断服务

| 中断 | 功能 |
|------|------|
| `SysTick_Handler` | 每 1ms 递增 `system_ms` 全局时间戳 |
| `USART3_IRQHandler` | 蓝牙 RX 中断 → `my_usart_irq_handler()` |
| `USART1_IRQHandler` | 预留 ISP 串口 (空处理) |
| `EXTI15_10_IRQHandler` | 历史遗留 (空处理) |
| `HardFault_Handler` | 硬件错误 → 死循环 |
| `MemManage/BusFault/UsageFault` | 异常处理 → 死循环 |

---

## 四、蓝牙通信协议

### 4.1 命令集

| 命令 | 功能 | 回复 |
|------|------|------|
| `F` | 单字节前进 | `OK: F` |
| `B` | 单字节后退 | `OK: B` |
| `L` | 单字节左转 | `OK: L` |
| `R` | 单字节右转 | `OK: R` |
| `S` | 单字节停止 | `OK: S` |
| `go` | 前进 (字符串) | `OK: go` |
| `stop` | 停止 (字符串) | `OK: stop` |
| `V0`..`V100` | 设置速度 0-100% | `SPEED:<n>` + `OK: V<n>` |
| `track` | 开启循迹 | `MODE:TRACKING` + `OK: track` |
| `track_off` | 关闭循迹 | `MODE:MANUAL` + `OK: track off` |
| `continue` | 继续循迹 (WAIT_USER 下) | `OK: continue` |
| `hold` | 保持等待 | `OK: hold` |
| `status` | 查询全状态 | `MODE:` + `SPEED:` + `DIST:` + `TRACK:` + `BT_RX_OVF:` + `OK: status` |
| `snap` | 拍照 (被拒绝) | `Error: K210 uses USB` |

### 4.2 状态帧

| 帧头 | 格式 | 说明 |
|------|------|------|
| `MODE:` | `MODE:MANUAL/TRACKING/OBSTACLE_STOP/WAIT_USER` | 当前工作模式 |
| `SPEED:` | `SPEED:<0-100>` | 电机速度百分比 |
| `DIST:` | `DIST:<N>mm` | 超声波距离, 变化 ≥10mm 才上报 |
| `TRACK:` | `TRACK:<L>,<M>,<R>` | 三传感器 0/1 状态 |
| `OBS:` | `OBS:<N>mm` | 遇障告警 (模式切换时发送) |
| `BT_RX_OVF:` | `BT_RX_OVF:<N>` | 蓝牙接收溢出计数 |
| `MCU:BOOT` | `MCU:BOOT[:PIN][:POR][:SFT][:IWDG][:WWDG][:LPWR]` | 启动原因 |
| `Error:` | `Error: ...` | 错误信息 |
| `K210:PONG` | (K210 USB 返回) | K210 心跳应答 |
| `K210:CAPTURE` | (K210 USB 返回) | 拍照开始确认 |
| `IMG:BEGIN` | `IMG:BEGIN:<长度>` | 图片数据起始 |
| `IMG:DATA` | `IMG:DATA:<Base64>` | 图片数据分块 (48B 原始 → Base64) |
| `IMG:END` | `IMG:END` | 图片数据结束 |

### 4.3 指令解析规则

1. **单字节命令** (`F`/`B`/`L`/`R`/`S`): 缓冲区为空时直接执行, 不等待换行
2. **字符串命令** (`go`/`stop`/`track`/...): 以 `\r` 或 `\n` 为结束符
3. **设置命令** (`V<数值>`): 自动解析数字部分, 超范围返回错误
4. **非 track 指令一律切回 MANUAL 模式**: 方向命令、速度设置、stop 等都会将模式重置为手动
5. **命令缓冲**: 最大 31 字符, 超出返回 `Error: command too long`

---

## 五、K210 视觉模块

### 5.1 概述

- **芯片**: Kendryte K210, 双核 RISC-V
- **固件**: MaixPy (MicroPython), 脚本 `k210/boot.py`
- **通信**: USB CDC (虚拟串口) 直连 PC, **不**与 STM32 直接交互
- **摄像头**: RGB565, QVGA (320×240)
- **JPEG 质量**: 25% | **冷却间隔**: 1000ms | **分包**: 48 字节/chunk

### 5.2 通信协议

```
PC → K210:  SNAP\r\n     (拍照命令)
            PING\r\n     (心跳检测)

K210 → PC: K210:READY              (上电就绪)
            K210:USB:READY:*        (USB 通道就绪)
            K210:PONG               (PING 回复)
            K210:CAPTURE            (拍照确认)
            IMG:BEGIN:<bytes>       (图片起始 + JPEG 总字节数)
            IMG:DATA:<base64>       (数据分块, 多行)
            IMG:END                 (图片结束)
            K210:ERR:<msg>          (错误信息)
```

### 5.3 工作流程

1. **Qt 上位机**同时连接两个串口:
   - 蓝牙 COM → 控制小车
   - K210 USB COM → 触发拍照/接收图片
2. STM32 检测障碍物 → 蓝牙回传 `OBS:<距离>mm`
3. Qt 识别 `OBS:` → 通过 K210 USB COM 发送 `SNAP\r\n`
4. K210 拍照 → JPEG 压缩 → Base64 分包 → `IMG:BEGIN/DATA/END` 回传
5. Qt 累积 `IMG:DATA` → 收到 `IMG:END` 后解码显示并保存

---

## 六、构建与烧录

### 6.1 STM32 固件构建

```powershell
# 命令行构建 (Keil UV4)
& 'D:\MDK\UV4\UV4.exe' -j0 -r 'D:\MDK\DOC\neo.uvprojx' -o 'D:\MDK\DOC\keil_build.log'

# 查看构建结果 (0 errors, warnings 可忽略)
type D:\MDK\DOC\keil_build.log
```

**编译配置**:

| 项目 | 值 |
|------|-----|
| 编译器 | ARM Compiler V6.16 (ARMCLANG) |
| 优化级别 | `-O2` |
| 语言标准 | C99 (v6Lang=3) |
| 宏定义 | `USE_STDPERIPH_DRIVER` |
| 包含路径 | `.\User;.\Lib\inc;.\Startup;CMSIS\Core\Include` |
| 输出 | `Objects\neo.hex` (Intel HEX) |
| 内存 | Flash 64KB (0x08000000), RAM 20KB (0x20000000) |

### 6.2 STM32 烧录

1. 烧录工具: `C:\Users\jixiegeming\Desktop\FlyMcu.exe`
2. 烧录串口: **USB-SERIAL CH340 (COM3)** — *不要使用蓝牙 COM*
3. ISP 跳线: `BOOT0=1, BOOT1=0` → 上电烧录
4. 烧录完成后: `BOOT0=0` → 断电重上电运行

### 6.3 K210 固件上传

```
# 通过 USB 串口将 boot.py 上传至 K210 的 Flash 文件系统
# 使 K210 上电自动执行
python k210/upload_k210.py
```

---

## 七、测试工具

### 7.1 虚拟 STM32 (`test/virtual_stm32.py`)

**无硬件测试**: TCP 服务器模拟完整的固件行为

```
# 启动虚拟设备
python test/virtual_stm32.py           # 默认端口 54321
python test/virtual_stm32.py --port 9000
python test/virtual_stm32.py --state   # 查询电机状态
```

- 主端口: TCP 54321 (命令响应)
- 状态端口: TCP 54322 (电机引脚状态查询)
- 模拟: 所有蓝牙命令、电机状态、循迹/障碍模式切换
- 多线程: 状态查询线程 + 主服务线程

### 7.2 自动化测试 (`test/test_serial.py`)

支持 TCP 虚拟测试和真实硬件串口测试:

```
# TCP 虚拟测试 (推荐, 无需硬件)
python test/test_serial.py --tcp

# 串口硬件测试
python test/test_serial.py COM3

# 列出串口
python test/test_serial.py --list

# 交互式监视模式
python test/test_serial.py --tcp --monitor
```

**测试套件** (共 29 项):

| 测试组 | 数量 | 说明 |
|--------|:---:|------|
| USB Serial 基础命令 | 8 | go/stop/F/B/L/R/S/unknown |
| 蓝牙风格命令 | 5 | F/B/L/R/S (换行格式) |
| 单字节方向命令 | 5 | F/B/L/R/S (无换行) |
| 扩展协议 | 7 | track/hold/continue/status/speed |
| 负向测试 | 1 | 非 wait 状态下 continue |
| 拍照流程 | 3 | K210 ping/snap/obs 事件 |

- TCP 模式下: 自动验证电机 GPIO 引脚状态 (PB8-PB15)
- 支持 `motor_running` / `car_mode` / `motor_speed` 等状态字段校验

---

## 八、关键设计决策

### 8.1 为什么 USART3 做蓝牙?

- USART1 (PA9/PA10) — 保留给 ISP 串口烧录, 不与蓝牙抢占
- USART2 (PA2/PA3) — PA3 已损坏, 放弃使用

### 8.2 为什么 K210 不直连 STM32?

简化硬件设计, 利用上位机 Qt 作为"桥梁":
- K210 USB 直连 PC → 更高的图像传输带宽
- Qt 作为中央调度: 收到 `OBS:` → 触发 `SNAP` → 接收图片 → 显示
- 避免 STM32 处理大块 JPEG 数据的负担

### 8.3 为什么不用 PID 控制循迹?

- 红外传感器为数字量 (0/1), 并非模拟连续量
- 固定比例差速 (60%/25%) 策略在实测中转弯平顺、无脱轨
- 保持固件简单, ~8KB 代码量

### 8.4 设计亮点

- **非阻塞状态机**: 基于 SysTick 的 1ms 时间戳体系, 全局无 delay() 阻塞
- **环形缓冲区**: 128 字节中断驱动, 高频按键不丢命令
- **多状态防抖**: 障碍物 3 次确认 / 清除 2 次确认 / 三黑线 100ms 保持
- **启动延时**: 500000 空转周期兼容 FlyMcu ISP 握手时序
- **CRC 无软延时**: 基于 SysTick->VAL 的微秒级延时函数

---

## 九、开发环境

| 工具 | 版本/路径 |
|------|-----------|
| Keil MDK | v5 (`D:\MDK\UV4\UV4.exe`) |
| ARM Compiler | V6.16 (ARMCLANG) |
| SPL | STM32F10x v3.x |
| DFP | Keil.STM32F1xx_DFP 2.4.0 |
| CMSIS | ARM::CMSIS 5.8.0 |
| K210 IDE | MaixPy IDE |
| 上位机 | Qt 5.14.2 (MinGW 64-bit) |
| 烧录工具 | FlyMcu (CH340, COM3) |

---

*文档生成日期: 2026-07-02*  
*基于源码: main.c v4, mycar.c v3, track.c v3, ultrasonic.c v2, myusart.c v2, boot.py v4*
